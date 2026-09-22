// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "alarmprefs.h"

#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>

#include "engnumbervalidator.h"
#include "settings.h"

AlarmPrefs::AlarmPrefs(QWidget *parent) : PrefWidget(parent)
{
  m_label = tr("Alarms");
  m_description = tr("<b>Alarms watch the reading and tell you when it leaves the range you expect:</b> "
                     "a banner over the display, a beep, a popup, a program, the recorder.");
  m_pixmap = new QPixmap(":/Symbols/alarm.xpm");

  auto *layout = new QVBoxLayout(this);
  auto *intro = new QLabel(tr("Each alarm watches the main reading. It raises when its condition has held for the "
                              "given time and clears once the reading is back beyond the hysteresis. Untick an "
                              "alarm to keep it without it firing."), this);
  intro->setWordWrap(true);
  layout->addWidget(intro);

  m_list = new QListWidget(this);
  layout->addWidget(m_list, 1);
  connect(m_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *) { edit(); });
  connect(m_list, &QListWidget::itemChanged, this, [this](QListWidgetItem *item)
  {
    toggle(m_list->row(item), item->checkState() == Qt::Checked);
  });
  connect(m_list, &QListWidget::itemSelectionChanged, this, [this]
  {
    const bool any = !m_list->selectedItems().isEmpty();
    m_edit->setEnabled(any);
    m_remove->setEnabled(any);
  });

  auto *buttons = new QHBoxLayout;
  auto *add = new QPushButton(tr("&Add..."), this);
  m_edit = new QPushButton(tr("&Edit..."), this);
  m_remove = new QPushButton(tr("&Remove"), this);
  m_edit->setEnabled(false);
  m_remove->setEnabled(false);
  connect(add, &QPushButton::clicked, this, &AlarmPrefs::add);
  connect(m_edit, &QPushButton::clicked, this, &AlarmPrefs::edit);
  connect(m_remove, &QPushButton::clicked, this, &AlarmPrefs::remove);
  buttons->addWidget(add);
  buttons->addWidget(m_edit);
  buttons->addWidget(m_remove);
  buttons->addStretch(1);
  layout->addLayout(buttons);
}

void AlarmPrefs::setUnit(const QString &unit)
{
  m_unit = unit;
  refresh();
}

void AlarmPrefs::defaultsSLOT()
{
  m_alarms = Alarm::listFromJson(m_cfg->getString("Alarms/list"));
  refresh();
}

void AlarmPrefs::factoryDefaultsSLOT()
{
  m_alarms.clear();
  refresh();
}

void AlarmPrefs::applySLOT()
{
  m_cfg->setString("Alarms/list", Alarm::listToJson(m_alarms));
}

void AlarmPrefs::refresh()
{
  const int current = m_list->currentRow();
  m_list->blockSignals(true);
  m_list->clear();
  for (const Alarm &al : m_alarms)
  {
    auto *item = new QListWidgetItem(QString("%1  —  %2").arg(al.name, al.describe(m_unit)), m_list);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(al.enabled ? Qt::Checked : Qt::Unchecked);
    QPixmap swatch(12, 12);
    swatch.fill(al.color);
    item->setIcon(QIcon(swatch));
  }
  m_list->blockSignals(false);
  if (current >= 0 && current < m_list->count())
    m_list->setCurrentRow(current);
}

void AlarmPrefs::add()
{
  Alarm al;
  al.name = tr("Alarm %1").arg(m_alarms.size() + 1);
  AlarmDlg dlg(al, m_unit, this);
  if (dlg.exec() == QDialog::Accepted)
  {
    m_alarms << dlg.alarm();
    refresh();
    m_list->setCurrentRow(m_alarms.size() - 1);
  }
}

void AlarmPrefs::edit()
{
  const int row = m_list->currentRow();
  if (row < 0 || row >= m_alarms.size())
    return;
  AlarmDlg dlg(m_alarms[row], m_unit, this);
  if (dlg.exec() == QDialog::Accepted)
  {
    m_alarms[row] = dlg.alarm();
    refresh();
  }
}

