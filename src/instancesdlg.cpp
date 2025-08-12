#include <QtGui>
#include <QtWidgets>

#include "instancesdlg.h"


InstancesDlg::InstancesDlg(Settings *settings, QString instance_id, QString config_path, QWidget *parent)
  :  QDialog(parent)
  , m_instanceId(instance_id.isEmpty() ? "default" : instance_id)
  , m_configPath(config_path)
  , m_settings(settings)
{
  setupUi(this);
  ui_instancesList->setStyleSheet("background-color: palette(window);");
}

void InstancesDlg::setInstancesOnline(QStringList instances)
{
  m_instancesOnline = instances;
  updateInstancesListBox();
}

void InstancesDlg::updateInstancesListBox()
{
  ui_instancesList->clear();
  m_instancesConfigured = m_settings->getConfigInstances();

  QStringList instances = m_instancesConfigured;
  for (auto &instanceId : m_instancesOnline)
    if (!instances.contains(instanceId))
      instances << instanceId;

  for (auto &instanceId : m_instancesConfigured)
  {
    QListWidgetItem *wi_item = new QListWidgetItem(ui_instancesList);
    wi_item->setSizeHint(QSize(150, 40));

    QPushButton *btn = new QPushButton(instanceId, ui_instancesList);
    btn->setProperty("instanceId", instanceId);
    QFont font = btn->font();
    if (instanceId != m_instanceId)
      font.setBold(true);
    font.setPointSize(font.pointSize() + 2);
    btn->setFont(font);

    if (instanceId == m_instanceId || (m_onDelete && m_instancesOnline.contains(instanceId)))
      btn->setEnabled(false);
    else if (m_onDelete)
    {
      btn->setCheckable(true);
    }
    else
    {
      btn->setCheckable(false);
    }

    connect(btn, SIGNAL(clicked()), this, SLOT(onInstanceButtonClicked()));

    ui_instancesList->addItem(wi_item);
    ui_instancesList->setItemWidget(wi_item, btn);
  }
}


void InstancesDlg::onInstanceButtonClicked()
{
  if (m_onDelete)
    return;

  QPushButton *btn = qobject_cast<QPushButton *>(sender());
  if (!btn)
    return;

  QString configId = btn->property("instanceId").toString();
  QString exePath = QCoreApplication::applicationFilePath();

  if (configId == m_instanceId)
    return;

  if (m_instancesOnline.contains(configId))
    Q_EMIT writeState("RAISE_"+configId);
  else
  {
    QStringList args;
    if (configId != "default")
      args << "--config-id" << configId;
    if (!m_configPath.isEmpty())
      args << "--config-dir" << m_configPath;

    QProcess::startDetached(exePath, args);
  }
}

void InstancesDlg::on_ui_instance_add_clicked()
{
  bool ok{};
  QString configId = QInputDialog::getText(this, tr("QtDMM - new instance"), tr("Instance name:"), QLineEdit::Normal, "", &ok).trimmed();
  if (!ok || configId.isEmpty() || configId == "default")
    return;

  if (m_instancesOnline.contains(configId) || m_instancesConfigured.contains(configId) )
  {
    QMessageBox::warning(this, APP_NAME,tr("Instance already exists."));
    return;
  }

  QString exePath = QCoreApplication::applicationFilePath();

  QStringList args;
  if (configId != "default")
    args << "--config-id" << configId;
  if (!m_configPath.isEmpty())
    args << "--config-dir" << m_configPath;

  QProcess::startDetached(exePath, args);
  Q_EMIT writeState("UPDATE_INSTANCES_"+QString::number(QDateTime::currentMSecsSinceEpoch()));
}


void InstancesDlg::on_ui_instance_del_clicked()
{
  if (m_onDelete && !ui_instance_del->isChecked())
  {
    for (int i = 0; i < ui_instancesList->count(); ++i)
    {
      QListWidgetItem *item = ui_instancesList->item(i);
      if (!item)
        continue;

      auto btn = qobject_cast<QPushButton*>(ui_instancesList->itemWidget(item));
      if (!btn || !btn->isChecked())
        continue;

      QString configId = btn->property("instanceId").toString();
      m_settings->deleteConfig(configId);
    }
    Q_EMIT writeState("UPDATE_INSTANCES_"+QString::number(QDateTime::currentMSecsSinceEpoch()));
  }

  m_onDelete = ui_instance_del->isChecked();
  updateInstancesListBox();
}


