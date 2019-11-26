// local_notifications.cpp
//
// Process local notifications for ripcd(8)
//
//   (C) Copyright 2002-2019 Fred Gleason <fredg@paravelsystems.com>
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

#include <signal.h>

#include <rdapplication.h>
#include <rdconf.h>

#include "ripcd.h"

void MainObject::stationHeartbeatSendData()
{
  RDNotification *notify=new RDNotification(RDNotification::HeartbeatType,
					    RDNotification::AddAction,
					    rda->station()->name());
  ripcd_notification_mcaster->
    send(notify->write(),rda->system()->notificationAddress(),
	 RD_NOTIFICATION_PORT);
  delete notify;

  int msecs=RIPCD_STATION_HEARTBEAT_MIN_SEND_INTERVAL+
    (RIPCD_STATION_HEARTBEAT_MAX_SEND_INTERVAL-
     RIPCD_STATION_HEARTBEAT_MIN_SEND_INTERVAL)*random()/RAND_MAX;
  rda->syslog(LOG_DEBUG,"hearbeat holdoff: %d msec",msecs);
  ripcd_station_heartbeat_send_timer->start(msecs);
}


void MainObject::stationHeartbeatData()
{
  QStringList removed_hostnames;
  QDateTime deadline=QDateTime::currentDateTime().
    addMSecs(-RIPCD_STATION_HEARTBEAT_DEADLINE_INTERVAL);

  for(QMap<QString,QDateTime>::const_iterator it=
	ripcd_station_heartbeats.begin();
      it!=ripcd_station_heartbeats.end();it++) {
    if(it.value()<deadline) {
      BroadcastCommand("HS "+it.key()+" 0!");
      removed_hostnames.push_back(it.key());
    }
  }
  for(int i=0;i<removed_hostnames.size();i++) {
    ripcd_station_heartbeats.remove(removed_hostnames.at(i));
    BroadcastCommand("HS "+removed_hostnames.at(i)+" 0!");
  }
  ripcd_station_heartbeat_timer->start(RIPCD_STATION_HEARTBEAT_CHECK_INTERVAL);
}


void MainObject::RunLocalNotifications(RDNotification *notify)
{
  if(notify->type()==RDNotification::HeartbeatType) {
    if(notify->action()==RDNotification::AddAction) {
      if(!ripcd_station_heartbeats.contains(notify->id().toString())) {
	BroadcastCommand("HS "+notify->id().toString()+" 1!");
      }
      ripcd_station_heartbeats[notify->id().toString()]=
	QDateTime::currentDateTime();
    }
    if(notify->action()==RDNotification::DeleteAction) {
      if(ripcd_station_heartbeats.contains(notify->id().toString())) {
	BroadcastCommand("HS "+notify->id().toString()+" 0!");
	ripcd_station_heartbeats.remove(notify->id().toString());
      }
    }
  }

  if((notify->type()==RDNotification::DropboxType)&&
     (notify->id().toString()==rda->config()->stationName())) {
    pid_t pid=RDGetPid(QString(RD_PID_DIR)+"/rdservice.pid");
    if(pid>0) {
      kill(pid,SIGUSR1);
    }
  }
}


void MainObject::SendExitingNotification()
{
  RDNotification *notify=new RDNotification(RDNotification::HeartbeatType,
					    RDNotification::DeleteAction,
					    rda->station()->name());
  ripcd_notification_mcaster->
    send(notify->write(),rda->system()->notificationAddress(),
	 RD_NOTIFICATION_PORT);
  delete notify;

  ripcd_station_heartbeat_send_timer->stop();
}


void MainObject::SendHostStatusList(int conn_id)
{
  QStringList hosts;
  hosts.push_back(rda->station()->name());  // Always include ourselves
  for(QMap<QString,QDateTime>::const_iterator it=
	ripcd_station_heartbeats.begin();
      it!=ripcd_station_heartbeats.end();it++) {
    hosts.push_back(it.key());
  }
  EchoCommand(conn_id,"RH "+hosts.join(",")+"!");
}
