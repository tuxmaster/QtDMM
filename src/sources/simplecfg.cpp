//======================================================================
// File:		simplecfg.cpp
// Author:	Matthias Toussaint
// Created:	Mon Okt 30 22:06:04 CET 2000
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

#include <simplecfg.h>
#include <qfile.h>
#include <q3textstream.h>
#include <qstringlist.h>

SimpleCfgGroup::SimpleCfgGroup( const QString & name ) :
  groupName( name )
{
}

SimpleCfgGroup::~SimpleCfgGroup()
{
}

void
SimpleCfgGroup::remove( const QString & key )
{
  if (map.contains(key))
  {
    map.remove( key );
  }
}

bool
SimpleCfgGroup::contains( const QString & key )
{
  return map.contains( key );
}

void
SimpleCfgGroup::setString( const QString & key, const QString & val )
{
  map.replace( key, val );
}

QString
SimpleCfgGroup::getString( const QString & key, const QString & def )
{
  if (map.contains( key )) 
  {
    return map[key];
  }
  else
  {
    setString( key, def );
  }
  
  return def;
}

void
SimpleCfgGroup::setInt( const QString & key, int val )
{
  QString tmp;
  tmp.setNum( val );
  setString( key, tmp );
}

int
SimpleCfgGroup::getInt( const QString & key, int def )
{
  if (map.contains( key )) 
  {
    return map[key].toInt();
  }
  else
  {
    setInt( key, def );
  }
  
  return def;
}

void
SimpleCfgGroup::setDouble( const QString & key, double val )
{
  QString tmp;
  tmp.setNum( val );
  setString( key, tmp );
}

double
SimpleCfgGroup::getDouble( const QString & key, double def )
{
  if (map.contains( key )) 
  {
    return map[key].toDouble();
  }
  else
  {
    setDouble( key, def );
  }
  
  return def;
}

void
SimpleCfgGroup::setRGB( const QString & key, QRgb val )
{
  QString tmp;
  
  tmp.setNum( val );
  setString( key, tmp );
}

QRgb
SimpleCfgGroup::getRGB( const QString & key, QRgb def )
{
  if (map.contains( key )) 
  {
    return map[key].toULong();
  }
  else
  {
    setRGB( key, def );
  }
  
  return def;
}

void
SimpleCfgGroup::setBool( const QString & key, bool val )
{
  setString( key, val ? "true" : "false" );
}

bool
SimpleCfgGroup::getBool( const QString & key, bool def )
{
  if (map.contains( key )) 
  {
    QString val = map[key].lower();
    
    if (val == "true" || val == "1" || val == "yes" || val == "on") return true;
    return false;
  }
  else
  {
    setBool( key, def );
  }
  
  return def;
}

SimpleCfg::SimpleCfg( const QString & name ) :
    fname( name )
{
}

SimpleCfg::~SimpleCfg()
{
  clear();
}

void
SimpleCfg::remove( const QString & group, const QString & key )
{
  if (map.contains(group))
  {
    map[group]->remove( key );
  }
}

void
SimpleCfg::remove( const QString & group )
{
  if (map.contains(group))
  {
    delete map[group];
    map.remove( group );
  }
}

bool
SimpleCfg::load()
{
  QFile file( fname );
  
  QString group;
  
  if (file.open( QIODevice::ReadOnly ))
  {
    clear();
    
    char *buf = new char [1000];
    
    while (file.readLine( buf, 999 ) != -1)
    {
      QString line = QString( buf ).stripWhiteSpace();
      line = line.simplifyWhiteSpace();
      
      // Check if comment (good old assembler line comments:)
      //
      if (line.left(1) != "#" && line.left(1) != ";")
      {
        // Check for group
        //
        if (line.left(1) == "[")
        {
          group = line.mid( 1, line.length()-2 );
          add( group );
        }
        else
        {
          // Search '='
          //
          int pos;
        
          if ((pos = line.find( '=' )) != -1)
          {
            QString key = line.left( pos-1 );
            QString val;
            
            if ((int)line.length()-pos-2 > 0)
              val = line.right( line.length()-pos-2 );
            else
              val = "";
            
            setString( group, key, val );
          }
        }
      }
    }
    
    file.close();
  } 
  else
  {
    return false;
  }

  return true;
}

bool
SimpleCfg::save()
{
  QFile file( fname );
  
  if (file.open( QIODevice::WriteOnly ))
  {
    Q3TextStream s(&file);
    
    s << m_comment;
    
    QMap<QString,SimpleCfgGroup *>::Iterator it;
        
    for (it=map.begin(); it != map.end(); ++it)
    {
      s << "[";
      s << it.data()->name();
      s << "]\n";
      
      QMap<QString,QString>::Iterator itg;
      
      for (itg=it.data()->begin(); itg != it.data()->end(); ++itg)
      {
        s << itg.key();
        s <<  " = ";
        s << itg.data();
        s << "\n";
      }
      
      s << "\n";
    }
    file.close();
  }
  else
  {
    return false;
  }
  
  return true;
}

