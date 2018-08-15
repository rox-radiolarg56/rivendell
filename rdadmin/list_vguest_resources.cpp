// list_vguest_resources.cpp
//
// List vGuest Resources.
//
//   (C) Copyright 2002-2018 Fred Gleason <fredg@paravelsystems.com>
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

#include <qdialog.h>
#include <qstring.h>
#include <q3textedit.h>
#include <qpainter.h>
#include <qmessagebox.h>

#include <rd.h>
#include <rddb.h>
#include <rdescape_string.h>

#include "edit_user.h"
#include "edit_vguest_resource.h"
#include "list_vguest_resources.h"

ListVguestResources::ListVguestResources(RDMatrix *matrix,
					 RDMatrix::VguestType type,int size,
					 QWidget *parent)
  : QDialog(parent)
{
  setModal(true);

  QString sql;
  QString str;

  list_matrix=matrix;
  list_type=type;
  list_size=size;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  //
  // Create Fonts
  //
  QFont bold_font=QFont("Helvetica",12,QFont::Bold);
  bold_font.setPixelSize(12);
  QFont font=QFont("Helvetica",12,QFont::Normal);
  font.setPixelSize(12);

  //
  // Resources List Box
  //
  list_list_view=new Q3ListView(this);
  list_list_view->
    setGeometry(10,24,sizeHint().width()-20,sizeHint().height()-94);
  QLabel *label=new QLabel(list_list_view,list_table,this);
  label->setFont(bold_font);
  label->setGeometry(14,5,85,19);
  list_list_view->setAllColumnsShowFocus(true);
  list_list_view->setItemMargin(5);
  switch(list_type) {
  case RDMatrix::VguestTypeRelay:
    setWindowTitle("RDAdmin - "+tr("vGuest Switches"));
    list_list_view->addColumn(tr("GPIO LINE"));
    break;

  case RDMatrix::VguestTypeDisplay:
    setWindowTitle("RDAdmin - "+tr("vGuest Displays"));
    list_list_view->addColumn(tr("DISPLAY"));
    break;
  }
  list_list_view->setColumnAlignment(0,Qt::AlignHCenter);
  list_list_view->addColumn(tr("ENGINE (Hex)"));
  list_list_view->setColumnAlignment(1,Qt::AlignHCenter);
  list_list_view->addColumn(tr("DEVICE (Hex)"));
  list_list_view->setColumnAlignment(2,Qt::AlignHCenter);
  switch(list_type) {
  case RDMatrix::VguestTypeRelay:
    list_list_view->addColumn(tr("SURFACE (Hex)"));
    list_list_view->setColumnAlignment(3,Qt::AlignHCenter);
    list_list_view->addColumn(tr("BUS/RELAY (Hex)"));
    list_list_view->setColumnAlignment(4,Qt::AlignHCenter);
    break;

  case RDMatrix::VguestTypeDisplay:
    list_list_view->addColumn(tr("SURFACE (Hex)"));
    list_list_view->setColumnAlignment(3,Qt::AlignHCenter);
    break;
  }
  connect(list_list_view,
	  SIGNAL(doubleClicked(Q3ListViewItem *,const QPoint &,int)),
	  this,
	  SLOT(doubleClickedData(Q3ListViewItem *,const QPoint &,int)));

  //
  //  Edit Button
  //
  QPushButton *button=new QPushButton(this);
  button->setGeometry(10,sizeHint().height()-60,80,50);
  button->setFont(bold_font);
  button->setText(tr("&Edit"));
  connect(button,SIGNAL(clicked()),this,SLOT(editData()));

  //
  //  Ok Button
  //
  button=new QPushButton(this);
  button->setGeometry(sizeHint().width()-180,sizeHint().height()-60,80,50);
  button->setDefault(true);
  button->setFont(bold_font);
  button->setText(tr("&OK"));
  connect(button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  //  Cancel Button
  //
  button=new QPushButton(this);
  button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,
			     80,50);
  button->setFont(bold_font);
  button->setText(tr("&Cancel"));
  connect(button,SIGNAL(clicked()),this,SLOT(cancelData()));

  //
  // Load Values
  //
  RefreshList();
}


QSize ListVguestResources::sizeHint() const
{
  return QSize(400,250);
} 


QSizePolicy ListVguestResources::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void ListVguestResources::editData()
{
  int engine_num=-1;
  int device_num=-1;
  int surface_num=-1;
  int relay_num=-1;

  Q3ListViewItem *item=list_list_view->selectedItem();
  if(item==NULL) {
    return;
  }
  if(!item->text(1).isEmpty()) {
    engine_num=item->text(1).toInt(NULL,16);
  }
  if(!item->text(2).isEmpty()) {
    device_num=item->text(2).toInt(NULL,16);
  }
  if(!item->text(3).isEmpty()) {
    surface_num=item->text(3).toInt(NULL,16);
  }
  switch(list_type) {
      case RDMatrix::VguestTypeRelay:
	if(!item->text(4).isEmpty()) {
	  relay_num=item->text(4).toInt(NULL,16);
	}
	break;
	
      case RDMatrix::VguestTypeDisplay:
	break;
  }
  EditVguestResource *dialog=
    new EditVguestResource(list_type,&engine_num,&device_num,&surface_num,
			   &relay_num,this);
  if(dialog->exec()==0) {
    if(engine_num>=0) {
      item->setText(1,QString().sprintf("%04X",engine_num));
    }
    else {
      item->setText(1,"");
    }
    if(device_num>=0) {
      item->setText(2,QString().sprintf("%04X",device_num));
    }
    else {
      item->setText(2,"");
    }
    if(surface_num>=0) {
      item->setText(3,QString().sprintf("%04X",surface_num));
    }
    else {
      item->setText(3,"");
    }
    switch(list_type) {
	case RDMatrix::VguestTypeRelay:
	  if(relay_num>=0) {
	    item->setText(4,QString().sprintf("%04X",relay_num));
	  }
	  else {
	    item->setText(4,"");
	  }
	  break;

	case RDMatrix::VguestTypeDisplay:
	  break;
    }
  }
  delete dialog;
}


