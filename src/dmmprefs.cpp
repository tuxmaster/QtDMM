//======================================================================
// File:		dmmprefs.cpp
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 15:26:51 CEST 2002
//----------------------------------------------------------------------
// This file is part of QtDMM.
//
// QtDMM is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// QtDMM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------
// Copyright (c) 2002 Matthias Toussaint
//======================================================================

#include <QtGui>
#include <QtWidgets>
#include <QSerialPortInfo>
#include <iostream>
#include <algorithm>
#include <vector>

#include "dmmprefs.h"
#include "protocols.h"
#include "calcexpr.h"
#include "sharedstatemanager.h"
#include "siprefix.h"
#include "settings.h"
#include "decoders.h"
#include "porthandler.h"

std::vector<DmmDecoder::DMMInfo> dmm_info = {};

DmmPrefs::DmmPrefs(QWidget *parent) : PrefWidget(parent)
{
  setupUi(this);
  m_portlist = new QStringListModel(this);

  m_label = tr("Multimeter");
  m_description = tr("<b>Here you can configure the serial port"
                     " and protocol for your DMM. There is"
                     " also a number of predefined models.</b>");
  m_pixmap = new QPixmap(":/Symbols/dmm.xpm");

  setupComboBoxModel();

  message2->hide();
  ui_calcGroup->hide();
  ui_virtualGroup->hide();
  connect(ui_calcExpression, &QLineEdit::textChanged, this, &DmmPrefs::updateCalcHint);
  connect(ui_virtualSignal, &QComboBox::currentIndexChanged, this, &DmmPrefs::updateVirtualFormula);
  for (QLineEdit *e : {ui_virtualMin, ui_virtualMax, ui_virtualPeriod, ui_virtualNoise})
    connect(e, &QLineEdit::textChanged, this, &DmmPrefs::updateVirtualFormula);
  connect(ui_virtualFormula, &QLineEdit::textChanged, this, [this]{ if (ui_virtualSignal->currentIndex() == 7) updateVirtualFormula(); });
  m_calcHintTimer.setInterval(1000);   // live values of the input instances
  connect(&m_calcHintTimer, &QTimer::timeout, this, &DmmPrefs::updateCalcHint);

  m_path = QDir::currentPath();
}


DmmPrefs::~DmmPrefs()
{
  delete m_pixmap;
}

// The persisted protocol: the name (ReadEvent::toString) since the protocol
// table exists, the combo index (= enum value) in older configuration files.
ReadEvent::DataFormat DmmPrefs::formatFromSetting(const QVariant &value)
{
  bool isNumber = false;
  const int number = value.toInt(&isNumber);
  if (isNumber)
    return (number >= 0 && number < ReadEvent::EndOfList) ? static_cast<ReadEvent::DataFormat>(number) : ReadEvent::Metex14;
  const ReadEvent::DataFormat df = ReadEvent::fromString(value.toString());
  return df == ReadEvent::Invalid ? ReadEvent::Metex14 : df;
}

void DmmPrefs::selectFormat(ReadEvent::DataFormat df)
{
  const int idx = protocolCombo->findData(int(df));
  protocolCombo->setCurrentIndex(idx >= 0 ? idx : 0);
}

