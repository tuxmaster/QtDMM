// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ble.h"

#include <QBluetoothAddress>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QSet>

#include "victronble.h"

Q_LOGGING_CATEGORY(lcBle, "qtdmm.ble")

BleAdvertisementDevice::BleAdvertisementDevice(const DmmDecoder::DMMInfo &, const QString &device, QObject *parent)
  : QIODevice(parent)
{
  // "<address> <key> [<main field> <second field>]"
  const QStringList parts = device.simplified().split(' ');
  m_address = parts.value(0).toUpper();
  m_key = VictronBle::keyFromHex(parts.value(1));
  m_mainField = parts.value(2);
  m_secondField = parts.value(3);
}

BleAdvertisementDevice::~BleAdvertisementDevice()
{
  close();
}

bool BleAdvertisementDevice::open(OpenMode mode)
{
  if (QBluetoothAddress(m_address).isNull())
  {
    setErrorString(tr("No Bluetooth address configured."));
    return false;
  }
  if (m_key.size() != 16)
  {
    setErrorString(tr("The encryption key must be 32 hex digits (VictronConnect: Product info, Instant readout via Bluetooth)."));
    return false;
  }

  m_agent = new QBluetoothDeviceDiscoveryAgent(this);
  // advertisements keep changing, so every update of a known device counts
  connect(m_agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BleAdvertisementDevice::onDevice);
  connect(m_agent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated, this,
          [this](const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields) { onDevice(info); });
  connect(m_agent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, [this]
  {
    const QString reason = m_agent->errorString();
    qCWarning(lcBle) << "scan error:" << reason;
    setErrorString(reason);
    Q_EMIT finished(reason);
  });
  // the scan stops itself after its timeout; keep it going while open
  connect(m_agent, &QBluetoothDeviceDiscoveryAgent::finished, this, [this]
  {
    if (isOpen() && m_agent)
      m_agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
  });
  m_agent->setLowEnergyDiscoveryTimeout(60000);
  m_agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
  if (m_agent->error() != QBluetoothDeviceDiscoveryAgent::NoError)
  {
    setErrorString(m_agent->errorString());
    delete m_agent;
    m_agent = nullptr;
    return false;
  }
  m_lastIv = -1;
  m_keyMismatches = 0;
  return QIODevice::open(mode | QIODevice::Unbuffered);
}

void BleAdvertisementDevice::close()
{
  if (m_agent)
  {
    m_agent->disconnect(this);
    m_agent->stop();
    m_agent->deleteLater();
    m_agent = nullptr;
  }
  QIODevice::close();
}

void BleAdvertisementDevice::onDevice(const QBluetoothDeviceInfo &info)
{
  if (info.address().toString().toUpper() != m_address)
    return;
  const QByteArray data = info.manufacturerData(VictronBle::CompanyId);
  const auto adv = VictronBle::parse(data);
  if (!adv)
    return;
  if (adv->iv == m_lastIv)
    return;   // the same advertisement, seen again
  const auto plain = VictronBle::decrypt(*adv, m_key);
  if (!plain)
  {
    // one stray record is not proof; three in a row are
    if (++m_keyMismatches >= 3)
    {
      const QString reason = tr("The encryption key does not match %1.").arg(VictronBle::modelName(adv->model));
      setErrorString(reason);
      Q_EMIT finished(reason);
    }
    return;
  }
  m_keyMismatches = 0;
  m_lastIv = adv->iv;
  qCDebug(lcBle) << VictronBle::modelName(adv->model) << "type" << adv->readoutType << "iv" << adv->iv << plain->toHex();
  m_rx += VictronBle::frame(adv->readoutType, *plain, m_mainField, m_secondField);
  Q_EMIT readyRead();
}

qint64 BleAdvertisementDevice::bytesAvailable() const
{
  return m_rx.size() + QIODevice::bytesAvailable();
}

qint64 BleAdvertisementDevice::readData(char *data, qint64 maxSize)
{
  const qint64 len = qMin(maxSize, qint64(m_rx.size()));
  memcpy(data, m_rx.constData(), len);
  m_rx.remove(0, len);
  return len;
}

qint64 BleAdvertisementDevice::writeData(const char *, qint64 len)
{
  return len;   // nothing to say to a broadcaster
}

QStringList BleAdvertisementDevice::scan(int ms)
{
  QBluetoothDeviceDiscoveryAgent agent;
  QMap<QString, QString> found;
  QObject::connect(&agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, [&](const QBluetoothDeviceInfo &info)
  {
    if (info.manufacturerIds().contains(VictronBle::CompanyId))
      found[info.address().toString()] = info.name();
  });
  agent.setLowEnergyDiscoveryTimeout(ms);
  agent.start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
  QElapsedTimer t;
  t.start();
  while (agent.isActive() && t.elapsed() < ms + 1000)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  agent.stop();
  QStringList list;
  for (auto it = found.constBegin(); it != found.constEnd(); ++it)
    list << QString("%1 %2").arg(it.key(), it.value());
  return list;
}
