#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QtTest>
#include <QRegularExpression>

#include "dmmdecoder.h"

QByteArray parseHexStringToByteArray(const QString &hexString)
{
  QByteArray result;
  QStringList tokens = hexString.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  for (const QString &token : tokens)
  {
    bool ok = false;
    char byte = static_cast<char>(token.toUInt(&ok, 16));
    if (ok)
      result.append(byte);
  }
  return result;
}

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  if (argc != 2)
  {
    qCritical() << "Usage: test_decoder <jsonfile>";
    return 1;
  }

  QString jsonFile = QString::fromLocal8Bit(argv[1]);
  QFile file(jsonFile);
  if (!file.open(QIODevice::ReadOnly))
  {
    qCritical() << "Failed to open file:" << jsonFile;
    return 2;
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  QJsonObject root = doc.object();
  QString decoderName = root["decoder"].toString();
  QJsonArray tests = root["tests"].toArray();

  auto decoder = DmmDecoder::getInstance(decoderName);
  if (!decoder)
  {
    qCritical() << "Unknown decoder:" << decoderName;
    return 3;
  }

  int failed = 0;

  for (int i = 0; i < tests.size(); ++i)
  {
    QJsonObject test = tests[i].toObject();
    QString hexString = test["hex"].toString();
    QJsonObject expected = test["expected"].toObject();
    QByteArray frame = parseHexStringToByteArray(hexString);

    auto fail = [&](const QString & what)
    {
      qWarning() << "Test case" << i << "failed:" << what;
      failed++;
    };

    // simulate reading from port
    char fifo[FIFO_LENGTH];
    char buffer[FIFO_LENGTH];
    size_t bytesToRead = decoder->getPacketLength();
    size_t length = 0;
    bool frameValid = false;

    for (size_t n = 0; n < 5 && !frameValid; n++)
    {
      for (size_t idx = 0; idx < frame.size(); idx++)
      {
        fifo[length] = QChar(frame[idx]).toLatin1();
        if (decoder->checkFormat(fifo, length))
        {
          length = (length - bytesToRead + 1 + FIFO_LENGTH) % FIFO_LENGTH;

          for (int i = 0; i < bytesToRead; ++i)
          {
            buffer[i] = fifo[length];
            length = (length + 1) % FIFO_LENGTH;
          }

          length = 0;

          frameValid = true;

          break;
          //Q_EMIT readEvent( QByteArray(m_buffer,bytesToRead), m_id, m_format );
          //m_id = (m_id + 1) % m_numValues;
        }
        else
          length = (length + 1) % FIFO_LENGTH;
      }
    }

    if (!frameValid)
    {
      fail(QString("[checkFormat] '%1' is not a valid input").arg(hexString));
      continue;
    }

    frame.resize(decoder->getPacketLength());
    auto result = decoder->decode(frame, 0);


    auto check = [&](const QString & key, const auto & actual, const QJsonValue & expectedVal)
    {
      using T = std::decay_t<decltype(actual)>;

      if constexpr(std::is_same_v<T, double>)
      {
        if (!qFuzzyCompare(actual + 1, expectedVal.toDouble() + 1))
          fail(QString("%1 mismatch: got %2, expected %3").arg(key).arg(actual).arg(expectedVal.toDouble()));
      }
      else if constexpr(std::is_same_v<T, QString>)
      {
        if (actual != expectedVal.toString())
          fail(QString("%1 mismatch: got '%2', expected '%3'").arg(key).arg(actual).arg(expectedVal.toString()));
      }
      else if constexpr(std::is_same_v<T, bool>)
      {
        if (actual != expectedVal.toBool())
          fail(QString("%1 mismatch: got %2, expected %3").arg(key).arg(actual).arg(expectedVal.toBool()));
      }
      else if constexpr(std::is_same_v<T, int>)
      {
        if (actual != expectedVal.toInt())
          fail(QString("%1 mismatch: got %2, expected %3").arg(key).arg(actual).arg(expectedVal.toInt()));
      }
      else
        qWarning() << "unknown type";
    };

    qInfo() << hexString << ": " << result->dval << result->val << result->unit << result->range << result->special << result->hold
            << result->showBar << result->id << result->error << result->dval2 << result->val2 << result->unit2 << result->id2;

    if (expected.contains("dval"))    check("dval",    result->dval,    expected["dval"]);
    if (expected.contains("val"))     check("val",     result->val,     expected["val"]);
    if (expected.contains("unit"))    check("unit",    result->unit,    expected["unit"]);
    if (expected.contains("range"))   check("range",   result->range,   expected["range"]);
    if (expected.contains("special")) check("special", result->special, expected["special"]);
    if (expected.contains("hold"))    check("hold",    result->hold,    expected["hold"]);
    if (expected.contains("showBar")) check("showBar", result->showBar, expected["showBar"]);
    if (expected.contains("id"))      check("id",      result->id,      expected["id"]);
    if (expected.contains("error"))   check("error",   result->error,   expected["error"]);
    if (expected.contains("dval2"))   check("dval2",   result->dval2,   expected["dval2"]);
    if (expected.contains("val2"))    check("val2",    result->val2,    expected["val2"]);
    if (expected.contains("unit2"))   check("unit2",   result->unit2,   expected["unit2"]);
    if (expected.contains("id2"))     check("id2",     result->id2,     expected["id2"]);
  }

  return failed == 0 ? 0 : 1;
}

