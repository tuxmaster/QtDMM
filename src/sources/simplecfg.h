//======================================================================
// File:		simplecfg.h
// Author:	Matthias Toussaint
// Created:	Mon Okt 30 22:01:05 CET 2000
//----------------------------------------------------------------------
// This file is part of QtDMM.
//
// QtDMM is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// QtDMM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------
// Copyright (c) 2000 Matthias Toussaint
//======================================================================

#ifndef SIMPLECFG_HH
#define SIMPLECFG_HH

#include <QtCore>

// I _REALLY_ like writing documentation. Here documentation
// efficiently hides the code ...
//
class SimpleCfg;

/** Encapsulates a group (section) in the configuration file.

	Groups in the configuration file start with a group name embraced
	with rectangular braces ([<i>groupname</i>]). The key/value pairs
	following this group name belong to this group.

	Normally you should not use this class but instead SimpleCfg which
	gives you full control over the configuration file.

	SimpleCfg is licensed under GPL version 3

	\author Matthias Toussaint, &copy; 2000 Matthias Toussaint
*/
class SimpleCfgGroup
{
  friend class SimpleCfg;

public:

 /// Return group name
  QString									name() const { return groupName; }
  /// Remove given key from group
  void										remove( const QString & key );
  /** Check if configuration contains key named <i>key</i>.
	  \param key Name of key to look for
	  \returns true if found
	*/
  bool										contains( const QString & key );
  /** Set integer value.
	  If the key does not exist it is created
	  \param key Key to be searched for
	  \param val Value to be assigned to key
	*/
  void										setInt( const QString & key, int val );
  /** Retrieve integer value.
	  If the key does not exist it is created with the default
	  value.
	  \param key Key to be searched for
	  \param def Default value to be returned if key not found
	  \returns Value of key or default value
	*/
  int										getInt( const QString & key, int def );
  /** Set double value.
	  If the key does not exist it is created
	  \param key Key to be searched for
	  \param val Value to be assigned to key
	*/
  void										setDouble( const QString & key, double val );
  /** Retrieve double value.
	  If the key does not exist it is created with the default
	  value.
	  \param key Key to be searched for
	  \param def Default value to be returned if key not found
	  \returns Value of key or default value
	*/
  double									getDouble( const QString & key, double def );
  /** Set string value.
	  If the key does not exist it is created
	  \param key Key to be searched for
	  \param val Value to be assigned to key
	*/
  void										setString( const QString & key, const QString & val );
  /** Retrieve string value.
	  If the key does not exist it is created with the default
	  value.
	  \param key Key to be searched for
	  \param def Default value to be returned if key not found
	  \returns Value of key or default value
	*/
  QString									getString( const QString & key, const QString & def );
  /** Set QRgb (unsigned long) value.
	  If the key does not exist it is created
	  \param key Key to be searched for
	  \param val Value to be assigned to key
	*/
  void										setRGB( const QString & key, QRgb val );
  /** Retrieve QRgb value (basically unsigned long).
	  If the key does not exist it is created with the default
	  value.
	  \param key Key to be searched for
	  \param def Default value to be returned if key not found
	  \returns Value of key or default value
	*/
  QRgb										getRGB( const QString & key, QRgb def );
  /** Set bool value.
	  If the key does not exist it is created
	  \param key Key to be searched for
	  \param val Value to be assigned to key
	*/
  void										setBool( const QString & key, bool val );
  /** Retrieve bool value.
	  If the key does not exist it is created with the default
	  value. The bool values appear as "true"/"false" in the
	  configuration file.
	  \param key Key to be searched for
	  \param val Default value to be returned if key not found
	  \returns Value of key or default value
	*/
  bool										getBool( const QString & key, bool def );
  /** Access iterator over keys.
	  If there are no keys this member returns SimpleCfgGroup::end().

	  If you want to iterate through the keys you'll need code like
	  the following:
	  \code

	  QMap<QString,QString>::Iterator it;

	  for (it=group.begin(); it != group.end(); ++it)
	  {
		// it.key() returns the key
		// it.data() returns the value as string
	  }

	  \endcode
	*/
  inline QMap<QString,QString>::Iterator	begin() { return map.begin(); }
  /** Access end value for iterator.
	  \see SimpleCfgGroup::begin()
	*/
  inline QMap<QString,QString>::Iterator	end()   { return map.end(); }

protected:
  /// Construct a group with name
  SimpleCfgGroup( const QString & name );

