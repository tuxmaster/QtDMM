//======================================================================
// File:		tipdlg.cpp
// Author:	Matthias Toussaint
// Created:	Sun Nov 11 19:59:19 CET 2001
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
// Copyright (c) 2001 Matthias Toussaint
//======================================================================


#include <QtGui>
#include <QtWidgets>

#include "tipdlg.h"

const char *TipDlg::s_tipText[] = {
	"<font size=+2>Welcome</font><p>QtDMM is a small DMM "
	"(Digital Multi Meter) readout software for Linux/UNIX"
	" and since version 0.8.11 for Mac OSX."
	" If you don't want to see the tips of the day you "
	"can switch them of with"
	" the checkbox below this text. The tips can be switched "
	"on again at any time"
	 " in the preferences dialog.",

	"<font size=+2>QtDMM can't connect?</font><p>"
	"Make shure you have read/write permission for the device"
	" the multimeter is connected to. In doubt call a:<br>"
		" <tt>chmod a+rw /dev/&lt;device&gt;</tt> (as root).<br>"
	"<tt>&lt;device&gt;</tt> will be something like <tt>ttyS0</tt>"
	" or <tt>usb/ttyUSB0</tt>. This depend on your configuration.",

	"<font size=+2>Quick help</font><p>"
	"Click on the context help button in the titlebar of the window."
	" You can click any of the controls or windows of QtDMM to get"
	" context sensitive help. This works in all windows of QtDMM."
	" Most newer windowmangers support this feature. ",

	"<font size=+2>Preferences</font><p>"
	"To configure QtDMM go to the preferences dialog. The preferences"
	" dialog can be reached by <b>File->Configure</b>", // or through the"
//    " context menu of the main window. Right click the main window with"
//    " your mouse to open the context menu",

/*    "<font size=+2>File dialog</font><p>"
	"Perhaps you already noticed that but the size and position of the"
	" file selection dialog is persistent. Once adjusted to your preferences"
	" it reopens with this size and position.", */

	"<font size=+2>Measuring averaged values</font><p>"
		"If you want averaged values just increase the"
		"\"Sample every\" value in the Recorder settings."
		" If you set it to 10sec and your DMM gives a value"
		" approx. every second, you'll get the average of"
		" the ten last measurements in the graph",

	"<font size=+2>Configuration file</font><p>"
	"QtDMM writes a small configuration file (~/.qtdmmrc). It contains"
	" informations about window settings and other preferences."
	" If you want to get rid of QtDMM don't"
	" forget to remove this file too.<br>If you want to stay with it, you may want "
	"to write an email to me explaining what could be improved."
	"<table width=\"100%\"><tr><td>Matthias Toussaint</td><td><font color=blue><u>qtdmm@mtoussaint.de"
	"</td></tr></table></u></font>",
	0,
	0 };

TipDlg::TipDlg(QWidget *parent) :  QDialog( parent),
  m_numTips(0),
  m_curTip(0)
{
  setupUi(this);
  ui_tip->setTextBackgroundColor(palette().background().color());

  connect( ui_closeBut, SIGNAL( clicked() ),this, SLOT( closeSLOT() ));
  connect( ui_previousBut, SIGNAL( clicked() ),this, SLOT( previousSLOT() ));
  connect( ui_nextBut, SIGNAL( clicked() ), this, SLOT( nextSLOT() ));
  connect( ui_showTip, SIGNAL( toggled(bool) ),this, SLOT( showTipsSLOT(bool) ));

  // count tips
  //
  for (int i=0; s_tipText[i]; ++i)
	  ++m_numTips;
  showTipText();
}

bool TipDlg::showTips() const
{
  return !ui_showTip->isChecked();
}

void TipDlg::setShowTipsSLOT( bool on )
{
  ui_showTip->setChecked( !on );
}

void TipDlg::previousSLOT()
{
  --m_curTip;
  if (m_curTip < 0)
	  m_curTip = m_numTips-1;
  showTipText();
}

void TipDlg::nextSLOT()
{
  ++m_curTip;
  m_curTip = m_curTip % m_numTips;
  showTipText();
}

void TipDlg::showTipText()
{
  ui_tip->setText( s_tipText[m_curTip] );
  Q_EMIT currentTip( m_curTip );
}

void TipDlg::setCurrentTip( int num )
{
  m_curTip = num;
  showTipText();
}

void TipDlg::closeSLOT()
{
  nextSLOT();
  hide();
}

void TipDlg::closeEvent( QCloseEvent *ev )
{
  nextSLOT();
  ev->accept();
}

void TipDlg::showTipsSLOT( bool on )
{
  Q_EMIT showTips( !on );
}
