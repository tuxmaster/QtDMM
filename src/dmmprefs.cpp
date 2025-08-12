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
#include "settings.h"
#include "decoders.h"
#include "porthandler.h"

std::vector<DmmDecoder::DMMInfo> dmm_info = {};

DmmPrefs::DmmPrefs(QWidget *parent) : PrefWidget(parent)
{
  setupUi(this);
  m_portlist = new QStringListModel(this);

  m_label = tr("Multimeter settings");
  m_description = tr("<b>Here you can configure the serial port"
                     " and protocol for your DMM. There is"
                     " also a number of predefined models.</b>");
  m_pixmap = new QPixmap(":/Symbols/dmm.xpm");

  setupComboBoxModel();

  message2->hide();

  m_path = QDir::currentPath();
}


DmmPrefs::~DmmPrefs()
{
  delete m_pixmap;
}

void DmmPrefs::setupComboBoxModel()
{
  ui_model->clear();
  ui_model->insertItem(-1, tr("Manual settings"));

  std::vector<DmmDecoder::DMMInfo> configs = DmmDecoder::getDeviceConfigurations();

  // Sortieren nach dem Namen
  std::sort(configs.begin(), configs.end(), [](const auto& a, const auto& b) {
    return a.name < b.name;
  });

  dmm_info.clear();
  for (const auto& cfg : configs) {
    dmm_info.push_back(cfg);
    ui_model->addItem(cfg.name);
  }

  QCompleter *completer = new QCompleter(ui_model->model(), this);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  completer->setFilterMode(Qt::MatchContains);
  completer->setCompletionMode(QCompleter::PopupCompletion);

  ui_model->setCompleter(completer);

  connect(ui_model->lineEdit(), &QLineEdit::editingFinished, this, [this]()
  {
    bool found = false;
    for (int i = 0; i < ui_model->count(); ++i)
    {
      if (ui_model->itemText(i).compare(ui_model->currentText(), Qt::CaseInsensitive) == 0)
      {
        found = true;
        break;
      }
    }
    if (!found)
      ui_model->setCurrentIndex(-1);
  });
}

QString DmmPrefs::deviceListText() const
{
  QString text;
  QString lastVendor;
  for (const auto& cfg : dmm_info)
  {
    if (lastVendor != cfg.vendor)
    {
      if (!text.isEmpty())
        text += "</td></tr>";
      text += "<tr><td><b>"+cfg.vendor+"</b></td><td>";
      lastVendor = cfg.vendor;
    }
    else
      text += ", ";

    text += cfg.model;
  }

  return text;
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
  baudRate->setCurrentText    (m_cfg->getString("Port settings/baud"));
  bitsCombo->setCurrentText   (m_cfg->getString("Port settings/bits", "7"));
  stopBitsCombo->setCurrentText(m_cfg->getString("Port settings/stop-bits", "1"));
  parityCombo->setCurrentText (m_cfg->getString("Port settings/parity"));
  displayCombo->setCurrentText(m_cfg->getString("DMM/display", "4000"));
  ui_externalSetup->setChecked(m_cfg->getInt("DMM/exterrnal-setup") == 1);

  uirts->setChecked(m_cfg->getBool("DMM/rts", true));
  uidtr->setChecked(m_cfg->getBool("DMM/dtr", false));

  protocolCombo->setCurrentIndex(m_cfg->getInt("DMM/data-format"));
  ui_numValues->setValue(m_cfg->getInt("DMM/number-of-values", 1));

  QString model = m_cfg->getString("DMM/model");

  ui_model->setCurrentIndex(0);

  int id = 0;
  for (const auto& cfg : dmm_info)
  {
    if (model == cfg.name)
    {
      ui_model->setCurrentIndex(id + 1);
      break;
    }
    id++;
  }

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

  protocolCombo->setCurrentIndex(0);
  ui_numValues->setValue(1);
  ui_model->setCurrentIndex(0);

  on_ui_model_activated(ui_model->currentIndex());
}

void DmmPrefs::applySLOT()
{
  m_cfg->setString("Port settings/device", port->currentText());
  m_cfg->setString("Port settings/baud", baudRate->currentText());
  m_cfg->setString("Port settings/bits", bitsCombo->currentText());
  m_cfg->setString("Port settings/stop-bits", stopBitsCombo->currentText());
  m_cfg->setInt("Port settings/parity", parityCombo->currentIndex());

  m_cfg->setString("DMM/display", displayCombo->currentText());
  m_cfg->setBool("DMM/external-setup", ui_externalSetup->isChecked());

  m_cfg->setInt("DMM/data-format", protocolCombo->currentIndex());
  m_cfg->setInt("DMM/number-of-values", ui_numValues->value());
  m_cfg->setString("DMM/model", (ui_model->currentIndex() == 0 ? "Manual" : dmm_info[ui_model->currentIndex() - 1].name));

  m_cfg->setBool("DMM/rts", uirts->isChecked());
  m_cfg->setBool("DMM/dtr", uidtr->isChecked());
}

