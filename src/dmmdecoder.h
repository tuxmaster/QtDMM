#pragma once

#include <QtCore>
#include <QString>
#include <QByteArray>
#include <optional>

#include "readevent.h"

/**
  description of dmminfo data
     vendor
     model
     (name) leaf empty "" will be filled automatically
     baud (600, 1200, 1800, 2400, 4800, 9600, 19200)
     protocol 0: 14 bytes polling 'D'
              1: 11 bytes continuous [PeakTech]
              2: 14 bytes continuous
              3: 15 bytes continuous
              4: 11 bytes continuous (M9803R)
              5: 14 bytes continuous (VC820)
              6: IsoTech
              7: VC940
              8: QM1537
              9: 9 bytes continuous (22-812)
             10: 23 bytes continuous (VC870)
             11: 22 bytes continuous (DO3122)
             12: 4 bytes half-ASCII (CyrustekES51922)
             13: 11 bytes continuous (CyrustekES51962)
             14: 15 bytes continuous (DTM0660)
             15: 11 bytes continuous (CyrustekES51981)
     bits
     stopBits
     number of values (For DMM's that send several lines at once)
     parity (0,1,2 - None,Even,Odd)
     display digits (2000, 4000, 6000, 8000, 20000, 22000, 40000, 50000, 100000, 200000, 400000, 1000000)
     External device setup 0, 1
     rts 0, 1
     dtr 0, 1
**/

class DmmDecoder : public QObject
{
  Q_OBJECT

public:
  class DmmResponse
  {
  public:
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

  class DMMInfo
  {
  public:
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
    bool  dtr;
  };

  virtual ~DmmDecoder() = default;
  virtual size_t                                getPacketLength(ReadEvent::DataFormat df) = 0;
  virtual bool                                  checkFormat(const char* data, size_t len, ReadEvent::DataFormat df) = 0; // TBD use qbytearray or similar instead for data
  virtual std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df) = 0;

  static std::vector<DMMInfo>                   getDeviceConfigurations();
  static void                                   addConfig(DMMInfo info);


protected:
  QString insertComma(const QString &val, int pos);
  QString insertCommaIT(const QString &val, int pos);
  void formatResultValue(int commaPos, const QString& prefix, const QString& baseUnit);
  bool bit(const QByteArray &data, int byte, int bit) const;
  QString toString() const;

  static std::vector<DMMInfo> *m_configurations;
  DmmResponse m_result;
};