void DmmPrefs::setupComboBoxModel()
{
  // one combo entry per row of the protocol table, the enum value as data
  protocolCombo->clear();
  for (const ProtocolInfo &p : protocols())
    protocolCombo->addItem(QCoreApplication::translate("Protocols", p.description), int(p.id));

  ui_vendor->clear();
  ui_vendor->insertItem(-1, tr("Manual settings"));
  ui_vendor->addItem(tr("All vendors"));

  std::vector<DmmDecoder::DMMInfo> configs = DmmDecoder::getDeviceConfigurations();

  // Sortieren nach dem Namen
  std::sort(configs.begin(), configs.end(), [](const auto& a, const auto& b) {
    return a.name < b.name;
  });

  dmm_info.clear();
  QStringList vendors;
  for (const auto& cfg : configs) {
    dmm_info.push_back(cfg);
    if (!vendors.contains(cfg.vendor))
      vendors.append(cfg.vendor);
  }

  vendors.sort(Qt::CaseInsensitive);
  // the virtual meters of QtDMM itself go first, real vendors alphabetically
  if (vendors.removeOne("QtDMM"))
    vendors.prepend("QtDMM");
  ui_vendor->addItems(vendors);

  // The completer searches the full, unfiltered device list (all vendors),
  // so typing a known model name still finds it directly - on selection the
  // editingFinished handler below switches the vendor combo to match.
  QStringList allNames;
  for (const auto& cfg : dmm_info)
    allNames.append(cfg.name);

  QCompleter *completer = new QCompleter(allNames, this);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  completer->setFilterMode(Qt::MatchContains);
  completer->setCompletionMode(QCompleter::PopupCompletion);

  ui_model->setCompleter(completer);

  connect(ui_model->lineEdit(), &QLineEdit::editingFinished, this, [this]()
  {
    QString text = ui_model->currentText();

    // already the current selection within the current vendor's list?
    for (int i = 0; i < ui_model->count(); ++i)
    {
      if (ui_model->itemText(i).compare(text, Qt::CaseInsensitive) == 0)
      {
        ui_model->setCurrentIndex(i);
        on_ui_model_activated(i);
        return;
      }
    }

    // otherwise search the full device list and switch vendor if needed
    for (size_t i = 0; i < dmm_info.size(); ++i)
    {
      if (dmm_info[i].name.compare(text, Qt::CaseInsensitive) == 0)
      {
        int vendorIdx = ui_vendor->findText(dmm_info[i].vendor);
        if (vendorIdx >= 0)
        {
          ui_vendor->setCurrentIndex(vendorIdx);
          populateModelsForVendor(dmm_info[i].vendor);
          int modelIdx = ui_model->findText(dmm_info[i].name);
          ui_model->setCurrentIndex(modelIdx);
          on_ui_model_activated(modelIdx);
        }
        return;
      }
    }

    ui_model->setCurrentIndex(-1);
  });
}

void DmmPrefs::populateModelsForVendor(const QString &vendor)
{
  ui_model->clear();

  m_currentVendorModels.clear();
  for (const auto& cfg : dmm_info)
    if (cfg.vendor == vendor)
      m_currentVendorModels.push_back(cfg);

  std::sort(m_currentVendorModels.begin(), m_currentVendorModels.end(), [](const auto& a, const auto& b) {
    return a.model < b.model;
  });

  for (const auto& cfg : m_currentVendorModels)
    ui_model->addItem(cfg.name);
}

void DmmPrefs::populateAllModels()
{
  ui_model->clear();

  // dmm_info is already sorted by name (vendor+model) in setupComboBoxModel().
  m_currentVendorModels = dmm_info;

  for (const auto& cfg : m_currentVendorModels)
    ui_model->addItem(cfg.name);
}

void DmmPrefs::on_ui_vendor_activated(int id)
{
  if (id == 0)
  {
    ui_model->clear();
    m_currentVendorModels.clear();
    enterManualMode();
  }
  else if (id == 1)
  {
    populateAllModels();
    if (!m_currentVendorModels.empty())
    {
      ui_model->setCurrentIndex(0);
      on_ui_model_activated(0);
    }
  }
  else
  {
    populateModelsForVendor(ui_vendor->itemText(id));
    if (!m_currentVendorModels.empty())
    {
      ui_model->setCurrentIndex(0);
      on_ui_model_activated(0);
    }
  }
}


