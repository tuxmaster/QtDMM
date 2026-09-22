// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "mdnsresponder.h"

#include "mdnsbrowser.h"

#include <QHostInfo>
#include <QLoggingCategory>
#include <QNetworkInterface>
#include <QUdpSocket>

Q_DECLARE_LOGGING_CATEGORY(lcMdns)

namespace
{
const QHostAddress kMdnsGroup(QStringLiteral("224.0.0.251"));
constexpr quint16 kMdnsPort = 5353;
const QString kEnumeration = QStringLiteral("_services._dns-sd._udp.local");
}

MdnsResponder::MdnsResponder(QObject *parent) : QObject(parent)
{
}

MdnsResponder::~MdnsResponder()
{
  stop();
}

bool MdnsResponder::prepare(const QString &service, const QString &instance, quint16 port, const QMap<QString, QString> &txt)
{
  stop();
  m_service = service.toLower();
  if (!m_service.endsWith(".local"))
    m_service += ".local";
  m_instance = instance;
  m_port = port;
  m_txt = txt;
  QString host = QHostInfo::localHostName().section('.', 0, 0);
  if (host.isEmpty())
    host = "qtdmm";
  m_host = host + ".local";
  return true;
}

bool MdnsResponder::start(const QString &service, const QString &instance, quint16 port, const QMap<QString, QString> &txt)
{
  prepare(service, instance, port, txt);   // names first, then the sockets

  for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces())
  {
    const auto flags = iface.flags();
    if (!(flags & QNetworkInterface::IsUp) || !(flags & QNetworkInterface::IsRunning)
        || (flags & QNetworkInterface::IsLoopBack) || !(flags & QNetworkInterface::CanMulticast))
      continue;
    for (const QNetworkAddressEntry &entry : iface.addressEntries())
    {
      if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
        continue;
      auto *s = new QUdpSocket(this);
      // the port is shared with Avahi / Bonjour / other instances
      if (!s->bind(QHostAddress::AnyIPv4, kMdnsPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
      {
        qCWarning(lcMdns) << "cannot bind 5353 on" << iface.humanReadableName() << s->errorString();
        delete s;
        continue;
      }
      s->setMulticastInterface(iface);
      if (!s->joinMulticastGroup(kMdnsGroup, iface))
      {
        qCWarning(lcMdns) << "cannot join" << iface.humanReadableName() << s->errorString();
        delete s;
        continue;
      }
      connect(s, &QUdpSocket::readyRead, this, &MdnsResponder::onReadyRead);
      m_sockets << Endpoint{s, entry.ip(), entry.prefixLength()};
      // unsolicited announcement (RFC 6762 8.3), twice
      const QByteArray a = announcement(entry.ip(), 120);
      s->writeDatagram(a, kMdnsGroup, kMdnsPort);
      s->writeDatagram(a, kMdnsGroup, kMdnsPort);
      qCDebug(lcMdns) << "announcing" << instanceName() << "on" << iface.humanReadableName() << entry.ip().toString();
    }
  }
  return !m_sockets.isEmpty();
}

void MdnsResponder::stop()
{
  for (const Endpoint &e : m_sockets)
  {
    e.socket->writeDatagram(announcement(e.address, 0), kMdnsGroup, kMdnsPort);   // goodbye
    e.socket->disconnect(this);
    e.socket->close();
    e.socket->deleteLater();
  }
  m_sockets.clear();
}

void MdnsResponder::onReadyRead()
{
  auto *s = qobject_cast<QUdpSocket *>(sender());
  QHostAddress ifaceAddress;
  int prefixLength = 0;
  for (const Endpoint &e : m_sockets)
    if (e.socket == s)
    {
      ifaceAddress = e.address;
      prefixLength = e.prefixLength;
    }
  while (s && s->hasPendingDatagrams())
  {
    QByteArray packet(int(s->pendingDatagramSize()), '\0');
    QHostAddress from;
    quint16 fromPort = 0;
    s->readDatagram(packet.data(), packet.size(), &from, &fromPort);
    // every socket sees every datagram of the group; the one bound to the
    // asker's subnet answers, so the A record fits the asker
    const bool mine = from.isInSubnet(ifaceAddress, prefixLength);
    if (!mine && m_sockets.size() > 1)
      continue;
    const QByteArray a = answer(packet, ifaceAddress);
    if (a.isEmpty())
      continue;
    // legacy (non-5353) askers and QU questions get a unicast answer
    bool unicast = fromPort != kMdnsPort;
    for (const MdnsMessage::Question &q : MdnsMessage::questions(packet))
      if (q.unicastResponse)
        unicast = true;
    if (unicast)
      s->writeDatagram(a, from, fromPort);
    else
      s->writeDatagram(a, kMdnsGroup, kMdnsPort);
  }
}

QByteArray MdnsResponder::announcement(const QHostAddress &address, quint32 ttl) const
{
  MdnsMessage::Record ptr;
  ptr.name = m_service;
  ptr.type = MdnsMessage::PTR;
  ptr.ttl = ttl;
  ptr.target = instanceName();
  MdnsMessage::Record srv;
  srv.name = instanceName();
  srv.type = MdnsMessage::SRV;
  srv.ttl = ttl;
  srv.port = m_port;
  srv.target = m_host;
  MdnsMessage::Record txt;
  txt.name = instanceName();
  txt.type = MdnsMessage::TXT;
  txt.ttl = ttl;
  txt.txt = m_txt;
  MdnsMessage::Record a;
  a.name = m_host;
  a.type = MdnsMessage::A;
  a.ttl = ttl;
  a.address = address;
  return MdnsMessage::response({ptr, srv, txt, a});
}

QByteArray MdnsResponder::answer(const QByteArray &packet, const QHostAddress &address, quint32 ttl) const
{
  const QList<MdnsMessage::Question> qs = MdnsMessage::questions(packet);
  bool wantsService = false, wantsEnumeration = false, wantsInstance = false, wantsHost = false;
  for (const MdnsMessage::Question &q : qs)
  {
    const QString name = q.name.toLower();
    const bool anyType = q.type == 255;
    if (name == m_service && (q.type == MdnsMessage::PTR || anyType))
      wantsService = true;
    else if (name == kEnumeration && (q.type == MdnsMessage::PTR || anyType))
      wantsEnumeration = true;
    else if (name == instanceName().toLower() && (q.type == MdnsMessage::SRV || q.type == MdnsMessage::TXT || anyType))
      wantsInstance = true;
    else if (name == m_host.toLower() && (q.type == MdnsMessage::A || anyType))
      wantsHost = true;
  }
  if (!wantsService && !wantsEnumeration && !wantsInstance && !wantsHost)
    return {};
  if (wantsService)
    return announcement(address, ttl);

  QList<MdnsMessage::Record> answers, additional;
  if (wantsEnumeration)
  {
    MdnsMessage::Record r;
    r.name = kEnumeration;
    r.type = MdnsMessage::PTR;
    r.ttl = ttl;
    r.target = m_service;
    answers << r;
  }
  MdnsMessage::Record a;
  a.name = m_host;
  a.type = MdnsMessage::A;
  a.ttl = ttl;
  a.address = address;
  if (wantsInstance)
  {
    MdnsMessage::Record srv;
    srv.name = instanceName();
    srv.type = MdnsMessage::SRV;
    srv.ttl = ttl;
    srv.port = m_port;
    srv.target = m_host;
    MdnsMessage::Record txt;
    txt.name = instanceName();
    txt.type = MdnsMessage::TXT;
    txt.ttl = ttl;
    txt.txt = m_txt;
    answers << srv << txt;
    additional << a;
  }
  if (wantsHost)
    answers << a;
  return MdnsMessage::response(answers, additional);
}
