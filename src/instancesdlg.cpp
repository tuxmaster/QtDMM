#include <QtGui>
#include <QtWidgets>

#include "instancesdlg.h"
#include "sharedstatemanager.h"
#include "siprefix.h"
#include "calcexpr.h"
#include "readevent.h"
#include <QRegularExpression>

namespace
{
// instance names double as variable names in formulas
const QRegularExpression kIdentifier("^[A-Za-z_][A-Za-z0-9_]*$");
}


InstancesDlg::InstancesDlg(Settings *settings, QString instance_id, QString config_path, QWidget *parent)
  :  QDialog(parent)
  , m_instanceId(instance_id.isEmpty() ? "default" : instance_id)
  , m_configPath(config_path)
  , m_settings(settings)
  , m_onDelete(false)
{
  setupUi(this);
  ui_instancesList->setStyleSheet("background-color: palette(window);");
  m_refresh.setInterval(1000);
  connect(&m_refresh, &QTimer::timeout, this, &InstancesDlg::updateValues);
}

void InstancesDlg::setStateManager(SharedStateManager *state)
{
  m_state = state;
}

void InstancesDlg::showEvent(QShowEvent *ev)
{
  QDialog::showEvent(ev);
  updateValues();
  m_refresh.start();
}

void InstancesDlg::hideEvent(QHideEvent *ev)
{
  m_refresh.stop();
  QDialog::hideEvent(ev);
}

// "12.01 V DC" for a running instance that publishes, "-" for one that shows
// OL, "offline" otherwise; the calculated instances show their formula's
// value like any other
void InstancesDlg::updateValues()
{
  const auto readings = m_state ? m_state->readings() : QMap<QString, SharedStateManager::Reading>();
  for (auto it = m_valueLabels.constBegin(); it != m_valueLabels.constEnd(); ++it)
  {
    QString text;
    if (readings.contains(it.key()))
    {
      const SharedStateManager::Reading &r = readings[it.key()];
      if (r.valid)
      {
        QString prefix;
        const QString value = SiPrefix::format(r.value, &prefix);
        text = QString("%1 %2%3 %4").arg(value.left(8), prefix, r.unit, r.special).trimmed();
      }
      else
        text = QStringLiteral("OL");
    }
    else if (m_instancesOnline.contains(it.key()))
      text = tr("running");
    else
      text = tr("offline");
    it.value()->setText(text);
  }
}

void InstancesDlg::setInstancesOnline(QStringList instances)
{
  m_instancesOnline = instances;
  updateInstancesListBox();
}

void InstancesDlg::updateInstancesListBox()
{
  ui_instancesList->clear();
  m_valueLabels.clear();
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

    // button left, the live reading right
    QWidget *row = new QWidget(ui_instancesList);
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 6, 0);
    layout->addWidget(btn, 1);
    QLabel *value = new QLabel(row);
    value->setMinimumWidth(120);
    value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(value, 0);
    m_valueLabels.insert(instanceId, value);

    ui_instancesList->addItem(wi_item);
    ui_instancesList->setItemWidget(wi_item, row);
  }
  updateValues();
}

void InstancesDlg::launchInstance(const QString &configId)
{
  if (m_instancesOnline.contains(configId))
  {
    Q_EMIT writeState("RAISE_"+configId);
    return;
  }
  QStringList args;
  if (configId != "default")
    args << "--config-id" << configId;
  if (!m_configPath.isEmpty())
    args << "--config-dir" << m_configPath;
  QProcess::startDetached(QCoreApplication::applicationFilePath(), args);
}