void DmmPrefs::defaultsSLOT()
{
  QStringList portlist = PortHandler::availablePorts();
  m_portlist->setStringList(portlist);
  port->setModel(m_portlist);

  // >>> temporary solution to make rfc2217 useable
  QStringList list = m_portlist->stringList();
  for(int i=0; i<10; i++)
  {
    QString dev = m_cfg->getString(QString("Port settings/custom_device%1").arg(i), "");
    if (dev.size()>0)
      list.append(dev);
  }
  m_portlist->setStringList(list);

  port->setCurrentText        (m_cfg->getString("Port settings/device"));
  ui_calcUnit->setText        (m_cfg->getString("DMM/calc-unit", "W"));
  ui_calcExpression->setText  (m_cfg->getString("DMM/calc-expression"));
  ui_virtualSignal->setCurrentIndex(m_cfg->getInt("DMM/virtual-waveform", 2));
  ui_virtualUnit->setText     (m_cfg->getString("DMM/virtual-unit", "V"));
  ui_virtualCoupling->setCurrentText(m_cfg->getString("DMM/virtual-coupling", "DC"));
  ui_virtualMin->setText      (m_cfg->getString("DMM/virtual-min", "0"));
  ui_virtualMax->setText      (m_cfg->getString("DMM/virtual-max", "10"));
  ui_virtualPeriod->setText   (m_cfg->getString("DMM/virtual-period", "20"));
  ui_virtualNoise->setText    (m_cfg->getString("DMM/virtual-noise", "0"));
  if (ui_virtualSignal->currentIndex() == 7)
    ui_virtualFormula->setText(m_cfg->getString("DMM/virtual-formula"));
  baudRate->setCurrentText    (m_cfg->getString("Port settings/baud"));
  bitsCombo->setCurrentText   (m_cfg->getString("Port settings/bits", "7"));
  stopBitsCombo->setCurrentText(m_cfg->getString("Port settings/stop-bits", "1"));
  parityCombo->setCurrentIndex(m_cfg->getInt("Port settings/parity"));   // stored as index by applySLOT()
  selectDisplay(m_cfg->getString("DMM/display", "4000"));
  ui_externalSetup->setChecked(m_cfg->getBool("DMM/external-setup", false));

  uirts->setChecked(m_cfg->getBool("DMM/rts", true));
  uidtr->setChecked(m_cfg->getBool("DMM/dtr", false));

  selectFormat(formatFromSetting(m_cfg->getString("DMM/data-format", "Metex14")));
  ui_numValues->setValue(m_cfg->getInt("DMM/number-of-values", 1));

  QString model = m_cfg->getString("DMM/model");

  ui_vendor->setCurrentIndex(0);
  ui_model->clear();
  m_currentVendorModels.clear();

  for (const auto& cfg : dmm_info)
  {
    if (model == cfg.name)
    {
      int vendorIdx = ui_vendor->findText(cfg.vendor);
      if (vendorIdx >= 0)
      {
        ui_vendor->setCurrentIndex(vendorIdx);
        populateModelsForVendor(cfg.vendor);
        int modelIdx = ui_model->findText(cfg.name);
        ui_model->setCurrentIndex(modelIdx);
      }
      break;
    }
  }

  if (ui_vendor->currentIndex() == 0)
    enterManualMode();
  else
    on_ui_model_activated(ui_model->currentIndex());
}

void DmmPrefs::factoryDefaultsSLOT()
{
  port->setCurrentIndex(0);
  baudRate->setCurrentIndex(0);
  bitsCombo->setCurrentIndex(2);
  stopBitsCombo->setCurrentIndex(1);
  parityCombo->setCurrentIndex(0);
  displayCombo->setCurrentIndex(1);
  ui_externalSetup->setChecked(false);

  selectFormat(ReadEvent::Metex14);
  ui_numValues->setValue(1);
  ui_vendor->setCurrentIndex(0);
  ui_model->clear();
  m_currentVendorModels.clear();

  enterManualMode();
}

void DmmPrefs::applySLOT()
{
  m_cfg->setString("Port settings/device", port->currentText());
  m_cfg->setString("DMM/calc-unit", ui_calcUnit->text().trimmed());
  m_cfg->setString("DMM/calc-expression", ui_calcExpression->text().trimmed());
  m_cfg->setInt("DMM/virtual-waveform", ui_virtualSignal->currentIndex());
  m_cfg->setString("DMM/virtual-unit", ui_virtualUnit->text().trimmed());
  m_cfg->setString("DMM/virtual-coupling", ui_virtualCoupling->currentText());
  m_cfg->setString("DMM/virtual-min", ui_virtualMin->text().trimmed());
  m_cfg->setString("DMM/virtual-max", ui_virtualMax->text().trimmed());
  m_cfg->setString("DMM/virtual-period", ui_virtualPeriod->text().trimmed());
  m_cfg->setString("DMM/virtual-noise", ui_virtualNoise->text().trimmed());
  m_cfg->setString("DMM/virtual-formula", ui_virtualFormula->text().trimmed());
  m_cfg->setString("Port settings/baud", baudRate->currentText());
  m_cfg->setString("Port settings/bits", bitsCombo->currentText());
  m_cfg->setString("Port settings/stop-bits", stopBitsCombo->currentText());
  m_cfg->setInt("Port settings/parity", parityCombo->currentIndex());

  m_cfg->setString("DMM/display", displayCombo->currentText());
  m_cfg->setBool("DMM/external-setup", ui_externalSetup->isChecked());

  m_cfg->setString("DMM/data-format", ReadEvent::toString(format()));
  m_cfg->setInt("DMM/number-of-values", ui_numValues->value());
  const int modelIdx = ui_model->currentIndex();
  const bool haveModel = ui_vendor->currentIndex() != 0
                         && modelIdx >= 0
                         && modelIdx < static_cast<int>(m_currentVendorModels.size());
  m_cfg->setString("DMM/model", haveModel ? m_currentVendorModels[modelIdx].name : "Manual");

  m_cfg->setBool("DMM/rts", uirts->isChecked());
  m_cfg->setBool("DMM/dtr", uidtr->isChecked());
}