void DmmPrefs::on_ui_externalSetup_toggled()
{
  if (ui_model->currentIndex() == 0)
  {
    baudRate->setDisabled(ui_externalSetup->isChecked());
    bitsCombo->setDisabled(ui_externalSetup->isChecked());
    stopBitsCombo->setDisabled(ui_externalSetup->isChecked());
    parityCombo->setDisabled(ui_externalSetup->isChecked());
  }
}

void DmmPrefs::on_ui_model_activated(int id)
{
  ui_filename->setDisabled(id != 0);
  ui_save->setDisabled(id != 0);
  ui_load->setDisabled(id != 0);

  baudRate->setDisabled(id != 0);
  ui_protocol->setDisabled(id != 0);
  ui_baudLabel->setDisabled(id != 0);
  ui_bitsLabel->setDisabled(id != 0);
  ui_stopLabel->setDisabled(id != 0);
  ui_displayLabel->setDisabled(id != 0);
  ui_parityLabel->setDisabled(id != 0);
  bitsCombo->setDisabled(id != 0);
  displayCombo->setDisabled(id != 0);
  stopBitsCombo->setDisabled(id != 0);
  parityCombo->setDisabled(id != 0);
  ui_numValues->setDisabled(id != 0);
  ui_externalSetup->setDisabled(id != 0);
  uirts->setDisabled(id != 0);
  uidtr->setDisabled(id != 0);


  if (id != 0)
    message->hide();
  else
    message->show();
  if (ui_model->itemText(id)[0] == '*')
    message2->show();
  else
    message2->hide();

  if (id == 0)
  {
    m_dmmInfo.name = "custom";
    m_dmmInfo.baud = baudRate->currentText().toInt();
    m_dmmInfo.protocol = static_cast<ReadEvent::DataFormat>(protocolCombo->currentIndex()); //!
    m_dmmInfo.bits =  bitsCombo->currentText().toInt();
    m_dmmInfo.stopBits = bitsCombo->currentText().toInt();
    m_dmmInfo.parity = parityCombo->currentIndex();
    m_dmmInfo.display = displayCombo->currentText().toInt();
    m_dmmInfo.numValues = ui_numValues->value();
    m_dmmInfo.externalSetup = ui_externalSetup->isChecked();
    m_dmmInfo.rts = uirts->isChecked();
    m_dmmInfo.dtr = uidtr->isChecked();
  }
  else
  {
    m_dmmInfo = dmm_info[id - 1];

    baudRate->setCurrentText(QString::number(dmm_info[id - 1].baud));
    protocolCombo->setCurrentIndex(dmm_info[id - 1].protocol);
    bitsCombo->setCurrentText(QString::number(dmm_info[id - 1].bits));
    stopBitsCombo->setCurrentText(QString::number(dmm_info[id - 1].stopBits));
    parityCombo->setCurrentIndex(dmm_info[id - 1].parity);
    displayCombo->setCurrentText(QString::number(dmm_info[id - 1].display));
    ui_numValues->setValue(dmm_info[id - 1].numValues);
    ui_externalSetup->setChecked(dmm_info[id - 1].externalSetup);
    uirts->setChecked(dmm_info[id - 1].rts);
    uidtr->setChecked(dmm_info[id - 1].dtr);

    ui_filename->setText("");
  }
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
  return static_cast<ReadEvent::DataFormat>(protocolCombo->currentIndex());
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
    displayCombo->setCurrentText(cfg.value("DMM/display", "4000").toString());
    ui_externalSetup->setChecked(cfg.value("DMM/external-setup", false).toBool());
    protocolCombo->setCurrentIndex(cfg.value("DMM/data-format", 0).toInt());
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
    cfg.setValue("DMM/data-format", protocolCombo->currentIndex());
    cfg.setValue("DMM/number-of-values", ui_numValues->value());

    cfg.setValue("DMM/rts", uirts->isChecked());
    cfg.setValue("DMM/dtr", uidtr->isChecked());
  }
}
