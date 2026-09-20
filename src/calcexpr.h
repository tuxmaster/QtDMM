#pragma once

#include <QString>
#include <QStringList>
#include <QMap>
#include <optional>
#include <vector>

/// A small arithmetic expression: what a calculated instance evaluates from
/// the readings of the other instances, e.g. "u * i" or "sqrt(a^2 + b^2)".
///
/// Grammar: the operators + - * / ^ (^ binds tightest and is right
/// associative, unary minus binds weaker than ^ so -2^2 is -4),
/// parentheses, numbers in decimal or exponent form with an optional SI
/// suffix ("1.5k", "22u", "4.7n"), variables (identifiers, in QtDMM the ids
/// of the running instances) and the functions sqrt, abs, log10, min(a,b)
/// and max(a,b). Names are case-sensitive.
///
/// parse() reports the first error with its position; eval() yields nothing
/// when a variable is missing or the result is not finite (1/0, sqrt(-1)).
class CalcExpr
{
public:
  /// Parses @p text. On failure returns nothing and sets @p error (and
  /// @p errorPos, the 0-based offset into @p text) when given.
  static std::optional<CalcExpr> parse(const QString &text, QString *error = nullptr, int *errorPos = nullptr);

  /// The distinct variable names used, in order of first appearance.
  QStringList variables() const { return m_variables; }
  /// Evaluates with the given variable values.
  std::optional<double> eval(const QMap<QString, double> &values) const;
  /// The expression as parsed (trimmed source text).
  QString text() const { return m_text; }

private:
  enum class Op { Num, Var, Neg, Add, Sub, Mul, Div, Pow, Sqrt, Abs, Log10, Min, Max };
  struct Node
  {
    Op op;
    double value = 0;   ///< Num
    int name = -1;      ///< Var: index into m_variables
    int a = -1, b = -1; ///< operands
  };
  struct Parser;

  std::vector<Node> m_nodes;
  QStringList m_variables;
  QString m_text;
  int m_root = -1;

  double evalNode(int idx, const QMap<QString, double> &values, bool &ok) const;
};