void DmmPrefs::on_ui_externalSetup_toggled()
{
  if (ui_vendor->currentIndex() == 0)
  {
    baudRate->setDisabled(ui_externalSetup->isChecked());
    bitsCombo->setDisabled(ui_externalSetup->isChecked());
    stopBitsCombo->setDisabled(ui_externalSetup->isChecked());
    parityCombo->setDisabled(ui_externalSetup->isChecked());
  }
}

bool DmmPrefs::isCalculated() const
{
  return ui_vendor->currentIndex() != 0 && m_dmmInfo.vendor == "QtDMM" && m_dmmInfo.model == "Calculated value";
}

bool DmmPrefs::isVirtual() const
{
  return ui_vendor->currentIndex() != 0 && m_dmmInfo.vendor == "QtDMM" && m_dmmInfo.model == "Virtual meter";
}

void DmmPrefs::updateVirtualFormula()
{
  const int wave = ui_virtualSignal->currentIndex();
  const bool custom = wave == 7;
  ui_virtualFormula->setReadOnly(!custom);
  for (QWidget *w : std::initializer_list<QWidget *>{ui_virtualMin, ui_virtualMax, ui_virtualPeriod, ui_virtualNoise})
    w->setEnabled(!custom);
  ui_virtualPeriod->setEnabled(!custom && wave >= 2);
  if (!custom)
    ui_virtualFormula->setText(CalcExpr::waveformFormula(static_cast<CalcExpr::Waveform>(wave), ui_virtualMin->text(), ui_virtualMax->text(),
                                              ui_virtualPeriod->text(), ui_virtualNoise->text()));
  QString error;
  int pos = -1;
  const bool ok = CalcExpr::parse(ui_virtualFormula->text(), &error, &pos).has_value();
  ui_virtualFormula->setStyleSheet(ok ? QString() : QStringLiteral("QLineEdit { color: #b00; }"));
  ui_virtualFormula->setToolTip(ok ? QString() : tr("Position %1: %2").arg(pos + 1).arg(error));
}

void DmmPrefs::setStateManager(SharedStateManager *state)
{
  m_state = state;
}

// A calculated value has no port and no protocol to set up; the formula
// group takes their place. The port field then carries "calc <unit> <formula>"
// (see device()), so DMM and PortHandler need no special settings keys.
void DmmPrefs::updateCalcMode()
{
  const bool calc = isCalculated();
  const bool virt = isVirtual();
  ButtonGroup11->setVisible(!calc && !virt);
  ui_protocol->setVisible(!calc && !virt);
  ui_calcGroup->setVisible(calc);
  ui_virtualGroup->setVisible(virt);
  if (virt)
    updateVirtualFormula();
  if (calc)
  {
    updateCalcHint();
    m_calcHintTimer.start();
  }
  else
    m_calcHintTimer.stop();
}

void DmmPrefs::updateCalcHint()
{
  QString error;
  int pos = -1;
  const auto expr = CalcExpr::parse(ui_calcExpression->text(), &error, &pos);
  QStringList lines;

  if (!expr)
  {
    if (!ui_calcExpression->text().trimmed().isEmpty())
      lines << QString("<span style=\"color:#b00\">%1</span>").arg(tr("Position %1: %2").arg(pos + 1).arg(error.toHtmlEscaped()));
  }
  else if (m_state)
  {
    const auto readings = m_state->readings();
    for (const QString &var : expr->variables())
    {
      QString id = var;
      for (auto it = readings.constBegin(); it != readings.constEnd(); ++it)
        if (it.key() == var || QString(it.key()).replace('-', '_') == var)
          id = it.key();
      if (readings.contains(id) && readings[id].valid)
        lines << QString("%1 = %2 %3").arg(var.toHtmlEscaped(), SiPrefix::format(readings[id].value), readings[id].unit.toHtmlEscaped());
      else if (readings.contains(id))
        lines << QString("%1 = OL").arg(var.toHtmlEscaped());
      else
        lines << QString("<span style=\"color:#b00\">%1</span>").arg(tr("%1: no such instance running").arg(var.toHtmlEscaped()));
    }
  }

  QStringList others;
  if (m_state)
    for (const QString &id : m_state->instances())
      if (id != m_state->id())
        others << id;
  if (!others.isEmpty())
    lines << tr("Running instances: %1").arg(others.join(", ").toHtmlEscaped());
  else
    lines << tr("No other instance is running.");

  ui_calcHint->setText(lines.join("<br>"));
}

