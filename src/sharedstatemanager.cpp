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


QJsonObject SharedStateManager::readJsonData()
{
  if (!m_memory.isAttached() && !m_memory.attach())
    return initialJson();

  m_memory.lock();

  QByteArray rawData(static_cast<const char*>(m_memory.constData()), m_memory.size());
  int end = rawData.indexOf('\0');
  if (end != -1)
    rawData.truncate(end);

  m_memory.unlock();
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);
  //qInfo() << QJsonDocument(doc).toJson(QJsonDocument::Compact);

  if (parseError.error != QJsonParseError::NoError || !doc.isObject())
  {
    qInfo("parse error");
    return initialJson();
  }

  return doc.object();
}

bool SharedStateManager::writeJsonData(const QJsonObject &obj)
{
  if (!m_memory.isAttached() && !m_memory.attach())
  {
    if (!m_memory.create(8192))
      return false;
  }
  QJsonDocument doc(obj);
  QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

  if (jsonData.size() > m_memory.size())
  {
    m_memory.detach();
    if (!m_memory.create(jsonData.size() + 1024))
      return false;
  }

  m_memory.lock();
  char *to = static_cast<char*>(m_memory.data());
  const char* from = jsonData.constData();
  memset(to, 0, m_memory.size());
  memcpy(to, from, jsonData.size());

  m_memory.unlock();

  return true;
}

bool SharedStateManager::registerInstance()
{
  if (!m_registered)
  {
    QJsonObject data = readJsonData();
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

    m_registered = writeJsonData(data);
    if (!m_registered)
      qInfo() << "reg failed";
    else
      qInfo() << "reg" << m_instanceId;

  }

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
      qInfo() << currentState << QJsonDocument(data).toJson(QJsonDocument::Compact);
      m_lastState = currentState;
      if (currentState.startsWith("UPDATE_INSTANCES"))
        Q_EMIT instancesChanged(m_instances);
      Q_EMIT stateChanged(currentState);
    }
  }
  if (m_emit_inUse)
    Q_EMIT instanceIdAlreadyInUse();

}

bool SharedStateManager::writeState(const QString &newState)
{
  QJsonObject data = readJsonData();
  data["state"] = newState;
  return writeJsonData(data);
}

void SharedStateManager::unregisterInstance()
{
  if (m_registered)
  {
    QJsonObject data = readJsonData();
    QJsonArray instances = data["instances"].toArray();

    QJsonArray updated;
    for (const QJsonValue &val : instances)
    {
      if (val.toString() != m_instanceId)
        updated.append(val);
    }

    data["instances"] = updated;
    writeJsonData(data);
  }
}

