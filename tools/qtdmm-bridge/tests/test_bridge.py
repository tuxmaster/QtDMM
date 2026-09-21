#!/usr/bin/env python3
"""Tests for qtdmm_bridge: the telnet/RFC 2217 state machine and the server
with a fake backend (no serial port, no pyserial needed).

    python3 -m unittest discover -s tools/qtdmm-bridge/tests
"""

import asyncio
import os
import sys
import threading
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import qtdmm_bridge as qb  # noqa: E402

IAC, SB, SE = qb.IAC, qb.SB, qb.SE


def sub(cmd, payload=b""):
    """A COM-PORT-OPTION subnegotiation as QtDMM sends it (IAC escaped)."""
    return bytes([IAC, SB, qb.OPT_COM_PORT, cmd]) + payload.replace(b"\xff", b"\xff\xff") + bytes([IAC, SE])


class FakeBackend(qb.Backend):
    """Records writes and settings; test code injects meter data."""

    instances = []

    def __init__(self, on_data, on_error):
        super().__init__(on_data, on_error)
        self.name = "fake"
        self.opened = False
        self.written = bytearray()
        self.applied = []
        self.purged = []
        self.fail_open = 0
        FakeBackend.instances.append(self)

    def open(self):
        if self.fail_open > 0:
            self.fail_open -= 1
            raise OSError("no such device")
        self.opened = True

    def close(self):
        self.opened = False

    @property
    def is_open(self):
        return self.opened

    def write(self, data):
        self.written += data

    def apply(self, settings):
        self.settings = settings
        self.applied.append(settings)
        return settings

    def purge(self, what):
        self.purged.append(what)

    def inject(self, data):
        self.on_data(data)

    def unplug(self):
        self.opened = False
        self.on_error("read failed: unplugged")


class ProtocolTest(unittest.TestCase):
    def setUp(self):
        self.sent = bytearray()
        self.data = bytearray()
        self.options = []
        self.proto = qb.Rfc2217Protocol(self.sent.extend, self.data.extend, self.option)

    def option(self, cmd, value):
        self.options.append((cmd, value))
        return value

    def test_plain_data_and_iac_escape(self):
        self.proto.feed(b"DC 1.2\xff\xff34 V\n")
        self.assertEqual(bytes(self.data), b"DC 1.2\xff34 V\n")
        self.assertEqual(self.sent, b"")
        self.assertEqual(qb.Rfc2217Protocol.escape(b"\x00\xff\x01"), b"\x00\xff\xff\x01")

    def test_suboption_split_across_feeds(self):
        pkt = sub(qb.SET_BAUDRATE, (19200).to_bytes(4, "big"))
        self.proto.feed(pkt[:3])
        self.proto.feed(pkt[3:7])
        self.proto.feed(pkt[7:])
        self.assertEqual(self.options, [(qb.SET_BAUDRATE, (19200).to_bytes(4, "big"))])
        self.assertEqual(bytes(self.sent), sub(qb.SET_BAUDRATE + 100, (19200).to_bytes(4, "big")))
        self.assertEqual(self.data, b"")

    def test_iac_inside_suboption(self):
        # a baud rate whose bytes contain 0xff arrives escaped
        pkt = sub(qb.SET_BAUDRATE, b"\x00\x00\xff\x00")
        self.proto.feed(pkt)
        self.assertEqual(self.options, [(qb.SET_BAUDRATE, b"\x00\x00\xff\x00")])

    def test_negotiation(self):
        self.proto.feed(bytes([IAC, qb.WILL, qb.OPT_COM_PORT, IAC, qb.DO, qb.OPT_BINARY, IAC, qb.DO, qb.OPT_ECHO]))
        self.assertEqual(bytes(self.sent), bytes([IAC, qb.DO, qb.OPT_COM_PORT, IAC, qb.WILL, qb.OPT_BINARY,
                                                  IAC, qb.WONT, qb.OPT_ECHO]))

    def test_signature(self):
        self.proto.feed(sub(qb.SIGNATURE))
        self.assertIn(b"qtdmm-bridge", bytes(self.sent))
        self.assertEqual(self.options, [])

    def test_unknown_command_ignored(self):
        self.proto.feed(bytes([IAC, qb.NOP]) + b"abc")
        self.assertEqual(bytes(self.data), b"abc")


