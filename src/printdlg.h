//======================================================================
// File:		printdlg.h
// Author:	Matthias Toussaint
// Created:	Wed Apr 11 16:53:46 CEST 2001
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

#pragma once

#include <QtGui>
#include "ui_uiprintdlg.h"


class QPrinter;
namespace qtdmm
{
  class PrintDlg : public QDialog, private Ui::UIPrintDlg
  {
    Q_OBJECT
  public:
    PrintDlg(QWidget *parent = Q_NULLPTR);
    void		setPrinter(QPrinter *prt);
    QString	title() const
    {
      return printTitle->text();
    }
    QString	comment() const
    {
      return printComment->toPlainText();
    }

  protected:
    QPrinter	*m_printer;

  protected Q_SLOTS:
    void		on_configBut_clicked();
    void		on_helpBut_clicked();
    void		createPrinterString();

  };
}
