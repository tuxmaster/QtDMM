#pragma once

#include <QtCore>
#include "ui_uiinstancesdlg.h"
#include "settings.h"

class InstancesDlg : public QDialog, private Ui::UIInstancesDlg
{
  Q_OBJECT
public:
  InstancesDlg(Settings *settings, QString session_id, QString config_path, QWidget *parent = Q_NULLPTR);

  void setInstancesOnline(QStringList instances);
  void updateInstancesListBox();

protected:
  QStringList m_instancesOnline;
  QStringList m_instancesConfigured;
  QString     m_configPath;
  QString     m_sessionId;
  Settings   *m_settings;
  bool        m_onDelete;

protected Q_SLOTS:
  void onInstanceButtonClicked();
  void on_ui_instance_add_clicked();
  void on_ui_instance_del_clicked();

Q_SIGNALS:
  void raiseApplicationWindow(const QString &);
  void writeState(const QString &);
};
