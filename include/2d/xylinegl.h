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
	\brief Class in charge of storing all information about a line into the chart.
	\date 05-2010
	\author Marcos Perez*/

/// @ingroup graph2d

#ifndef XYLINE_OGLGRAPH2D
#define XYLINE_OGLGRAPH2D
#include "chartswindowsbase.h"
#include "string"
#include "stdio.h"
using namespace std;

#ifndef LOG10
#define LOG10 log((double)10)
#endif

#ifndef STRUCT_PUNTO
#define STRUCT_PUNTO

/** \struct punto
		\brief Struct to store interst points.*/
struct punto
{
	double *ppuntosx;		//!< X values pointer.
	double *ppuntosy;		//!< y values pointer.
	int npuntos;				//!< Number of values.
	string info;				//!< Point information.
};

#endif

/** \class CXYLineGL
		\brief Stores information about every line into the chart, also stores auxiliary points.*/

class  CXYLineGL  
{
public:
	/** \brief Constructor.
			\param i_nPoints: Fixed number of points.
			\param name Line name.
			\param color line color.
			\param i_visible Visibility 1 visible, 0 hidden.*/
	CXYLineGL(int i_nPoints,string name,COLORREF color,bool i_visible=true);
	virtual ~CXYLineGL();//!< Destructor.
	/** \brief Get x pointer.
			\return X vector.*/
	double* GetPX() { return pX; }
	/** \brief Get y pointer.
			\return y.*/
	double* GetPY() { return pY; }
	/** \brief Total number of points.
			\return Number points.*/
	int GetNPoints() { return nPoints; }
	/** \brief Get used points.
			\return Used points.*/
	int GetUsedPoints() { return nUsedPoints; }
	/** \brief Sets current used points.
			\param uP New used points.
			\return Current used points.*/
	int SetUsedPoints(int uP) { return nUsedPoints=uP; }
	/** \brief Adds a point, If usedpoints == npoints removes firts and add at the end.
			\param x X value.
			\param y Y value.
			\return Used points.*/
	int AddPoint(double x,double y);
  /** \brief Add aux points, points outside of the line but drawed as single points.
     \param x X pointer.
     \param y y pointer.
     \param nponints Number of points.
     \param info Information.	 */
	void AddAuxPoints(double *x, double *y, int npoints, string info);
private:
	int nPoints;								//!< Number of point in the line.
	int nUsedPoints;						//!< Number of used points <= nPoints.
	double *pX;									//!< X points pointer.
	double *pY;									//!< Y points pointer.
public:
	int m_barchart;							//!< True if bar diagram.
	bool visible;								//!< True if line is visible.
	string lineName;						//!< Line name showed in the legend.
	COLORREF color;							//!< Line color.
	struct punto m_auxPoints;		//!< Auxiliary points.
	CXYLineGL* nextLine;				//!< Pointer to next line.
};

#endif //XYLINE_OGLGRAPH2D