void AlarmPrefs::remove()
{
  const int row = m_list->currentRow();
  if (row < 0 || row >= m_alarms.size())
    return;
  m_alarms.removeAt(row);
  refresh();
}

void AlarmPrefs::toggle(int row, bool on)
{
  if (row >= 0 && row < m_alarms.size())
    m_alarms[row].enabled = on;
}

// ---------------------------------------------------------------------------

AlarmDlg::AlarmDlg(const Alarm &alarm, const QString &unit, QWidget *parent)
  : QDialog(parent), m_unit(unit), m_colorValue(alarm.color)
{
  setWindowTitle(tr("Alarm"));
  auto *layout = new QVBoxLayout(this);

  auto *top = new QFormLayout;
  m_name = new QLineEdit(alarm.name, this);
  m_enabled = new QCheckBox(tr("&Enabled"), this);
  m_enabled->setChecked(alarm.enabled);
  auto *nameRow = new QHBoxLayout;
  nameRow->addWidget(m_name, 1);
  nameRow->addWidget(m_enabled);
  auto *nameLabel = new QLabel(tr("&Name:"), this);
  nameLabel->setBuddy(m_name);
  top->addRow(nameLabel, nameRow);
  layout->addLayout(top);

  // --- condition ---
  auto *cond = new QGroupBox(tr("Condition"), this);
  auto *cf = new QFormLayout(cond);
  m_condition = new QComboBox(cond);
  m_condition->addItem(tr("Reading below"), Alarm::Below);
  m_condition->addItem(tr("Reading above"), Alarm::Above);
  m_condition->addItem(tr("Reading outside a range"), Alarm::Outside);
  m_condition->addItem(tr("Reading inside a range"), Alarm::Inside);
  m_condition->addItem(tr("Overload (OL)"), Alarm::Overload);
  m_condition->addItem(tr("No readings for a while"), Alarm::NoReadings);
  m_condition->setCurrentIndex(m_condition->findData(alarm.condition));
  cf->addRow(tr("&Condition:"), m_condition);
  auto *validator = new EngNumberValidator(this);
  m_a = new QLineEdit(EngNumberValidator::engText(alarm.a), cond);
  m_a->setValidator(validator);
  m_aLabel = new QLabel(cond);
  cf->addRow(m_aLabel, m_a);
  m_b = new QLineEdit(EngNumberValidator::engText(alarm.b), cond);
  m_b->setValidator(validator);
  m_bLabel = new QLabel(tr("&Upper bound (%1):").arg(unit), cond);
  m_bLabel->setBuddy(m_b);
  cf->addRow(m_bLabel, m_b);
  m_seconds = new QLineEdit(QString::number(alarm.seconds), cond);
  m_seconds->setValidator(new QDoubleValidator(0, 1e6, 1, this));
  m_secondsLabel = new QLabel(cond);
  cf->addRow(m_secondsLabel, m_seconds);
  m_hysteresis = new QLineEdit(EngNumberValidator::engText(alarm.hysteresis), cond);
  m_hysteresis->setValidator(validator);
  m_hystLabel = new QLabel(tr("&Hysteresis (%1):").arg(unit), cond);
  m_hystLabel->setBuddy(m_hysteresis);
  m_hysteresis->setToolTip(tr("The reading must come back by this much before the alarm clears - keeps it from "
                              "flickering when the reading sits at the threshold."));
  cf->addRow(m_hystLabel, m_hysteresis);
  layout->addWidget(cond);

  // --- actions ---
  auto *act = new QGroupBox(tr("Actions"), this);
  auto *af = new QFormLayout(act);
  m_message = new QLineEdit(alarm.message, act);
  m_message->setPlaceholderText(tr("the condition in words"));
  af->addRow(tr("&Message:"), m_message);
  m_color = new QPushButton(act);
  m_color->setFlat(false);
  auto paintColor = [this]
  {
    QPixmap pm(40, 14);
    pm.fill(m_colorValue);
    m_color->setIcon(QIcon(pm));
    m_color->setText(m_colorValue.name());
  };
  paintColor();
  connect(m_color, &QPushButton::clicked, this, [this, paintColor]
  {
    const QColor c = QColorDialog::getColor(m_colorValue, this, tr("Alarm colour"));
    if (c.isValid())
    {
      m_colorValue = c;
      paintColor();
    }
  });
  af->addRow(tr("Co&lour:"), m_color);
  m_banner = new QCheckBox(tr("Banner over the display (with Acknowledge)"), act);
  m_banner->setChecked(alarm.banner);
  m_beep = new QCheckBox(tr("Beep"), act);
  m_beep->setChecked(alarm.beep);
  m_popup = new QCheckBox(tr("Popup window"), act);
  m_popup->setChecked(alarm.popup);
  m_raise = new QCheckBox(tr("Bring the QtDMM window to the front"), act);
  m_raise->setChecked(alarm.raiseWindow);
  m_markGraph = new QCheckBox(tr("Mark in the recorder graph"), act);
  m_markGraph->setChecked(alarm.markGraph);
  m_markTable = new QCheckBox(tr("Mark in the readings table"), act);
  m_markTable->setChecked(alarm.markTable);
  for (QCheckBox *cb : {m_banner, m_beep, m_popup, m_raise, m_markGraph, m_markTable})
    af->addRow(QString(), cb);
  m_recorder = new QComboBox(act);
  m_recorder->addItem(tr("leave alone"), Alarm::RecorderNone);
  m_recorder->addItem(tr("start"), Alarm::RecorderStart);
  m_recorder->addItem(tr("stop"), Alarm::RecorderStop);
  m_recorder->setCurrentIndex(m_recorder->findData(alarm.recorder));
  af->addRow(tr("&Recorder:"), m_recorder);
  m_command = new QLineEdit(alarm.command, act);
  m_command->setPlaceholderText(tr("none"));
  m_command->setToolTip(tr("Program to run when the alarm raises. %v is the value, %u its unit, %n the alarm's name."));
  af->addRow(tr("Run &program:"), m_command);
  layout->addWidget(act);

  auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addWidget(bb);

  connect(m_condition, qOverload<int>(&QComboBox::currentIndexChanged), this, &AlarmDlg::updateFields);
  updateFields();
}

