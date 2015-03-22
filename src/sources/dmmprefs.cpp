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

#include "dmmprefs.h"
#include "simplecfg.h"

#include <dmm.xpm>

// when all needed parameter are found this hardcoded version will
// be replaced by a file
//    name
//    baud (600=0,1200,1800,2400,4800,9600,19200)
//    protocol (0: 14 bytes polling 'D'
//              1: 11 bytes continuous [PeakTech]
//              2: 14 continuous
//              3: 15 continuous
//              4: 11 bin continuous (M9803R)
//              5: 14 bin continuous (VC820)
//              6: IsoTech
//              7: VC940
//              8: QM1537
//              9: 9 binary bytes continuous (22-812)
//    bits
//    stopBits
//    number of values (For DMM's that send several lines at once)
//    parity (0,1,2 - None,Even,Odd)
//    [don't ask for any logic behind the digits, changing would break configs]
//    display digits (0,1,2,3 - 2000, 4000, 20000, 50000, 100000, 200000, 400000,
//                              1000000, 6000, 40000)
//    External device setup 0, 1
//- Added




struct DMMInfo dmm_info[] = {
							  {"Digitek DT4000ZC", 3, 8, 8, 1, 1, 0, 1, 0, 0,1,1,1},
							  {"Digitek DT-9062", 3, 5, 8, 1, 1, 0, 1, 0, 0,1,1,1},
							  {"Digitek INO2513", 3, 5, 8, 1, 1, 0, 1, 0, 0,1,1,1},  // no image

							  {"Digitech QM1350", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},  // no image
							  {"Digitech QM1462", 3, 5, 8, 1, 1, 0, 1, 0, 0,1,1,1},  // no image
							  {"Digitech QM1538", 3, 5, 8, 1, 1, 0, 1, 0, 0,1,1,1},  // no image
							  {"Digitech QM1537", 3, 8, 8, 1, 1, 0, 1, 0, 0,1,1,1},  // no image

							  {"ELV M9803R", 5, 4, 7, 1, 1, 1, 1, 0, 0,1,1,1},       // no image

							  {"Iso-Tech IDM 73", 6, 6, 7, 1, 1, 2, 8, 0, 0,1,1,1},   // no image

							  {"MASTECH MAS-343", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"MASTECH MAS-345", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"MASTECH M9803R", 5, 4, 7, 1, 1, 1, 1, 0, 0,1,1,1},

							  {"McVoice M-345pro", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"McVoice M-980T", 5, 4, 7, 1, 1, 0, 1, 0, 0,1,1,1},

							  {"Metex M-3660D", 1, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"Metex M-3830D", 1, 0, 7, 2, 4, 0, 1, 0, 0,1,1,1},      // no image
							  {"Metex M-3850D", 1, 0, 7, 2, 4, 0, 1, 0, 0,1,1,1},
							  {"Metex M-3850M", 5, 0, 7, 2, 4, 0, 1, 0, 0,1,1,1},
							  {"Metex M-3870D", 1, 0, 7, 1, 1, 0, 1, 0, 0,1,1,1},
							  {"Metex M-4650C", 1, 0, 7, 2, 4, 0, 2, 0, 0,1,1,1},
							  {"Metex ME-11", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"Metex ME-22", 3, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"Metex ME-32", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"Metex ME-42", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"Metex universal system 9160", 1, 0, 7, 2, 4, 0, 1, 0, 0,1,1,1},

							  {"PeakTech 3330", 3, 5, 8, 1, 1, 0, 1, 0, 0,1,1,1},
							  {"PeakTech 3430", 6, 8, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"PeakTech 4010", 5, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"PeakTech 4015A", 5, 0, 7, 2, 4, 0, 4, 0, 0,1,1,1},
							  {"PeakTech 4360", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"PeakTech 4390", 5, 0, 7, 2, 4, 0, 1, 0, 0,1,1,1},
							  {"PeakTech 451", 0, 1, 7, 2, 1, 0, 1, 0, 0,1,1,1},       // no image

