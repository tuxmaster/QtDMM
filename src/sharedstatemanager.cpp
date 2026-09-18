#include "sharedstatemanager.h"
#include <QBuffer>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QStringDecoder>

SharedStateManager::SharedStateManager(const QString &instanceId, QObject *parent)
  : QObject(parent),
    m_memory("qtdmm_ipc_memory"),
    m_lastState(),
    m_instanceId(instanceId),
    m_registered(false),
    m_emit_inUse(false)
{
  m_timer.setInterval(250);
  connect(&m_timer, &QTimer::timeout, this, &SharedStateManager::checkForChanges);
  m_timer.start();
}

SharedStateManager::~SharedStateManager()
{
  unregisterInstance();
}

QJsonObject SharedStateManager::initialJson()
{
  QJsonObject data;
  data["instances"] = QJsonArray();
  data["state"] = "";
  return data;
}


// Allocated generously up front: QSharedMemory cannot be grown while another
// process is still attached (create() fails with AlreadyExists), so growing
// on demand is unsafe with multiple running instances. A few hundred instance
// ids plus the state string comfortably fit well within this size.
static constexpr int kSharedMemorySize = 65536;

bool SharedStateManager::ensureAttached()
{
  if (m_memory.isAttached())
    return true;
  if (m_memory.attach())
    return true;
  return m_memory.create(kSharedMemorySize);
}

QJsonObject SharedStateManager::readJsonDataLocked()
{
  QByteArray rawData(static_cast<const char*>(m_memory.constData()), m_memory.size());
  int end = rawData.indexOf('\0');
  if (end != -1)
    rawData.truncate(end);

  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);

  if (parseError.error != QJsonParseError::NoError || !doc.isObject())
  {
    qInfo("parse error");
    return initialJson();
  }

  return doc.object();
}

QJsonObject SharedStateManager::readJsonData()
{
  if (!ensureAttached())
    return initialJson();

  m_memory.lock();
  QJsonObject obj = readJsonDataLocked();
  m_memory.unlock();

  return obj;
}

bool SharedStateManager::writeJsonDataLocked(const QJsonObject &obj)
{
  QJsonDocument doc(obj);
  QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

  if (jsonData.size() > m_memory.size())
  {
    qWarning() << "shared state too large (" << jsonData.size() << "bytes) for the"
               << m_memory.size() << "byte shared memory segment, dropping update";
    return false;
  }

  char *to = static_cast<char*>(m_memory.data());
  memset(to, 0, m_memory.size());
  memcpy(to, jsonData.constData(), jsonData.size());

  return true;
}

bool SharedStateManager::modifyJsonData(const std::function<bool(QJsonObject &)> &mutator)
{
  if (!ensureAttached())
    return false;

  m_memory.lock();
  QJsonObject data = readJsonDataLocked();
  bool shouldWrite = mutator(data);
  bool ok = shouldWrite && writeJsonDataLocked(data);
  m_memory.unlock();

  return ok;
}

bool SharedStateManager::registerInstance()
{
  if (m_registered)
    return true;

  m_registered = modifyJsonData([this](QJsonObject &data)
  {
    QJsonArray instances = data["instances"].toArray();

    for (const QJsonValue &val : instances)
    {
      if (val.toString() == m_instanceId)
      {
        qWarning() << m_instanceId << "id exists";
        m_emit_inUse = true;
        return false;
      }
    }

    instances.append(m_instanceId);
    data["instances"] = instances;
    return true;
  });

  if (!m_registered)
    qInfo() << "reg failed";

  return m_registered;
}

void SharedStateManager::checkForChanges()
{
  if (m_registered)
  {
    QJsonObject data = readJsonData();

    QString currentState = data["state"].toString();
    QStringList instances;
    for (const QVariant &v : data["instances"].toArray().toVariantList())
      instances << v.toString();

    if (instances.count() != m_instances.count())
    {
      m_instances = instances;
      Q_EMIT instancesChanged(m_instances);
    }

    if (currentState != m_lastState)
    {
      //qInfo() << currentState << QJsonDocument(data).toJson(QJsonDocument::Compact);
      m_lastState = currentState;
      if (currentState.startsWith("UPDATE_INSTANCES"))
        Q_EMIT instancesChanged(m_instances);
      Q_EMIT stateChanged(currentState);
    }
  }
  if (m_emit_inUse)
  {
    m_emit_inUse = false;
    Q_EMIT instanceIdAlreadyInUse();
  }
}

bool SharedStateManager::writeState(const QString &newState)
{
  return modifyJsonData([&newState](QJsonObject &data)
  {
    data["state"] = newState;
    return true;
  });
}

void SharedStateManager::unregisterInstance()
{
  if (!m_registered)
    return;

  modifyJsonData([this](QJsonObject &data)
  {
    QJsonArray instances = data["instances"].toArray();

    QJsonArray updated;
    for (const QJsonValue &val : instances)
    {
      if (val.toString() != m_instanceId)
        updated.append(val);
    }

    data["instances"] = updated;
    return true;
  });
}

