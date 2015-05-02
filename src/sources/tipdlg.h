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

#include <QtCore>

#include "ui_uitipdlg.h"

class TipDlg : public QDialog,private Ui::UITipDlg
{
  Q_OBJECT
	public:
	  TipDlg( QWidget *parent=Q_NULLPTR );


	  bool						showTips() const;

	  static const QStringList	s_tipText;

	  void						setCurrentTip( int c );
	  int						currentTip() const { return m_curTip; }

	Q_SIGNALS:
	  void						showTips( bool );
	  void						currentTip( int );

	public Q_SLOTS:
	  void						setShowTipsSLOT( bool );

	protected:
	  int						m_numTips;
	  int						m_curTip;
	  QString					m_formatTip;

	  void						showTipText();
	  void						closeEvent( QCloseEvent * )Q_DECL_OVERRIDE;

	protected Q_SLOTS:
	  void						on_ui_nextBut_clicked();
	  void						on_ui_previousBut_clicked();
	  void						on_ui_closeBut_clicked();
	  void						on_ui_showTip_toggled( bool );

};

#endif // TIPDLG_HH