  /// Rename the group
  void					setName( const QString & name );
  /// Read from QTextStream
  void					read( QTextStream & );
  /// Write to QTextStream
  void					write( QTextStream & );

  QMap<QString,QString>	map;
  QString               groupName;

};

/** Encapsulation of an "Windows-ini-style" ASCII configuration file. You
	know these
	\code
	[operating system]
	windows=sux
	unix=rocks
	\endcode
	configuration files.

	Usage is quite simple. Just create a SimpleCfg object and retrieve
	the needed values in your code. All retrieval member need a default
	value. This ensures that first usage of the configuration file
	(Means file does not exist on harddisk) will create a file with
	hopefully sane (you have to specify them :) default values. Next time
	you'll find a complete configuration file on disk.

	You could use the above example as follows:
	\code
	SimpleCfg cfg( "filename" );
	cfg.load();   // The file is _not_ loaded in the constructor!

	cerr << "Windows " << cfg.getString( "operating system", "windows", "is good" )
		 << endl;
	cerr << "Unix " << cfg.getString( "operating system", "unix", "is old crap" )
		 << endl;

	cfg.save();   // If file did not exist it is created now.
	\endcode
	If the file could be read you would see the following output:
	\code
	Windows sux
	Unix rocks
	\endcode
	If the file could not be read:
	\code
	Windows is good
	Unix is old crap
	\endcode
	So always ensure your configuration files can be read :)
	\warning
	You can not have two or more groups with the same name. Same applies
	to keys within a group.

	SimpleCfg is licensed under GPL

	\author Matthias Toussaint, &copy; 2000 Matthias Toussaint
  */
class SimpleCfg
{
public:
  /// Construct empty configuration with filename
  SimpleCfg( const QString & );
  /** Set header comment for configuration file

	  \param comment This string will be printed at the beginning of the
	   file. MAKE SHURE IT CONTAINS COMMENT SIGNS AT THE VERY BEGINNING
	   OF EACH LINE (#)
	*/
  void												setComment( const QString & comment );

