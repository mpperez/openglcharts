/*(C) Copyright 2017 mpperez
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
/** \file
	\brief qopengl3dcharts.h file.
	\date 05-2017
	\author Marcos Perez*/

/// @ingroup graph3d

#ifndef QOPENGL3DCHARTS_H
#define QOPENGL3DCHARTS_H

#include "qglobal.h"
#if ((!defined(QT_VERSION)))
	#error "Only works on Qt, and there is no QT_VERSION..."
#endif

#include <QGLWidget>
#include <QtGui>
#include <QtOpenGL/QGLWidget>
#include <QWidget>
#include <QMutex>

#include "charts3dwindowsbase.h"
//#define GLUT_DISABLE_ATEXIT_HACK  //if there is no text
#include "oglgraph3d.h"
#include "xylinegl3d.h"

using namespace std;
#include <time.h>
#include <list>
#include <vector>
#include <stdio.h>
#include <map>
#include <string>
/** \class QOGLWindow
		\brief Chart main dialog. Creates the dialog using Qt interface, implements all mouse/keyboard interface to use zooms.*/

class Opengl3DCharts: public QGLWidget ,public Charts3DWindowBase , public  OGLGraph3d
{
// friend class COGLGraph;
	Q_OBJECT
//****************************************** FUNCIONs ***************************************
//*******************************************************************************************
 public:
	/** \brief Constructor.
		 \param pParent: Pointer to parent.*/
	Opengl3DCharts(QWidget *pParent=NULL);
	 virtual ~Opengl3DCharts();//!< Destructor
private:
	/** \brief Opengl initialization.*/
	void initializeGL();
	/** \brief Tooltip , TODO.*/
	void CreaTooltipLX();
	/** \brief Shows dialog.*/
	void ShowDialog();

//
// AUX MENUS
//
private slots:
  /** \brief ShowContextMenu
     \param pos Mouse position.	 */
	void ShowContextMenu(const CHARTS3D_POINTD& pos);
//
//   SAVING IMAGES,GRAFS,DAT....
//
  /**  \brief Save as dialog. */
	bool SaveAs();
	/** \brief Saves the chart as image.
			\param direccion  Complete path of the file.*/
	void GuardaImagen( string direccion);
	/** \brief Save as BMP.
			\param pBitmapBits: Image data.
			\param lWidth: Width.
			\param lHeight: Height.
			\param	wBitsPerPixel: Bits per pixel.
			\param lpszFileName:  Complete path of the file.*/
	void SaveBitmapToFile( GLubyte* pBitmapBits, long lWidth, long lHeight,unsigned int wBitsPerPixel, string lpszFileName );
//
//EVENTS
//
public slots:
	void OnRePaint();
	/** \brief Opengl redraw message.*/
	void paintGL();
//
//DIALOG SLOTS
//
private:
	void wheelEvent(QWheelEvent *event);
	void keyPressEvent(QKeyEvent* e);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent ( QMouseEvent * event );
	void mouseDoubleClickEvent ( QMouseEvent * event );
	void resizeEvent(QResizeEvent *);
	void closeEvent (QCloseEvent *);
//
//INHERITED
//#endif//QT_TARGET
	CHARTS3D_POINTD GetMousePoint();
	void RePaint();
public:
	/** \brief Creates the dialog and positioning it.
			\param Parent Parent.
			\param width  Window width.
			\param height Window height.
			\param x Window x position.
			\param y Window y position.
			\param popup Trure if popup false if inside parent.
			\param mspaint Miliseconsfor automatic repainting 0 for manual repainting.
			\return 0 if error.*/
	bool CreateAndPosition(void *parent,int x, int y, int width, int height,  int popup, int mspaint);
  /** \brief Change window size.
     \param width
     \param height	 */
	void SetWindowSize(int width,int height){this->SetWindowSize(width,height);}
  /** \brief Move window.
     \param x
     \param y	 */
	void SetWindowPosition(int x,int y){this->move(x,y);}
	/** \brief Change the stile of the dialog to insert into another dialog or separates.
		\param popup 1 if popup 0 if child*/
	void SetPopupDialog(int popup);
	/** \brief Change borde stile to enable resizing...*/
	void ChangeDlgBorder();
  /** \brief Sets window title.
     \param title	 */
	void SetWindowTitle(string tittle){QGLWidget::setWindowTitle(tittle.data());}
//
//SIGNALS
//
signals:
	void SGRePaint();
private:
	/** \brief Common inicialization.*/
	void CommonInit();
///****************************************** VARIABLES **************************************
///*******************************************************************************************
	QPoint m_posdialogo;			//!< Position of the dialog.
private:
	QWidget * m_pparent;			//!< Parent.
	QTimer	*m_timer_paint;		//!< Timer for reapainting.
protected:
///VARIABLES DE CONTROL DEL DIALOGO
	bool m_initialized;			//!< True if OPengl is initialized.
	int	m_width;						//!< Width.
	int m_height;						//!< Height
	bool m_child;						//!< True if child dialog.
	static int m_ndialogs;	//!< Total dialogs.
};

#endif//QOPENGL3DCHARTS_H
