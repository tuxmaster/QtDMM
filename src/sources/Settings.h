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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtCore>
#include <QColor>

class Settings : public QObject
{
		Q_OBJECT
	public:
		explicit				Settings(QObject *parent = Q_NULLPTR);
		explicit				Settings(QObject *parent, const QString &fileName);
								~Settings();

		const bool				&fileExists() const {return m_fileExists;}
		const bool				&fileConverted() const {return m_fileConverted;}

		const QString			&fileName()const {return m_filename;}

		void					save();
		void					clear();

		int						getInt(const QString &name,const int &def=0) const;
		void					setInt(const QString &name,const int &value);

		QColor					getColor(const QString &name,const QColor &def=Qt::white)const;
		void					setColor(const QString &name,const QColor &value);

		QString					getString(const QString &name,const QString &def="") const;
		void					setString(const QString &name,const QString &value);

		bool					getBool(const QString &name,const bool &def=false)const;
		void					setBool(const QString &name,const bool &value);

	private:
		bool					m_fileExists;
		bool					m_fileConverted;
		QString					m_filename;
		QSettings				*m_qsettings;
		QHash<QString,QVariant>	*m_tmpConfig;
};

#endif // SETTINGS_H
