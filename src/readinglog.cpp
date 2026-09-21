// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "readinglog.h"

#include <QCoreApplication>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

#include "siprefix.h"

ReadingLog::ReadingLog(QObject *parent) : QAbstractTableModel(parent)
{
}

int ReadingLog::rowCount(const QModelIndex &parent) const
{
  return parent.isValid() ? 0 : m_entries.size();
}

int ReadingLog::columnCount(const QModelIndex &parent) const
{
  return parent.isValid() ? 0 : ColumnCount;
}

QString ReadingLog::formatTime(const QDateTime &when)
{
  return when.toString("yyyy-MM-dd HH:mm:ss.zzz");
}

QString ReadingLog::modeText(const QString &special)
{
  // the decoders' codes (see DmmDecoder::DmmResponse::special)
  if (special == "ACDC") return tr("AC+DC");
  if (special == "DI")   return tr("Diode");
  if (special == "BUZ")  return tr("Continuity");
  if (special == "OH")   return tr("Resistance");
  if (special == "CA")   return tr("Capacitance");
  if (special == "FR")   return tr("Frequency");
  if (special == "TE")   return tr("Temperature");
  return special;   // AC, DC and anything new
}

QVariant ReadingLog::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || index.row() >= m_entries.size())
    return QVariant();
  const Entry &e = m_entries[index.row()];

  if (role == DvalRole)
    return e.dval;
  if (role == Qt::TextAlignmentRole)
    return int(index.column() == Value ? Qt::AlignRight | Qt::AlignVCenter : Qt::AlignLeft | Qt::AlignVCenter);
  if (role != Qt::DisplayRole)
    return QVariant();

  switch (index.column())
  {
    case Time:  return formatTime(e.when);
    case Value: return e.val.trimmed();
    case Unit:  return e.unit;
    case Mode:  return e.id > 0 ? tr("2nd") + (e.special.isEmpty() ? QString() : " " + modeText(e.special)) : modeText(e.special);
    case Range: return e.range;
    case Hold:  return e.hold ? tr("HOLD") : QString();
  }
  return QVariant();
}

QVariant ReadingLog::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Vertical)
    return section + 1;
  switch (section)
  {
    case Time:  return tr("Time");
    case Value: return tr("Value");
    case Unit:  return tr("Unit");
    case Mode:  return tr("Mode");
    case Range: return tr("Range");
    case Hold:  return tr("Hold");
  }
  return QVariant();
}

void ReadingLog::append(const Entry &entry)
{
  if (m_entries.size() >= m_maxRows)
  {
    const int excess = m_entries.size() - m_maxRows + 1;
    beginRemoveRows(QModelIndex(), 0, excess - 1);
    m_entries.remove(0, excess);
    endRemoveRows();
  }
  beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size());
  m_entries.append(entry);
  endInsertRows();
}

void ReadingLog::clear()
{
  if (m_entries.isEmpty())
    return;
  beginResetModel();
  m_entries.clear();
  endResetModel();
}

void ReadingLog::setMaxRows(int rows)
{
  m_maxRows = qMax(1, rows);
  if (m_entries.size() > m_maxRows)
  {
    const int excess = m_entries.size() - m_maxRows;
    beginRemoveRows(QModelIndex(), 0, excess - 1);
    m_entries.remove(0, excess);
    endRemoveRows();
  }
}

ReadingLog::Stats ReadingLog::stats() const
{
  static const QRegularExpression letters("[A-Za-z]");
  Stats s;
  s.count = m_entries.size();
  double sum = 0;
  for (const Entry &e : m_entries)
  {
    if (e.id != 0 || e.val.contains(letters))   // secondary values, OL
      continue;
    if (s.numeric == 0)
      s.min = s.max = e.dval;
    s.min = qMin(s.min, e.dval);
    s.max = qMax(s.max, e.dval);
    sum += e.dval;
    ++s.numeric;
  }
  if (s.numeric)
    s.mean = sum / s.numeric;
  if (!m_entries.isEmpty())
    s.unit = SiPrefix::split(m_entries.last().unit).baseUnit;
  return s;
}

QString ReadingLog::toText(const QList<int> &rows) const
{
  QStringList lines;
  QStringList header;
  for (int c = 0; c < ColumnCount; ++c)
    header << headerData(c, Qt::Horizontal, Qt::DisplayRole).toString();
  lines << header.join('\t');

  QList<int> which = rows;
  if (which.isEmpty())
    for (int r = 0; r < m_entries.size(); ++r)
      which << r;
  std::sort(which.begin(), which.end());
  for (int r : which)
  {
    if (r < 0 || r >= m_entries.size())
      continue;
    QStringList cells;
    for (int c = 0; c < ColumnCount; ++c)
      cells << data(index(r, c), Qt::DisplayRole).toString();
    lines << cells.join('\t');
  }
  return lines.join('\n') + '\n';
}

bool ReadingLog::write(const QString &path, QString *error) const
{
  auto fail = [&](const QString &text)
  {
    if (error)
      *error = text;
    return false;
  };
  if (m_entries.isEmpty())
    return fail(tr("Nothing to export."));
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return fail(tr("Cannot open file."));
  QTextStream ts(&file);
  ts << "timestamp;value;unit;mode;range;hold\n";
  for (const Entry &e : m_entries)
    ts << QString("%1;%2;%3;%4;%5;%6\n")
            .arg(e.when.toString("yyyy-MM-ddTHH:mm:ss,zzz"), e.val.trimmed(), e.unit,
                 e.id > 0 ? "2nd " + modeText(e.special) : modeText(e.special), e.range, e.hold ? "1" : "0");
  return true;
}