void ListVguestResources::doubleClickedData(Q3ListViewItem *item,
					    const QPoint &pt,int col)
{
  editData();
}


void ListVguestResources::okData()
{
  QString sql;
  RDSqlQuery *q;
  int engine_num=-1;
  int device_num=-1;
  int surface_num=-1;
  int relay_num=-1;

  Q3ListViewItem *item=list_list_view->firstChild();
  while(item!=NULL) {
    engine_num=-1;
    device_num=-1;
    surface_num=-1;
    relay_num=-1;
    if(!item->text(1).isEmpty()) {
      engine_num=item->text(1).toInt(NULL,16);
    }
    if(!item->text(2).isEmpty()) {
      device_num=item->text(2).toInt(NULL,16);
    }
    if(!item->text(3).isEmpty()) {
      surface_num=item->text(3).toInt(NULL,16);
    }
    switch(list_type) {
	case RDMatrix::VguestTypeRelay:
	  if(!item->text(4).isEmpty()) {
	    relay_num=item->text(4).toInt(NULL,16);
	  }
	  break;

	case RDMatrix::VguestTypeDisplay:
	  break;
    }
    sql=QString("select ID from VGUEST_RESOURCES where ")+
      "(STATION_NAME=\""+RDEscapeString(list_matrix->station())+"\")&&"+
      QString().sprintf("(MATRIX_NUM=%d)&&",list_matrix->matrix())+
      QString().sprintf("(VGUEST_TYPE=%d)&&",list_type)+
      QString().sprintf("(NUMBER=%d)",item->text(0).toInt());
    q=new RDSqlQuery(sql);
    if(q->first()) {
      sql=QString("update VGUEST_RESOURCES set ")+
	QString().sprintf("ENGINE_NUM=%d,",engine_num)+
	QString().sprintf("DEVICE_NUM=%d,",device_num)+
	QString().sprintf("SURFACE_NUM=%d,",surface_num)+
	QString().sprintf("RELAY_NUM=%d	where ",relay_num)+
	"(STATION_NAME=\""+RDEscapeString(list_matrix->station())+"\")&&"+
	QString().sprintf("(MATRIX_NUM=%d)&&",list_matrix->matrix())+
	QString().sprintf("(VGUEST_TYPE=%d)&&",list_type)+
	QString().sprintf("(NUMBER=%d)",item->text(0).toInt());
    }
    else {
      sql=QString("insert into VGUEST_RESOURCES set ")+
	"STATION_NAME=\""+RDEscapeString(list_matrix->station())+"\","+
	QString().sprintf("MATRIX_NUM=%d,",list_matrix->matrix())+
	QString().sprintf("VGUEST_TYPE=%d,",list_type)+
	QString().sprintf("NUMBER=%d,",item->text(0).toInt())+
	QString().sprintf("ENGINE_NUM=%d,",engine_num)+
	QString().sprintf("DEVICE_NUM=%d,",device_num)+
	QString().sprintf("SURFACE_NUM=%d,",surface_num)+
	QString().sprintf("RELAY_NUM=%d",relay_num);
    }
    delete q;
    q=new RDSqlQuery(sql);
    delete q;
    item=item->nextSibling();
  }
  done(0);
}


void ListVguestResources::cancelData()
{
  done(-1);
}


void ListVguestResources::RefreshList()
{
  QString sql;
  RDSqlQuery *q;
  Q3ListViewItem *item;
  int n=1;

  sql=QString("select ")+
    "NUMBER,"+       // 00
    "ENGINE_NUM,"+   // 01
    "DEVICE_NUM,"+   // 02
    "SURFACE_NUM,"+  // 03
    "RELAY_NUM,"+    // 04
    "BUSS_NUM "+     // 05
    "from VGUEST_RESOURCES where "+
    "(STATION_NAME=\""+RDEscapeString(list_matrix->station())+"\")&&"+
    QString().sprintf("(MATRIX_NUM=%d)&&",list_matrix->matrix())+
    QString().sprintf("(VGUEST_TYPE=%d) ",list_type)+
    "order by NUMBER";
  q=new RDSqlQuery(sql);
  list_list_view->clear();
  while(q->next()) {
    while(q->value(0).toInt()>n) {
      item=new Q3ListViewItem(list_list_view);
      item->setText(0,QString().sprintf("%03d",n++));
    }
    item=new Q3ListViewItem(list_list_view);
    item->setText(0,QString().sprintf("%03d",q->value(0).toInt()));
    if(q->value(1).toInt()>=0) {
      item->setText(1,QString().sprintf("%04X",q->value(1).toInt()));
    }
    if(q->value(2).toInt()>=0) {
      item->setText(2,QString().sprintf("%04X",q->value(2).toInt()));
    }
    if(q->value(3).toInt()>=0) {
      item->setText(3,QString().sprintf("%04X",q->value(3).toInt()));
    }
    switch(list_type) {
	case RDMatrix::VguestTypeRelay:
	  if(q->value(4).toInt()>=0) {
	    item->setText(4,QString().sprintf("%04X",q->value(4).toInt()));
	  }
	  break;

	case RDMatrix::VguestTypeDisplay:
	  break;
    }
    n++;
  }
  for(int i=n;i<(list_size+1);i++) {
    item=new Q3ListViewItem(list_list_view);
    item->setText(0,QString().sprintf("%03d",i));
  } 
  delete q;
}