void DmmPrefs::enterManualMode()
{
  ui_filename->setDisabled(false);
  ui_save->setDisabled(false);
  ui_load->setDisabled(false);

  baudRate->setDisabled(false);
  ui_protocol->setDisabled(false);
  ui_baudLabel->setDisabled(false);
  ui_bitsLabel->setDisabled(false);
  ui_stopLabel->setDisabled(false);
  ui_displayLabel->setDisabled(false);
  ui_parityLabel->setDisabled(false);
  bitsCombo->setDisabled(false);
  displayCombo->setDisabled(false);
  stopBitsCombo->setDisabled(false);
  parityCombo->setDisabled(false);
  ui_numValues->setDisabled(false);
  ui_externalSetup->setDisabled(false);
  uirts->setDisabled(false);
  uidtr->setDisabled(false);

  message->show();
  message2->hide();

  m_dmmInfo.name = "custom";
  m_dmmInfo.baud = baudRate->currentText().toInt();
  m_dmmInfo.protocol = format();
  m_dmmInfo.bits =  bitsCombo->currentText().toInt();
  m_dmmInfo.stopBits = stopBitsCombo->currentText().toInt();
  m_dmmInfo.parity = parityCombo->currentIndex();
  m_dmmInfo.display = displayCombo->currentText().toInt();
  m_dmmInfo.numValues = ui_numValues->value();
  m_dmmInfo.externalSetup = ui_externalSetup->isChecked();
  m_dmmInfo.rts = uirts->isChecked();
  m_dmmInfo.dtr = uidtr->isChecked();

  updateCalcMode();
}

void DmmPrefs::on_ui_model_activated(int id)
{
  if (id < 0 || id >= static_cast<int>(m_currentVendorModels.size()))
    return;

  ui_filename->setDisabled(true);
  ui_save->setDisabled(true);
  ui_load->setDisabled(true);

  baudRate->setDisabled(true);
  ui_protocol->setDisabled(true);
  ui_baudLabel->setDisabled(true);
  ui_bitsLabel->setDisabled(true);
  ui_stopLabel->setDisabled(true);
  ui_displayLabel->setDisabled(true);
  ui_parityLabel->setDisabled(true);
  bitsCombo->setDisabled(true);
  displayCombo->setDisabled(true);
  stopBitsCombo->setDisabled(true);
  parityCombo->setDisabled(true);
  ui_numValues->setDisabled(true);
  ui_externalSetup->setDisabled(true);
  uirts->setDisabled(true);
  uidtr->setDisabled(true);

  message->hide();
  if (ui_model->itemText(id).contains('*'))   // "Model *": settings from chip data, unconfirmed
    message2->show();
  else
    message2->hide();

  m_dmmInfo = m_currentVendorModels[id];

  baudRate->setCurrentText(QString::number(m_currentVendorModels[id].baud));
  selectFormat(m_currentVendorModels[id].protocol);
  bitsCombo->setCurrentText(QString::number(m_currentVendorModels[id].bits));
  stopBitsCombo->setCurrentText(QString::number(m_currentVendorModels[id].stopBits));
  parityCombo->setCurrentIndex(m_currentVendorModels[id].parity);
  selectDisplay(QString::number(m_currentVendorModels[id].display));
  ui_numValues->setValue(m_currentVendorModels[id].numValues);
  ui_externalSetup->setChecked(m_currentVendorModels[id].externalSetup);
  uirts->setChecked(m_currentVendorModels[id].rts);
  uidtr->setChecked(m_currentVendorModels[id].dtr);

  ui_filename->setText("");
  updateCalcMode();
}

bool DmmPrefs::rts() const
{
  return uirts->isChecked();
}

bool DmmPrefs::dtr() const
{
  return uidtr->isChecked();
}

QSerialPort::Parity DmmPrefs::parity() const
{
  QSerialPort::Parity rValue;
  switch (parityCombo->currentIndex())
  {
    case 0:
      rValue = QSerialPort::NoParity;
      break;
    case 1:
      rValue = QSerialPort::EvenParity;
      break;
    case 2:
      rValue = QSerialPort::OddParity;
      break;
    default:
      qWarning() << "Wrong parity value. Using None";
      rValue = QSerialPort::NoParity;
      break;
  }
  return rValue;
}

