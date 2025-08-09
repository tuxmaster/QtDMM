#pragma once

#include <QtCore>
#include "ui_uiinstancesdlg.h"

class InstancesDlg : public QDialog, private Ui::UIInstancesDlg
{
  Q_OBJECT
public:
  InstancesDlg(QString session_id, QString config_path, QWidget *parent = Q_NULLPTR);

  void setInstancesOnline(QStringList instances);
  void setInstancesConfigured(QStringList instances);
  void updateInstancesListBox();

protected:
  QStringList m_instancesOnline;
  QStringList m_instancesConfigured;
  QString m_configPath;
  QString m_sessionId;

protected Q_SLOTS:
  void on_ui_instancesList_itemDoubleClicked(QListWidgetItem *item);

Q_SIGNALS:
  void raiseApplicationWindow(const QString &);
};
