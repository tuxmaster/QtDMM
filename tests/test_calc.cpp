// Tests for CalcExpr, the formula evaluator of calculated instances.
#include <QCoreApplication>
#include <QDebug>
#include <cmath>

#include "calcexpr.h"

static int failed = 0;

static void check(bool cond, const QString &what)
{
  if (!cond)
  {
    qWarning() << "FAILED:" << what;
    failed++;
  }
}

static void expectValue(const QString &text, double expected, const QMap<QString, double> &vars = {})
{
  QString err;
  auto e = CalcExpr::parse(text, &err);
  if (!e)
  {
    check(false, QString("'%1' should parse, got: %2").arg(text, err));
    return;
  }
  auto v = e->eval(vars);
  check(v.has_value(), QString("'%1' should evaluate").arg(text));
  if (v)
    check(std::fabs(*v - expected) <= 1e-9 * std::max(1.0, std::fabs(expected)),
          QString("'%1' = %2, expected %3").arg(text).arg(*v, 0, 'g', 12).arg(expected, 0, 'g', 12));
}

static void expectParseError(const QString &text, int pos = -1)
{
  QString err;
  int at = -1;
  auto e = CalcExpr::parse(text, &err, &at);
  check(!e, QString("'%1' should not parse").arg(text));
  if (!e)
  {
    check(!err.isEmpty(), QString("'%1': error text set").arg(text));
    if (pos >= 0)
      check(at == pos, QString("'%1': error at %2, expected %3 (%4)").arg(text).arg(at).arg(pos).arg(err));
  }
}

static void expectEvalFails(const QString &text, const QMap<QString, double> &vars = {})
{
  auto e = CalcExpr::parse(text);
  check(e.has_value(), QString("'%1' should parse").arg(text));
  if (e)
    check(!e->eval(vars).has_value(), QString("'%1' should not evaluate").arg(text));
}

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  // --- 1. numbers and precedence ---
  expectValue("42", 42);
  expectValue("1.5", 1.5);
  expectValue(".5", 0.5);
  expectValue("1e3", 1000);
  expectValue("2.5e-3", 0.0025);
  expectValue("2+3*4", 14);
  expectValue("(2+3)*4", 20);
  expectValue("10-4-3", 3);            // left associative
  expectValue("64/4/2", 8);
  expectValue("2^3^2", 512);           // right associative
  expectValue("-2^2", -4);             // unary minus binds weaker than ^
  expectValue("(-2)^2", 4);
  expectValue("2^-1", 0.5);
  expectValue("--3", 3);
  expectValue("+3", 3);
  expectValue(" 1 + 2 ", 3);

  // --- 2. SI suffixes ---
  expectValue("1.5k", 1500);
  expectValue("22u", 22e-6);
  expectValue("22µ", 22e-6);
  expectValue("4.7n", 4.7e-9);
  expectValue("3M", 3e6);
  expectValue("2m*3", 0.006);
  expectValue("1k+1", 1001);

  // --- 3. variables and functions ---
  QMap<QString, double> vars{{"u", 12.0}, {"i", 0.5}, {"r_1", 100}, {"default", 2}};
  expectValue("u*i", 6, vars);
  expectValue("u / i", 24, vars);
  expectValue("u^2/r_1", 1.44, vars);
  expectValue("default*2", 4, vars);
  expectValue("sqrt(u*3)", 6, vars);
  expectValue("abs(-u)", 12, vars);
  expectValue("log10(1000)", 3);
  expectValue("min(u, i)", 0.5, vars);
  expectValue("max(u, i*100)", 50, vars);
  expectValue("sqrt(abs(-16))", 4);

  {
    auto e = CalcExpr::parse("u * i + u - sqrt(r_1)");
    check(e && e->variables() == QStringList{"u", "i", "r_1"}, "variables() lists each name once, in order");
    e = CalcExpr::parse("2 + 3");
    check(e && e->variables().isEmpty(), "constant expression has no variables");
    e = CalcExpr::parse("  u*i ");
    check(e && e->text() == "u*i", "text() is the trimmed source");
  }

  // --- 4. parse errors, with position ---
  expectParseError("", 0);
  expectParseError("   ", 0);
  expectParseError("2 +", 3);
  expectParseError("(2+3", 4);
  expectParseError("2+3)", 3);
  expectParseError("2 3", 2);
  expectParseError("foo(2)", 0);
  expectParseError("min(1)", 5);
  expectParseError("sqrt()", 5);
  expectParseError("2 $ 3", 2);
  expectParseError("3x", 1);          // unknown suffix
  expectParseError("3kg", 1);
  expectParseError("*2", 0);
  expectParseError("2**3", 2);

  // --- 5. evaluation failures ---
  expectEvalFails("u * i", {{"u", 1}});   // i missing
  expectEvalFails("1/0");
  expectEvalFails("sqrt(-1)");
  expectEvalFails("log10(0)");
  expectEvalFails("u", {});

  if (failed == 0)
    qInfo() << "All calc expression tests passed.";
  else
    qWarning() << failed << "calc expression test(s) failed.";
  return failed == 0 ? 0 : 1;
}