  /** Retrieve filename
	  \return filename
	*/
  QString											filename() const { return fname; }
  /** Rename file.
	  Does not load the new file. Next save and load will use this filename.
	  \param fn filename to be used from now on.
	*/
  void												setFilename( const QString & fn ) { fname = fn; }
  /** Load configuration file from disk.
	  \return true on success
	*/
  bool												load();
  /** Store configuration file to disk.
	  \return true on success
   */
  bool												save();
  /// Clear configuration
  void												clear();
  /** Remove all empty keys.
	  Calling this member all keys that have an empty value are removed.
	  This configuration file will look a bit nicer if you call this
	  before saving.
	  \warning It might fuck up your configuration if
	  a value was empty by intention and the default value for reading
	  is <b>not</b> empty. If you don't know what you are doing
	  don't call this member!
	*/
  void												removeEmpty();
  /** Remove key <i>key</i> from group <i>group</i>.
	  \param group Group to be used
	  \param key Key to be removed
	*/
  void												remove( const QString & group, const QString & key );
  /** Remove group <i>group</i>.
	  \param group Group to be removed
	*/
  void												remove( const QString & group );
  /** Check if configuration contains group named <i>group</i>.
	  \param group Name of group to look for
	  \returns true if found
	*/
  bool												contains( const QString & group );
  /** Check if configuration contains key in group named <i>group</i>.
	  \param group Name of group to look for
	  \param key Name of key to look for
	  \returns true if found
	*/
  bool												contains( const QString & group, const QString & key );
  /** Add empty group.
	  An empty group is added. This is needed for the load member. If
	  the group already exists this request is ignored.
	  \param group Group to be added
	*/
  void												add( const QString & group );
  /** Set integer value.
	  If the key does not exist it is created
	  \param group Group to be used
	  \param key Key to be searched for
	  \param val Value to be assigned to key
	*/
  void												setInt( const QString &group, const QString & key, int val );
  /** Retrieve integer value.
	  If the key does not exist it is created with the default
	  value.
	  \param group Group to be used
	  \param key Key to be searched for
	  \param def Default value to be returned if key not found
	  \returns Value of key or default value
	*/
  int												getInt( const QString &group, const QString & key, int def );
  /** Set double value.
	  If the key does not exist it is created
	  \param group Group to be used
	  \param key Key to be searched for
	  \param val Value to be assigned to key
	*/
  void												setDouble( const QString &group, const QString & key, double val );
  /** Retrieve double value.
	  If the key does not exist it is created with the default
	  value.
	  \param group Group to be used
	  \param key Key to be searched for
	  \param def Default value to be returned if key not found
	  \returns Value of key or default value
	*/
  double											getDouble( const QString &group, const QString & key, double def );
  /** Set string value.
	  If the key does not exist it is created
	  \param group Group to be used
	  \param key Key to be searched for
	  \param val Value to be assigned to key
	*/
  void												setString( const QString &group, const QString & key, const QString & val );
  /** Retrieve string value.
	  If the key does not exist it is created with the default
	  value.
	  \param group Group to be used
	  \param key Key to be searched for
	  \param def Default value to be returned if key not found
	  \returns Value of key or default value
	*/
  QString											getString( const QString &group, const QString & key, const QString & def );
  /** Set QRgb (unsigned long) value.
	  If the key does not exist it is created.
	  \param group Group to be used
	  \param key Key to be searched for
	  \param val Value to be assigned to key
	*/
  void												setRGB( const QString &group, const QString & key, QRgb val );
  /** Retrieve QRgb (unsigned long) value.
	  If the key does not exist it is created with the default
	  value.
	  \param group Group to be used
	  \param key Key to be searched for
	  \param def Default value to be returned if key not found
	  \returns Value of key or default value
	*/
  QRgb												getRGB( const QString &group, const QString & key, QRgb def );
  /** Set bool value.
	  If the key does not exist it is created.
	  \param group Group to be used
	  \param key Key to be searched for
	  \param val Value to be assigned to key
	*/
  void												setBool( const QString &group, const QString & key, bool val );
  /** Retrieve bool value.
	  If the key does not exist it is created with the default
	  value.
	  \param group Group to be used
	  \param key Key to be searched for
	  \param def Default value to be returned if key not found
	  \returns Value of key or default value
	*/
  bool												getBool( const QString &group, const QString & key, bool def );

  /** Access iterator over groups.
	  If there are no groups this member returns SimpleCfg::end().

	  If you want to iterate through the whole configuration you'll
	  need code like the following:
	  \code

	  SimpleCfg cfg( "filename" );
	  cfg.load();

	  QMap<QString,SimpleCfgGroup *>::Iterator group;

	  for (group=cfg.begin(); group != cfg.end(); ++group)
	  {
		// group.key() returns the group name as QString
		// group.data() returns a pointer to the group

		cerr << "[" << group.key().latin1() << "]" << endl;

		QMap<QString,QString>::Iterator key;

		for (key=group.data()->begin(); key != group.data()->end(); ++key)
		{
		  // key.key() returns the key name
		  // key.data() returns the value as QString

		  cerr << key.key().latin1() << " = " << key.data().latin1() << endl;
		}
	  }

	  \endcode
	  The above code will print the content of your configuration file (without
	  comments and alphabetically sorted) to stderr.
	  \see SimpleCfg::end()
	*/
  inline QMap<QString,SimpleCfgGroup *>::Iterator	begin() { return map.begin(); }
  /** Access end value for iterator.
	  \see SimpleCfg::begin()
	*/
  inline QMap<QString,SimpleCfgGroup *>::Iterator	end()   { return map.end(); }

protected:
  QMap<QString,SimpleCfgGroup *>					map;
  QString											fname;
  QString											curGroup;
  QString											m_comment;

};

#endif
