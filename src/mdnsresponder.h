// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QHostAddress>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

class QUdpSocket;
class QNetworkInterface;

/// Announces one service by mDNS/DNS-SD (RFC 6762/6763), the counterpart of
/// MdnsBrowser: the SCPI server registers as "<instance>._scpi-raw._tcp.local"
/// so that lxi-tools (`lxi discover -m`) and other DNS-SD browsers find
/// QtDMM. Listens on 224.0.0.251:5353 on every interface, answers PTR
/// questions for the service (and the service-type enumeration
/// "_services._dns-sd._udp.local") with PTR, SRV, TXT and A records for the
/// interface the question came in on, announces itself when started and
/// says goodbye (ttl 0) when stopped. Coexists with Avahi/Bonjour because
/// the port is bound shared.
class MdnsResponder : public QObject
{
  Q_OBJECT
public:
  explicit MdnsResponder(QObject *parent = nullptr);
  ~MdnsResponder() override;

  /// Starts announcing @p instance ("QtDMM dory") of @p service
  /// ("_scpi-raw._tcp") on @p port with @p txt records. The host name is
  /// "<localHostName>.local". Returns false when no interface could be
  /// joined.
  bool start(const QString &service, const QString &instance, quint16 port, const QMap<QString, QString> &txt);
  void stop();
  bool isActive() const { return !m_sockets.isEmpty(); }

  /// Full instance name as announced ("QtDMM dory._scpi-raw._tcp.local").
  QString instanceName() const { return m_instance + '.' + m_service; }
  QString hostName() const { return m_host; }

  /// The answer to @p packet for a responder reachable at @p address, or
  /// empty when the packet asks for nothing of ours. Public for the test.
  QByteArray answer(const QByteArray &packet, const QHostAddress &address, quint32 ttl = 120) const;

private:
  void onReadyRead();
  QByteArray announcement(const QHostAddress &address, quint32 ttl) const;

  QString m_service;    ///< "_scpi-raw._tcp.local", lower case
  QString m_instance;   ///< as given
  QString m_host;       ///< "dory.local"
  quint16 m_port = 0;
  QMap<QString, QString> m_txt;
  QList<QPair<QUdpSocket *, QHostAddress>> m_sockets;   ///< socket, interface address
};
