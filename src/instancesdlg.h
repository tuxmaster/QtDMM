#pragma once

#include <QtCore>
#include "ui_uiinstancesdlg.h"
#include "settings.h"

/// Dialog listing the configured instance ids, which of them are running,
/// and buttons to start, raise, add or remove instances.
class InstancesDlg : public QDialog, private Ui::UIInstancesDlg
{
  Q_OBJECT
public:
  InstancesDlg(Settings *settings, QString instance_id, QString config_path, QWidget *parent = Q_NULLPTR);

  /// The ids currently registered in the shared state (SharedStateManager).
  void setInstancesOnline(QStringList instances);
  void updateInstancesListBox();

protected:
  QStringList m_instancesOnline;
  QStringList m_instancesConfigured;
  QString     m_configPath;
  QString     m_instanceId;
  Settings   *m_settings;
  bool        m_onDelete;

protected Q_SLOTS:
  void onInstanceButtonClicked();
  void on_ui_instance_add_clicked();
  void on_ui_instance_del_clicked();

Q_SIGNALS:
  void raiseApplicationWindow(const QString &);
  /// A state string for SharedStateManager::writeState().
  void writeState(const QString &);
};
