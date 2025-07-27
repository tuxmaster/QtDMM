#pragma once

#include <QIODevice>

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
  bool isOpen() const { return m_isOpen; }

  qint64 bytesAvailable() const override;

signals:
  void finished();

protected:
  qint64 readData(char *data, qint64 maxSize) override;
  qint64 writeData(const char *data, qint64 len) override;

  DmmDecoder::DMMInfo m_dmmInfo;
  bool m_isOpen = false;
  QString m_type, m_device;


};