// Writes a complete configuration for a calculated instance - the model
// "QtDMM / Calculated value", its formula and unit, and the port entry DMM
// uses - so the new instance connects on its first start without a visit
// to the settings dialog.
QString InstancesDlg::createCalculatedInstance(const QString &configId, const QString &configPath,
                                               const QString &unit, const QString &formula, bool start)
{
  QString file;
  {
    Settings cfg(configId, configPath);
    cfg.setString("DMM/model", "QtDMM Calculated value");
    cfg.setInt("DMM/data-format", ReadEvent::Sigrok);
    cfg.setString("DMM/display", "400000");
    cfg.setString("DMM/calc-unit", unit);
    cfg.setString("DMM/calc-expression", formula);
    cfg.setString("Port settings/device", QString("calc %1 %2").arg(unit, formula));
    cfg.setBool("QtDMM/show-tip", false);
    // what GuiPrefs writes on OK; without them ConfigDlg greets the file as
    // an upgrade from before 0.8.4
    cfg.setInt("QtDMM/version", 0);
    cfg.setInt("QtDMM/revision", 84);
    cfg.save();
    file = cfg.fileName();
  }   // the QSettings object syncs the file when it goes away

  if (start)
  {
    QStringList args;
    args << "--config-id" << configId;
    if (!configPath.isEmpty())
      args << "--config-dir" << configPath;
    QProcess::startDetached(QCoreApplication::applicationFilePath(), args);
  }
  return file;
}

void InstancesDlg::on_ui_instance_calc_clicked()
{
  QDialog dlg(this);
  dlg.setWindowTitle(tr("QtDMM - new calculated instance"));
  QFormLayout *form = new QFormLayout(&dlg);
  QLineEdit *name = new QLineEdit(&dlg);
  name->setPlaceholderText("p");
  QLineEdit *unit = new QLineEdit("W", &dlg);
  unit->setMaximumWidth(80);
  QLineEdit *formula = new QLineEdit(&dlg);
  formula->setPlaceholderText("u * i");
  QLabel *hint = new QLabel(&dlg);
  hint->setWordWrap(true);
  QStringList others;
  for (const QString &id : m_instancesOnline)
    others << QString(id).replace('-', '_');
  hint->setText(others.isEmpty() ? tr("No instance is running yet; the variables are the instance names.")
                                 : tr("Variables (running instances): %1").arg(others.join(", ")));
  form->addRow(tr("&Name:"), name);
  form->addRow(tr("&Unit:"), unit);
  form->addRow(tr("&Formula:"), formula);
  form->addRow(hint);
  QLabel *error = new QLabel(&dlg);
  error->setStyleSheet("color: #b00;");
  error->setWordWrap(true);
  form->addRow(error);
  QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
  form->addRow(buttons);
  connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
  connect(buttons, &QDialogButtonBox::accepted, &dlg, [&]
  {
    const QString id = name->text().trimmed();
    QString parseError;
    int pos = -1;
    if (!kIdentifier.match(id).hasMatch() || id == "default")
      error->setText(tr("Instance names may contain letters, digits and underscores and must not start with a digit."));
    else if (m_instancesOnline.contains(id) || m_instancesConfigured.contains(id))
      error->setText(tr("Instance already exists."));
    else if (unit->text().trimmed().isEmpty() || unit->text().contains(' '))
      error->setText(tr("Please give a unit without spaces."));
    else if (!CalcExpr::parse(formula->text(), &parseError, &pos))
      error->setText(tr("Position %1: %2").arg(pos + 1).arg(parseError));
    else
      dlg.accept();
  });
  if (dlg.exec() != QDialog::Accepted)
    return;

  createCalculatedInstance(name->text().trimmed(), m_configPath, unit->text().trimmed(), formula->text().trimmed(), true);
  Q_EMIT writeState("UPDATE_INSTANCES_"+QString::number(QDateTime::currentMSecsSinceEpoch()));
}


void InstancesDlg::onInstanceButtonClicked()
{
  if (m_onDelete)
    return;

  QPushButton *btn = qobject_cast<QPushButton *>(sender());
  if (!btn)
    return;

  QString configId = btn->property("instanceId").toString();

  if (configId == m_instanceId)
    return;
  launchInstance(configId);
}

void InstancesDlg::on_ui_instance_add_clicked()
{
  bool ok{};
  QString configId = QInputDialog::getText(this, tr("QtDMM - new instance"), tr("Instance name:"), QLineEdit::Normal, "", &ok).trimmed();
  if (!ok || configId.isEmpty() || configId == "default")
    return;

  // instance names double as variable names in the formulas of calculated
  // instances, so they must be identifiers ("u", "psu_1"; not "uni-t 803")
  if (!kIdentifier.match(configId).hasMatch())
  {
    QMessageBox::warning(this, APP_NAME,
                         tr("Instance names may contain letters, digits and underscores and must not start with a digit."));
    return;
  }

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