class ServerTest(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        FakeBackend.instances.clear()
        self.cfg = qb.PortConfig(tcp_port=0, device="fake", name="fake")
        self.server = qb.PortServer(self.cfg, FakeBackend, "127.0.0.1")
        self.server.RETRY_SECONDS = 0.05
        await self.server.start()

    async def asyncTearDown(self):
        # a failed test may leave a client connected; do not hang on it
        await asyncio.wait_for(self.server.stop(), 3)

    async def connect(self):
        """Connects and waits for the server to open a backend for it."""
        before = len(FakeBackend.instances)
        reader, writer = await asyncio.open_connection("127.0.0.1", self.server.port)
        for _ in range(100):
            if len(FakeBackend.instances) > before and FakeBackend.instances[-1].opened:
                break
            await asyncio.sleep(0.01)
        self.assertGreater(len(FakeBackend.instances), before, "server did not open a backend")
        return reader, writer, FakeBackend.instances[-1]

    async def read_some(self, reader, n, timeout=1.0):
        return await asyncio.wait_for(reader.readexactly(n), timeout)

    async def test_qtdmm_negotiation(self):
        reader, writer, backend = await self.connect()
        # exactly what RFC2217SerialDevice::sendRFC2217Negotiation() sends for 19200 7O1, DTR on, RTS off
        writer.write(sub(qb.SET_BAUDRATE, (19200).to_bytes(4, "big")) + sub(qb.SET_DATASIZE, b"\x07")
                     + sub(qb.SET_PARITY, b"\x03") + sub(qb.SET_STOPSIZE, b"\x01")
                     + sub(qb.SET_CONTROL, b"\x08") + sub(qb.SET_CONTROL, b"\x0c"))
        await writer.drain()
        expected = (sub(101, (19200).to_bytes(4, "big")) + sub(102, b"\x07") + sub(103, b"\x03") + sub(104, b"\x01")
                    + sub(105, b"\x08") + sub(105, b"\x0c"))
        got = await self.read_some(reader, len(expected))
        self.assertEqual(got, expected)
        s = backend.settings
        self.assertEqual((s.baudrate, s.bytesize, s.parity, s.stopbits, s.dtr, s.rts), (19200, 7, "O", 1, True, False))
        writer.close()

    async def test_data_both_ways_with_escaping(self):
        reader, writer, backend = await self.connect()
        # meter -> client: 0xff must be doubled
        backend.inject(b"\x1b\x00\xff\x30")
        self.assertEqual(await self.read_some(reader, 5), b"\x1b\x00\xff\xff\x30")
        # client -> meter: poll request, escaped 0xff undone
        writer.write(b"\x00\x86\x66\xff\xff")
        await writer.drain()
        for _ in range(50):
            if len(backend.written) >= 4:
                break
            await asyncio.sleep(0.01)
        self.assertEqual(bytes(backend.written), b"\x00\x86\x66\xff")
        writer.close()

    async def test_data_from_reader_thread(self):
        reader, writer, backend = await self.connect()
        t = threading.Thread(target=backend.inject, args=(b"hello",))
        t.start()
        t.join()
        self.assertEqual(await self.read_some(reader, 5), b"hello")
        writer.close()

    async def test_new_client_replaces_old(self):
        reader1, writer1, backend1 = await self.connect()
        reader2, writer2, backend2 = await self.connect()
        self.assertIsNot(backend1, backend2)
        # the first connection is closed by the server, its device released
        self.assertEqual(await asyncio.wait_for(reader1.read(), 1.0), b"")
        self.assertFalse(backend1.opened)
        backend2.inject(b"x")
        self.assertEqual(await self.read_some(reader2, 1), b"x")
        writer2.close()

    async def test_device_appears_later(self):
        # the device is not there yet when the client connects
        orig_init = FakeBackend.__init__

        def init(self, on_data, on_error):
            orig_init(self, on_data, on_error)
            self.fail_open = 2

        FakeBackend.__init__ = init
        try:
            reader, writer = await asyncio.open_connection("127.0.0.1", self.server.port)
            await asyncio.sleep(0.02)
            backend = FakeBackend.instances[-1]
            self.assertFalse(backend.opened)
            for _ in range(100):
                if backend.opened:
                    break
                await asyncio.sleep(0.01)
            self.assertTrue(backend.opened, "backend should be retried until open() succeeds")
            writer.close()
        finally:
            FakeBackend.__init__ = orig_init

    async def test_device_unplugged_and_back(self):
        reader, writer, backend = await self.connect()
        backend.unplug()
        for _ in range(100):
            if backend.opened:
                break
            await asyncio.sleep(0.01)
        self.assertTrue(backend.opened, "backend should be reopened after the device came back")
        # the client never noticed
        backend.inject(b"y")
        self.assertEqual(await self.read_some(reader, 1), b"y")
        writer.close()

    async def test_purge_and_unknown_suboption(self):
        reader, writer, backend = await self.connect()
        writer.write(sub(qb.PURGE_DATA, b"\x03") + sub(99, b"\x01") + b"Z")
        await writer.drain()
        self.assertEqual(await self.read_some(reader, len(sub(112, b"\x03"))), sub(112, b"\x03"))
        for _ in range(50):
            if backend.written:
                break
            await asyncio.sleep(0.01)
        self.assertEqual(backend.purged, [3])
        self.assertEqual(bytes(backend.written), b"Z")
        writer.close()


class SerialLoopTest(unittest.IsolatedAsyncioTestCase):
    """End to end through the real SerialBackend on pyserial's loop:// port,
    which echoes everything written to it."""

    async def test_echo_through_loop_port(self):
        try:
            import serial  # noqa: F401
        except ImportError:
            self.skipTest("pyserial not installed")
        cfg = qb.PortConfig(tcp_port=0, device="loop://")
        server = qb.PortServer(cfg, qb.make_backend_factory(cfg.device), "127.0.0.1")
        await server.start()
        try:
            reader, writer = await asyncio.open_connection("127.0.0.1", server.port)
            writer.write(sub(qb.SET_BAUDRATE, (2400).to_bytes(4, "big")))
            writer.write(b"\x1b\x00\xff\xff\x30")
            await writer.drain()
            reply = await asyncio.wait_for(reader.readexactly(len(sub(101, (2400).to_bytes(4, "big"))) + 5), 3)
            self.assertEqual(reply, sub(101, (2400).to_bytes(4, "big")) + b"\x1b\x00\xff\xff\x30")
            writer.close()
        finally:
            await asyncio.wait_for(server.stop(), 3)


class ConfigTest(unittest.TestCase):
    def test_port_arg(self):
        p = qb.parse_port_arg("4000=/dev/ttyUSB0")
        self.assertEqual((p.tcp_port, p.device, p.name), (4000, "/dev/ttyUSB0", "/dev/ttyUSB0"))
        for bad in ("4000", "x=/dev/ttyUSB0", "4000="):
            with self.assertRaises(Exception):
                qb.parse_port_arg(bad)

    def test_toml_roundtrip(self):
        import tempfile

        ports = [qb.PortConfig(4000, "/dev/ttyUSB0", "UT61E"), qb.PortConfig(4001, "loop://")]
        text = qb.print_config(ports, "127.0.0.1")
        with tempfile.NamedTemporaryFile("w", suffix=".toml", delete=False) as f:
            f.write(text + '\n[[port]]\ntcp = 4002\ndevice = "/dev/ttyUSB1"\nbaudrate = 2400\ndtr = false\n')
            name = f.name
        try:
            loaded, glob = qb.load_config(name)
        finally:
            os.unlink(name)
        self.assertEqual(glob["bind"], "127.0.0.1")
        self.assertEqual([(p.tcp_port, p.device, p.name) for p in loaded],
                         [(4000, "/dev/ttyUSB0", "UT61E"), (4001, "loop://", "loop://"), (4002, "/dev/ttyUSB1", "/dev/ttyUSB1")])
        s = loaded[2].initial_settings()
        self.assertEqual((s.baudrate, s.dtr, s.rts), (2400, False, True))

    def test_hid_reserved(self):
        with self.assertRaises(SystemExit):
            qb.make_backend_factory("hid:1a86:e008")


if __name__ == "__main__":
    unittest.main()
