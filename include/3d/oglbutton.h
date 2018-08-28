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
	\brief Creates the behaviour of a button.
	\date 06-2017
	\author Marcos Perez*/

/// @ingroup graph3d

#ifndef OGLBUTTON_H
#define OGLBUTTON_H

#include "string"
using namespace std;
#include "map"

#define BUTTON_FONT GLUT_BITMAP_HELVETICA_18 /*!<	FONT TYPE, AVAILABLE: \n(GLUT_BITMAP_8_BY_13, GLUT_BITMAP_9_BY_15 , GLUT_BITMAP_TIMES_ROMAN_10,GLUT_BITMAP_TIMES_ROMAN_24
																						GLUT_BITMAP_HELVETICA_10, GLUT_BITMAP_HELVETICA_12, GLUT_BITMAP_HELVETICA_18 )*/

#define BUTTON_FONT_WIDTH 10								//!<	FONT WIDTH
#define BUTTON_FONT_HEIGHT 18								//!<	FONT HEIGHT

/** \class COGLGraph
    \brief Basic button for X window system.*/
class OGLButton
{
//****************************************** FUNCTIONS **************************************
//*******************************************************************************************

public:
	OGLButton(const char* text="", int posx=0, int posy=0);
	~OGLButton();
  /** \brief Calculates if a point is inside the button.
     \param xposition
     \param yposition
     \return	True if is inside */
	bool PointInbutton(int xposition,int yposition);

  int GetBackground(){return pushed ? backgroundPushed  : background;}
  /** \brief SetPushed Forces button state.
     \param state New state.   */
  void SetPushed(bool state){pushed=state;}
  /** \brief Gets current state.
     \return True if pushed.   */
  int IsPushed(){return pushed;}
  /** \brief Draws the button.	 */
	void DrawButton( );

//****************************************** VARIABLES **************************************
//*******************************************************************************************
public:
  string value;			//!< String showed.
  int xpos;         //!< X bottom position.
  int ypos;       	//!< Y bottom position.
  int width;        //!< Width.
  int height;       //!< Height
  int color;        //!< Color.
  int background;		//!< Background color.
  int backgroundPushed;			//!< Background color if pushed.
  bool pushed;			//!< State, true if pushed.
};
#endif // OGLBUTTON_H
