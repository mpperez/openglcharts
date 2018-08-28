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
	\brief xopengl3dcharts.h file.
	\date 05-2017
	\author Marcos Perez*/

/// @ingroup graph3d

#ifndef XOPENGL3DCHARTS_H
#define XOPENGL3DCHARTS_H

#include "charts3dwindowsbase.h"
#include "oglgraph3d.h"
#include "xylinegl3d.h"
#include <time.h>
#include <stdio.h>
#include <string>
#include<stdlib.h>
#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>
#include <X11/cursorfont.h>

#ifndef BUTTON_EVENT
#define BUTTON_EVENT
enum butevent{PRESSED,RELEASED,MOVE};		//!< Enum for button messages.
#endif

/** \class XOGLWindow
		\brief Chart main dialog. Creates the dialog using X interface, implements all mouse/keyboard interface to use zooms.*/

class Opengl3DCharts : public Charts3DWindowBase, public OGLGraph3d
{

friend class OGLGraph3d;
///****************************************** FUNCIONS ***************************************
///*******************************************************************************************
public:

	Opengl3DCharts(void * parent=NULL);
	virtual ~Opengl3DCharts();
protected:
  /** \brief X window event thread. Messages are dispached here.
     \param param param to this.
     \return */
	static void* ThreadXWindow(void* param);
  /** \brief Init Xdisplay.	 */
	void InitXdisplay();
	/** \brief Opengl initialization.*/
	void initializeGL();
  /** \brief Create chart window.	 */
	void CreateWindow();
  /** \brief Creates the arrow cursor when moving using middle button.	 */
	void CreateCursor();
public:

//
// WINDOW EVENTS
//
  /** \brief Mouse move event.
     \param xbuttonmask	 */
	void MouseMoveEvent(XMotionEvent xbuttonmask);
  /** \brief Key press event.	 */
	void KeyPressEvent(XKeyEvent xkey);
  /** \brief MouseEvent	 */
  void MouseEvent(XButtonEvent xbutton, butevent type);

//
//INHERITED
//
  void RePaint();
	CHARTS3D_POINTD GetMousePoint();
	/** \brief Creates the dialog and positioning it.
			\param Parent Parent.
			\param width  Window width.
			\param height Window height.
			\param x Window x position.
			\param y Window y position.
			\param popup Trure if popup false if inside parent.
			\param mspaint Miliseconsfor automatic repainting 0 for manual repainting.
			\return 0 if error.*/
	bool CreateAndPosition(void *parent,int x, int y,  int width, int height, int popup, int mspaint);
  /** \brief Sets window tittle.
     \param tittle	 */
	void SetWindowTitle(string tittle);
  /** \brief Change window size.
     \param width
     \param height	 */
	void SetWindowSize(int width,int height);
  /** \brief Move window.
     \param x
     \param y	 */
	void SetWindowPosition(int x,int y);

///****************************************** VARIABLES **************************************
///*******************************************************************************************
protected:
	pthread_t m_windowthread; //!< Event thread, every dialog belongs to its own thread.
  int	m_width;      				//!< Width.
  int m_height;           	//!< Height
	int m_xposition;					//!< Dialog x position.
	int m_yposition;					//!< Dialog y position.
  bool m_popup;           	//!< True if popup. TODO
public:
	Display	*m_display;				//!< Display
	Window	m_rootwd;					//!< Root window
	Window  m_window;					//!< Window
	GLXContext m_glcontext;		//!< Opengl context.
	XVisualInfo *m_visualinfo;//!< Visual info
	XSetWindowAttributes  m_swatt;//!< Window attributes.
	Colormap m_colormap;			//!< Colormap
	static int m_ngraphs;			//!< Total number of graphs
	int m_currgraph;					//!< Current graph
	Cursor m_movecursor;			//!< Double arrow cursor for moving insithe chart.
};

#endif // XOPENGL3DCHARTS
