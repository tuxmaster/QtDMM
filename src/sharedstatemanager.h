#pragma once

#include <QObject>
#include <QSharedMemory>
#include <QTimer>
#include <QJsonObject>
#include <functional>

class SharedStateManager : public QObject
{
  Q_OBJECT

public:
  explicit SharedStateManager(const QString &instanceId, QObject *parent = nullptr);
  ~SharedStateManager();

  bool registerInstance();
  void unregisterInstance();

  void checkForChanges();
  QStringList instances() { return m_instances; };

signals:
  void stateChanged(const QString &newState);
  void instanceIdAlreadyInUse();
  void instancesChanged(QStringList &instances);

public Q_SLOTS:
  bool writeState(const QString &newState);

private:
  QJsonObject readJsonData();
  QJsonObject initialJson();
  bool ensureAttached();
  QJsonObject readJsonDataLocked();
  bool writeJsonDataLocked(const QJsonObject &obj);
  // Reads, lets mutator modify the data, and writes it back, all under a single
  // held lock so the read-check-write sequence is atomic across instances.
  // mutator returns false to veto the write (e.g. duplicate instance id found).
  bool modifyJsonData(const std::function<bool(QJsonObject &)> &mutator);

  QSharedMemory m_memory;
  QTimer m_timer;
  QString m_lastState;
  QString m_instanceId;
  bool m_registered;
  bool m_emit_inUse;
  QStringList m_instances;
};
