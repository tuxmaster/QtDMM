#include "sharedstatemanager.h"
#include <QBuffer>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QStringDecoder>
#include <QCoreApplication>

#ifdef Q_OS_UNIX
#include <csignal>
#include <sys/types.h>
#endif

namespace
{
// True if a process with this pid still exists. Used to tell a genuinely
// still-running instance apart from a stale registration left behind by one
// that crashed/was killed without reaching its destructor (unregisterInstance()
// never ran, so its "instances" entry survives in shared memory indefinitely -
// System V shared memory on Linux is not cleaned up just because every
// attached process has died).
bool isProcessAlive(qint64 pid)
{
#ifdef Q_OS_UNIX
  if (pid <= 0)
    return false;
  // Signal 0: no signal is sent, only existence/permission is checked.
  return ::kill(static_cast<pid_t>(pid), 0) == 0 || errno != ESRCH;
#else
  Q_UNUSED(pid);
  return true; // no cheap liveness check available; assume alive (old behavior)
#endif
}

// Instance entries are {"id": ..., "pid": ...} objects; also accepts the old
// plain-string format (no pid available then) for backward compatibility.
QString instanceId(const QJsonValue &val)
{
  if (val.isObject())
    return val.toObject()["id"].toString();
  return val.toString();
}
}

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
    QJsonArray kept;
    bool conflict = false;

    for (const QJsonValue &val : instances)
    {
      QJsonObject entry = val.toObject();
      // Backward compatible with the old plain-string format (no pid to check
      // liveness of) - keep it as-is, err on the side of treating it as a
      // real, live conflict rather than silently dropping unknown data.
      QString id = entry.isEmpty() ? val.toString() : entry["id"].toString();
      qint64 pid = entry.isEmpty() ? -1 : static_cast<qint64>(entry["pid"].toDouble());

      if (id == m_instanceId)
      {
        if (pid > 0 && !isProcessAlive(pid))
        {
          qInfo() << m_instanceId << "had a stale registration (pid" << pid << "no longer running), replacing it";
          continue; // drop the stale entry instead of keeping it
        }

        qWarning() << m_instanceId << "id exists";
        m_emit_inUse = true;
        conflict = true;
      }

      kept.append(val);
    }

    if (conflict)
      return false;

    QJsonObject self;
    self["id"] = m_instanceId;
    self["pid"] = QCoreApplication::applicationPid();
    kept.append(self);
    data["instances"] = kept;
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
    for (const QJsonValue &v : data["instances"].toArray())
      instances << instanceId(v);

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
      if (instanceId(val) != m_instanceId)
        updated.append(val);
    }

    data["instances"] = updated;
    return true;
  });
}

