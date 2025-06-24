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

#include "dmmprefs.h"
#include "settings.h"

/**
  when all needed parameter are found this hardcoded version will
  be replaced by a file
     name
     baud 0: 600
          1: 1200
          2: 1800
          3: 2400
          4: 4800
          5: 9600
          6: 19200
     protocol 0: 14 bytes polling 'D'
              1: 11 bytes continuous [PeakTech]
              2: 14 continuous
              3: 15 continuous
              4: 11 bin continuous (M9803R)
              5: 14 bin continuous (VC820)
              6: IsoTech
              7: VC940
              8: QM1537
              9: 9 binary bytes continuous (22-812)
             10: 23 bytes continuous (VC870)
             11: 22 bytes continuous (DO3122)
             12: 4 bytes half-ASCII (CyrustekES51922)
     bits
     stopBits
     number of values (For DMM's that send several lines at once)
     parity (0,1,2 - None,Even,Odd)
     [don't ask for any logic behind the digits, changing would break configs]
     display digits 0: 2000
                    1: 4000
                    2: 20000
                    3: 50000
                    4: 100000
                    5: 200000
                    6: 400000
                    7: 1000000
                    8: 6000
                    9: 40000
                   10: 22000
     External device setup 0, 1
     rts 0, 1
     cts 0, 1
     dsr 0, 1
     dtr 0, 1
**/

struct DMMInfo dmm_info[] =
{
  {"Digitek DT4000ZC", 3, 8, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Digitek DT-9062", 3, 5, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Digitek INO2513", 3, 5, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},

