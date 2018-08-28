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

/*! \file
  \brief Linux & General definitions.
	\date 05-2017
  \author Marcos Perez*/

/// @ingroup graph2d

#ifndef CHARTSWINDOWSBASE_H
#define CHARTSWINDOWSBASE_H

#include "GL/gl.h"
#include "string"
using namespace  std;


typedef GLint COLORREF;

enum chartkeys{k_left=1,k_right=0x10,k_up=0x100,k_down=0x1000,k_control=0x10000,k_shift=0x100000,k_F12=0x1000000};

#ifndef POINT_CLASS
#define POINT_CLASS

/** \class POINTD
		\brief Implements a point X Y using doubles.*/
class CHARTS_POINTD
{
public:
	CHARTS_POINTD(double x1=0, double y1=0){x=x1;y=y1;};
  double x;
  double y;
};
#endif//POINT_CLASS

#ifndef CHARTSWINDOWBASE
#define CHARTSWINDOWBASE

#define NO_MOUSE_BUTTON    0x00000000   //!< MOUSE EVENT NO BUTTON
#define MOUSE_LEFT_BUTTON  0x00000001   //!< MOUSE EVENT LEFT BUTTON
#define MOUSE_RIGHT_BUTTON 0x00000002   //!< MOUSE EVENT RIGHT BUTTON
#define MOUSE_MID_BUTTON   0x00000004   //!< MOUSE EVENT LEFT BUTTON
#define META_MODDIFIER     0x01000000   //!< MOUSE EVENT META MODIFIER
#define SHIFT_MODDIFIER    0x02000000   //!< MOUSE EVENT SHIFT MODIFIER
#define CONTROL_MODDIFIER  0x04000000   //!< MOUSE EVENT CONTROL MODIFIER
#define ALT_MODDIFIER      0x08000000   //!< MOUSE EVENT ALT MODIFIER

#define ID_LEFT_KEY         32771       //!< KEYBOARD EVENT LEFT KEY
#define ID_RIGHT_KEY        32772       //!< KEYBOARD EVENT RIGHT KEY
#define ID_DOWN_KEY         32774       //!< KEYBOARD EVENT DOWN KEY
#define ID_UP_KEY           32775       //!< KEYBOARD EVENT UP KEY
#define ID_VISUALIZAR       32776       //!<

/** \class ChartsWindowBase
		\brief Virtual class with minimum public member of inherited window classes(Qt, Xwindow, MFC...).*/
class ChartsWindowBase
{
public:
	ChartsWindowBase(){;}
	/** \brief Change the stile of the dialog to insert into another dialog or separates.
		\param popup 1 if popup 0 if child*/
	virtual void SetPopupDialog(int popup)=0;

	/** \brief Change borde stile to enable resizing...*/
	virtual void ChangeDlgBorder()=0;

	/**
   * \brief Sets window title.
     \param title
	 */
	virtual void SetWindowTitle(string )=0;

	/** \brief Creates the dialog and positioning it.
			\param Parent Parent.
			\param width  Window width.
			\param height Window height.
			\param x Window x position.
			\param y Window y position.
			\param popup Trure if popup false if inside parent.
			\param mspaint Miliseconsfor automatic repainting 0 for manual repainting.
			\return 0 if error.*/
	virtual bool CreateAndPosition(void *parent,int x, int y, int width, int height,  int popup, int mspaint)=0;

  /** \brief Change window size.
     \param width
     \param height
	 */
	virtual void SetWindowSize(int width,int height)=0;

  /** \brief Move window.
     \param x
     \param y
	 */
	virtual void SetWindowPosition(int x,int y)=0;
};
#endif //CHARTSWINDOWBASE
#endif


