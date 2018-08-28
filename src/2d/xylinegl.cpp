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
	\brief xylinegl implementation file.
	\date 05-2010
	\author Marcos Perez*/
#ifdef WIN_TARGET
#include "stdafx.h"
#endif

#include <string.h>
#include "xylinegl.h"
#include "math.h"

CXYLineGL::CXYLineGL(int i_nPoints,string name,COLORREF i_color,bool i_visible)
{
	nPoints=i_nPoints;
	lineName=name;
	nextLine=NULL;
	color=i_color;
	visible=i_visible;
	m_barchart=0;
	nUsedPoints=nPoints;
	pX=new double[nPoints];

	for(int r=0;r<nPoints;r++)
		pX[r]=(float)r;

	pY=new double[nPoints];

	m_auxPoints.npuntos=0;
	m_auxPoints.ppuntosx=NULL;
	m_auxPoints.ppuntosy=NULL;
}

CXYLineGL::~CXYLineGL()
{
	if (pX!=NULL)
	{
		delete [] pX;
		pX=NULL;
	}
	if (pY!=NULL)
	{
		delete [] pY;
		pY=NULL;
	}
	if (nextLine!=NULL)
	{
		delete nextLine;
		nextLine=NULL;
	}
}

int CXYLineGL::AddPoint(double x,double y) 
{
	if (nUsedPoints<nPoints)
	{
		pX[nUsedPoints]=x;
		pY[nUsedPoints]=y;
		nUsedPoints++;
	}
	else 
	{
		memmove(pX,pX+1,(nPoints-1)*sizeof(double));
		memmove(pY,pY+1,(nPoints-1)*sizeof(double));
		pX[nPoints-1]=x;
		pY[nPoints-1]=y;
	}
	return nUsedPoints-1;
}	

void CXYLineGL::AddAuxPoints(double *x,double *y, int npoints,string info)
{
	if(m_auxPoints.ppuntosx!=NULL)
	{
		delete []m_auxPoints.ppuntosx;
	}
	if(x!=NULL)
	{
		m_auxPoints.ppuntosx	= new double[npoints];
		memcpy(m_auxPoints.ppuntosx,x,npoints*sizeof(double));
	}
	if(m_auxPoints.ppuntosy!=NULL)
	{
		delete []m_auxPoints.ppuntosy;
	}
	if(x!=NULL)
	{
		m_auxPoints.ppuntosy	= new double[npoints];
		memcpy(m_auxPoints.ppuntosy,y,npoints*sizeof(double));
	}
	m_auxPoints.info=info;
}






