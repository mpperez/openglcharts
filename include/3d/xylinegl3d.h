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
along with this library;*/
/*! \file 
	\brief Clase que almacena la informacion de una linea de la grafica.
	\date 05-2010
	\author Marcos Perez*/

/// @ingroup graph3d

#ifndef XYLINE_Opengl3DCharts
#define XYLINE_Opengl3DCharts

#include "string"
#include "stdio.h"
#include "charts3dwindowsbase.h"
#include "colormapgl3d.h"
using namespace std;


#define LINEASMAN	80											//!<	RENDERIZADO POR LINEAS UNA POR LINEA DE EJEZ

#define LOG10 log((double)10)
/** \struct punto3d
    \brief Sturct in charge of storing auxiliar points information.
*/
#ifndef STRUCT_PUNTO3D
#define STRUCT_PUNTO3D
struct punto3d
{
  double ppuntosx[20];	//!< X values of points information.
  double ppuntosy[20];	//!< Y values of points information.
  double ppuntosz[20];	//!< Z values of points information.

  int npuntos;						//!< Total point number <20.
  string* info;					//!< Ausiliar point information
};
#endif
/** \class CXYLineGL3D 
    \brief Stores information about every dataset of the chart. Every dataset contains 3D plot,
    a XY lines repeated Z deepth lines, X and Z values are fixed for all dataset.*/

class  CXYLineGL3D  
{
public:
	/** \brief Constructor.
      \param i_nPoints Dataset XY line number points.
      \param i_nZpoints Time number of lines or depth lines.
      \param name Line name.
      \param color Line color.
      \param i_visible True for initially show.*/
	CXYLineGL3D(int i_nPoints,int i_nZpoints,string name,COLORREF color,bool i_visible=true);
	virtual ~CXYLineGL3D();//!< Destructor.
  /** \brief Change number of points of the dataset.
      \param i_nPoints XY number of points.
      \param nZlines Depth number of XY lines.*/
	void CambiaParametros(int i_nPoints,int nZlines);
  /** \brief Creates the dataset buffers.*/
  void CreateBuffers();
  /** \brief Free dataset buffer.*/
  void FreeBuffers();
  /** \brief Return X values pointer.
			\return Puntero al eje x.*/
	double* GetPX() { return pX; }
  /** \brief Return Y pointer for a specified Z line index.
      \param z Z line index.
      \return Y pointer.*/
  double* GetPY(int z) { return pY[z]; }
  /** \brief Return Y color values for aspecified Z line index.
      \param z Z line index.
      \return Color pointer.*/
	struct strColorTable3D* GetPYC(int z) {		return pYC[z]; }
  /** \brief Return Z  values pointer.
      \return Pointer to Z axis.*/
  double* GetPZ() { return pZ; 	}
  /** \brief Return total Z lines.
      \return Total Z lines.*/
  int GetNLinesT() { return m_nLinesT;}
  /** \brief Return number of used depth lines.
      \return .*/
  int GetUsedLinesT() {	return m_nUsedLinesT;	}
  /** \brief Set current number of used depth lines.
      \param uP New number of used lines.
      \return Number of used lines.*/
	int SetUsedLinesT(int uP) { return m_nUsedLinesT=uP<m_nLinesT ? uP: m_nLinesT; }
  /** \brief Adds new time line, if max lines is reached frees firs line and add to te end.
      \param y  values of new line.
      \return Number of used lines.*/
  int AddLineT(double*y);
  /** \brief Changes line color.
      \param i_color: New line color.	*/
	void ChangePen(COLORREF i_color);
private:
  int m_nLinesT;						//!< Total time/depth lines z axis.
  double*pX;								//!< X values.
  double**pY;								//!< Y values table.
  double*pZ;								//!< Z values .
  double*m_pEjeY;						//!< Y vector aux pointer.
  struct strColorTable3D *m_pEjeYC;			//!< Color table init point.
  struct strColorTable3D **pYC;					//!< Color table .
	//HANDLE m_hparametros;					//!< Evento para cambiar parametros.

public:
  int m_nUsedLinesT;						//!< Used time/depth z lines.
  int nPoints;									//!< Number of points in every line XY.
  int m_bloqueador;							//!< Aux .
  bool visible;									//!< True if visible.
  string lineName;							//!< Line name.
  COLORREF color;								//!< Line color.
  struct punto3d m_puntos_interes;//!< Aux points to draw.
  CXYLineGL3D* nextLine;				//!< Point to next dataset.
};

#endif // XYLINE_Opengl3DCharts
