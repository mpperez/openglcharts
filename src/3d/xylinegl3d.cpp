// XYLineGL.cpp: implementation of the CXYLineGL3D class.
/*! \file
	\brief Archivo de implementacion las linea 3D de OpenGL.
	\date 05-2010
	\author Marcos Perez*/
#include <string.h>
#include "xylinegl3d.h"

#include "math.h"
//#define OPENMP_GRAPHS


#ifdef OPENMP_GRAPHS
#include <omp.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CXYLineGL3D::CXYLineGL3D(int i_nPoints,int i_zPoints,string name,COLORREF i_color,bool i_visible)
{
	nPoints=i_nPoints;
	m_nLinesT=i_zPoints;
	lineName=name;
	m_nUsedLinesT=0;
	nextLine=NULL;
	color=i_color;
	visible=i_visible;
	m_bloqueador=0;
	CreateBuffers();

	m_puntos_interes.npuntos=0;
}

CXYLineGL3D::~CXYLineGL3D()
{
	FreeBuffers();
	if (nextLine!=NULL)
	{
		delete nextLine;
		nextLine=NULL;
	}
}
void CXYLineGL3D::CreateBuffers()
{
	pX=new double[nPoints];
	for(int r=0;r<nPoints;r++)
		pX[r]=(float)r;
	pZ=new double[m_nLinesT];
	pY=new double*[m_nLinesT];
	m_pEjeY=new double[nPoints*m_nLinesT];
	for(int c=0;c<m_nLinesT;c++)
		pY[c]=&m_pEjeY[nPoints*c];
	pYC=new struct strColorTable3D*[m_nLinesT];
	m_pEjeYC=new struct strColorTable3D[nPoints*m_nLinesT];
	for(int c=0;c<m_nLinesT;c++)
		pYC[c]=&m_pEjeYC[nPoints*c];
}
void CXYLineGL3D::FreeBuffers()
{
	if (pX!=NULL)
	{
		delete [] pX;
		pX=NULL;
	}
	if (pZ!=NULL)
	{
		delete [] pZ;
		pZ=NULL;
	}
	if (pY!=NULL)
	{
		delete [] pY;
		pY=NULL;
	}
	if (m_pEjeY!=NULL)
	{
		delete []m_pEjeY ;
		m_pEjeY=NULL;
	}
	if (pYC!=NULL)
	{
		delete [] pYC;
		pYC=NULL;
	}
	if (m_pEjeYC!=NULL)
	{
		delete []m_pEjeYC ;
		m_pEjeYC=NULL;
	}
}
void CXYLineGL3D::CambiaParametros(int i_nPoints,int nZlines)
{

	FreeBuffers();
	nPoints=i_nPoints;
	m_nLinesT=nZlines;
	m_nUsedLinesT=0;
	CreateBuffers();

}
void CXYLineGL3D::ChangePen(COLORREF i_color)
{

	color=i_color;
}

int CXYLineGL3D::AddLineT(double*y) 
{
	if (m_nUsedLinesT<m_nLinesT)
	{
		memcpy(pY[m_nUsedLinesT],y,nPoints*sizeof(double));
		m_nUsedLinesT++;
	}
	else 
	{
		memmove(pY[0],pY[1],(nPoints)*sizeof(double)*(m_nLinesT-1));
		memcpy(pY[m_nLinesT-1],y,nPoints*sizeof(double));
	}

	return m_nUsedLinesT-1;
}	







