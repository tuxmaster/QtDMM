// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

class QUdpSocket;
class QTimer;

/// Just enough DNS to read mDNS answers: the records of one message, names
/// with compression pointers resolved. Pure, so it is testable on captured
/// packets; MdnsBrowser feeds it what arrives on port 5353.
namespace MdnsMessage
{
  enum Type { A = 1, PTR = 12, TXT = 16, AAAA = 28, SRV = 33 };

  struct Record
  {
    QString name;          ///< owner name as sent, without trailing dot
    quint16 type = 0;
    quint32 ttl = 0;
    // by type:
    QString target;        ///< PTR: instance name; SRV: host name
    quint16 port = 0;      ///< SRV
    QHostAddress address;  ///< A / AAAA
    QMap<QString, QString> txt;   ///< TXT: key=value pairs (keys lower case)
  };

  /// The answer/authority/additional records of @p packet; empty when it
  /// is not a well-formed DNS message.
  QList<Record> parse(const QByteArray &packet);

  /// A query for the PTR records of @p service ("_qtdmm-bridge._tcp.local").
  QByteArray query(const QString &service);

  /// One question of a message.
  struct Question
  {
    QString name;
    quint16 type = 0;
    bool unicastResponse = false;   ///< QU bit: the asker wants a unicast answer
  };

  /// The questions of @p packet, if it is a query (QR bit clear).
  QList<Question> questions(const QByteArray &packet);

  /// A response message carrying @p answers and @p additional records,
  /// class IN with the cache-flush bit as an authoritative mDNS answer
  /// (RFC 6762 10.2). Records are written from their by-type fields; a
  /// ttl of 0 is the goodbye packet.
  QByteArray response(const QList<Record> &answers, const QList<Record> &additional = {});
}

/// Finds services in the local network by mDNS (RFC 6762), the way the
/// qtdmm-bridge announces itself: sends one PTR query per interface to
/// 224.0.0.251:5353, collects answers for a while and reports every
/// instance whose SRV record (host, port) has arrived. No system resolver
/// (Avahi, Bonjour) is needed.
class MdnsBrowser : public QObject
{
  Q_OBJECT
public:
  /// One announced service.
  struct Service
  {
    QString instance;      ///< "dory UT61E"
    QString host;          ///< "dory.local"
    quint16 port = 0;
    QHostAddress address;  ///< IPv4 of the host when it came with the answer
    QMap<QString, QString> txt;   ///< the bridge's device, name, version
    bool operator==(const Service &o) const { return instance == o.instance && host == o.host && port == o.port; }
  };

  explicit MdnsBrowser(QObject *parent = nullptr);
  ~MdnsBrowser() override;

  /// Starts a browse for @p service ("_qtdmm-bridge._tcp.local") that lasts
  /// @p ms; found() comes per service, finished() at the end.
  void browse(const QString &service, int ms = 2500);
  void stop();
  bool isActive() const { return m_active; }

  /// Everything found so far in this browse.
  QList<Service> services() const { return m_services; }

  /// Feeds one received packet; public for the test.
  void handlePacket(const QByteArray &packet);

Q_SIGNALS:
  void found(const MdnsBrowser::Service &service);
  void finished();

private:
  void onReadyRead();
  void assemble();

  QString m_service;
  QList<QUdpSocket *> m_sockets;
  QTimer *m_timer;
  bool m_active = false;
  QList<Service> m_services;
  // pieces as they arrive, by name
  QStringList m_instances;                          // PTR targets, lower case
  QMap<QString, QString> m_display;                 // lower case -> as announced
  QMap<QString, QPair<QString, quint16>> m_srv;     // instance -> host, port
  QMap<QString, QMap<QString, QString>> m_txt;      // instance -> txt
  QMap<QString, QHostAddress> m_addresses;          // host -> IPv4
};
