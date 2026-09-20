#pragma once

#include <QIODevice>
#include <QTimer>
#include <QMap>

#include "dmmdecoder.h"
#include "calcexpr.h"

class SharedStateManager;

/// A virtual meter whose reading is a formula over the other instances'
/// readings, e.g. power from a voltage and a current instance ("u * i").
///
/// The inputs come from SharedStateManager::readings(); every poll interval
/// the formula is evaluated and the result is offered as a sigrok-style
/// text line ("DC 6.02 W AUTO", padded to 30 bytes like SigrokDevice does),
/// so the ASCII decoder (ReadEvent::Sigrok) and everything after it work
/// unchanged. Instance ids are the variables; a hyphen in an id can be
/// written as an underscore in the formula. The variable t is reserved: the
/// seconds since the device was opened, which together with sin(), rand()
/// and friends makes a signal generator ("QtDMM / Virtual meter" is this
/// device with a formula built from a few fields). While an input is missing,
/// invalid or older than three seconds the line carries "inf", which the
/// display shows as OL, and status() explains which input is the problem.
class CalcDevice : public QIODevice
{
  Q_OBJECT
public:
  /// @param info   the "QtDMM / Calculated value" entry
  /// @param device "<unit>[/<coupling>] <formula>", e.g. "W u * i" or
  ///               "V/AC 12 + sin(2*pi*t/5)"; coupling defaults to DC
  /// @param state  where the other instances' readings come from
  /// @param parent parent object
  explicit CalcDevice(const DmmDecoder::DMMInfo &info, const QString &device,
                      SharedStateManager *state, QObject *parent = nullptr);

  /// Nothing to enumerate; adds nothing and returns false.
  static bool availablePorts(QStringList &portlist);

  /// Parses the formula; fails (see errorString()) when it does not parse
  /// or the unit is missing.
  bool open(OpenMode mode) override;
  void close() override;
  qint64 bytesAvailable() const override;
  bool isSequential() const override { return true; }

  /// The unit given with the device string.
  QString unit() const { return m_unit; }
  /// "DC" or "AC" as given with the device string.
  QString coupling() const { return m_special; }
  /// The formula as parsed, empty before open().
  QString formula() const { return m_expr ? m_expr->text() : QString(); }

  /// The result as the meter would show it: scaled into [1, 1000) with an
  /// SI prefix and rounded to the digits of the model's display count
  /// (40000 -> 5 digits: 6.6242, 123.45). Exposed for tests.
  static QString formatValue(double value, int counts, QString *prefix);

  /// Builds the decoder line for the current inputs; exposed for tests.
  /// @param now    reference time for the staleness check (ms since epoch)
  ///               and for t (seconds since open())
  /// @param status receives the message status() would carry, empty when fine
  QByteArray currentLine(qint64 now, QString *status = nullptr) const;

  /// Readings older than this are treated as missing.
  static constexpr qint64 kStaleMs = 3000;
  /// Poll interval; matches SharedStateManager's.
  static constexpr int kIntervalMs = 250;

Q_SIGNALS:
  /// Human-readable state for the status bar: which input is missing or
  /// stale, or empty when everything is fine. Emitted after the first
  /// poll and then on change only.
  void status(const QString &message);

protected:
  qint64 readData(char *data, qint64 maxSize) override;
  qint64 writeData(const char *data, qint64 len) override;

private Q_SLOTS:
  void tick();

private:
  static constexpr int kLineLength = 30;

  DmmDecoder::DMMInfo m_dmmInfo;
  SharedStateManager *m_state;
  QString m_unit;
  QString m_special = QStringLiteral("DC");
  qint64 m_openedMs = 0;        ///< when open() was called, for t
  QString m_source;             ///< formula text from the device string
  std::optional<CalcExpr> m_expr;
  QTimer m_timer;
  QByteArray m_pending;         ///< the line not yet read by ReaderThread
  QString m_lastStatus;
  bool m_statusSent = false;    ///< the first tick always reports
};
