#pragma once

#include <QString>
#include <QByteArray>
#include <optional>

#include "readevent.h"

class DmmDriver
{
  Q_DECLARE_TR_FUNCTIONS(DmmDriver)

public:
  struct DmmResponse
  {
    double dval;
    QString val;
    QString unit;
    QString special;
    QString range;
    bool hold;
    bool showBar;
    int id;
    QString error;
    double dval2;
    QString val2;
    QString unit2;
    int id2;
  };

  virtual ~DmmDriver() = default;
  virtual std::optional<DmmDriver::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df) = 0;

protected:
  QString insertComma(const QString &val, int pos);
  QString insertCommaIT(const QString &val, int pos);
  void formatResultValue(int commaPos, const QString& prefix, const QString& baseUnit);
  bool bit(const QByteArray &data, int byte, int bit) const;
  QString toString() const;

  DmmResponse m_result;
};
