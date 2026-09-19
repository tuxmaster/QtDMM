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

// hiddevice.h  (wrapp‑t hidapi oder libusb)
class HIDSerialDevice : public QIODevice {
    Q_OBJECT
public:
  explicit HIDSerialDevice(const DmmDecoder::DMMInfo info, QString device, QObject *p = Q_NULLPTR);
 ~HIDSerialDevice();

  static bool availablePorts(QStringList &portlist);
  void close() override;

  // Must stay const: QIODevice::bytesAvailable() is const and virtual, so a
  // non-const version silently hides it instead of overriding, and callers
  // holding a QIODevice* (ReaderThread) get the base implementation.
  qint64 bytesAvailable() const override;

  Q_SIGNALS:
  void finished();

public Q_SLOTS:
  void run();

protected:
  static bool availablePorts(QStringList &portlist,unsigned short vendor_id, unsigned short product_id);
  DmmDecoder::DMMInfo m_dmmInfo;
  static const unsigned int m_buflen = 1024;
  volatile bool m_isOpen = false;
  hid_device *m_handle = Q_NULLPTR;
  unsigned int m_buffer_r = 0;
  unsigned int m_buffer_w = 0;
  unsigned char m_buffer[m_buflen];

  qint64 readData(char *data, qint64 maxSize)  override;
  qint64 writeData(const char *data, qint64 len) override;
};
