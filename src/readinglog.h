// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QAbstractTableModel>
#include <QColor>
#include <QDateTime>
#include <QVector>

/// Every reading the meter sent, one row each, as a table model: the raw
/// protocol of a session next to the recorder's time-gridded graph. Keeps
/// the newest maxRows() entries (a ring), knows min/max/mean of what it
/// holds and writes itself as CSV. No widgets here, so it can be tested;
/// ReadingLogWid shows it.
class ReadingLog : public QAbstractTableModel
{
  Q_OBJECT
public:
  /// One reading as DMM::value() delivered it.
  struct Entry
  {
    QDateTime when;
    double dval = 0;      ///< SI base units, for statistics and sorting
    QString val;          ///< as displayed ("1.234", "OL")
    QString unit;         ///< with prefix ("mV")
    QString special;      ///< "DC", "AC", "DI", ...
    QString range;        ///< "AUTO", "MANU" or empty
    bool hold = false;
    int id = 0;           ///< 0 = main display, 1+ = secondary values
    QColor alarmColor;    ///< set when an alarm raised on this reading
    QString alarmName;
  };

  enum Column { Time, Value, Unit, Mode, Range, Hold, ColumnCount };
  /// Qt::UserRole on any cell gives the row's dval as double.
  static constexpr int DvalRole = Qt::UserRole;

  /// Statistics over the numeric (non-overload) main readings in the log.
  struct Stats
  {
    int count = 0;        ///< rows in the log, overloads included
    int numeric = 0;      ///< rows min/max/mean are computed from
    double min = 0, max = 0, mean = 0;
    QString unit;         ///< unit of the newest main reading, prefix stripped
  };

  explicit ReadingLog(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  /// Appends a row; the oldest one goes when the log is full. Dropped
  /// while paused.
  void append(const Entry &entry);
  /// Paused: readings pass by without being logged (the pause button).
  bool isPaused() const { return m_paused; }
  void setPaused(bool paused) { m_paused = paused; }
  void clear();
  /// An alarm raised on the newest reading: the row gets the colour.
  void markLast(const QColor &color, const QString &name);
  const Entry &entry(int row) const { return m_entries[row]; }

  int maxRows() const { return m_maxRows; }
  /// Rows to keep, at least 1; trims the log when it shrinks.
  void setMaxRows(int rows);

  Stats stats() const;

  /// Text of the rows @p rows (all when empty) - tab separated with a
  /// header, as a spreadsheet pastes it.
  QString toText(const QList<int> &rows = {}) const;
  /// CSV export: `timestamp;value;unit;mode;range;hold`, ISO timestamps
  /// with milliseconds, the value as displayed (with the SI prefix on the
  /// unit). Returns false with @p error on failure or when empty.
  bool write(const QString &path, QString *error = nullptr) const;

  /// "2026-09-21 14:03:05.250", the Time column.
  static QString formatTime(const QDateTime &when);
  /// The Mode column: the decoders' codes as words ("OH" -> "Resistance").
  static QString modeText(const QString &special);

private:
  QVector<Entry> m_entries;
  int m_maxRows = 10000;
  bool m_paused = false;
};
