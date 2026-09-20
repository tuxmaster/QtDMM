#pragma once

#include <QObject>
#include <QSharedMemory>
#include <QTimer>
#include <QJsonObject>
#include <functional>

/// Lets several running QtDMM instances see each other and exchange a state.
///
/// All instances share one QSharedMemory segment holding a JSON object with
/// the list of registered instances (id + pid) and a single state string.
/// writeState() replaces that string; every instance polls the segment on a
/// timer and emits stateChanged() when it differs from what it saw last.
/// MainWin uses this for synchronised recording ("RECORD"/"STOP") and to
/// raise a specific instance ("RAISE_<id>"). Registrations of instances that
/// died without unregistering are detected by their pid and removed.
class SharedStateManager : public QObject
{
  Q_OBJECT

public:
  explicit SharedStateManager(const QString &instanceId, QObject *parent = nullptr);
  ~SharedStateManager();

  /// Adds this instance to the shared list. Returns false and later emits
  /// instanceIdAlreadyInUse() when a live instance with the same id exists.
  bool registerInstance();
  void unregisterInstance();

  /// Poll step (timer driven): emits the change signals.
  void checkForChanges();
  /// Ids of the instances seen at the last poll.
  QStringList instances() { return m_instances; };

signals:
  /// The shared state string changed to @p newState.
  void stateChanged(const QString &newState);
  void instanceIdAlreadyInUse();
  /// The set of registered instances changed.
  void instancesChanged(QStringList &instances);

public Q_SLOTS:
  /// Replaces the shared state string, seen by all instances at their next poll.
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
