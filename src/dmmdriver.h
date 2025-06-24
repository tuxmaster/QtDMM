#pragma once

#include <QtCore>
#include <QString>
#include <QByteArray>
#include <optional>

#include "readevent.h"

class DmmDriver : public QObject
{
  Q_OBJECT

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

  struct DMMInfo
  {
    QString vendor;
    QString model;
    QString name;
    int   baud;
    int   protocol;
    int   bits;
    int   stopBits;
    int   numValues;
    int   parity;
    int   display;
    bool  externalSetup;
    bool  rts;
    bool  cts;
    bool  dsr;
    bool  dtr;
  };

  virtual ~DmmDriver() = default;
  virtual std::optional<DmmDriver::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df) = 0;
  static std::vector<DMMInfo> getDeviceConfigurations() { return m_configurations; };
  static std::vector<DMMInfo> m_configurations;

protected:
  QString insertComma(const QString &val, int pos);
  QString insertCommaIT(const QString &val, int pos);
  void formatResultValue(int commaPos, const QString& prefix, const QString& baseUnit);
  bool bit(const QByteArray &data, int byte, int bit) const;
  QString toString() const;

  DmmResponse m_result;
};
