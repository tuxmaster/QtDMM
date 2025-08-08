#pragma once

#include <QObject>
#include <QSharedMemory>
#include <QTimer>
#include <QJsonObject>

class SharedStateManager : public QObject
{
  Q_OBJECT

public:
  explicit SharedStateManager(const QString &instanceId, QObject *parent = nullptr);
  ~SharedStateManager();

  bool registerInstance();
  void unregisterInstance();

  bool writeState(const QString &newState);
  void checkForChanges();
  QVariantList instanceCount() { return m_instances; };

signals:
  void stateChanged(const QString &newState);
  void instanceIdAlreadyInUse();
  void instanceCountChanged(QVariantList &instances);

private:
  QJsonObject readJsonData();
  bool writeJsonData(const QJsonObject &obj);
  QJsonObject initialJson();

  QSharedMemory m_memory;
  QTimer m_timer;
  QString m_lastState;
  QString m_instanceId;
  bool m_registered;
  bool m_emit_inUse;
  QVariantList m_instances;
};