							  {"Radioshack 22-805 DMM", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"Radioshack RS22-168A", 1, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},   // no image
							  {"Radioshack 22-812", 4, 9, 8, 1, 1, 0, 1, 0, 0,1,1,1},

							  {"TekPower TP4000ZC", 3, 8, 8, 1, 1, 0, 1, 0, 0,1,1,1},

							  {"Tenma 72-7745", 3, 5, 8, 1, 1, 0, 1, 0, 0,1,1,1},
							  {"Tenma 72-1016", 6, 6, 7, 1, 2, 2, 8, 0, 0,1,1,1},

							  {"Sinometer MAS-343", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},

							  {"Uni-Trend UT30A", 3, 5, 8, 1, 1, 0, 1, 0, 0,1,1,1},
							  {"Uni-Trend UT30E", 3, 5, 8, 1, 1, 0, 1, 0, 0,1,1,1},   // no image

							  {"Voltcraft M-3610D", 1, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},  // no image
							  {"Voltcraft M-3650D", 1, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"Voltcraft M-3860", 5, 0, 7, 2, 4, 0, 2, 0, 0,1,1,1},   // no image
							  {"Voltcraft M-4650CR", 1, 2, 7, 2, 1, 0, 2, 0 , 0,1,1,1}, // no image
							  {"Voltcraft M-4660", 1, 0, 7, 2, 4, 0, 3, 0, 0,1,1,1},
							  {"Voltcraft ME-11", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"Voltcraft ME-22T", 3, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"Voltcraft ME-32", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"Voltcraft VC 670", 4, 2, 7, 1, 1, 0, 3, 0, 0,1,1,1},
							  {"Voltcraft VC 820", 3, 5, 8, 1, 1, 0, 1, 0, 0,1,1,1},
							  {"Voltcraft VC 840", 3, 5, 8, 1, 1, 0, 1, 0, 0,1,1,1},
							  {"Voltcraft VC 920", 3, 7, 7, 1, 1, 2, 9, 0, 0,1,1,1},
							  {"Voltcraft VC 940", 3, 7, 7, 1, 1, 2, 9, 0, 0,1,1,1},

							  {"*Voltcraft ME-42", 0, 0, 7, 2, 1, 0, 1, 0, 0,1,1,1},
							  {"*Voltcraft M-4660A", 5, 0, 7, 2, 4, 0, 3, 0, 0,1,1,1},
							  {"*Voltcraft M-4660M", 5, 0, 7, 2, 4, 0, 3, 0, 0,1,1,1},
							  {"*Voltcraft MXD-4660A", 5, 0, 7, 2, 4, 0, 3, 0, 0,1,1,1},
							  {"*Voltcraft VC 630", 4, 2, 7, 1, 1, 0, 3, 0, 0,1,1,1},
							  {"*Voltcraft VC 650", 4, 2, 7, 1, 1, 0, 3, 0, 0,1,1,1},
							  {"*Voltcraft VC 635", 3, 3, 7, 1, 1, 0, 3, 0, 0,1,1,1},
							  {"*Voltcraft VC 655", 3, 3, 7, 1, 1, 0, 3, 0, 0,1,1,1},

							  {"",0,0,0,0,0,0,0,0,0,0,0,0} // End Of List
							};

DmmPrefs::DmmPrefs( QWidget *parent, const char *name ) : UIDmmPrefs( parent, name )
{
  m_label = tr( "Multimeter settings" );
  m_description = tr( "<b>Here you can configure the serial port"
					  " and protocol for your DMM. There is"
					  " also a number of predefined models.</b>" );
  m_pixmap = new QPixmap( (const char **)dmm_xpm );

  ui_model->clear();
  ui_model->insertItem( "Manual settings" );

  int id = 0;
  while (*dmm_info[id].name)
	ui_model->insertItem( dmm_info[id++].name );

  message2->hide();

  connect( ui_model, SIGNAL( activated( int ) ),
		   this, SLOT( modelSLOT( int )));
  connect( ui_externalSetup, SIGNAL( toggled( bool ) ),
		   this, SLOT( externalSetupSLOT()));
  connect( ui_load, SIGNAL( clicked() ),
		   this, SLOT( loadSLOT()));
  connect( ui_save, SIGNAL( clicked() ),
		   this, SLOT( saveSLOT()));

  m_path = QDir::currentDirPath();

#ifdef Q_WS_MACX
  connect( ui_scanPorts, SIGNAL( clicked() ),
		   this, SLOT( scanDevicesSLOT() ));

  portNumber->hide();
  scanDevicesSLOT();
#else
  ui_scanPorts->hide();
#endif
}


