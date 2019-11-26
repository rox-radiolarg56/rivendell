// rdnotification.cpp
//
// A container class for a Rivendell Notification message.
//
//   (C) Copyright 2018-2019 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <QStringList>

#include "rdnotification.h"
#include "rdstringlist.h"

RDNotification::RDNotification(Type type,Action action,const QVariant &id)
{
  notify_type=type;
  notify_action=action;
  notify_ids.push_back(id);
}


RDNotification::RDNotification()
{
  notify_type=RDNotification::NullType;
  notify_action=RDNotification::NoAction;
  notify_ids.push_back(QVariant());
}


RDNotification::Type RDNotification::type() const
{
  return notify_type;
}


void RDNotification::setType(RDNotification::Type type)
{
  notify_type=type;
}


RDNotification::Action RDNotification::action() const
{
  return notify_action;
}


void RDNotification::setAction(RDNotification::Action action)
{
  notify_action=action;
}


QVariant RDNotification::id() const
{
  return notify_ids.at(0);
}


QList<QVariant> RDNotification::ids() const
{
  return notify_ids;
}


void RDNotification::setId(const QVariant &id)
{
  notify_ids.clear();
  notify_ids.push_back(id);
}


void RDNotification::setIds(const QList<QVariant> &ids)
{
  notify_ids=ids;
}


bool RDNotification::read(const QString &str)
{
  notify_type=RDNotification::NullType;
  notify_action=RDNotification::NoAction;
  notify_ids.clear();

  RDStringList args=RDStringList().split(' ',str,QChar('\\'));
  if(args.size()==4) {
    if(args[0]!="NOTIFY") {
      notify_ids.push_back(QVariant());
      return false;
    }
    for(int i=0;i<RDNotification::LastType;i++) {
      RDNotification::Type type=(RDNotification::Type)i;
      if(args[1]==RDNotification::typeString(type)) {
	notify_type=type;
	switch(type) {
	case RDNotification::CartType:
	case RDNotification::CatchEventType:
	case RDNotification::PypadType:
	  notify_ids.push_back(QVariant(args.at(3).toUInt()));
	  break;

	case RDNotification::DropboxType:
	case RDNotification::HeartbeatType:
	case RDNotification::LogType:
	  notify_ids.push_back(QVariant(args[3].replace("\\ "," ")));
	  break;

	case RDNotification::CatchDeckStatusType:
	  // FIXME!
	  //notify_ids.push_back(QVariant(args[3]));
	  break;

	case RDNotification::NullType:
	case RDNotification::LastType:
	  break;
	}
      }
    }
    if(notify_type==RDNotification::NullType) {
      notify_ids.push_back(QVariant());
      return false;
    }
    for(int i=0;i<RDNotification::LastAction;i++) {
      RDNotification::Action action=(RDNotification::Action)i;
      if(args[2]==RDNotification::actionString(action)) {
	notify_action=action;
      }
    }
    if(notify_action==RDNotification::NoAction) {
      notify_ids.push_back(QVariant());
      return false;
    }
  }
  return true;
}


QString RDNotification::write() const
{
  QString ret="";

  ret+="NOTIFY ";
  ret+=RDNotification::typeString(notify_type)+" ";
  ret+=RDNotification::actionString(notify_action)+" ";
  switch(notify_type) {
  case RDNotification::CartType: 
  case RDNotification::CatchEventType: 
  case RDNotification::PypadType: 
    ret+=QString().sprintf("%u",notify_ids.at(0).toUInt());
    break;

  case RDNotification::DropboxType: 
  case RDNotification::HeartbeatType: 
  case RDNotification::LogType: 
    ret+=notify_ids.at(0).toString().replace(" ","\\ ");
    break;

  case RDNotification::CatchDeckStatusType: 
    // FIXME!
    //ret+=notify_id.toString();
    break;

  case RDNotification::NullType:
  case RDNotification::LastType:
    break;
  }
  return ret;
}


QString RDNotification::typeString(RDNotification::Type type)
{
  QString ret="UNKNOWN";

  switch(type) {
  case RDNotification::CartType:
    ret="CART";
    break;

  case RDNotification::LogType:
    ret="LOG";
    break;

  case RDNotification::PypadType:
    ret="PYPAD";
    break;

  case RDNotification::DropboxType:
    ret="DROPBOX";
    break;

  case RDNotification::CatchEventType:
    ret="CATCH_EVENT";
    break;

  case RDNotification::CatchDeckStatusType:
    ret="CATCH_DECK_STATUS";
    break;

  case RDNotification::HeartbeatType:
    ret="HEARTBEAT";
    break;

  case RDNotification::NullType:
  case RDNotification::LastType:
    break;
  }
  return ret;
}


QString RDNotification::actionString(Action action)
{
  QString ret="UNKNOWN";

  switch(action) {
  case RDNotification::AddAction:
    ret="ADD";
    break;

  case RDNotification::DeleteAction:
    ret="DELETE";
    break;

  case RDNotification::ModifyAction:
    ret="MODIFY";
    break;

  case RDNotification::NoAction:
  case RDNotification::LastAction:
    break;
  }
  return ret;
}
