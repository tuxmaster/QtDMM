#pragma once

#include <QObject>
#include <QSharedMemory>
#include <QTimer>
#include <QJsonObject>
#include <QMap>
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
///
/// Each instance also publishes its current main reading in its entry
/// (publishReading()); readings() gives the last seen readings of all
/// instances, which is what a calculated value (P = U * I) is computed from.
///
/// The segment key can be overridden with the environment variable
/// QTDMM_IPC_KEY so tests do not interfere with a running QtDMM.
class SharedStateManager : public QObject
{
  Q_OBJECT

public:
  /// One instance's main reading as published in the shared state.
  struct Reading
  {
    double  value = 0;      ///< in SI base units (DmmResponse::dval)
    QString unit;           ///< base unit without prefix, e.g. "V"
    QString special;        ///< "DC", "AC", ... as the decoder delivers it
    qint64  msecs = 0;      ///< QDateTime::currentMSecsSinceEpoch() when published
    bool    valid = false;  ///< false for overload / no numeric value
  };

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
  /// This instance's id.
  QString id() const { return m_instanceId; }
  /// Readings of all instances (including this one) as of the last poll,
  /// keyed by instance id. Instances that have not published yet are absent.
  QMap<QString, Reading> readings() const { return m_readings; }
  /// Writes this instance's reading into its shared entry. Unchanged values
  /// are re-published only once a second, so an idle meter costs nothing.
  void publishReading(const Reading &reading);

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
  QMap<QString, Reading> m_readings;
  Reading m_published;
};
