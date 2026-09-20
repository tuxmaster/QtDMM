#include "calcexpr.h"
#include "siprefix.h"

#include <QCoreApplication>
#include <cmath>

// Recursive descent over the source string. Precedence, loosest first:
//   expr   := term (('+' | '-') term)*
//   term   := unary (('*' | '/') unary)*
//   unary  := '-' unary | power
//   power  := atom ('^' unary)?
//   atom   := number | name | name '(' expr (',' expr)* ')' | '(' expr ')'
struct CalcExpr::Parser
{
  const QString &s;
  int pos = 0;
  CalcExpr &e;
  QString error;
  int errorPos = -1;

  Parser(const QString &src, CalcExpr &expr) : s(src), e(expr) {}

  bool failed() const { return errorPos >= 0; }
  int fail(const QString &msg, int at)
  {
    if (!failed())
    {
      error = msg;
      errorPos = at;
    }
    return -1;
  }

  void skipSpace()
  {
    while (pos < s.size() && s[pos].isSpace())
      pos++;
  }
  bool peek(QChar c)
  {
    skipSpace();
    return pos < s.size() && s[pos] == c;
  }
  bool accept(QChar c)
  {
    if (!peek(c))
      return false;
    pos++;
    return true;
  }

  int add(Node n)
  {
    e.m_nodes.push_back(n);
    return static_cast<int>(e.m_nodes.size()) - 1;
  }

  int expr()
  {
    int left = term();
    while (!failed())
    {
      if (accept('+'))      left = add({Op::Add, 0, -1, left, term()});
      else if (accept('-')) left = add({Op::Sub, 0, -1, left, term()});
      else break;
    }
    return left;
  }

  int term()
  {
    int left = unary();
    while (!failed())
    {
      if (accept('*'))      left = add({Op::Mul, 0, -1, left, unary()});
      else if (accept('/')) left = add({Op::Div, 0, -1, left, unary()});
      else break;
    }
    return left;
  }

  int unary()
  {
    if (accept('-'))
      return add({Op::Neg, 0, -1, unary(), -1});
    accept('+');
    return power();
  }

  int power()
  {
    int base = atom();
    if (!failed() && accept('^'))
      return add({Op::Pow, 0, -1, base, unary()});
    return base;
  }

  static bool isNameStart(QChar c) { return c.isLetter() || c == '_'; }
  static bool isNameChar(QChar c) { return c.isLetterOrNumber() || c == '_'; }

  int atom()
  {
    skipSpace();
    if (pos >= s.size())
      return fail(QCoreApplication::translate("CalcExpr", "Unexpected end of expression"), pos);

    const QChar c = s[pos];
    if (c == '(')
    {
      pos++;
      int inner = expr();
      if (!failed() && !accept(')'))
        return fail(QCoreApplication::translate("CalcExpr", "Missing ')'"), pos);
      return inner;
    }
    if (c.isDigit() || c == '.')
      return number();
    if (isNameStart(c))
      return name();
    return fail(QCoreApplication::translate("CalcExpr", "Unexpected '%1'").arg(c), pos);
  }

  int number()
  {
    const int start = pos;
    while (pos < s.size() && (s[pos].isDigit() || s[pos] == '.'))
      pos++;
    // exponent: "1e-3", but only when digits follow, so "2e" is a number
    // followed by a bad suffix rather than a broken exponent
    if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E'))
    {
      int p = pos + 1;
      if (p < s.size() && (s[p] == '+' || s[p] == '-'))
        p++;
      if (p < s.size() && s[p].isDigit())
      {
        pos = p;
        while (pos < s.size() && s[pos].isDigit())
          pos++;
      }
    }
    bool ok = false;
    double v = s.mid(start, pos - start).toDouble(&ok);
    if (!ok)
      return fail(QCoreApplication::translate("CalcExpr", "Bad number '%1'").arg(s.mid(start, pos - start)), start);

    // SI suffix directly after the digits: 1.5k, 22u, 4n7 is not supported
    if (pos < s.size() && isNameStart(s[pos]))
    {
      const QString suffix = s.mid(pos, 1);
      const double factor = SiPrefix::factor(suffix);
      if (factor == 1.0 && suffix != "u")
        return fail(QCoreApplication::translate("CalcExpr", "Unknown unit prefix '%1'").arg(suffix), pos);
      if (pos + 1 < s.size() && isNameChar(s[pos + 1]))
        return fail(QCoreApplication::translate("CalcExpr", "Unknown unit prefix '%1'").arg(s.mid(pos, 2)), pos);
      v *= factor;
      pos++;
    }
    return add({Op::Num, v, -1, -1, -1});
  }

