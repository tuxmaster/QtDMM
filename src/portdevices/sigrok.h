#pragma once

#include <libsigrokcxx/libsigrokcxx.hpp>
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
#include <glibmm/main.h>
G_GNUC_END_IGNORE_DEPRECATIONS

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
  bool isOpen() const
  {
    return m_isOpen;
  }

  qint64 bytesAvailable() const override;

signals:
  void finished();


protected:
  qint64 readData(char *data, qint64 maxSize) override;
  qint64 writeData(const char *data, qint64 len) override;

private slots:
  void onReadyRead();
  void onConnected();
  void onDisconnected();
  void onError(QAbstractSocket::SocketError err);

private:
  DmmDecoder::DMMInfo m_dmmInfo;
  bool m_isOpen = false;

};

class SigrokWorker : public QObject
{
  Q_OBJECT
public:
  SigrokWorker(QString type, QString device, std::function<void(const QString &)> dataCb);

public slots:
  void start();
  void stop();

signals:
  void finished();
  void error(const QString &msg);

private:
  QString m_type, m_device;
  std::function<void(const QString &)> m_dataCb;
  std::shared_ptr<sigrok::Context> m_ctx;
  std::shared_ptr<sigrok::Session> m_session;
  Glib::RefPtr<Glib::MainLoop> m_loop;
};

