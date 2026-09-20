#pragma once

#include <QtCore>
#include <QTimer>
#include <QMap>
#include "ui_uiinstancesdlg.h"
#include "settings.h"

class QLabel;
class SharedStateManager;

/// Dialog listing the configured instance ids, which of them are running
/// and what they currently read, with buttons to start, raise, add or
/// remove instances and to create a calculated instance from a formula.
class InstancesDlg : public QDialog, private Ui::UIInstancesDlg
{
  Q_OBJECT
public:
  InstancesDlg(Settings *settings, QString instance_id, QString config_path, QWidget *parent = Q_NULLPTR);

  /// The ids currently registered in the shared state (SharedStateManager).
  void setInstancesOnline(QStringList instances);
  void updateInstancesListBox();
  /// Source of the live readings shown next to each instance.
  void setStateManager(SharedStateManager *state);
  /// Writes the settings of a new calculated instance and starts it.
  /// Exposed for tests: returns the settings file written.
  static QString createCalculatedInstance(const QString &configId, const QString &configPath,
                                          const QString &unit, const QString &formula, bool start);

protected:
  QStringList m_instancesOnline;
  QStringList m_instancesConfigured;
  QString     m_configPath;
  QString     m_instanceId;
  Settings   *m_settings;
  bool        m_onDelete;
  SharedStateManager *m_state = Q_NULLPTR;
  QMap<QString, QLabel *> m_valueLabels;   ///< per instance id, rebuilt with the list
  QTimer      m_refresh;

  /// Starts (or raises) the instance with this id.
  void launchInstance(const QString &configId);
  void showEvent(QShowEvent *) override;
  void hideEvent(QHideEvent *) override;

protected Q_SLOTS:
  void onInstanceButtonClicked();
  void on_ui_instance_add_clicked();
  void on_ui_instance_del_clicked();
  void on_ui_instance_calc_clicked();
  /// Refreshes the value labels from the shared readings.
  void updateValues();

Q_SIGNALS:
  void raiseApplicationWindow(const QString &);
  /// A state string for SharedStateManager::writeState().
  void writeState(const QString &);
};
