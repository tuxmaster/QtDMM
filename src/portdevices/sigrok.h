#pragma once

#include <QIODevice>
#include <QProcess>
#include "dmmdecoder.h"

class SigrokDevice : public QIODevice
{
  Q_OBJECT
public:
  explicit SigrokDevice(const DmmDecoder::DMMInfo &info,
                        QString device,
                        QObject *parent = nullptr);
  ~SigrokDevice();

  static bool availablePorts(QStringList &portlist);

  void close() override;
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

