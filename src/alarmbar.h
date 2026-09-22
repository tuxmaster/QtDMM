// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QWidget>
#include <QColor>

class QLabel;
class QPushButton;

/// The alarm banner above the display: the raised alarms' messages on the
/// alarm colour, with Acknowledge. Hidden while nothing is raised.
class AlarmBar : public QWidget
{
  Q_OBJECT
public:
  explicit AlarmBar(QWidget *parent = nullptr);

  /// Shows @p text (one line per alarm) on @p color; empty text hides the bar.
  void setAlarms(const QString &text, const QColor &color);

Q_SIGNALS:
  void acknowledged();

private:
  QLabel *m_text;
  QPushButton *m_ack;
};
