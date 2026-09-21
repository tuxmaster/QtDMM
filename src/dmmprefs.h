//======================================================================
// File:		dmmprefs.h
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 15:08:57 CEST 2002
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

#pragma once

#include <QtSerialPort>
#include <vector>

#include "ui_uidmmprefs.h"
#include "readevent.h"
#include "dmmdecoder.h"
#include <QTimer>

class SharedStateManager;

/// Settings page "Multimeter": vendor/model choice (from the registered
/// DmmDecoder::DMMInfo entries), port and the serial parameters, which
/// become editable in manual mode. Descriptions can be saved to and loaded
/// from .cfg files.
class DmmPrefs : public PrefWidget, private Ui::UIDmmPrefs
{
  Q_OBJECT
public:
  DmmPrefs(QWidget *parent = Q_NULLPTR);
  ~DmmPrefs();

  /// The DMMInfo of the chosen model, or the manual settings.
  DmmDecoder::DMMInfo dmmInfo() { return m_dmmInfo; };
  QSerialPort::Parity parity() const;
  QSerialPort::DataBits bits() const;
  QSerialPort::StopBits stopBits() const;
  int            speed() const;
  int            numValues() const;
  bool           externalSetup() const;
  bool           rts() const;
  bool           dtr() const;
  ReadEvent::DataFormat format() const;
  /// Display counts (4000, 6000, ...).
  int            display() const;
  /// Selects the display counts, adding the entry when the combo lacks it.
  void           selectDisplay(const QString &counts);
  /// Selects the protocol combo entry for @p df.
  void           selectFormat(ReadEvent::DataFormat df);
  /// Protocol from a settings value: name, or the enum number of old files.
  static ReadEvent::DataFormat formatFromSetting(const QVariant &value);
  QString        dmmName() const;
  /// The port entry as typed or chosen, e.g. "/dev/ttyUSB0" or "HID 0x1a86:0xe008 ...";
  /// for a calculated value "calc <unit> <formula>".
  QString        device() const;
  /// True while the model "QtDMM / Calculated value" is chosen.
  bool           isCalculated() const;
  /// True while the model "QtDMM / Virtual meter" is chosen.
  bool           isVirtual() const;
  /// Victron over Bluetooth LE: the Bluetooth group replaces the port.
  bool           isBluetooth() const;
  /// Source of the other instances' readings, shown as a hint below the formula.
  void           setStateManager(SharedStateManager *state);

public Q_SLOTS:
  virtual void   defaultsSLOT() Q_DECL_OVERRIDE;
  virtual void   factoryDefaultsSLOT() Q_DECL_OVERRIDE;
  virtual void   applySLOT() Q_DECL_OVERRIDE;

protected Q_SLOTS:
  void           on_ui_vendor_activated(int);
  void           on_ui_model_activated(int);
  /// Load a DMM description (.cfg).
  void           on_ui_load_clicked();
  /// Save the current settings as a DMM description (.cfg).
  void           on_ui_save_clicked();
  void           on_ui_externalSetup_toggled();
  /// Re-parses the formula and refreshes the hint (variables, live values, errors).
  void           updateCalcHint();
  /// Rebuilds the virtual meter's formula from the waveform fields.
  void           updateVirtualFormula();
  /// Validates address and key, explains what is missing.
  void           updateBleHint();
  /// Five-second scan for Victron devices, fills the device combo.
  void           on_ui_bleScan_clicked();

protected:
  QString        m_path;
  DmmDecoder::DMMInfo m_dmmInfo;
  QStringListModel *m_portlist;
  std::vector<DmmDecoder::DMMInfo> m_currentVendorModels;

  void setupComboBoxModel();
  void populateModelsForVendor(const QString &vendor);
  void populateAllModels();
  void enterManualMode();
  /// Shows the formula group instead of the port/protocol groups, or back.
  void updateCalcMode();

  SharedStateManager *m_state = Q_NULLPTR;
  QTimer m_calcHintTimer;
};
