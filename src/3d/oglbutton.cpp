/* oglbutton.cpp : implementation file
(C) Copyright 2017 mpperez
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
#include "oglbutton.h"
#include "GL/gl.h"
#include "GL/glut.h"

OGLButton::OGLButton(const char *text,int posx,int posy)
{
	 value= text;
	 xpos=posx;
	 ypos=posy;
	 width=value.size()*BUTTON_FONT_WIDTH+1*BUTTON_FONT_WIDTH;
	 height=BUTTON_FONT_HEIGHT*2;
	 color=0x00ffffff;
	 background=0x0;
	 backgroundPushed=0x007a7a0a;
	 pushed=0;
}

OGLButton::~OGLButton()
{

}
bool OGLButton::PointInbutton(int xposition,int yposition)
{
	double pointX = xposition;
	 double pointY = yposition;
				 // Just had to change around the math
	 if (pointX < (xpos +this->width) && pointX > xpos  &&
						pointY < (ypos+this->height) && pointY > ypos )
		 return true;
	 else
		return  false;
}

void OGLButton::DrawButton( )
{
	int colorini[4];
	glGetIntegerv(GL_CURRENT_COLOR,(int*)&colorini);
	int colorfondo=GetBackground();
	float posz=1;
	//draw background.
	glBegin( GL_QUADS );
	//glColor3ub( RED(colorfondo),GREEN(colorfondo),BLUE(colorfondo) );
	glColor4ubv((GLubyte*)&colorfondo);
	glVertex3d(xpos , ypos-BUTTON_FONT_HEIGHT/5 ,posz-0.01f);
	glVertex3d(xpos+width , ypos-BUTTON_FONT_HEIGHT/5 ,posz-0.01f );

	glVertex3d(xpos +width, ypos+BUTTON_FONT_HEIGHT -BUTTON_FONT_HEIGHT/5,posz-0.01f );
	glVertex3d( xpos , ypos+BUTTON_FONT_HEIGHT-BUTTON_FONT_HEIGHT/5 ,posz-0.01f );
	glEnd();

	//glColor3ub( RED(colorletra),GREEN(colorletra),BLUE(colorletra) );
	glColor4ubv((GLubyte*)&color);
	void * font=BUTTON_FONT;
	char* buf=(char*)value.c_str();
	glRasterPos3d(xpos,ypos ,posz );
	for(unsigned int c=0;c<value.size();c++)
	{
		glutBitmapCharacter(font,buf[c]);
	 //glutStrokeCharacter(GLUT_STROKE_ROMAN,buf[c]);
	}
	glColor3ub( colorini[0],colorini[1],colorini[2] );
}
