#pragma once

#include <QtCore>
#include <QIODevice>
#include <QThread>

// Distro packages (Debian, FreeBSD ports) install hidapi.h below hidapi/; the
// hidapi CMake target (Windows via FetchContent) and Homebrew put it top-level.
#if __has_include(<hidapi/hidapi.h>)
  #include <hidapi/hidapi.h>
#else
  #include <hidapi.h>
#endif

#include "dmmdecoder.h"
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcHid)

/// A meter behind a USB HID "serial" cable.
///
/// These cables enumerate as HID devices rather than serial ports. Three
/// chips are known (Chip):
///  - WCH CH9325 / Hoitek HE2325U (UT-D04 and older UNI-T cables, many
///    others): the UART speed is set with a feature report, the meter's
///    bytes then arrive in 8-byte input reports (0xF0 | count, up to 7
///    payload bytes with the top bit set).
///  - Silicon Labs CP2110 (UNI-T UT-D09 first revision, 10c4:ea80): UART
///    enabled and configured with two feature reports (0x41, 0x50), data in
///    reports whose id is the byte count (1..63). Per sigrok's
///    serial_hid_cp2110.c / SiLabs AN434.
///  - WCH CH9329 in custom-HID mode (UNI-T UT-D09 second revision,
///    1a86:e429): a plain UART tunnel, fixed 9600 8N1, no feature report,
///    64-byte input reports (count, then up to 63 raw bytes). Layout per
///    sigrok PR #298.
///  - Brymen BU-86X infrared adapter (0820:0001): raw 8-byte reports that
///    are nothing but UART bytes, fixed speed, no configuration. The meter
///    only answers to a request (DmmDecoder::pollRequest()), so this is the
///    one cable QtDMM writes to.
/// Both UT-D09 revisions look alike; lsusb tells them apart.
/// hidapi is polled in run(), which runs in a worker thread and fills a ring
/// buffer; the QIODevice side (readData(), bytesAvailable()) serves
/// ReaderThread from that buffer and emits readyRead().
class HIDSerialDevice : public QIODevice {
    Q_OBJECT
public:
  /// The cable chip, which decides the report layout.
  enum class Chip { CH9325, CP2110, CH9329, BU86X };

  /// @param info   the meter, for its baud rate and data bits
  /// @param device an entry from availablePorts(): "HID 0xvvvv:0xpppp path"
  ///               (the vid:pid selects the Chip; a bare path means CH9325)
  /// @param p      parent object
  explicit HIDSerialDevice(const DmmDecoder::DMMInfo info, QString device, QObject *p = Q_NULLPTR);
 ~HIDSerialDevice();

  /// Chip for a port entry / vid:pid; unknown ids are treated as CH9325.
  static Chip chipFor(unsigned short vendorId, unsigned short productId);
  static Chip chipForEntry(const QString &entry);
  /// The path part of a port entry ("HID 0x1a86:0xe008 /dev/hidraw2" -> "/dev/hidraw2").
  static QString pathForEntry(const QString &entry);
  /// Extracts the UART bytes of one input report into @p out (at least 63
  /// bytes); returns the count, -1 for a malformed report. Pure, for tests.
  static int unpackReport(Chip chip, const unsigned char *report, int reportLen, unsigned char *out);
  /// The output report(s) that carry @p data to the cable, report id
  /// placeholder included; empty when the chip cannot send (CH9325).
  static QByteArray packWrite(Chip chip, const QByteArray &data);
  /// The CP2110 UART_CONFIG feature report (9 bytes incl. report id 0x50)
  /// for the given line settings; parity 0 none / 1 even / 2 odd. Pure.
  static QByteArray cp2110ConfigReport(int baud, int bits, int parity, int stopBits);
  Chip chip() const { return m_chip; }

  /// Appends the known cable chips found via hidapi to @p portlist.
  static bool availablePorts(QStringList &portlist);
  /// Fails when the hidapi handle could not be opened, so DMM reports an
  /// error instead of waiting for frames that never come.
  bool open(OpenMode mode) override;
  /// True once the cable has delivered an input report (even an empty one),
  /// i.e. the cable itself is configured and alive.
  bool cableAnswers() const { return m_reportsSeen > 0; }
  /// True once an input report carried UART bytes. A cable that answers but
  /// never carries data means the meter is not sending (UNI-T: RS232 button).
  bool dataSeen() const { return m_dataSeen; }
  void close() override;

  /// Bytes waiting in the ring buffer. Must stay const: QIODevice's version
  /// is const and virtual, so a non-const one would hide instead of override
  /// it and ReaderThread (holding a QIODevice*) would get the base version.
  qint64 bytesAvailable() const override;

  Q_SIGNALS:
  /// The worker loop has ended (after close()).
  void finished();

public Q_SLOTS:
  /// The hidapi read loop; runs in the worker thread until close().
  void run();

protected:
  static bool availablePorts(QStringList &portlist,unsigned short vendor_id, unsigned short product_id);
  DmmDecoder::DMMInfo m_dmmInfo;
  Chip m_chip = Chip::CH9325;
  static const unsigned int m_buflen = 1024;
  volatile bool m_isOpen = false;
  volatile int m_reportsSeen = 0;
  volatile bool m_dataSeen = false;
  hid_device *m_handle = Q_NULLPTR;
  unsigned int m_buffer_r = 0;
  unsigned int m_buffer_w = 0;
  unsigned char m_buffer[m_buflen];

  qint64 readData(char *data, qint64 maxSize)  override;
  qint64 writeData(const char *data, qint64 len) override;
};