void AlarmDlg::updateFields()
{
  const auto c = Alarm::Condition(m_condition->currentData().toInt());
  const bool range = c == Alarm::Outside || c == Alarm::Inside;
  const bool value = c == Alarm::Below || c == Alarm::Above || range;
  m_aLabel->setText(range ? tr("&Lower bound (%1):").arg(m_unit) : tr("&Threshold (%1):").arg(m_unit));
  m_aLabel->setBuddy(m_a);
  m_a->setEnabled(value);
  m_aLabel->setEnabled(value);
  m_b->setEnabled(range);
  m_bLabel->setEnabled(range);
  m_secondsLabel->setText(c == Alarm::NoReadings ? tr("&Silence (s):") : tr("&For at least (s):"));
  m_secondsLabel->setBuddy(m_seconds);
  m_hysteresis->setEnabled(value);
  m_hystLabel->setEnabled(value);
}

Alarm AlarmDlg::alarm() const
{
  Alarm al;
  al.name = m_name->text().trimmed().isEmpty() ? tr("Alarm") : m_name->text().trimmed();
  al.enabled = m_enabled->isChecked();
  al.condition = Alarm::Condition(m_condition->currentData().toInt());
  al.a = EngNumberValidator::value(m_a->text().isEmpty() ? "0" : m_a->text());
  al.b = EngNumberValidator::value(m_b->text().isEmpty() ? "0" : m_b->text());
  al.seconds = m_seconds->text().toDouble();
  al.hysteresis = EngNumberValidator::value(m_hysteresis->text().isEmpty() ? "0" : m_hysteresis->text());
  al.message = m_message->text().trimmed();
  al.color = m_colorValue;
  al.banner = m_banner->isChecked();
  al.beep = m_beep->isChecked();
  al.popup = m_popup->isChecked();
  al.raiseWindow = m_raise->isChecked();
  al.markGraph = m_markGraph->isChecked();
  al.markTable = m_markTable->isChecked();
  al.recorder = Alarm::RecorderAction(m_recorder->currentData().toInt());
  al.command = m_command->text().trimmed();
  return al;
}
