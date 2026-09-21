#pragma once

#include <QtCore>
#include <QString>
#include <QByteArray>
#include <optional>

#include "readevent.h"

/// Base class of all protocol decoders and registry of the supported meters.
///
/// A decoder turns one frame of meter bytes into a DmmResponse. Each protocol
/// (ReadEvent::DataFormat) has one subclass in src/decoders/; that file also
/// registers the meters speaking the protocol with addConfig() from a static
/// initialiser, so adding a meter is a one-line change there and the
/// supported-devices page (tests/generate_docs.py) picks it up.
///
/// How a frame reaches decode(): ReaderThread feeds every received byte into
/// a ring buffer and calls checkFormat() on it; once that returns true the
/// last getPacketLength() bytes are handed to decode().
///
/// The frame formats are documented in docs/protocols/.
class DmmDecoder : public QObject
{
  Q_OBJECT

public:
  /// A single decoded measurement.
  ///
  /// Contract for the value triple - decoders MUST follow this, because the
  /// display and the recorder consume the two forms differently:
  ///
  ///   - val, unit: display form, unit carries the SI prefix ("1.234", "kOhm")
  ///   - dval: the same measurement in SI *base* units (1234.0)
  ///
  /// DisplayWid shows val+unit verbatim, while DMMGraph plots dval and strips
  /// the prefix off the unit for its axis label (DMMGraph::setUnit). A decoder
  /// that leaves dval unscaled therefore records values whose magnitude jumps
  /// by the prefix factor whenever the meter changes range.
  ///
  /// Use formatResultValue() to get this right; it inserts the decimal point,
  /// scales dval and assembles unit in one step.
  class DmmResponse
  {
  public:
    double dval;      ///< value in SI base units, what the graph and recorder use
    QString val;      ///< value as shown on the meter's display, e.g. "-1.234" or "0.L"
    QString unit;     ///< display unit including SI prefix, e.g. "kOhm", "mV"
    QString special;  ///< coupling/function: "AC", "DC", "ACDC", "DI", "BUZ", ...
    QString range;    ///< "AUTO", "MANU" or empty
    bool hold;        ///< the meter's HOLD annunciator is on
    bool showBar;     ///< the meter shows a bar graph for this reading
    bool lowBat;      ///< low battery indicator
    int id;           ///< which value this is (0 = main display) for multi-value meters
    QString error;    ///< non-empty when the frame was understood but flagged an error
    double dval2;     ///< optional second value in the same frame (e.g. frequency)
    QString val2;     ///< display form of the second value
    QString unit2;    ///< unit of the second value
    int id2;          ///< id of the second value, 0 when there is none
  };

  /// Describes one supported meter: what the settings page fills in when the
  /// model is chosen. Registered by the decoder files via addConfig().
  class DMMInfo
  {
  public:
    QString vendor;                 ///< manufacturer as shown in the model list
    QString model;
    QString name;                   ///< "vendor model", filled in by addConfig()
    int   baud;                     ///< 600 ... 19200
    ReadEvent::DataFormat protocol; ///< which decoder handles the meter
    int   bits;                     ///< data bits, 5..8
    int   stopBits;                 ///< 1 or 2
    int   numValues;                ///< frames per reading, for meters that send several lines
    int   parity;                   ///< 0 none, 1 even, 2 odd
    int   display;                  ///< display counts: 2000, 4000, 6000, ... 1000000
    bool  externalSetup;            ///< true: leave the port settings alone, the user configured it
    bool  rts;                      ///< drive RTS high (some cables are powered from it)
    bool  dtr;                      ///< drive DTR high
    QString sigrokExe;              ///< sigrok-cli path for SigrokDevice, empty otherwise
  };

  explicit DmmDecoder(ReadEvent::DataFormat df);
  virtual ~DmmDecoder() = default;
  /// Fixed length of one frame in bytes.
  virtual size_t                     getPacketLength() = 0;
  /// Frame delimiter: true when the ring buffer @p data, whose newest byte is
  /// at index @p len, ends with a complete frame (typically checked on the
  /// terminator and the high nibbles of the VC820-style byte counters).
  virtual bool                       checkFormat(const char* data, size_t len) = 0; // TBD use qbytearray or similar instead for data
  /// Bytes to send to the meter to make it emit a frame; empty for meters
  /// that stream on their own. ReaderThread writes it once per read cycle.
  virtual QByteArray pollRequest() const { return QByteArray(); }
  /// Decodes one frame of getPacketLength() bytes. @p id tells which of
  /// DMMInfo::numValues frames this is. Returns nothing when the frame is
  /// corrupt; the caller then keeps the previous reading.
  virtual std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id) = 0;

  ReadEvent::DataFormat getType() { return m_type; };
  /// Short protocol name for --debug output and the device table.
  QString name() const { return m_name; };

  /// All meters registered with addConfig(), in registration order.
  static std::vector<DMMInfo>        getDeviceConfigurations();
  /// Registers a meter; called from the decoder files' static initialisers.
  static void                        addConfig(DMMInfo info);
  /// Factory: the decoder for a protocol, null for ReadEvent::Invalid.
  static std::shared_ptr<DmmDecoder> getInstance(ReadEvent::DataFormat df);
  /// Factory by protocol name (ReadEvent::toString()).
  static std::shared_ptr<DmmDecoder> getInstance(QString df);


protected:
  /// Decimal point for the 4-digit decoders (VC820, DTM0660) whose value
  /// string is "<sign><blank>dddd": inserts it after @p pos digits.
  QString insertComma(const QString &val, int pos);
  /// Inserts the decimal point after @p pos digits, skipping a leading sign
  /// or blank ("-1234", 2 -> "-12.34"). pos 0 leaves the string unchanged.
  QString insertCommaIT(const QString &val, int pos);
  /// Scale factor for an SI prefix ("k" -> 1e3). Unknown prefixes yield 1.0.
  static double prefixFactor(const QString &prefix);
  /// Finishes m_result from the raw digit string in m_result.val: inserts
  /// the decimal point at @p commaPos, sets unit to prefix+baseUnit and dval
  /// to the value scaled by the prefix. See the DmmResponse contract.
  void formatResultValue(int commaPos, const QString& prefix, const QString& baseUnit);
  /// Tests bit @p bit (0 = LSB) of byte @p byte of the frame.
  bool bit(const QByteArray &data, int byte, int bit) const;
  /// Copies the ASCII digits data[first..last] into a string, with an
  /// optional leading minus.
  QString makeValue(const QByteArray &data, int first, int last, bool neg=false);
  /// One-line dump of m_result for --debug output.
  QString toString() const;

  static std::vector<DMMInfo> *m_configurations;
  DmmResponse m_result;
  ReadEvent::DataFormat m_type;
  QString m_name;
};




