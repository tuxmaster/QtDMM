// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "spreadsheet.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QTimeZone>

#include "3rdparty/miniz/miniz.h"

namespace
{
QString tr(const char *text)
{
  return QCoreApplication::translate("SpreadsheetWriter", text);
}

QString xmlEscape(const QString &s)
{
  QString e = s;
  e.replace('&', "&amp;").replace('<', "&lt;").replace('>', "&gt;").replace('"', "&quot;");
  return e;
}

// column index (0-based) -> "A", "Z", "AA"
QString columnName(int index)
{
  QString name;
  for (int i = index; i >= 0; i = i / 26 - 1)
    name.prepend(QChar('A' + i % 26));
  return name;
}

// Excel's serial date: days since 1899-12-30, fraction = time of day (local time)
double excelSerial(const QDateTime &dt)
{
  const QDateTime local = dt.toLocalTime();
  const QDate d = local.date();
  const qint64 days = QDate(1899, 12, 30).daysTo(d);
  const double frac = local.time().msecsSinceStartOfDay() / 86400000.0;
  return days + frac;
}

QString number(double v)
{
  return QString::number(v, 'g', 15);
}

bool isNumber(const QVariant &v)
{
  const auto t = v.typeId();
  return t == QMetaType::Double || t == QMetaType::Float || t == QMetaType::Int || t == QMetaType::LongLong
         || t == QMetaType::UInt || t == QMetaType::ULongLong;
}

// the widest content per column, in characters, for the column widths
QList<int> columnWidths(const QStringList &header, const QList<QList<QVariant>> &rows)
{
  QList<int> widths;
  for (const QString &h : header)
    widths << h.size();
  for (const QList<QVariant> &row : rows)
    for (int c = 0; c < row.size(); ++c)
    {
      const QVariant &v = row[c];
      const int w = v.typeId() == QMetaType::QDateTime ? 23 : isNumber(v) ? number(v.toDouble()).size() : v.toString().size();
      if (c >= widths.size())
        widths << w;
      else
        widths[c] = qMax(widths[c], w);
    }
  return widths;
}
}

std::optional<SpreadsheetWriter::Format> SpreadsheetWriter::formatForFile(const QString &path)
{
  const QString suffix = QFileInfo(path).suffix().toLower();
  if (suffix == "xlsx")
    return Xlsx;
  if (suffix == "ods")
    return Ods;
  return std::nullopt;
}

SpreadsheetWriter::SpreadsheetWriter(const QString &sheetName) : m_sheetName(sheetName)
{
}

void SpreadsheetWriter::setHeader(const QStringList &header)
{
  m_header = header;
}

void SpreadsheetWriter::addRow(const QList<QVariant> &row)
{
  m_rows << row;
}

// ---------------------------------------------------------------- xlsx

