/*
	Copyright (C) 2015 Frank Büttner frank@familie-büttner.de

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include "settings.h"

#include <QFileInfo>

Settings::Settings(QObject *parent) : QObject(parent)
{
  m_fileConverted = false;
  m_qsettings = new QSettings(this);
  QFile file(m_qsettings->fileName());
  m_fileExists = file.exists();
  m_tmpConfig = new QHash<QString, QVariant>;
  m_filename = m_qsettings->fileName();
}

Settings::~Settings()
{
  delete m_tmpConfig;
}

Settings::Settings(const QString &session_id, const QString &config_path, QObject *parent)
  : Settings(parent)
{
  QFileInfo fileInfo(m_qsettings->fileName());
  m_configPath = fileInfo.absolutePath();
  m_configBaseFileName = fileInfo.baseName();
  m_configFileNameSuffix = fileInfo.suffix();
  m_sessionId = session_id.isEmpty() ? "default" : session_id;

  if (!session_id.isEmpty() || !config_path.isEmpty())
  {
    m_configPath = config_path.isEmpty() ? fileInfo.absolutePath() : config_path;
    m_configBaseFileName = fileInfo.baseName();
    m_configFileNameSuffix = fileInfo.suffix();

    QString session_fileName = m_configBaseFileName+(session_id.isEmpty() ? "" : "_"+session_id)+"."+m_configFileNameSuffix;
    session_fileName.prepend(m_configPath+"/");

    delete m_qsettings;
    m_qsettings = new QSettings(session_fileName, QSettings::IniFormat, this);
    QFile file(m_qsettings->fileName());
    m_fileExists = file.exists();
    m_filename = m_qsettings->fileName();
    m_fileConverted = false;
    //qInfo() << "config file: " << m_qsettings->fileName();
  }
}

QStringList Settings::getConfigInstances()
{
  QDir dir(m_configPath);
  if (!dir.exists())
    return {};
  qInfo() << m_configPath << m_configBaseFileName << m_configFileNameSuffix;
  QString pattern = QString("%1*%2")
                      .arg(m_configBaseFileName)
                      .arg(m_configFileNameSuffix.isEmpty() ? "" : "." + m_configFileNameSuffix);

  QStringList result;
  result << m_sessionId;
  for (const QString &file : dir.entryList({ pattern }, QDir::Files, QDir::Name))
  {
    QString base = QFileInfo(file).completeBaseName();

    if (base.startsWith(m_configBaseFileName))
      base = base.mid(m_configBaseFileName.length());

    if (base.startsWith("_"))
      base.remove(0, 1);

    result << (base.isEmpty() ? "default" : base);
  }

  result.removeDuplicates();
  result.sort(Qt::CaseInsensitive);
  if (result.contains("default"))
  {
    result.removeAll("default");
    result.prepend("default");
  }

  for (auto item : result)
    qInfo() << item;

  return result;
}

int Settings::getInt(const QString &name, const int &def) const
{
  return m_qsettings->value(name, def).toInt();
}

void Settings::setInt(const QString &name, const int &value)
{
  m_tmpConfig->insert(name, value);
}

QColor Settings::getColor(const QString &name, const QColor &def) const
{
  QVariant value(m_qsettings->value(name));
  if (!value.isNull())
    return m_qsettings->value(name).value<QColor>();
  return def;
}

void Settings::setColor(const QString &name, const QColor &value)
{
  m_tmpConfig->insert(name, value);
}

QString Settings::getString(const QString &name, const QString &def) const
{
  return m_qsettings->value(name, def).toString();
}

void Settings::setString(const QString &name, const QString &value)
{
  m_tmpConfig->insert(name, value);
}

bool Settings::getBool(const QString &name, const bool &def) const
{
  return m_qsettings->value(name, def).toBool();
}

void Settings::setBool(const QString &name, const bool &value)
{
  m_tmpConfig->insert(name, value);
}

void Settings::save()
{
  for (auto parameter : m_tmpConfig->keys())
    m_qsettings->setValue(parameter, m_tmpConfig->value(parameter));
  m_tmpConfig->clear();
}

void Settings::clear()
{
  m_tmpConfig->clear();
}
