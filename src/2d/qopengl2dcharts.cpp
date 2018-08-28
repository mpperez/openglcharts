/* (C) Copyright 2017 mpperez
 Author: mpperez
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library;
*/
/*! \file
	\brief qopengl2dcharts.cpp file.
	\date 05-2016
	\author Marcos Perez*/

#pragma message("Using QT interface")

#include "qopengl2dcharts.h"

#include "qfiledialog.h"
#include <QMenu>
#include <QApplication>

#include <stdio.h>
#include <string.h>
#ifndef ABS
#define ABS(a) ((a) > 0 ? (a) : -(a))
#endif
#ifndef LOG10
#define LOG10 log((double)10)
#endif

int Opengl2DCharts::m_ndialogs=0;

Opengl2DCharts::Opengl2DCharts( QWidget *pParent)
		:  QGLWidget(QGLFormat(QGL::SampleBuffers),(QWidget*) pParent)
{
  CommonInit();
  connect(this,SIGNAL(SGRePaint()),this,SLOT(OnRePaint()))	;

	m_width=this->width();
  m_height=this->height();
  m_timer_paint =new QTimer(this);

	connect(m_timer_paint, SIGNAL(timeout()), this, SLOT(OnRePaint()));
  show();

}

Opengl2DCharts::~Opengl2DCharts()
{
	Opengl2DCharts::m_ndialogs--;
}

bool Opengl2DCharts::CreateAndPosition( void *parent,int x,int y,int width,int height,int popup,int mspaint)
{
	QRect pos(x,y,width,height);

	m_pparent=(QWidget*)parent;
  m_width=pos.width();
  m_height=pos.height();

	setContextMenuPolicy(Qt::CustomContextMenu) ;

	InitGraph();
	SetPopupDialog(popup);
  setGeometry(pos);

	if(mspaint!=0)
	{
		m_timer_paint->start(mspaint);
	}
  m_initialized=1;
  return 1;
}

void Opengl2DCharts::initializeGL()
{
  glClearColor(1.0f,1.0f,1.0f, 0.0f);
  setAutoFillBackground(false);
	glViewport(0,0,width(), height());
  glMatrixMode(GL_PROJECTION);

	glClearDepth(1.0f);
  glEnable( GL_DEPTH_TEST );
	glEnable( GL_TEXTURE_2D );
  glLineWidth( 2.0f );
  glPointSize( 1.0f );
}

void Opengl2DCharts::CreaTooltipLX()
{

}

void Opengl2DCharts::ShowDialog()
{
	show();
	move(m_posdialogo.x(),m_posdialogo.y());
}

///
///DIALOG VISIBILITY
///
void Opengl2DCharts::SetPopupDialog(int popup)
{
	if(popup)
	{
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
		setParent(NULL);
#endif
		show();
	 // RePaint();
		m_child=0;
	}
	else
	{
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
		setWindowFlags( Qt::FramelessWindowHint);
		show();
		setParent(m_pparent);
#endif
		show();
	 // RePaint();
		m_child=1;
	}
}

void Opengl2DCharts::ChangeDlgBorder()
{
	if(m_child==1)
	 return; //only if in desktop
/*	if(windowFlags() & Qt::FramelessWindowHint)
		setWindowFlags(!Qt::FramelessWindowHint);
	else
    setWindowFlags( Qt::FramelessWindowHint);*/
	show();
}

///
/// AUX MENUS
///
void Opengl2DCharts::ShowContextMenu(const CHARTS_POINTD &pos)
{
	// for most widgets
	QPoint globalPos = this->mapToGlobal(QPoint(pos.x,pos.y));

	int iLine;
	CXYLineGL *pt;
	QMenu myMenu;
	pt=GetLines();
	int nlin=GetnLines();
	for (iLine=0;pt!=NULL;pt=pt->nextLine,iLine++)
	{
		QAction *act=myMenu.addAction(pt->lineName.c_str());

		act->setCheckable ( 1 );
		act->setChecked( pt->visible?1:0 );
		act->setData(iLine);
	}
	myMenu.addSeparator();
	QAction *act=myMenu.addAction("Autozoom");
	act->setData(1+nlin);
	act=myMenu.addAction("Guardar");
	act->setData(2+nlin);

	QAction* selectedItem = myMenu.exec(globalPos);

	if (selectedItem)
	{
		int value = selectedItem->data().toInt();
		if(value<nlin)
			GetLine(value)->visible=!GetLine(value)->visible;
		else
		{
			if(value==nlin+1)
				AutoZoom();
			if(value==nlin+2)
				SaveAs();
		}
	}
}