QByteArray SpreadsheetWriter::xlsxSheet() const
{
  QString x;
  x += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
       "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">";
  x += "<sheetViews><sheetView workbookViewId=\"0\"><pane ySplit=\"1\" topLeftCell=\"A2\" activePane=\"bottomLeft\" state=\"frozen\"/></sheetView></sheetViews>";
  const QList<int> widths = columnWidths(m_header, m_rows);
  if (!widths.isEmpty())
  {
    x += "<cols>";
    for (int c = 0; c < widths.size(); ++c)
      x += QString("<col min=\"%1\" max=\"%1\" width=\"%2\" customWidth=\"1\"/>").arg(c + 1).arg(widths[c] + 2);
    x += "</cols>";
  }
  x += "<sheetData>";
  int r = 1;
  if (!m_header.isEmpty())
  {
    x += QString("<row r=\"%1\">").arg(r);
    for (int c = 0; c < m_header.size(); ++c)
      x += QString("<c r=\"%1%2\" t=\"inlineStr\" s=\"1\"><is><t>%3</t></is></c>").arg(columnName(c)).arg(r).arg(xmlEscape(m_header[c]));
    x += "</row>";
    ++r;
  }
  for (const QList<QVariant> &row : m_rows)
  {
    x += QString("<row r=\"%1\">").arg(r);
    for (int c = 0; c < row.size(); ++c)
    {
      const QVariant &v = row[c];
      const QString ref = columnName(c) + QString::number(r);
      if (v.typeId() == QMetaType::QDateTime)
        x += QString("<c r=\"%1\" s=\"2\"><v>%2</v></c>").arg(ref, number(excelSerial(v.toDateTime())));
      else if (isNumber(v))
        x += QString("<c r=\"%1\"><v>%2</v></c>").arg(ref, number(v.toDouble()));
      else if (!v.toString().isEmpty())
        x += QString("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, xmlEscape(v.toString()));
    }
    x += "</row>";
    ++r;
  }
  x += "</sheetData></worksheet>";
  return x.toUtf8();
}

// ---------------------------------------------------------------- ods

QByteArray SpreadsheetWriter::odsContent() const
{
  QString x;
  x += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" "
       "xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\" "
       "xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" "
       "xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" "
       "xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" "
       "xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" office:version=\"1.2\">";
  // a date style with milliseconds, a bold header, column widths
  x += "<office:automatic-styles>"
       "<number:date-style style:name=\"NDate\"><number:year number:style=\"long\"/><number:text>-</number:text>"
       "<number:month number:style=\"long\"/><number:text>-</number:text><number:day number:style=\"long\"/>"
       "<number:text> </number:text><number:hours number:style=\"long\"/><number:text>:</number:text>"
       "<number:minutes number:style=\"long\"/><number:text>:</number:text>"
       "<number:seconds number:style=\"long\" number:decimal-places=\"3\"/></number:date-style>"
       "<style:style style:name=\"ceDate\" style:family=\"table-cell\" style:data-style-name=\"NDate\"/>"
       "<style:style style:name=\"ceHead\" style:family=\"table-cell\"><style:text-properties fo:font-weight=\"bold\"/></style:style>";
  const QList<int> widths = columnWidths(m_header, m_rows);
  for (int c = 0; c < widths.size(); ++c)
    x += QString("<style:style style:name=\"co%1\" style:family=\"table-column\"><style:table-column-properties "
                 "style:column-width=\"%2cm\"/></style:style>").arg(c).arg((widths[c] + 2) * 0.2, 0, 'f', 2);
  x += "</office:automatic-styles>";
  x += QString("<office:body><office:spreadsheet><table:table table:name=\"%1\">").arg(xmlEscape(m_sheetName));
  for (int c = 0; c < widths.size(); ++c)
    x += QString("<table:table-column table:style-name=\"co%1\"/>").arg(c);
  if (!m_header.isEmpty())
  {
    x += "<table:table-header-rows><table:table-row>";
    for (const QString &h : m_header)
      x += QString("<table:table-cell table:style-name=\"ceHead\" office:value-type=\"string\"><text:p>%1</text:p></table:table-cell>").arg(xmlEscape(h));
    x += "</table:table-row></table:table-header-rows>";
  }
  for (const QList<QVariant> &row : m_rows)
  {
    x += "<table:table-row>";
    for (const QVariant &v : row)
    {
      if (v.typeId() == QMetaType::QDateTime)
      {
        const QString iso = v.toDateTime().toLocalTime().toString("yyyy-MM-ddTHH:mm:ss.zzz");
        x += QString("<table:table-cell table:style-name=\"ceDate\" office:value-type=\"date\" office:date-value=\"%1\"><text:p>%2</text:p></table:table-cell>")
               .arg(iso, iso.left(10) + " " + iso.mid(11));
      }
      else if (isNumber(v))
        x += QString("<table:table-cell office:value-type=\"float\" office:value=\"%1\"><text:p>%1</text:p></table:table-cell>").arg(number(v.toDouble()));
      else if (v.toString().isEmpty())
        x += "<table:table-cell/>";
      else
        x += QString("<table:table-cell office:value-type=\"string\"><text:p>%1</text:p></table:table-cell>").arg(xmlEscape(v.toString()));
    }
    x += "</table:table-row>";
  }
  x += "</table:table></office:spreadsheet></office:body></office:document-content>";
  return x.toUtf8();
}

// ---------------------------------------------------------------- zip

namespace
{
// A plain ZIP writer: local headers with the sizes known up front, no data
// descriptors, no extra fields, then the central directory. miniz's own
// writer sets the data-descriptor flag on every entry, which LibreOffice
// rejects for the stored "mimetype" entry an ODS must begin with. Deflate
// and CRC come from miniz.
class ZipWriter
{
public:
  bool add(const QByteArray &name, const QByteArray &data, bool store)
  {
    QByteArray payload;
    quint16 method = 0;
    if (!store && !data.isEmpty())
    {
      size_t outSize = 0;
      void *out = tdefl_compress_mem_to_heap(data.constData(), size_t(data.size()), &outSize, TDEFL_DEFAULT_MAX_PROBES);
      if (!out)
        return false;
      payload = QByteArray(static_cast<const char *>(out), int(outSize));
      mz_free(out);
      method = 8;
    }
    else
      payload = data;
    const quint32 crc = quint32(mz_crc32(MZ_CRC32_INIT, reinterpret_cast<const mz_uint8 *>(data.constData()), size_t(data.size())));

    Entry e;
    e.name = name;
    e.crc = crc;
    e.compressed = quint32(payload.size());
    e.uncompressed = quint32(data.size());
    e.method = method;
    e.offset = quint32(m_data.size());
    m_entries << e;

    // local file header
    put32(0x04034b50);
    put16(20);            // version needed
    put16(0x0800);        // flags: UTF-8 names
    put16(method);
    put16(0); put16(0x21);   // time 00:00:00, date 1980-01-01
    put32(crc);
    put32(e.compressed);
    put32(e.uncompressed);
    put16(quint16(name.size()));
    put16(0);             // extra
    m_data += name;
    m_data += payload;
    return true;
  }

  QByteArray finish()
  {
    const quint32 cdStart = quint32(m_data.size());
    for (const Entry &e : m_entries)
    {
      put32(0x02014b50);
      put16(20); put16(20);
      put16(0x0800);
      put16(e.method);
      put16(0); put16(0x21);
      put32(e.crc);
      put32(e.compressed);
      put32(e.uncompressed);
      put16(quint16(e.name.size()));
      put16(0); put16(0);   // extra, comment
      put16(0);             // disk
      put16(0); put32(0);   // attributes
      put32(e.offset);
      m_data += e.name;
    }
    const quint32 cdSize = quint32(m_data.size()) - cdStart;
    put32(0x06054b50);
    put16(0); put16(0);
    put16(quint16(m_entries.size())); put16(quint16(m_entries.size()));
    put32(cdSize);
    put32(cdStart);
    put16(0);
    return m_data;
  }

private:
  struct Entry { QByteArray name; quint32 crc, compressed, uncompressed, offset; quint16 method; };
  void put16(quint16 v) { m_data.append(char(v & 0xff)).append(char(v >> 8)); }
  void put32(quint32 v) { put16(quint16(v & 0xffff)); put16(quint16(v >> 16)); }
  QByteArray m_data;
  QList<Entry> m_entries;
};
}

bool SpreadsheetWriter::write(const QString &path, Format format, QString *error) const
{
  auto fail = [&](const QString &text)
  {
    if (error)
      *error = text;
    return false;
  };
  if (m_rows.isEmpty())
    return fail(tr("Nothing to export."));

  ZipWriter zip;
  auto add = [&](const char *name, const QByteArray &data, bool store = false)
  {
    return zip.add(QByteArray(name), data, store);
  };

  bool ok = true;
  if (format == Xlsx)
  {
    ok = ok && add("[Content_Types].xml",
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
      "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
      "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
      "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
      "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
      "<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>"
      "</Types>");
    ok = ok && add("_rels/.rels",
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
      "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>"
      "</Relationships>");
    ok = ok && add("xl/workbook.xml", QString(
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
      "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
      "<sheets><sheet name=\"%1\" sheetId=\"1\" r:id=\"rId1\"/></sheets></workbook>").arg(xmlEscape(m_sheetName)).toUtf8());
    ok = ok && add("xl/_rels/workbook.xml.rels",
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
      "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
      "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>"
      "</Relationships>");
    // styles: 0 default, 1 bold header, 2 date with milliseconds
    ok = ok && add("xl/styles.xml",
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
      "<numFmts count=\"1\"><numFmt numFmtId=\"164\" formatCode=\"yyyy\\-mm\\-dd\\ hh:mm:ss.000\"/></numFmts>"
      "<fonts count=\"2\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font><font><b/><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
      "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
      "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
      "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
      "<cellXfs count=\"3\">"
      "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"
      "<xf numFmtId=\"0\" fontId=\"1\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\"/>"
      "<xf numFmtId=\"164\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyNumberFormat=\"1\"/>"
      "</cellXfs>"
      "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>"
      "</styleSheet>");
    ok = ok && add("xl/worksheets/sheet1.xml", xlsxSheet());
  }
  else
  {
    // the mimetype must be the first entry and stored uncompressed
    ok = ok && add("mimetype", "application/vnd.oasis.opendocument.spreadsheet", true);
    ok = ok && add("META-INF/manifest.xml",
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      "<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\" manifest:version=\"1.2\">"
      "<manifest:file-entry manifest:full-path=\"/\" manifest:version=\"1.2\" manifest:media-type=\"application/vnd.oasis.opendocument.spreadsheet\"/>"
      "<manifest:file-entry manifest:full-path=\"content.xml\" manifest:media-type=\"text/xml\"/>"
      "</manifest:manifest>");
    ok = ok && add("content.xml", odsContent());
  }
  if (!ok)
    return fail(tr("Cannot write the archive."));

  QFile file(path);
  const QByteArray bytes = zip.finish();
  if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size())
    return fail(tr("Cannot open file."));
  return true;
}
