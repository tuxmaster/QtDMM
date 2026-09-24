// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "blegatt.h"

#include <QBluetoothAddress>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QLowEnergyController>
#include <QLowEnergyDescriptor>
#include <QLowEnergyService>
#include <QSet>
#include <QTimer>

Q_DECLARE_LOGGING_CATEGORY(lcBle)

namespace
{
// Microchip/ISSC "Transparent UART" (BM70/RN4870 modules): UNI-T UT60BT,
// UT161 and the UT-D07B adapter
const QBluetoothUuid kIsscService(QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455"));
const QBluetoothUuid kIsscNotify(QStringLiteral("49535343-1e4d-4bd9-ba61-23c647249616"));
const QBluetoothUuid kIsscWrite(QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3"));
const QBluetoothUuid kIsscWriteFallback(QStringLiteral("49535343-6daa-4d02-abf6-19569aca69fe"));

// connecting and finding the characteristics takes a second or two; a
// meter that is switched off or out of range never answers
constexpr int kConnectTimeoutMs = 20000;
}

std::optional<BleGattDevice::Profile> BleGattDevice::profile(ReadEvent::DataFormat format)
{
  switch (format)
  {
    case ReadEvent::UniTiDMM:
      return Profile{ kIsscService, kIsscNotify, { kIsscWrite, kIsscWriteFallback }, { "UT60BT", "UT161", "UT-D07" } };
    default:
      return std::nullopt;
  }
}

BleGattDevice::BleGattDevice(const DmmDecoder::DMMInfo &info, const QString &device, QObject *parent)
  : QIODevice(parent)
  , m_address(device.simplified().section(' ', 0, 0).toUpper())
  , m_profile(profile(info.protocol))
{
}

BleGattDevice::~BleGattDevice()
{
  close();
}

bool BleGattDevice::open(OpenMode mode)
{
  if (QBluetoothAddress(m_address).isNull())
  {
    setErrorString(tr("No Bluetooth address configured."));
    return false;
  }
  if (!m_profile)
  {
    setErrorString(tr("This meter does not use a Bluetooth LE connection."));
    return false;
  }
  if (!lcBle().isDebugEnabled())
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth.bluez.warning=false\nqt.bluetooth.bluez.info=false"));

  QBluetoothDeviceInfo info(QBluetoothAddress(m_address), QString(), 0);
  info.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
  m_controller = QLowEnergyController::createCentral(info, this);
  m_serviceFound = false;
  m_ready = false;

  connect(m_controller, &QLowEnergyController::connected, this, [this]
  {
    qCDebug(lcBle) << m_address << "connected, discovering services";
    m_controller->discoverServices();
  });
  connect(m_controller, &QLowEnergyController::serviceDiscovered, this, &BleGattDevice::onServiceDiscovered);
  connect(m_controller, &QLowEnergyController::discoveryFinished, this, &BleGattDevice::onDiscoveryFinished);
  connect(m_controller, &QLowEnergyController::disconnected, this, [this]
  {
    fail(tr("The meter %1 closed the Bluetooth connection.").arg(m_address));
  });
  connect(m_controller, &QLowEnergyController::errorOccurred, this, [this](QLowEnergyController::Error)
  {
    fail(tr("Bluetooth: %1").arg(m_controller->errorString()));
  });

  m_connectTimeout = new QTimer(this);
  m_connectTimeout->setSingleShot(true);
  connect(m_connectTimeout, &QTimer::timeout, this, [this]
  {
    if (!m_ready)
      fail(tr("No answer from %1 - is the meter on and its Bluetooth switched on?").arg(m_address));
  });
  m_connectTimeout->start(kConnectTimeoutMs);

  m_repoll = new QTimer(this);
  m_repoll->setSingleShot(true);
  m_repoll->setInterval(kRepollMs);
  connect(m_repoll, &QTimer::timeout, this, &BleGattDevice::sendPoll);

  m_controller->connectToDevice();
  return QIODevice::open(mode | QIODevice::Unbuffered);
}

void BleGattDevice::close()
{
  m_ready = false;
  m_lastPoll.clear();
  for (QTimer **t : { &m_connectTimeout, &m_repoll })
    if (*t)
    {
      (*t)->stop();
      (*t)->deleteLater();
      *t = nullptr;
    }
  if (m_service)
  {
    m_service->disconnect(this);
    m_service->deleteLater();
    m_service = nullptr;
  }
  if (m_controller)
  {
    m_controller->disconnect(this);
    m_controller->disconnectFromDevice();
    m_controller->deleteLater();
    m_controller = nullptr;
  }
  m_rx.clear();
  QIODevice::close();
}

void BleGattDevice::fail(const QString &reason)
{
  if (!isOpen())
    return;
  qCDebug(lcBle) << m_address << reason;
  setErrorString(reason);
  m_ready = false;
  Q_EMIT finished(reason);
}

void BleGattDevice::onServiceDiscovered(const QBluetoothUuid &uuid)
{
  if (uuid == m_profile->service)
    m_serviceFound = true;
}

void BleGattDevice::onDiscoveryFinished()
{
  if (!m_serviceFound)
  {
    fail(tr("%1 does not offer the expected Bluetooth service - is it the right meter?").arg(m_address));
    return;
  }
  m_service = m_controller->createServiceObject(m_profile->service, this);
  if (!m_service)
  {
    fail(tr("Could not open the Bluetooth service of %1.").arg(m_address));
    return;
  }
  connect(m_service, &QLowEnergyService::stateChanged, this, &BleGattDevice::onServiceState);
  connect(m_service, &QLowEnergyService::characteristicChanged, this,
          [this](const QLowEnergyCharacteristic &c, const QByteArray &value)
  {
    if (c.uuid() != m_profile->notify)
      return;
    qCDebug(lcBle) << m_address << "rx" << value.toHex(' ');
    m_rx += value;
    // the answer is in: ask again, but not faster than kRepollMs
    if (!m_lastPoll.isEmpty() && m_repoll && !m_repoll->isActive())
      m_repoll->start();
    Q_EMIT readyRead();
  });
  connect(m_service, &QLowEnergyService::errorOccurred, this, [this](QLowEnergyService::ServiceError e)
  {
    // a failed write of one poll is not the end of the link; the rest is
    if (e == QLowEnergyService::CharacteristicWriteError)
      qCDebug(lcBle) << m_address << "write failed";
    else
      fail(tr("Bluetooth service error %1 on %2.").arg(int(e)).arg(m_address));
  });
  m_service->discoverDetails();
}

void BleGattDevice::onServiceState()
{
  if (m_service->state() != QLowEnergyService::RemoteServiceDiscovered)
    return;
  const QLowEnergyCharacteristic notify = m_service->characteristic(m_profile->notify);
  if (!notify.isValid())
  {
    fail(tr("%1 has no notify characteristic.").arg(m_address));
    return;
  }
  for (const QBluetoothUuid &uuid : m_profile->write)
  {
    const QLowEnergyCharacteristic c = m_service->characteristic(uuid);
    if (c.isValid() && (c.properties() & (QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::WriteNoResponse)))
    {
      m_writeChar = c;
      break;
    }
  }
  if (!m_writeChar.isValid())
  {
    fail(tr("%1 has no writable characteristic.").arg(m_address));
    return;
  }
  const QLowEnergyDescriptor cccd = notify.clientCharacteristicConfiguration();
  if (cccd.isValid())
    m_service->writeDescriptor(cccd, QLowEnergyCharacteristic::CCCDEnableNotification);
  m_ready = true;
  if (m_connectTimeout)
    m_connectTimeout->stop();
  qCDebug(lcBle) << m_address << "ready";
}

qint64 BleGattDevice::bytesAvailable() const
{
  return m_rx.size() + QIODevice::bytesAvailable();
}

qint64 BleGattDevice::readData(char *data, qint64 maxSize)
{
  const qint64 len = qMin(maxSize, qint64(m_rx.size()));
  memcpy(data, m_rx.constData(), len);
  m_rx.remove(0, len);
  return len;
}

qint64 BleGattDevice::writeData(const char *data, qint64 len)
{
  m_lastPoll = QByteArray(data, int(len));
  sendPoll();
  return len;
}

void BleGattDevice::sendPoll()
{
  if (!m_ready || !m_service || m_lastPoll.isEmpty())
    return;   // not connected yet: the reader repeats its poll
  const auto mode = (m_writeChar.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                    ? QLowEnergyService::WriteWithoutResponse : QLowEnergyService::WriteWithResponse;
  m_service->writeCharacteristic(m_writeChar, m_lastPoll, mode);
}

QStringList BleGattDevice::scan(ReadEvent::DataFormat format, int ms)
{
  const auto p = profile(format);
  if (!p)
    return {};
  QBluetoothDeviceDiscoveryAgent agent;
  agent.setLowEnergyDiscoveryTimeout(ms);
  QStringList found;
  QSet<QString> seen;
  auto take = [&](const QBluetoothDeviceInfo &info)
  {
    const QString address = info.address().toString().toUpper();
    if (seen.contains(address))
      return;
    bool match = info.serviceUuids().contains(p->service);
    for (const QString &prefix : p->namePrefixes)
      if (info.name().startsWith(prefix, Qt::CaseInsensitive))
        match = true;
    if (!match)
      return;
    seen.insert(address);
    found << address + ' ' + info.name();
  };
  QObject::connect(&agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, take);
  QObject::connect(&agent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated,
                   [&](const QBluetoothDeviceInfo &info, QBluetoothDeviceInfo::Fields) { take(info); });
  QElapsedTimer t;
  t.start();
  agent.start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
  while (agent.isActive() && t.elapsed() < ms + 1000)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  agent.stop();
  return found;
}
