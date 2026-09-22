// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "prefwidget.h"
#include "alarm.h"

class QListWidget;
class QPushButton;

/// Settings page "Alarms": the list of alarms with add / edit / remove
/// (AlarmDlg edits one). Stored as JSON under Alarms/list.
class AlarmPrefs : public PrefWidget
{
  Q_OBJECT
public:
  explicit AlarmPrefs(QWidget *parent = nullptr);

  QList<Alarm> alarms() const { return m_alarms; }
  /// Base unit of the current reading, shown next to the thresholds.
  void setUnit(const QString &unit);

public Q_SLOTS:
  void defaultsSLOT() override;
  void factoryDefaultsSLOT() override;
  void applySLOT() override;

private:
  void refresh();
  void add();
  void edit();
  void remove();
  void toggle(int row, bool on);

  QList<Alarm> m_alarms;
  QString m_unit = "V";
  QListWidget *m_list;
  QPushButton *m_edit, *m_remove;
};

/// Editor for one alarm: name, condition, duration, hysteresis, actions.
class AlarmDlg : public QDialog
{
  Q_OBJECT
public:
  AlarmDlg(const Alarm &alarm, const QString &unit, QWidget *parent = nullptr);
  Alarm alarm() const;

private:
  void updateFields();

  QString m_unit;
  class QLineEdit *m_name, *m_a, *m_b, *m_seconds, *m_hysteresis, *m_message, *m_command;
  class QComboBox *m_condition, *m_recorder;
  class QCheckBox *m_enabled, *m_banner, *m_beep, *m_popup, *m_raise, *m_markGraph, *m_markTable;
  class QPushButton *m_color;
  class QLabel *m_aLabel, *m_bLabel, *m_secondsLabel, *m_hystLabel;
  QColor m_colorValue;
};
