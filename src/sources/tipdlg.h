//======================================================================
// File:		tipdlg.h
// Author:	Matthias Toussaint
// Created:	Sun Nov 11 19:20:02 CET 2001
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

#ifndef TIPDLG_HH
#define TIPDLG_HH

#include <uitipdlg.h>
//Added by qt3to4:
#include <QCloseEvent>

class TipDlg : public UITipDlg
{
  Q_OBJECT
public:
  TipDlg( QWidget *parent=0, const char *name=0 );
  virtual ~TipDlg();
  
  bool showTips() const;
  
  static const char *s_tipText[];
  
  void setCurrentTip( int c );
  int currentTip() const { return m_curTip; }

signals:
	void showTips( bool );
	void currentTip( int );
  
public slots:
  void setShowTipsSLOT( bool );
	
protected:
  int m_numTips;
  int m_curTip;
  QString m_formatTip;
	
  void showTipText();
  void closeEvent( QCloseEvent * );
  
protected slots:
  void nextSLOT();
  void previousSLOT();
  void closeSLOT();
  void showTipsSLOT( bool );
  
};

#endif // TIPDLG_HH