void DmmPrefs::scanDevicesSLOT()
{
#ifdef Q_WS_MACX
  port->clear();
  QDir dir( "/dev" );
  QStringList files = dir.entryList( "cu.*", QDir::System );
  for (unsigned i=0; i<files.count(); ++i)
	port->insertItem( QString( "/dev/" ) + files[i] );
  files = dir.entryList( "tty.*", QDir::System );
  for (unsigned i=0; i<files.count(); ++i)
	port->insertItem( QString( "/dev/" ) + files[i] );
#endif
}

QString DmmPrefs::deviceListText() const
{
  QString text;

  QString name;

  int id = 0;
  while (*dmm_info[id].name)
  {
	QStringList token = QStringList::split( " ", QString( dmm_info[id].name ) );

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

	  for (unsigned i=1; i<token.count(); ++i)
	  {
		if (i!=1)
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
  port->setCurrentItem( m_cfg->getInt( "Port settings", "device", 0 ) );
  portNumber->setValue( m_cfg->getInt( "Port settings", "device-number", 0 ) );
  baudRate->setCurrentItem( m_cfg->getInt( "Port settings", "baud", 0 ) );
  bitsCombo->setCurrentItem( m_cfg->getInt( "Port settings", "bits", 7 )-5 );
  stopBitsCombo->setCurrentItem( m_cfg->getInt( "Port settings", "stop-bits", 2 )-1);
  parityCombo->setCurrentItem( m_cfg->getInt( "Port settings", "parity", 0 ) );
  displayCombo->setCurrentItem( m_cfg->getInt( "DMM", "display", 1 ) );
  ui_externalSetup->setChecked( m_cfg->getInt( "DMM", "exterrnal-setup", 0 ) == 1 );

  protocolCombo->setCurrentItem( m_cfg->getInt( "DMM", "data-format", 0 ));
  ui_numValues->setValue( m_cfg->getInt( "DMM", "number-of-values", 1 ));

  QString model = m_cfg->getString( "DMM", "model", "" );

  ui_model->setCurrentItem( 0 );
  int id=0;
  while (*dmm_info[id].name)
  {
	if (model == dmm_info[id].name)
	{
	  ui_model->setCurrentItem( id+1 );
	  break;
	}
	id++;
  }

  modelSLOT( ui_model->currentItem() );
}

void DmmPrefs::factoryDefaultsSLOT()
{
  port->setCurrentItem( 0 );
  baudRate->setCurrentItem( 0 );
  bitsCombo->setCurrentItem( 2 );
  stopBitsCombo->setCurrentItem( 1 );
  parityCombo->setCurrentItem( 0 );
  displayCombo->setCurrentItem( 1 );
  ui_externalSetup->setChecked( false );

  protocolCombo->setCurrentItem( 0 );
  ui_numValues->setValue( 1 );
  ui_model->setCurrentItem( 0 );

  modelSLOT( ui_model->currentItem() );
}

void DmmPrefs::applySLOT()
{
  m_cfg->setInt( "Port settings", "device", port->currentItem() );
  m_cfg->setInt( "Port settings", "device-number", portNumber->value() );
  m_cfg->setInt( "Port settings", "baud", baudRate->currentItem() );
  m_cfg->setInt( "Port settings", "bits", bitsCombo->currentItem()+5 );
  m_cfg->setInt( "Port settings", "stop-bits", stopBitsCombo->currentItem()+1 );
  m_cfg->setInt( "Port settings", "parity", parityCombo->currentItem() );

  m_cfg->setInt( "DMM", "display", displayCombo->currentItem() );
  m_cfg->setBool( "DMM", "external-setup", ui_externalSetup->isChecked() );

  m_cfg->setInt( "DMM", "data-format", protocolCombo->currentItem() );
  m_cfg->setInt( "DMM", "number-of-values", ui_numValues->value() );
  m_cfg->setString( "DMM", "model", (ui_model->currentItem() == 0 ? "Manual" : dmm_info[ui_model->currentItem()-1].name ));
}

void DmmPrefs::externalSetupSLOT()
{
  if (ui_model->currentItem() == 0)
  {
	baudRate->setDisabled( ui_externalSetup->isChecked() );
	bitsCombo->setDisabled( ui_externalSetup->isChecked() );
	stopBitsCombo->setDisabled( ui_externalSetup->isChecked() );
	parityCombo->setDisabled( ui_externalSetup->isChecked() );
  }
}

void DmmPrefs::modelSLOT( int id )
{
  ui_filename->setDisabled( id != 0 );
  ui_save->setDisabled( id != 0 );
  ui_load->setDisabled( id != 0 );

  baudRate->setDisabled( id != 0 );
  ui_protocol->setDisabled( id != 0 );
  ui_baudLabel->setDisabled( id != 0 );
  ui_bitsLabel->setDisabled( id != 0 );
  ui_stopLabel->setDisabled( id != 0 );
  ui_displayLabel->setDisabled( id != 0 );
  ui_parityLabel->setDisabled( id != 0 );
  bitsCombo->setDisabled( id != 0 );
  displayCombo->setDisabled( id != 0 );
  stopBitsCombo->setDisabled( id != 0 );
  parityCombo->setDisabled( id != 0 );
  ui_numValues->setDisabled( id != 0 );
  ui_externalSetup->setDisabled( id != 0 );
  uirts->setDisabled( id != 0 );
  uicts->setDisabled( id != 0 );
  uidsr->setDisabled( id != 0 );
  uidtr->setDisabled( id != 0 );

  if (id != 0)
	  message->hide();
  else
	  message->show();
  if (ui_model->text(id)[0] == '*')
	message2->show();
  else
	message2->hide();

  if (id > 0)
  {
	baudRate->setCurrentItem( dmm_info[id-1].baud );
	protocolCombo->setCurrentItem( dmm_info[id-1].protocol );
	bitsCombo->setCurrentItem( dmm_info[id-1].bits-5 );
	stopBitsCombo->setCurrentItem( dmm_info[id-1].stopBits-1 );
	parityCombo->setCurrentItem( dmm_info[id-1].parity );
	displayCombo->setCurrentItem( dmm_info[id-1].display );
	ui_numValues->setValue( dmm_info[id-1].numValues );
	ui_externalSetup->setChecked( dmm_info[id-1].externalSetup );
	uirts->setChecked( dmm_info[id-1].rts );
	uicts->setChecked( dmm_info[id-1].cts );
	uidsr->setChecked( dmm_info[id-1].dsr );
	uidtr->setChecked( dmm_info[id-1].dtr );

	ui_filename->setText( "" );
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

int DmmPrefs::parity() const
{
  return parityCombo->currentItem();
}

int DmmPrefs::bits() const
{
  return 5+bitsCombo->currentItem();
}

int DmmPrefs::stopBits() const
{
  return 1+stopBitsCombo->currentItem();
}

int DmmPrefs::speed() const
{
  switch (baudRate->currentItem())
  {
	  case 0:
		return 600;
	  case 1:
		return 1200;
	  case 2:
		return 1800;
	  case 3:
		return 2400;
	  case 4:
		return 4800;
	  case 5:
		return 9600;
	  case 6:
		return 19200;
  }
  return 600;
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
  return (ReadEvent::DataFormat)protocolCombo->currentItem();
}

int DmmPrefs::display() const
{
  return displayCombo->currentItem();
}

QString DmmPrefs::dmmName() const
{
  return ui_model->currentText();
}

QString DmmPrefs::device() const
{
#ifdef Q_WS_MACX
  return port->currentText();
#else
  QString txt;
  txt.sprintf( "%s%d", port->currentText().latin1(), portNumber->value() );
  return txt;
#endif
}

void DmmPrefs::loadSLOT()
{
  QString filename = QFileDialog::getOpenFileName( m_path,
									"DMM description (*.ini)",
									this,
									"open file dialog",
									tr("Load DMM description") );

  if (!filename.isNull())
  {
	QFileInfo info( filename );
	m_path = info.filePath();
	ui_filename->setText( info.fileName() );

	SimpleCfg cfg( filename );
	cfg.load();

	port->setCurrentItem( cfg.getInt( "Port settings", "device", 0 ) );
	portNumber->setValue( cfg.getInt( "Port settings", "device-number", 0 ) );
	baudRate->setCurrentItem( cfg.getInt( "Port settings", "baud", 0 ) );
	bitsCombo->setCurrentItem( cfg.getInt( "Port settings", "bits", 7 )-5 );
	stopBitsCombo->setCurrentItem( cfg.getInt( "Port settings", "stop-bits", 2 )-1);
	parityCombo->setCurrentItem( cfg.getInt( "Port settings", "parity", 0 ) );
	displayCombo->setCurrentItem( cfg.getInt( "DMM", "display", 1 ) );
	ui_externalSetup->setChecked( cfg.getBool( "DMM", "external-setup", false ) );
	protocolCombo->setCurrentItem( cfg.getInt( "DMM", "data-format", 0 ));
	ui_numValues->setValue( cfg.getInt( "DMM", "number-of-values", 1 ));
	uirts->setChecked( cfg.getBool( "DMM", "rts", true ));
	uicts->setChecked( cfg.getBool( "DMM", "cts", false ));
	uidsr->setChecked( cfg.getBool( "DMM", "dsr", false ));
	uidtr->setChecked( cfg.getBool( "DMM", "dtr", false ));
  }
}

void DmmPrefs::saveSLOT()
{
  QString filename = QFileDialog::getSaveFileName( m_path,
									"DMM description (*.ini)",
									this,
									"save file dialog",
									tr("Save DMM description") );

  if (!filename.isNull())
  {
	QFileInfo info( filename );
	m_path = info.filePath();
	ui_filename->setText( info.fileName() );

	SimpleCfg cfg( filename );
	cfg.setComment(
	  "#####################################################################\n"
	  "# This file was automagically created by QtDMM a simple DMM readout #\n"
	  "# software. QtDMM is copyright  by Matthias Toussaint. Don't change #\n"
	  "# this file unless you know what you are doing.                     #\n"
	  "#                                                                   #\n"
	  "# Contact: qtdmm@mtoussaint.de                                      #\n"
	  "#####################################################################\n\n" );

	cfg.setInt( "Port settings", "device", port->currentItem() );
	cfg.setInt( "Port settings", "device-number", portNumber->value() );
	cfg.setInt( "Port settings", "baud", baudRate->currentItem() );
	cfg.setInt( "Port settings", "bits", bitsCombo->currentItem()+5 );
	cfg.setInt( "Port settings", "stop-bits", stopBitsCombo->currentItem()+1 );
	cfg.setInt( "Port settings", "parity", parityCombo->currentItem() );

	cfg.setInt( "DMM", "display", displayCombo->currentItem() );
	cfg.setBool( "DMM", "external-setup", ui_externalSetup->isChecked() );
	cfg.setInt( "DMM", "data-format", protocolCombo->currentItem() );
	cfg.setInt( "DMM", "number-of-values", ui_numValues->value() );

	cfg.setBool( "DMM", "rts", uirts->isChecked() );
	cfg.setBool( "DMM", "cts", uicts->isChecked() );
	cfg.setBool( "DMM", "dsr", uidsr->isChecked() );
	cfg.setBool( "DMM", "dtr", uidtr->isChecked() );

	cfg.save();
  }
}