  int name()
  {
    const int start = pos;
    while (pos < s.size() && isNameChar(s[pos]))
      pos++;
    const QString id = s.mid(start, pos - start);

    if (!accept('('))
    {
      int idx = e.m_variables.indexOf(id);
      if (idx < 0)
      {
        e.m_variables << id;
        idx = e.m_variables.size() - 1;
      }
      return add({Op::Var, 0, idx, -1, -1});
    }

    // function call
    Op op;
    int arity = 1;
    if (id == "sqrt")       op = Op::Sqrt;
    else if (id == "abs")   op = Op::Abs;
    else if (id == "log10") op = Op::Log10;
    else if (id == "min")   { op = Op::Min; arity = 2; }
    else if (id == "max")   { op = Op::Max; arity = 2; }
    else
      return fail(QCoreApplication::translate("CalcExpr", "Unknown function '%1'").arg(id), start);

    int a = expr();
    int b = -1;
    if (!failed() && arity == 2)
    {
      if (!accept(','))
        return fail(QCoreApplication::translate("CalcExpr", "'%1' needs two arguments").arg(id), pos);
      b = expr();
    }
    if (!failed() && !accept(')'))
      return fail(QCoreApplication::translate("CalcExpr", "Missing ')'"), pos);
    return add({op, 0, -1, a, b});
  }
};

std::optional<CalcExpr> CalcExpr::parse(const QString &text, QString *error, int *errorPos)
{
  CalcExpr e;
  e.m_text = text.trimmed();
  Parser p(e.m_text, e);

  if (e.m_text.isEmpty())
    p.fail(QCoreApplication::translate("CalcExpr", "Empty expression"), 0);
  else
  {
    e.m_root = p.expr();
    p.skipSpace();
    if (!p.failed() && p.pos < e.m_text.size())
      p.fail(QCoreApplication::translate("CalcExpr", "Unexpected '%1'").arg(e.m_text[p.pos]), p.pos);
  }

  if (p.failed())
  {
    if (error)
      *error = p.error;
    if (errorPos)
      *errorPos = p.errorPos;
    return std::nullopt;
  }
  return e;
}

std::optional<double> CalcExpr::eval(const QMap<QString, double> &values) const
{
  if (m_root < 0)
    return std::nullopt;
  bool ok = true;
  const double v = evalNode(m_root, values, ok);
  if (!ok || !std::isfinite(v))
    return std::nullopt;
  return v;
}

double CalcExpr::evalNode(int idx, const QMap<QString, double> &values, bool &ok) const
{
  const Node &n = m_nodes[idx];
  auto A = [&]{ return evalNode(n.a, values, ok); };
  auto B = [&]{ return evalNode(n.b, values, ok); };
  switch (n.op)
  {
    case Op::Num:   return n.value;
    case Op::Var:
    {
      auto it = values.constFind(m_variables[n.name]);
      if (it == values.constEnd())
      {
        ok = false;
        return 0;
      }
      return it.value();
    }
    case Op::Neg:   return -A();
    case Op::Add:   return A() + B();
    case Op::Sub:   return A() - B();
    case Op::Mul:   return A() * B();
    case Op::Div:   return A() / B();
    case Op::Pow:   return std::pow(A(), B());
    case Op::Sqrt:  return std::sqrt(A());
    case Op::Abs:   return std::fabs(A());
    case Op::Log10: return std::log10(A());
    case Op::Min:   return std::min(A(), B());
    case Op::Max:   return std::max(A(), B());
  }
  ok = false;
  return 0;
}