  {"Digitech QM1350", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Digitech QM1462", 3, 5, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Digitech QM1538", 3, 5, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Digitech QM1537", 3, 8, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},

  {"Duratool DO3122", 5, 11, 8, 1, 1, 0, 1, 0, 0, 0, 0, 0},

  {"ELV M9803R", 5, 4, 7, 1, 1, 1, 1, 0, 0, 1, 1, 1},

  {"HoldPeak HP-90EPC", 3, 5, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},

  {"Iso-Tech IDM 73", 6, 6, 7, 1, 1, 2, 8, 0, 0, 1, 1, 1},

  {"MASTECH MAS-343", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"MASTECH MAS-345", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"MASTECH M9803R", 5, 4, 7, 1, 1, 1, 1, 0, 0, 1, 1, 1},

  {"McVoice M-345pro", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"McVoice M-980T", 5, 4, 7, 1, 1, 0, 1, 0, 0, 1, 1, 1},

  {"Metex M-3660D", 1, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Metex M-3830D", 1, 0, 7, 2, 4, 0, 1, 0, 0, 1, 1, 1},
  {"Metex M-3840D", 1, 0, 7, 2, 4, 0, 1, 0, 0, 1, 1, 1},
  {"Metex M-3850D", 1, 0, 7, 2, 4, 0, 1, 0, 0, 1, 1, 1},
  {"Metex M-3850M", 5, 0, 7, 2, 4, 0, 1, 0, 0, 1, 1, 1},
  {"Metex M-3870D", 1, 0, 7, 1, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Metex M-4650C", 1, 0, 7, 2, 4, 0, 2, 0, 0, 1, 1, 1},
  {"Metex ME-11", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Metex ME-22", 3, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Metex ME-32", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Metex ME-42", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Metex universal system 9160", 1, 0, 7, 2, 4, 0, 1, 0, 0, 1, 1, 1},

  {"PeakTech 3330", 3, 5, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},
  {"PeakTech 3430", 6, 8, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"PeakTech 4010", 5, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"PeakTech 4015A", 5, 0, 7, 2, 4, 0, 4, 0, 0, 1, 1, 1},
  {"PeakTech 4360", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"PeakTech 4390", 5, 0, 7, 2, 4, 0, 1, 0, 0, 1, 1, 1},
  {"PeakTech 451", 0, 1, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},

  {"Radioshack 22-805 DMM", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Radioshack RS22-168A", 1, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Radioshack 22-812", 4, 9, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},

  {"TekPower TP4000ZC", 3, 8, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},

  {"Tenma 72-7732", 3, 7, 7, 1, 1, 2, 9, 0, 0,1,1,1},
  {"Tenma 72-7745", 3, 5, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Tenma 72-1016", 6, 6, 7, 1, 2, 2, 8, 0, 0, 1, 1, 1},

  {"Sinometer MAS-343", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},

  {"Uni-Trend UT30A", 3, 5, 8, 1, 1, 0, 1, 0, 0,1,1,1},
  {"Uni-Trend UT30E", 3, 5, 8, 1, 1, 0, 1, 0, 0,1,1,1},
  {"Uni-Trend UT61B", 3, 8, 8, 1, 1, 0, 1, 0, 0,1,1,1},
  {"Uni-Trend UT61C", 3, 8, 8, 1, 1, 0, 8, 0, 0,0,0,1},
  {"Uni-Trend UT61D", 3, 8, 8, 1, 1, 0, 8, 0, 0,0,0,1},
  {"Uni-Trend UT61E", 6, 12, 7, 1, 1, 2, 10, 0, 0,0,0,1},
  {"Uni-Trend UT71BCDE", 3, 7, 7, 1, 1, 2, 5, 0, 0,1,1,1},
  {"Uni-Trend UT803", 6, 13, 7, 1, 1, 2, 8, 0, 0,0,0,1},

  {"Vichy VC99", 3, 8, 8, 1, 1, 0, 8, 0, 0, 0, 0, 1},

  {"Voltcraft M-3610D", 1, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Voltcraft M-3650D", 1, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Voltcraft M-3860", 5, 0, 7, 2, 4, 0, 2, 0, 0, 1, 1, 1},
  {"Voltcraft M-4650CR", 1, 2, 7, 2, 1, 0, 2, 0, 0, 1, 1, 1},
  {"Voltcraft M-4660", 1, 0, 7, 2, 4, 0, 3, 0, 0, 1, 1, 1},
  {"Voltcraft ME-11", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Voltcraft ME-22T", 3, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Voltcraft ME-32", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Voltcraft VC 670", 4, 2, 7, 1, 1, 0, 3, 0, 0, 1, 1, 1},
  {"Voltcraft VC 820", 3, 5, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Voltcraft VC 840", 3, 5, 8, 1, 1, 0, 1, 0, 0, 1, 1, 1},
  {"Voltcraft VC 870", 5, 10, 8, 1, 2, 0, 9, 0, 0, 1, 1, 1},
  {"Voltcraft VC 920", 3, 7, 7, 1, 1, 2, 9, 0, 0, 1, 1, 1},
  {"Voltcraft VC 940", 3, 7, 7, 1, 1, 2, 9, 0, 0, 1, 1, 1},
  {"Voltcraft VC 960", 3, 7, 7, 1, 1, 2, 9, 0, 0,1,1,1},

  {"*Voltcraft ME-42", 0, 0, 7, 2, 1, 0, 1, 0, 0, 1, 1, 1},
  {"*Voltcraft M-4660A", 5, 0, 7, 2, 4, 0, 3, 0, 0, 1, 1, 1},
  {"*Voltcraft M-4660M", 5, 0, 7, 2, 4, 0, 3, 0, 0, 1, 1, 1},
  {"*Voltcraft MXD-4660A", 5, 0, 7, 2, 4, 0, 3, 0, 0, 1, 1, 1},
  {"*Voltcraft VC 630", 4, 2, 7, 1, 1, 0, 3, 0, 0, 1, 1, 1},
  {"*Voltcraft VC 650", 4, 2, 7, 1, 1, 0, 3, 0, 0, 1, 1, 1},
  {"*Voltcraft VC 635", 3, 3, 7, 1, 1, 0, 3, 0, 0, 1, 1, 1},
  {"*Voltcraft VC 655", 3, 3, 7, 1, 1, 0, 3, 0, 0, 1, 1, 1},

  {"", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} // End Of List
};

DmmPrefs::DmmPrefs(QWidget *parent) : PrefWidget(parent)
{
  setupUi(this);
  m_portlist = new QStringListModel(this);
  QStringList portlist;
  for (auto port : QSerialPortInfo::availablePorts())
  {
#ifdef Q_OS_WIN
    portlist << port.portName();
#else
    portlist << port.systemLocation();
#endif
    qDebug() << port.portName() << "--" << port.manufacturer() << "--" << port.description() << "--" << port.systemLocation();
  }
  m_portlist->setStringList(portlist);
  port->setModel(m_portlist);
  m_label = (tr("Multimeter settings"));
  m_description = tr("<b>Here you can configure the serial port"
                     " and protocol for your DMM. There is"
                     " also a number of predefined models.</b>");
  m_pixmap = new QPixmap(":/Symbols/dmm.xpm");

  ui_model->clear();
  ui_model->insertItem(-1, tr("Manual settings"));

  int id = 0;
  while (*dmm_info[id].name)
    ui_model->addItem(dmm_info[id++].name);

  message2->hide();

  m_path = QDir::currentPath();

}
DmmPrefs::~DmmPrefs()
{
  delete m_pixmap;
}

QString DmmPrefs::deviceListText() const
{
  QString text;

  QString name;

  int id = 0;
  while (*dmm_info[id].name)
  {
    QStringList token = QString(dmm_info[id].name).split(" ");

    if (token[0][0] != '*')
    {
      if (name != token[0])
      {
        if (!text.isEmpty())
          text += "</td></tr>";
        text += "<tr><td><b>";
        text += token[0];
        text += "</b></td><td>";
        name = token[0];
      }
      else
        text += ", ";

      for (unsigned i = 1; i < static_cast<unsigned>(token.count()); ++i)
      {
        if (i != 1)
          text += " ";
        text += token[i];
      }
    }

    ++id;
  }

  return text;
}

void DmmPrefs::defaultsSLOT()
{
  port->setCurrentText(m_cfg->getString("Port settings/device"));
  baudRate->setCurrentIndex(m_cfg->getInt("Port settings/baud"));
  bitsCombo->setCurrentIndex(m_cfg->getInt("Port settings/bits", 7) - 5);
  stopBitsCombo->setCurrentIndex(m_cfg->getInt("Port settings/stop-bits", 2) - 1);
  int pValue = m_cfg->getInt("Port settings/parity");
  if (pValue < 0 || pValue>2)
  {
    qWarning() << "Wrong parity value. Using None";
    pValue = 0;
  }
  parityCombo->setCurrentIndex(pValue);
  displayCombo->setCurrentIndex(m_cfg->getInt("DMM/display", 1));
  ui_externalSetup->setChecked(m_cfg->getInt("DMM/exterrnal-setup") == 1);

  uirts->setChecked(m_cfg->getBool("DMM/rts", true));
  uicts->setChecked(m_cfg->getBool("DMM/cts", false));
  uidsr->setChecked(m_cfg->getBool("DMM/dsr", false));
  uidtr->setChecked(m_cfg->getBool("DMM/dtr", false));

  protocolCombo->setCurrentIndex(m_cfg->getInt("DMM/data-format"));
  ui_numValues->setValue(m_cfg->getInt("DMM/number-of-values", 1));

  QString model = m_cfg->getString("DMM/model");

  ui_model->setCurrentIndex(0);
  int id = 0;
  while (*dmm_info[id].name)
  {
    if (model == dmm_info[id].name)
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
  m_cfg->setInt("Port settings/baud", baudRate->currentIndex());
  m_cfg->setInt("Port settings/bits", bitsCombo->currentIndex() + 5);
  m_cfg->setInt("Port settings/stop-bits", stopBitsCombo->currentIndex() + 1);
  m_cfg->setInt("Port settings/parity", parityCombo->currentIndex());

  m_cfg->setInt("DMM/display", displayCombo->currentIndex());
  m_cfg->setBool("DMM/external-setup", ui_externalSetup->isChecked());

  m_cfg->setInt("DMM/data-format", protocolCombo->currentIndex());
  m_cfg->setInt("DMM/number-of-values", ui_numValues->value());
  m_cfg->setString("DMM/model", (ui_model->currentIndex() == 0 ? "Manual" : dmm_info[ui_model->currentIndex() - 1].name));

  m_cfg->setBool("DMM/rts", uirts->isChecked());
  m_cfg->setBool("DMM/cts", uicts->isChecked());
  m_cfg->setBool("DMM/dsr", uidsr->isChecked());
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
  uicts->setDisabled(id != 0);
  uidsr->setDisabled(id != 0);
  uidtr->setDisabled(id != 0);

  if (id != 0)
    message->hide();
  else
    message->show();
  if (ui_model->itemText(id)[0] == '*')
    message2->show();
  else
    message2->hide();

  if (id > 0)
  {
    baudRate->setCurrentIndex(dmm_info[id - 1].baud);
    protocolCombo->setCurrentIndex(dmm_info[id - 1].protocol);
    bitsCombo->setCurrentIndex(dmm_info[id - 1].bits - 5);
    stopBitsCombo->setCurrentIndex(dmm_info[id - 1].stopBits - 1);
    parityCombo->setCurrentIndex(dmm_info[id - 1].parity);
    displayCombo->setCurrentIndex(dmm_info[id - 1].display);
    ui_numValues->setValue(dmm_info[id - 1].numValues);
    ui_externalSetup->setChecked(dmm_info[id - 1].externalSetup);
    uirts->setChecked(dmm_info[id - 1].rts);
    uicts->setChecked(dmm_info[id - 1].cts);
    uidsr->setChecked(dmm_info[id - 1].dsr);
    uidtr->setChecked(dmm_info[id - 1].dtr);

    ui_filename->setText("");
  }
}

bool DmmPrefs::rts() const
{
  return uirts->isChecked();
}

bool DmmPrefs::cts() const
{
  return uicts->isChecked();
}

bool DmmPrefs::dsr() const
{
  return uidsr->isChecked();
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
  bool ok = false;
  int baud = baudRate->currentText().split(" ").first().toInt(&ok);

  return ok ? baud : 600;
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
  bool ok = false;
  int value_counts = displayCombo->currentText().toInt(&ok);

  return (ok ? value_counts : 0);
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
  QString filename = QFileDialog::getOpenFileName(this, tr("Load DMM description"), m_path,	tr("DMM description (*.ini)"));

  if (!filename.isNull())
  {
    QFileInfo info(filename);
    m_path = info.filePath();
    ui_filename->setText(info.fileName());

    QSettings cfg(filename, QSettings::IniFormat);

    port->setCurrentIndex(cfg.value("Port settings/device", 0).toInt());
    baudRate->setCurrentIndex(cfg.value("Port settings/baud", 0).toInt());
    bitsCombo->setCurrentIndex(cfg.value("Port settings/bits", 7).toInt() - 5);
    stopBitsCombo->setCurrentIndex(cfg.value("Port settings/stop-bits", 2).toInt() - 1);
    parityCombo->setCurrentIndex(cfg.value("Port settings/parity", 0).toInt());
    displayCombo->setCurrentIndex(cfg.value("DMM/display", 1).toInt());
    ui_externalSetup->setChecked(cfg.value("DMM/external-setup", false).toBool());
    protocolCombo->setCurrentIndex(cfg.value("DMM/data-format", 0).toInt());
    ui_numValues->setValue(cfg.value("DMM/number-of-values", 1).toInt());
    uirts->setChecked(cfg.value("DMM/rts", true).toBool());
    uicts->setChecked(cfg.value("DMM/cts", false).toBool());
    uidsr->setChecked(cfg.value("DMM/dsr", false).toBool());
    uidtr->setChecked(cfg.value("DMM/dtr", false).toBool());
  }
}

void DmmPrefs::on_ui_save_clicked()
{
  QString filename = QFileDialog::getSaveFileName(this, tr("Save DMM description"), m_path, tr("DMM description (*.ini)"));
  if (!filename.isNull())
  {
    QFileInfo info(filename);
    m_path = info.filePath();
    ui_filename->setText(info.fileName());

    QSettings cfg(filename, QSettings::IniFormat);
    cfg.setValue("Port settings/device", port->currentIndex());
    cfg.setValue("Port settings/baud", baudRate->currentIndex());
    cfg.setValue("Port settings/bits", bitsCombo->currentIndex() + 5);
    cfg.setValue("Port settings/stop-bits", stopBitsCombo->currentIndex() + 1);
    cfg.setValue("Port settings/parity", parityCombo->currentIndex());

    cfg.setValue("DMM/display", displayCombo->currentIndex());
    cfg.setValue("DMM/external-setup", ui_externalSetup->isChecked());
    cfg.setValue("DMM/data-format", protocolCombo->currentIndex());
    cfg.setValue("DMM/number-of-values", ui_numValues->value());

    cfg.setValue("DMM/rts", uirts->isChecked());
    cfg.setValue("DMM/cts", uicts->isChecked());
    cfg.setValue("DMM/dsr", uidsr->isChecked());
    cfg.setValue("DMM/dtr", uidtr->isChecked());
  }
}
