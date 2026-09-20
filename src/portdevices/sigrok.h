#pragma once

#include <QIODevice>
#include <QProcess>
#include "dmmdecoder.h"

/// Uses sigrok-cli as the data source for meters QtDMM has no decoder for.
///
/// sigrok-cli is started as a child process with the driver named in
/// DMMInfo::sigrokExe and its text output is reformatted into fixed-length
/// lines the ASCII decoder understands (see src/decoders/ascii.h).
class SigrokDevice : public QIODevice
{
  Q_OBJECT
public:
  /// @param info   the meter; DMMInfo::sigrokExe names the sigrok-cli binary
  /// @param device passed to sigrok-cli as --driver, e.g. "uni-t-ut61e-ser:conn=/dev/ttyUSB0"
  /// @param parent parent object
  explicit SigrokDevice(const DmmDecoder::DMMInfo &info,
                        QString device,
                        QObject *parent = nullptr);
  ~SigrokDevice();

  /// No scan implemented; adds nothing and returns false.
  static bool availablePorts(QStringList &portlist);

  void close() override;
  /// Starts sigrok-cli (DMMInfo::sigrokExe, or "sigrok-cli" from the PATH)
  /// with --continuous and opens this device for reading.
  bool init();

  qint64 bytesAvailable() const override;

signals:
  void finished();

private slots:
  void onProcessReadyRead();

private:
  qint64 readData(char *data, qint64 maxSize) override;
  qint64 writeData(const char *data, qint64 len) override;

  static constexpr int m_fixedLineLength = 30;

  DmmDecoder::DMMInfo m_dmmInfo;
  QString m_type, m_device;
  QProcess *m_process;
  QByteArray m_buffer;
  QByteArray m_outLine;
  QString m_sigrok;
};

