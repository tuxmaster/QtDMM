// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QWidget>

#include "readinglog.h"

class QCheckBox;
class QLabel;
class QSpinBox;
class QTableView;
class QToolButton;

/// The readings table panel: a ReadingLog in a table that follows the
/// newest row, a statistics line, and clear / export / copy. Lives in a
/// dock next to the display and the analog meter (Ctrl+4).
class ReadingLogWid : public QWidget
{
  Q_OBJECT
public:
  explicit ReadingLogWid(QWidget *parent = nullptr);

  ReadingLog *log() { return m_log; }
  /// Rows to keep (persisted by MainWin).
  int maxRows() const;
  void setMaxRows(int rows);

public Q_SLOTS:
  void clearSLOT();
  void exportSLOT();
  /// Selected rows (all when nothing is selected) to the clipboard, tab
  /// separated.
  void copySLOT();

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
  void updateStats();
  void followSLOT();

private:
  ReadingLog *m_log;
  QTableView *m_view;
  QToolButton *m_pause;
  QCheckBox *m_follow;
  QSpinBox *m_maxRows;
  QLabel *m_stats;
};
