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

/// A meter behind a USB HID "serial" cable (WCH CH9325 / Hoitek HE2325U).
///
/// These cables enumerate as HID devices rather than serial ports. The UART
/// speed is set with a feature report and the meter's bytes then arrive in
/// input reports (0xF0 | count, up to 7 payload bytes). hidapi is polled in
/// run(), which runs in a worker thread and fills a ring buffer; the
/// QIODevice side (readData(), bytesAvailable()) serves ReaderThread from
/// that buffer and emits readyRead().
class HIDSerialDevice : public QIODevice {
    Q_OBJECT
public:
  /// @param info   the meter, for its baud rate and data bits
  /// @param device an entry from availablePorts(): "HID 0xvvvv:0xpppp path"
  /// @param p      parent object
  explicit HIDSerialDevice(const DmmDecoder::DMMInfo info, QString device, QObject *p = Q_NULLPTR);
 ~HIDSerialDevice();

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
