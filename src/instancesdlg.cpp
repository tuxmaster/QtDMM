#include <QtGui>
#include <QtWidgets>

#include "instancesdlg.h"


InstancesDlg::InstancesDlg(QString session_id, QString config_path, QWidget *parent)
  :  QDialog(parent)
  , m_sessionId(session_id.isEmpty()? "default" : session_id)
  , m_configPath(config_path)
{
  setupUi(this);
  ui_instancesList->setStyleSheet("background-color: palette(window);");
}

void InstancesDlg::setInstancesOnline(QStringList instances)
{
  m_instancesOnline = instances;
  updateInstancesListBox();
}

void InstancesDlg::setInstancesConfigured(QStringList instances)
{
  m_instancesConfigured = instances;
  updateInstancesListBox();
}

void InstancesDlg::updateInstancesListBox()
{
    ui_instancesList->clear();

    for (auto &instanceId : m_instancesConfigured)
    {
        // Platzhalter-Item (nur Container)
        QListWidgetItem *wi_item = new QListWidgetItem(ui_instancesList);
        wi_item->setSizeHint(QSize(150, 40));

        // Button erstellen
        QPushButton *btn = new QPushButton(instanceId, ui_instancesList);
        btn->setProperty("instanceId", instanceId);
        QFont font = btn->font();
        if (instanceId != m_sessionId)
          font.setBold(true);
        font.setPointSize(font.pointSize() + 2);
        btn->setFont(font);

        // Falls es die aktuelle Session ist -> grau machen
        if (instanceId == m_sessionId)
            btn->setEnabled(false);

        // Klick-Handler
        connect(btn, &QPushButton::clicked, this, [this, btn]() {
            QString configId = btn->property("instanceId").toString();
  QString exePath = QCoreApplication::applicationFilePath();

  if (configId == m_sessionId)
    return;

  if (m_instancesOnline.contains(configId))
  {
    Q_EMIT raiseApplicationWindow(configId);
  }
  else
  {
    QStringList args;
    if (configId != "default")
      args << "--config-id" << configId;
    if (!m_configPath.isEmpty())
      args << "--config-dir" << m_configPath;

    QProcess::startDetached(exePath, args);
  }
        });

        // Item + Button verbinden
        ui_instancesList->addItem(wi_item);
        ui_instancesList->setItemWidget(wi_item, btn);
    }
}


void InstancesDlg::on_ui_instancesList_itemDoubleClicked(QListWidgetItem *item)
{

}