///
///   SAVING IMAGES,GRAFS,DAT....
///
bool Opengl2DCharts::SaveAs()
{
	QString filtro= "Imagen *.bmp (*.bmp);;Grafica *.graf (*.graf);;Datos *.dat (*.dat)";
	QFileDialog dialog(this, "Guardar", QDir::currentPath(),filtro);
	QRegExp filter_regex(QLatin1String("(?:^\\*\\.(?!.*\\()|\\(\\*\\.)(\\w+)"));
	QStringList filters = filtro.split(QLatin1String(";;"));

	dialog.setAcceptMode(QFileDialog::AcceptSave);

	if (dialog.exec() == QDialog::Accepted)
	{
		QString file_name = dialog.selectedFiles().first();
		QFileInfo info(file_name);
		if (info.suffix().isEmpty() && !dialog.selectedNameFilter().isEmpty())
		{
			if (filter_regex.indexIn(dialog.selectedNameFilter()) != -1)
			{
				QString extension = filter_regex.cap(1);
				file_name += QLatin1String(".") + extension;
			}
		}

		string direccion=file_name.toStdString ();

		if(file_name.indexOf(".bmp")!=-1)
		{
			SaveImage(direccion);
		}

		if(file_name.indexOf(".graf")!=-1)
		{
			SaveCurrent(direccion);
		}

		if(file_name.indexOf(".dat")!=-1)
		{
			SaveDat(direccion);
		}
	}
	return 1;
}

void Opengl2DCharts::SaveImage( string direccion)
{
	GLubyte *raw=new  GLubyte [m_width*m_height*4*sizeof(char)];

	GetScreenShot(raw,0,0,m_width,m_height,4*8);
	SaveBitmapToFile( (GLubyte*)raw, m_width,m_height,4*8,direccion );
	delete []raw;
}

void Opengl2DCharts::SaveBitmapToFile( GLubyte* pBitmapBits, long lWidth, long lHeight,unsigned int wBitsPerPixel, string lpszFileName )
{
	QImage imag;

	if(wBitsPerPixel==32)
	{
		imag=QImage(pBitmapBits,lWidth,lHeight,QImage::Format_RGB32);
		imag.mirrored(0,1).save(QString(lpszFileName.c_str()));
	}
}

///
///EVENTS
///
void Opengl2DCharts::OnRePaint()
{
  if(this->isMinimized()||!m_initialized)
    return;

	glDraw();
}

void Opengl2DCharts::paintGL()//from glDraw();
{
	if(!TryBlockPaint())
	{
		printf("painteventwhilepainting\n");
		return;
	}

	if(RedrawChart())//from oglgraph
  {
    //glFlush();
  }
	unBlockPaint();
}

///
///DIALOG
///
void Opengl2DCharts::wheelEvent(QWheelEvent *event)
{
	CHARTS_POINTD point(event->pos().x(),event->pos().y());
  OnMouseWheel(point,event->modifiers(),event->delta());
}

void Opengl2DCharts::keyPressEvent(QKeyEvent* e)
{
   switch(e->key())
   {
    case Qt::Key_Left:
     OnKeyPress(k_left);
     break;
    case Qt::Key_Right:
     OnKeyPress(k_right);
      break;
    case Qt::Key_Up:
     OnKeyPress(k_up);
      break;
    case Qt::Key_Down:
     OnKeyPress(k_down);
      break;
   }
}

void Opengl2DCharts::mouseMoveEvent(QMouseEvent *event)
{
	CHARTS_POINTD point=GetMousePoint();

	OnMouseMove(point,event->modifiers()| event->buttons());
}

void Opengl2DCharts::mousePressEvent(QMouseEvent *event)
{
	CHARTS_POINTD point(event->x(),event->y());

	if(event->buttons() & Qt::LeftButton)
  {
    OnLMousePress(point,event->modifiers()|event->buttons());
    if(event->modifiers()& Qt::ControlModifier)
    ChangeDlgBorder();
  }

  if(event->buttons() & Qt::MiddleButton)
  {
    OnMMousePress(point,event->modifiers()|event->buttons());
    setCursor(Qt::CrossCursor);
  }

  if(event->buttons() & Qt::RightButton)
  {
    OnRMousePress(point,event->modifiers()|event->buttons());
    ShowContextMenu(point);
  }
}

void Opengl2DCharts::mouseReleaseEvent(QMouseEvent *event)
{
	CHARTS_POINTD point(event->x(),event->y());

	if(event->button() == Qt::LeftButton)
	{
		OnLMouseRelease(point,event->modifiers()|event->buttons());
	}

  if(event->button() == Qt::MiddleButton)
  {
    OnMMouseRelease(point,event->modifiers()|event->buttons());
    setCursor(Qt::ArrowCursor);
  }

  if(event->button() == Qt::RightButton)
	{
		OnRMouseRelease(point,event->modifiers()|event->buttons());
	}
}

void Opengl2DCharts::mouseDoubleClickEvent ( QMouseEvent * event )
{
	if(event->buttons() & Qt::LeftButton)
	{
		CHARTS_POINTD point(event->x(),event->y());
		OnDoubleClic(point,event->modifiers()|event->buttons());
	}
	else
		return;
}

void Opengl2DCharts::resizeEvent(QResizeEvent *)
{
  m_width=width();
  m_height=height();
  OnResizeEvent(m_width,m_height);
}

void Opengl2DCharts::closeEvent ( QCloseEvent * )
{
  OnCloseEvent();
  hide();
}

///
///INHERITED
///
void Opengl2DCharts::RePaint()
{
	emit SGRePaint();
	//glDraw();
}
CHARTS_POINTD Opengl2DCharts::GetMousePoint()
{
	CHARTS_POINTD pt;
  QPoint ptl;
  ptl=cursor().pos();
  ptl=this->mapFromGlobal(ptl);
  pt.x=ptl.x();
  pt.y=ptl.y();
  return pt;
}

void Opengl2DCharts::CommonInit()
{
 m_initialized=0;
}
