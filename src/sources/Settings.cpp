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

#include "Settings.h"

Settings::Settings(QObject *parent) : QObject(parent)
{
	m_fileConverted=false;
	m_qsettings=new QSettings(this);
	QFile file(m_qsettings->fileName());
	m_fileExists=file.exists();
	m_tmpConfig=new QHash<QString,QVariant>;
	m_filename=m_qsettings->fileName();
}
Settings::~Settings()
{
	delete m_tmpConfig;
}
Settings::Settings(QObject *parent, const QString &fileName) : Settings(parent)
{
	QFile file(fileName);
	if (file.exists())
	{
		QSettings *old= new QSettings(fileName,QSettings::IniFormat,this);
		//migrate
		for(auto name : old->allKeys())
			m_qsettings->setValue(name,old->value(name));
		old->deleteLater();
		m_fileExists=true;
		//rename old config file
		if(QFile::exists(QString("%1.old").arg(fileName)))
			QFile::remove(QString("%1.old").arg(fileName));
		file.rename(QString("%1.old").arg(fileName));
		m_fileConverted=true;
	}
}
int Settings::getInt(const QString &name, const int &def) const
{
	return m_qsettings->value(name,def).toInt();
}
void Settings::setInt(const QString &name, const int &value)
{
	m_tmpConfig->insert(name,value);
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
	m_tmpConfig->insert(name,value);
}
QString Settings::getString(const QString &name, const QString &def) const
{
	return m_qsettings->value(name,def).toString();
}
void Settings::setString(const QString &name, const QString &value)
{
	m_tmpConfig->insert(name,value);
}
bool Settings::getBool(const QString &name, const bool &def) const
{
	return m_qsettings->value(name,def).toBool();
}
void Settings::setBool(const QString &name, const bool &value)
{
	m_tmpConfig->insert(name,value);
}
void Settings::save()
{
	for(auto parameter :m_tmpConfig->keys())
		m_qsettings->setValue(parameter,m_tmpConfig->value(parameter));
	m_tmpConfig->clear();
}
void Settings::clear()
{
	m_tmpConfig->clear();
}
