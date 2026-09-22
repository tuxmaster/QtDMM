// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "prefwidget.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QSpinBox;

/// Settings page "SCPI server": lets other programs read the meter over
/// TCP in SCPI (port 5025, lxi-tools style), with optional mDNS
/// announcement. Keys Scpi/enabled, Scpi/port, Scpi/all-interfaces,
/// Scpi/mdns.
class ScpiPrefs : public PrefWidget
{
  Q_OBJECT
public:
  explicit ScpiPrefs(QWidget *parent = nullptr);

  bool enabled() const;
  int port() const;
  bool allInterfaces() const;
  bool mdns() const;

  /// Live status from the running server, shown below the settings.
  void setStatus(const QString &text);

public Q_SLOTS:
  void defaultsSLOT() override;
  void factoryDefaultsSLOT() override;
  void applySLOT() override;

private:
  void updateHint();

  QCheckBox *m_enabled, *m_mdns;
  QSpinBox *m_port;
  QComboBox *m_bind;
  QLabel *m_hint, *m_status;
};