bool
SimpleCfg::contains( const QString & group )
{
  return map.contains( group );
}

bool
SimpleCfg::contains( const QString & group, const QString & key )
{
  if (map.contains( group ))
  {
    return map[group]->contains( key );
  }
  
  return false;
}

void
SimpleCfg::add( const QString & group )
{
  if (map.contains( group )) return;
  
  SimpleCfgGroup *g = new SimpleCfgGroup( group );
  map.insert( group, g ); 
}

void
SimpleCfg::setString( const QString & group, const QString & key, 
                      const QString & val )
{
  if (map.contains( group ))
  {
    map[group]->setString( key, val );
  }
  else
  {
    SimpleCfgGroup *g = new SimpleCfgGroup( group );
    g->setString( key, val );
    
    map.insert( group, g );
  }    
}

QString
SimpleCfg::getString( const QString & group, const QString & key, 
                      const QString & def )
{
  if (map.contains( group )) 
  {
    SimpleCfgGroup *g = map[group];
    
    return g->getString( key, def );
  }
  else
  {
    SimpleCfgGroup *g = new SimpleCfgGroup( group );
    g->setString( key, def );
    
    map.insert( group, g );
  }
  
  return def;
}

void
SimpleCfg::setInt( const QString & group, const QString & key, 
                   int val )
{
  QString tmp;
  tmp.setNum( val );
  setString( group, key, tmp );
}

int
SimpleCfg::getInt( const QString & group, const QString & key, 
                   int def )
{
  if (map.contains( group )) 
  {
    SimpleCfgGroup *g = map[group];
    
    return g->getInt( key, def );
  }
  else
  {
    SimpleCfgGroup *g = new SimpleCfgGroup( group );
    g->setInt( key, def );
    
    map.insert( group, g );
  }
  
  return def;
}

void
SimpleCfg::setDouble( const QString & group, const QString & key, 
                      double val )
{
  QString tmp;
  tmp.setNum( val );
  setString( group, key, tmp );
}

double
SimpleCfg::getDouble( const QString & group, const QString & key, 
                      double def )
{
  if (map.contains( group )) 
  {
    SimpleCfgGroup *g = map[group];
    
    return g->getDouble( key, def );
  }
  else
  {
    SimpleCfgGroup *g = new SimpleCfgGroup( group );
    g->setDouble( key, def );
    
    map.insert( group, g );
  }
  
  return def;
}

void
SimpleCfg::setRGB( const QString & group, const QString & key, 
                   QRgb val )
{
  QString tmp;
  tmp.setNum( val );
  setString( group, key, tmp );
}

QRgb
SimpleCfg::getRGB( const QString & group, const QString & key, 
                   QRgb def )
{
  if (map.contains( group )) 
  {
    SimpleCfgGroup *g = map[group];
    
    return g->getRGB( key, def );
  }
  else
  {
    SimpleCfgGroup *g = new SimpleCfgGroup( group );
    g->setRGB( key, def );
    
    map.insert( group, g );
  }
  
  return def;
}

void
SimpleCfg::setBool( const QString & group, const QString & key, 
                    bool val )
{
  setString( group, key, val ? "true" : "false" );
}

bool
SimpleCfg::getBool( const QString & group, const QString & key, 
                    bool def )
{
  if (map.contains( group )) 
  {
    SimpleCfgGroup *g = map[group];
    
    return g->getBool( key, def );
  }
  else
  {
    SimpleCfgGroup *g = new SimpleCfgGroup( group );
    g->setBool( key, def );
    
    map.insert( group, g );
  }
  
  return def;
}

void
SimpleCfg::clear()
{
  QMap<QString,SimpleCfgGroup *>::Iterator it;

  for (it=map.begin(); it != map.end(); ++it)
  {
    delete it.data();
  }
  
  map.clear();
}

void
SimpleCfg::removeEmpty()
{
  QMap<QString,SimpleCfgGroup *>::Iterator git;

  QStringList list;
  
  for (git=map.begin(); git != map.end(); ++git)
  {
    QMap<QString,QString>::Iterator kit;
   
    list.clear();
    
    for (kit=git.data()->begin(); kit != git.data()->end(); ++kit)
    {
      if (kit.data().isEmpty())
      {
        // value is empty -> shedule for removal
        list.append( kit.key() );
      }
    } 
    
    // Now remove empty keys
    for (QStringList::Iterator rit=list.begin(); rit != list.end(); ++rit ) 
    {
      git.data()->remove( *rit );
    }
  }
}
  
void
SimpleCfg::setComment( const QString & comment )
{
  m_comment = comment;
}