QSerialPort::DataBits DmmPrefs::bits() const
{
  return static_cast<QSerialPort::DataBits>(5 + bitsCombo->currentIndex());
}

QSerialPort::StopBits DmmPrefs::stopBits() const
{
  return static_cast<QSerialPort::StopBits>(1 + stopBitsCombo->currentIndex());
}

int DmmPrefs::speed() const
{
  return baudRate->currentText().toInt();
}

bool DmmPrefs::externalSetup() const
{
  return ui_externalSetup->isChecked();
}

int DmmPrefs::numValues() const
{
  return ui_numValues->value();
}

ReadEvent::DataFormat DmmPrefs::format() const
{
  return static_cast<ReadEvent::DataFormat>(protocolCombo->currentData().toInt());
}

// The counts combo is not editable; a value it does not list (a new model's
// counts, or a hand-edited config) would silently leave the previous entry
// selected - so unknown values are added instead.
void DmmPrefs::selectDisplay(const QString &counts)
{
  int idx = displayCombo->findText(counts);
  if (idx < 0)
  {
    displayCombo->addItem(counts);
    idx = displayCombo->count() - 1;
  }
  displayCombo->setCurrentIndex(idx);
}

int DmmPrefs::display() const
{
  return displayCombo->currentText().toInt();
}

QString DmmPrefs::dmmName() const
{
  return ui_model->currentText();
}

QString DmmPrefs::device() const
{
  if (isCalculated())
    return QString("calc %1 %2").arg(ui_calcUnit->text().trimmed(), ui_calcExpression->text().trimmed());
  if (isVirtual())
    return QString("calc %1/%2 %3").arg(ui_virtualUnit->text().trimmed(), ui_virtualCoupling->currentText(),
                                        ui_virtualFormula->text().trimmed());
  return port->currentText();
}

void DmmPrefs::on_ui_load_clicked()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Load DMM description"), m_path,	tr("DMM description (*.cfg)"));

  if (!filename.isNull())
  {
    QFileInfo info(filename);
    m_path = info.filePath();
    ui_filename->setText(info.fileName());

    QSettings cfg(filename, QSettings::IniFormat);

    port->setCurrentText(cfg.value("Port settings/device", "").toString());
    baudRate->setCurrentText(cfg.value("Port settings/baud", "9600").toString());
    bitsCombo->setCurrentText(cfg.value("Port settings/bits", "7").toString());
    stopBitsCombo->setCurrentText(cfg.value("Port settings/stop-bits", "2").toString());
    parityCombo->setCurrentIndex(cfg.value("Port settings/parity", 0).toInt());
    selectDisplay(cfg.value("DMM/display", "4000").toString());
    ui_externalSetup->setChecked(cfg.value("DMM/external-setup", false).toBool());
    selectFormat(formatFromSetting(cfg.value("DMM/data-format", "Metex14")));
    ui_numValues->setValue(cfg.value("DMM/number-of-values", 1).toInt());
    uirts->setChecked(cfg.value("DMM/rts", true).toBool());
    uidtr->setChecked(cfg.value("DMM/dtr", false).toBool());
  }
}

void DmmPrefs::on_ui_save_clicked()
{
  QString filename = QFileDialog::getSaveFileName(this, tr("Save DMM description"), m_path, tr("DMM description (*.cfg)"));
  if (!filename.isNull())
  {
    QFileInfo info(filename);
    m_path = info.filePath();
    ui_filename->setText(info.fileName());

    QSettings cfg(filename, QSettings::IniFormat);
    cfg.setValue("Port settings/device", port->currentText());
    cfg.setValue("Port settings/baud", baudRate->currentText());
    cfg.setValue("Port settings/bits", bitsCombo->currentText());
    cfg.setValue("Port settings/stop-bits", stopBitsCombo->currentText());
    cfg.setValue("Port settings/parity", parityCombo->currentIndex());

    cfg.setValue("DMM/display", displayCombo->currentText());
    cfg.setValue("DMM/external-setup", ui_externalSetup->isChecked());
    cfg.setValue("DMM/data-format", ReadEvent::toString(format()));
    cfg.setValue("DMM/number-of-values", ui_numValues->value());

    cfg.setValue("DMM/rts", uirts->isChecked());
    cfg.setValue("DMM/dtr", uidtr->isChecked());
  }
}
