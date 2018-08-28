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
	\brief Archivo de implementacion las graficas 2D de OpenGL.
	\date 05-2010
	\author Marcos Perez*/

#include <sstream>
#include <iomanip>
#include "oglgraph3d.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>//vnsprintf
#include <string.h>

#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"


#define MAX(X,Y) (X>Y ? X :Y)
#define MIN(X,Y) (X<Y ? X :Y)

#define RAD(x)	x*3.14159265358/180
#define GRADS(x)	x/3.14159265358*180

vector<OGLGraph3d*> OGLGraph3d::m_pcharts;

OGLGraph3d::OGLGraph3d( )
{
	CommonInit();
	pthread_mutex_init (&m_dataMutex, NULL);
	if (sem_init(&m_paintSemaphore, 0, 1) == -1)
	{
		printf("error when globalsem initiation\n");
	}
}

OGLGraph3d::~OGLGraph3d()
{

  if (m_plines!=NULL)
  {
		RemoveLines(0);
		m_plines=NULL;
  }
	if(OGLGraph3d::m_pcharts.size()>0)
		OGLGraph3d::m_pcharts.pop_back();
#ifdef COLORMAP3D
	for(int r=0;r<m_nlines;r++)
		if(m_colormaps[r]!=NULL)
			delete m_colormaps[r];
#endif

}

bool OGLGraph3d::InitGraph( bool altocontraste,bool ejxhora)
{
	m_ejzhora=ejxhora;
	SetColors(altocontraste);
	CalculaLeyenda();
	glClearColor(OGL3D_RED(m_backgroundColor)/255.0f,OGL3D_GREEN(m_backgroundColor)/255.0f,OGL3D_BLUE(m_backgroundColor)/255.0f, 0.0f);
	return 1;
}

void OGLGraph3d::CommonInit()
{
	m_nlines=0;
	m_plines=NULL;
	m_plines=NULL;
	m_max=-1;
	m_lineat2d=-1;
	m_angx=m_angy=0;
	m_zoom_ventana=0;//zoom ventana desactivado
	m_posejesscroll=0;//zoom ejes desactivado
	m_posscroll=0;//zoom boton central desactivado
	m_colormanual=-1.0f;//distribucion colores manual desactivado
	m_focusLineIndex=-1;
	m_distancias=0;
	m_renderizado=POLIGONOS;
	m_drawCurrentPos=0;
	m_offsetz=0;
	m_drawZoomBox=0;
	m_widthLegend=0;

#ifdef COLORMAP3D
	memset(m_colormaps,0,20*sizeof(CColorMapGL3D*));
#endif
	m_mapdibujos.clear();
	OGLGraph3d::m_pcharts.push_back(this);
	CreateButtons();
}

///
///CONFIGURATION
///
void OGLGraph3d::SetGraphTitle(string tittle,string ejx/*=""*/,string ejy/*=""*/)
{
	 m_tittle=tittle;
	 m_xasisName=ejx;
	 m_yAxisName=ejy;
}

void OGLGraph3d::SetColors(int hightcontrast)
{
	m_backgroundColor=hightcontrast ? OGL3D_BACKGROUNDN :  OGL3D_BACKGROUND;
	m_fontColor=hightcontrast ?  OGL3D_FONTCOLORN : OGL3D_FONTCOLOR;

	m_backgroundcolormaxvalue=hightcontrast ? OGL3D_BACKGROUND_MAX_VALUEN : OGL3D_BACKGROUND_MAX_VALUE ;

	m_distancesColor=hightcontrast ? OGL3D_MANUAL_MEASURE_LINESN : OGL3D_MANUAL_MEASURE_LINES;
	m_backgroundDistances=hightcontrast ? OGL3D_BACKGROUND_MANUAL_MEASUREN : OGL3D_BACKGROUND_MANUAL_MEASURE;

	m_axisColor=hightcontrast ? OGL3D_AXIS_COLORN  : OGL3D_AXIS_COLOR;

	m_zoomAxisColor=hightcontrast ? OGL3D_AXIS_COLORN : OGL3D_AXIS_ZOOM_COLOR;
	m_backgroundLegendColor=hightcontrast ? OGL3D_BACKGROUND_LECEND_COLORN : OGL3D_BACKGROUND_LECEND_COLOR;
	m_zoomBoxColor=hightcontrast ? OGL3D_ZOOM_BOX_COLORN : OGL3D_ZOOM_BOX_COLOR;;
	m_auxPointsColor=hightcontrast ? OGL3D_SECONDARY_POINTS_COLORN :OGL3D_SECONDARY_POINTS_COLOR;

	m_backgroundCurrentValue=hightcontrast ? OGL3D_BACKGROUND_CURRENT_POSITIONN : OGL3D_BACKGROUND_CURRENT_POSITION;
	m_currentPositionColor=hightcontrast ? OGL3D_AXIS_ZOOM_COLORN : OGL3D_AXIS_ZOOM_COLOR;
}

///
///ELEMENTS
///
int  OGLGraph3d::AddLine(int nPoints,int zLines,string name,COLORREF color,bool visible)
{
	CXYLineGL3D** pt;

	LockDataMutex();

	for (pt=&m_plines;*pt!=NULL;pt=&((*pt)->nextLine));
	*pt=new CXYLineGL3D(nPoints,zLines,name,color,visible);
	m_widthLegend=m_widthLegend< name.size()*OGL3D_FONT_WIDTH ? name.size()*OGL3D_FONT_WIDTH:m_widthLegend;
	m_nlines++;
	if (m_nlines>100)
	{
		m_plines=m_plines->nextLine;
		m_nlines--;
		delete pt;
	}

  UnlockDataMutex();
	m_focusLineIndex=0;
	CalculaLeyenda();
	return m_nlines;
}

#ifdef COLORMAP3D
int OGLGraph3d::AddLine(int nPoints,int zLines,string name,string rutamap,bool visible)
{
	CXYLineGL3D** pt;
	LockDataMutex();
	if(m_colormaps[m_nlines]!=NULL)
		delete m_colormaps[m_nlines];

	for (pt=&m_plines;*pt!=NULL;pt=&((*pt)->nextLine));
	m_colormaps[m_nlines]=new CColorMapGL3D(rutamap.data());
	if(m_colormaps[m_nlines]!=NULL)
		*pt=new CXYLineGL3D(nPoints,zLines,name,m_paleta[m_nlines],visible);
	else
	{
		float*col=m_colormaps[m_nlines]->colorTable[m_colormaps[m_nlines]->nColors-1].color;
		COLORREF color=OGL3D_REDC((int)(255*col[0]))+(OGL3D_GREENC((int)(255*col[1]))<<8)+(OGL3D_BLUEC((int)(255*col[1]))<<16);

		*pt=new CXYLineGL3D(nPoints,zLines,name,color,visible);
	}

	m_widthLegend=m_widthLegend<name.size()*OGL3D_FONT_WIDTH ? name.size()*OGL3D_FONT_WIDTH:m_widthLegend;
	m_nlines++;
	if (m_nlines>100)
	{
		m_plines=m_plines->nextLine;
		m_nlines--;
		delete pt;
	}
	UnlockDataMutex();
	m_focusLineIndex=0;
	CalculaLeyenda();
	return m_nlines;
}
#endif
int  OGLGraph3d::GetnLines()
{
  return m_nlines;
}

CXYLineGL3D *OGLGraph3d::GetLine(int iLine)
{
	CXYLineGL3D** pt;
	int i;
	for (i=0,pt=&m_plines;i<iLine && (*pt!=NULL);i++,pt=&((*pt)->nextLine));
	return *pt;
}

CXYLineGL3D* OGLGraph3d::GetLines()
{
  return GetLine(0);
}

void OGLGraph3d::AddDraw(string identifier, enum_tipodibujos3d drawtype, float posx, float posy, float posz, float size, COLORREF colorline, COLORREF backgroundcolor)
{
	drawingsstr tab;
	tab.drawingType=drawtype;
	tab.posx=posx;
	tab.posy=posy;
	tab.posz=posz;
	tab.tamano=size;
	tab.colorbrush=backgroundcolor;
	tab.colorpen = colorline;
	m_mapdibujos.insert(pairdrawing(identifier,tab));
}

void OGLGraph3d::DeleteDraw(string identifier)
{
	m_mapdibujos.erase(identifier);
}

void OGLGraph3d::DeleteAlldraws()
{
	m_mapdibujos.clear();
}

void OGLGraph3d::RemoveLines(int start)
{
	CXYLineGL3D *pt,**ptpt;
	if (m_nlines==0)
		return;
  LockDataMutex();
	pt=GetLine((start>=0) ? start : 0);
	if (pt!=NULL)
	{
		delete pt;
		pt=NULL;
		m_nlines=start;
	}
	ptpt=(start==0) ? &m_plines : &(GetLine(start-1)->nextLine);
	*ptpt=NULL;
  UnlockDataMutex();

}

void OGLGraph3d::UpdateLineValues(int nline,double *Xvalues,double *Yvalues,int nvalues)
{
	if(nvalues<=0 || Yvalues==NULL || GetLine(nline)==NULL)
    return;
  LockDataMutex();
	double *y=GetLine(nline)->GetPY(0);
	double *x=GetLine(nline)->GetPX();
	int npoints=GetLine(nline)->GetNLinesT();

	//Copy points into line values, if nvalues less than line points rotate and copy.
	if(nvalues<npoints)
  {
		memmove(y,y+nvalues,(npoints-nvalues)*sizeof(double));
		memcpy(y+npoints-nvalues,Yvalues,nvalues*sizeof(double));

		if(Xvalues!=NULL)
		{
			memmove(x,x+nvalues,(npoints-nvalues)*sizeof(double));
			memcpy(x+npoints-nvalues,Xvalues,nvalues*sizeof(double));
		}
  }
  else
	{
		memcpy(y,Yvalues,npoints*sizeof(double));

		if(Xvalues!=NULL)
			memcpy(x,Xvalues,npoints*sizeof(double));
	}
  UnlockDataMutex();
}

void OGLGraph3d::CreateButtons()
{
	buttonAutozoom=OGLButton("Front",25,5+this->m_nlegend_lines*OGL3D_FONT_HEIGHT);
	buttonAutozoomTop=OGLButton("Top",90,5+this->m_nlegend_lines*OGL3D_FONT_HEIGHT);
	checkAutofix=OGLButton("AutoFix",135,5+this->m_nlegend_lines*OGL3D_FONT_HEIGHT);
}

///
///SYNCRO
///
void OGLGraph3d::LockDataMutex()
{
#ifdef WIN_TARGET
  m_ptmutex->Lock();
#elif defined __linux__
	pthread_mutex_lock( &m_dataMutex );
#endif
}

void OGLGraph3d::UnlockDataMutex()
{
#ifdef WIN_TARGET
  m_ptmutex->Unlock();
#elif defined __linux__
	pthread_mutex_unlock( &m_dataMutex );
#endif
}

int OGLGraph3d::BlockPaint(int ms)
{
#	if defined(__linux__)
	struct timespec tms;
	if (clock_gettime(CLOCK_REALTIME, &tms) == -1)
		printf("Error globalsemaphore get clock_gettime\n");
	tms.tv_sec+=ms/1000;
	tms.tv_nsec+=(long)ms%1000*1e6;
	if(tms.tv_nsec>1e9)
	{
		tms.tv_nsec=0;
		tms.tv_sec++;
	}
	 return sem_timedwait(&m_paintSemaphore,&tms);
#	elif defined(WINDOWS)
	WaitForSingleObject(m_paintSemaphore,ms);
#	endif
}

int OGLGraph3d::unBlockPaint()
{
#if defined(__linux__)
 return !sem_post(&m_paintSemaphore);
#elif defined(WINDOWS)
	long * prev;
	ReleaseSemaphore(m_paintSemaphore,1,&prev);
#endif
}

int OGLGraph3d::TryBlockPaint()
{
#if defined(__linux__)
	return !sem_trywait(&m_paintSemaphore);
#elif defined(WINDOWS)

#endif
}

///
///PAINTING
///
int  OGLGraph3d::RedrawChart()
{
	glClearColor(OGL3D_RED(m_backgroundColor)/255.0f,OGL3D_GREEN(m_backgroundColor)/255.0f,OGL3D_BLUE(m_backgroundColor)/255.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_INTERLACE_SGIX);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable (GL_DEPTH_TEST);
	glDepthFunc(GL_DEPTH_RANGE);//GL_ALWAYS
	//se coge el rectangulo entero EN PIXELES
	DrawTittle();
	DibujarLeyenda();
	DrawButtons();

	//Change camera perspective
	UpdateView();
	//UpdateCamera();

	//rectangulo menos el titulo EN MM
  //DrawGraphBox();

	DrawAxis();

	double minescala[2],maxescala[2], paso[2];
	string format[2];
	DibujaTextoReferencias( minescala, maxescala,paso,format);
	//se coge el rectangulo delimitado por zoom
	if(m_plines!=NULL)
	{
		if(m_plines->GetUsedLinesT()>2 && m_renderizado!=LINEASMAN&&m_lineat2d<0)
			DrawAsPoligons();
		else
			DrawAsLines();
	}
	DrawDraws();
	if(m_drawManualDistance==2)
		DrawManualMeasurement();
  DibujaReferencias(minescala, maxescala,paso);
	if(m_drawZoomBox)
		DrawZoomBox();
	//if(m_drawCurrentPos)
	//	DrawCurrentPosition();
	if(m_focusLineIndex>=0 && m_nlines>0)
	{
	//	DibujaMaximo(m_max<0?MaximoLocal() : m_max);

	}

  return 1;
}

void OGLGraph3d::SetExternalDrawingFunction(int (*functionptr) ())
{
	posdrawuserfnctionpointer=functionptr;
}

void OGLGraph3d::UpdateCamera()
{
	glViewport(OGL3D_LEFT_MARGIN,(int)m_freeSpaceXaxis,(int)m_width-(int)m_freeSpaceYaxis-OGL3D_RIGHT_MARGIN-OGL3D_LEFT_MARGIN,floor(m_height-m_freeSpaceXaxis-OGL3D_TITTLE_HEIGHT) );//trozo de ventana
	glMatrixMode(GL_PROJECTION);

	glMatrixMode(GL_MODELVIEW);
	float ratio=1;//(m_zoomx[0]+m_zoomx[1])/(m_zoomy[0]+m_zoomy[1]);
	gluPerspective(45.0f, ratio, m_zoomz[0], m_zoomz[1]);
	gluLookAt(	camera.campos.x, camera.campos.y, camera.campos.z,
			camera.campos.x+camera.camdir.x, camera.campos.y+camera.camdir.y,  camera.campos.z+camera.camdir.z,
			0.0f, 1.0f,  0.0f);
}

void OGLGraph3d::UpdateView()
{
	int widt=(int)m_width-OGL3D_RIGHT_MARGIN-OGL3D_LEFT_MARGIN;
	int heigh=floor(m_height-m_freeSpaceXaxis-OGL3D_TITTLE_HEIGHT);//todo

	glViewport(OGL3D_LEFT_MARGIN,(int)m_freeSpaceXaxis, widt,heigh);//trozo de ventana

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double cosy=cos(RAD(m_angy));
	double siny=sin(RAD(m_angy));
	double midx=(m_zoomx[0]+m_zoomx[1])/2;

	glTranslatef(midx,0,0 );//nos situamos en el centro
	glRotated(m_angy, 0,1.0,0.0);//rotamos el eje vertical
	glRotated(m_angx,cosy, 0.0,siny);//rotamos el antiguo eje x para hacer el cabeceo
	glTranslated(-midx*cosy, 0.0,-midx*siny);//volvemos aorigen

	glOrtho(m_zoomGLx[0], m_zoomGLx[1],m_zoomGLy[0], m_zoomGLy[1],-m_zoomGLz[1],-m_zoomGLz[0]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void OGLGraph3d::DrawGraphBox()
{
	glLineWidth(1);
	glEnable (GL_LINE_SMOOTH);
	glDisable (GL_LINE_STIPPLE);
	glLineWidth(4);
	glBegin( GL_LINE_LOOP	);
	glColor3ub(0,250,0);
	glVertex3f(m_zoomx[0],m_zoomy[0], m_zoomz[0] );
	glVertex3f(m_zoomx[1],m_zoomy[0], m_zoomz[0] );
	glVertex3f(m_zoomx[1],m_zoomy[1], m_zoomz[0]  );
	glVertex3f(m_zoomx[0],m_zoomy[1], m_zoomz[0]  );
	glEnd();

	glBegin( GL_LINE_LOOP	);
	glColor3ub(250,0,0);
	glVertex3f(m_zoomx[0],m_zoomy[1], m_zoomz[1]  );
	glVertex3f(m_zoomx[0],m_zoomy[0], m_zoomz[1] );
	glVertex3f(m_zoomx[1],m_zoomy[0], m_zoomz[1] );
	glVertex3f(m_zoomx[1],m_zoomy[1], m_zoomz[1]  );
	glEnd();

	glBegin( GL_LINES	);
	glColor3ub(0,0,0);
	glVertex3f(m_zoomx[0],m_zoomy[0], m_zoomz[0] );
	glVertex3f(m_zoomx[0],m_zoomy[0] , m_zoomz[1] );

	glVertex3f(m_zoomx[1],m_zoomy[0] , m_zoomz[0] );
	glVertex3f(m_zoomx[1],m_zoomy[0] , m_zoomz[1] );

	glVertex3f(m_zoomx[1],m_zoomy[1], m_zoomz[0]  );
	glVertex3f(m_zoomx[1],m_zoomy[1], m_zoomz[1]  );

	glVertex3f(m_zoomx[0],m_zoomy[1], m_zoomz[0]  );
	glVertex3f(m_zoomx[0],m_zoomy[1], m_zoomz[1]  );

	glEnd();

}

void OGLGraph3d::DrawTittle()
{
	glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
	glFrustum(0.0,0,0,(float)0, 0, 1);
  glMatrixMode(GL_MODELVIEW);
	glViewport(0,0,m_width,m_height );//trozo de ventana
	glOrtho(0,m_width ,0, m_height,-1,1);
	glColor3ub(200,200,200  );
	glBegin( GL_QUADS );
	//parte del titulo
	glVertex3f(0,m_height,-0.5f);
	glVertex3f(m_width,m_height,-0.5f);
	glVertex3f(m_width,m_height-OGL3D_TITTLE_HEIGHT,-0.5f);
	glVertex3f(0,m_height-OGL3D_TITTLE_HEIGHT ,-0.5f);
	//parte del eje x
	glVertex3f(0,m_freeSpaceXaxis,-0.5f);
	glVertex3f(m_width,m_freeSpaceXaxis,-0.5f);
	glVertex3f(m_width,0,-0.5f);
	glVertex3f(0,0 ,-0.5f);
	//parte del eje y
	glVertex3f(0,0,-0.5f);
	glVertex3f(m_freeSpaceYaxis,0,-0.5f);
	glVertex3f(m_freeSpaceYaxis,m_height,-0.5f);
	glVertex3f(0 ,m_height,-0.5f);
	//left margin
	glVertex3f(0,m_freeSpaceXaxis,1);
	glVertex3f(0,m_height,1);
	glVertex3f(OGL3D_LEFT_MARGIN,m_height,1);
	glVertex3f(OGL3D_LEFT_MARGIN ,m_freeSpaceXaxis,1);
	//margen derecho //SOBRE T0D0
	glVertex3f(m_width-OGL3D_RIGHT_MARGIN,m_freeSpaceXaxis,1);
	glVertex3f(m_width-OGL3D_RIGHT_MARGIN,m_height,1);
	glVertex3f(m_width,m_height,1);
	glVertex3f(m_width ,m_freeSpaceXaxis,1);
	glEnd(  );
	
	DrawTextInPixels(m_tittle,m_width/2-m_tittle.size()/2.0*OGL3D_FONT_WIDTH,m_height-OGL3D_FONT_HEIGHT*1,0,0,0x00f0f0f0,0x00ff0000);
	DrawTextInPixels(m_yAxisName,OGL3D_FONT_WIDTH*2,m_height-OGL3D_FONT_HEIGHT*1.1);
	DrawTextInPixels(m_xasisName,m_width-OGL3D_FONT_WIDTH*(m_xasisName.size()),3);
}

void OGLGraph3d::DibujarLeyenda()
{
	glViewport(0,0,m_width,m_height);//trozo de ventana
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,m_width ,0, m_height,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable (GL_LINE_SMOOTH);
	glDisable (GL_LINE_STIPPLE);
	glLineWidth(3);
	if (m_plines!=NULL)
	{
		CXYLineGL3D *pt;
		int nline;
		m_xLegendPosition=m_width/2-(m_linesperline<m_nlines?m_linesperline :m_nlines)*(m_widthLegend)/2;
		int line=0;//arriba a abajo
		int col=0;//columna
		for (nline=0,pt=m_plines;pt!=NULL;pt=pt->nextLine,nline++)
		{
			DrawTextInPixels(pt->lineName,m_xLegendPosition+ col*(m_widthLegend+20),4+m_nlegend_lines*OGL3D_FONT_HEIGHT-OGL3D_FONT_HEIGHT*(line+1),0.2f,1,m_backgroundLegendColor,pt->color);
			col++;
			if(col>=m_linesperline)
			{
				col=0;
				line++;
			}
		}
	}
}

void OGLGraph3d::DrawButtons()
{
	buttonAutozoom.DrawButton();
	buttonAutozoomTop.DrawButton();
	checkAutofix.DrawButton();

}

void OGLGraph3d::DrawAxis()
{
	glEnable (GL_LINE_SMOOTH);
	glDisable (GL_LINE_STIPPLE);

	glLineWidth(1);
	glBegin( GL_LINES );
	glColor4ubv((GLubyte*)&m_axisColor);
	glVertex3f(m_zoomx[0]>0 ? m_zoomx[0]:0,0, 0 );		//ejex
	glVertex3f(m_zoomx[1],0, 0 );

	glVertex3f(m_zoomx[0]>0?m_zoomx[0]:0,m_zoomy[0]>0?m_zoomy[0]:0, 0 );		//ejey
	glVertex3f(m_zoomx[0]>0?m_zoomx[0]:0, m_zoomy[1], 0 );
	glEnd();glBegin( GL_LINES );
	glColor3ub( 254,100,0 );

	glVertex3f(m_zoomx[0]>0?m_zoomx[0]:0,0,m_zoomz[0]>0?m_zoomz[0]:0);		//ejez
	glVertex3f(m_zoomx[0]>0?m_zoomx[0]:0,0 ,m_zoomz[1]);
	glEnd();
	if(m_posejesscroll)//zoom por boton central en los ejes
	{
		double x,y;
		this->PixelsToWorld(NULL,&x,&y,NULL);
		glColor4ubv((GLubyte*)&m_zoomAxisColor);
		glBegin( GL_POINTS );
		glVertex3f(m_ptoMejesinicial[0],m_ptoMejesinicial[1], 0.2f );		//puntos final e inicial
		if(m_posejesscroll==OGL3D_YAXIS)
			glVertex3f(m_ptoMejesinicial[0],y, 0 );
		else
			glVertex3f(x,m_ptoMejesinicial[1], 0 );
		glEnd();
		glBegin( GL_LINES );
		glVertex3f(m_ptoMejesinicial[0],m_ptoMejesinicial[1], 0.2f );	//linea paralela al eje
		if(m_posejesscroll==OGL3D_YAXIS)
			glVertex3f(m_ptoMejesinicial[0],y, 0 );
		else
			glVertex3f(x,m_ptoMejesinicial[1], 0 );
		glEnd();
	}
}

void OGLGraph3d::DibujaTextoReferencias(GLdouble minescala[2], GLdouble maxescala[2], GLdouble paso[2], string  [2])
{
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(5,0xAAAA  );
	glDisable (GL_LINE_SMOOTH);
	glLineWidth(0.4f);
	string txt;
	float  ind;
	glColor4ubv((GLubyte*)&m_fontColor);
	double cosy=cos(RAD(m_angy));
	double siny=sin(RAD(m_angy));
	double sinx=sin(RAD(m_angx));

	int precission=3;
	GLdouble zoomz[2]={m_zoomz[0],m_zoomz[1]/*GetLine(0)->GetPZ()[GetLine(0)->GetUsedLinesT()-1]*/};

	double posxaxis= 0< m_zoomx[0] || 0> m_zoomx[1] ? m_zoomx[0] : 0;
	double posyaxis= 0< m_zoomy[0] || 0> m_zoomy[1] ? m_zoomy[1] : 0;
	double poszaxis=m_angy>70 ? m_zoomz[0]:  m_zoomz[1];//	0< m_zoomz[0] || 0> m_zoomz[1] ? m_zoomz[1] : 0;

	//Z axis.
	OptimunAxisScale(zoomz,-1,6,&minescala[0],&maxescala[0],&paso[0],&precission);
	if(paso[0]>0.000001)/////horiz Z
	{
		//move text to left if text in left
		double incx=ABS(m_angy) <90 ?   m_fontWidth*cosy : 0;

		double fonth=m_fontHeight*(m_zoomx[1]-m_zoomx[0])/(float)(m_zoomy[1]-m_zoomy[0])*siny;
		fonth=ABS(fonth);

		for (ind=minescala[0];ind<=maxescala[0];ind+=paso[0])
		{
			//if(m_angy<0)//move up
				if(m_ejzhora)
			{
				string txtstr=PasaFecha((long)ind*1000+m_offsetz*1000);
				DrawText(txtstr,ind-txtstr.size()/2.0*m_fontWidth,posyaxis,poszaxis);
			}
			else
			{
				std::ostringstream tmp;
				tmp<< std::fixed;
				tmp<<std::setprecision(precission);
				tmp<<ind;
				DrawText(tmp.str(),posxaxis-tmp.tellp()*incx-fonth,posyaxis,ind);
			}
		}
	}

	// y axis.
	OptimunAxisScale(m_zoomy,-1,6,&minescala[0],&maxescala[0],&paso[0],&precission);
	if(paso[0]>0.000001)/////verticales
	{
		double zpos= m_fontHeight*(m_zoomz[1]-m_zoomz[0])/(double)(m_zoomy[1]-m_zoomy[0])*ABS(sinx)*cosy;

		for (ind=minescala[0];ind<=maxescala[0];ind+=paso[0])
		{

			std::ostringstream tmp;
			tmp<<std::setprecision(precission);
			tmp<< std::fixed;
			tmp<<ind;
			string sd=tmp.str();
			int ncaract=tmp.tellp();
			double xinc=0;

			double zposlateral=0;

			//Add z position if x axis is visible in lateral view
			if(m_angy <70 && m_angy>0)
				zposlateral=0;
			else
				zposlateral=0;//ncaract*m_fontWidth*(m_zoomz[1]-m_zoomz[0])/(double)(m_zoomx[1]-m_zoomx[0])*siny;

			if(ABS(m_angy) <90)
				xinc=ncaract*m_fontWidth;

			DrawText(tmp.str(), posxaxis- xinc, ind-m_fontHeight/4, poszaxis+zpos-zposlateral);
		}
	}

	//x axis
	OptimunAxisScale(m_zoomx,-1,6,&minescala[1],&maxescala[1],&paso[1],&precission);
	if(paso[1]>0.00001)
	{
		double zpos= m_fontHeight*(m_zoomz[1]-m_zoomz[0])/(double)(m_zoomy[1]-m_zoomy[0])*ABS(sinx)*cosy;
		double xpos=+m_fontHeight*(m_zoomx[1]-m_zoomx[0])/(double)(m_zoomy[1]-m_zoomy[0])*0.5*siny;

		for (ind=minescala[1];ind<=m_zoomx[1];ind+=paso[1])
		{
			std::ostringstream tmp;
			tmp<<std::setprecision(precission);
			tmp<< std::fixed;
			tmp<<ind;
			string sd=tmp.str();
			int ncaract=tmp.tellp();

			double zposlateral=0;

			//Add z position if x axis is visible in lateral view
			//if(ABS(m_angy) >70)
				zposlateral=0;//ncaract*m_fontWidth*(m_zoomz[1]-m_zoomz[0])/(double)(m_zoomx[1]-m_zoomx[0])*siny;
			if(m_angy <70 && m_angy>0)
				zposlateral=0;

			glColor3ub(OGL3D_RED(m_fontColor),OGL3D_GREEN(m_fontColor),OGL3D_BLUE(m_fontColor) );
			DrawText(sd,ind- ncaract/2.0*m_fontWidth*cosy+xpos,posyaxis,poszaxis+zpos-zposlateral);
		}
	}
}

void OGLGraph3d::DrawAsLines()
{
	glEnable (GL_LINE_SMOOTH);
	glDisable (GL_LINE_STIPPLE);
	glLineWidth(OGL3D_LINE_WIDTH);

	CXYLineGL3D*pt;
	if (m_plines!=NULL)
	{
		LockDataMutex();
		for (pt=m_plines;pt!=NULL;pt=pt->nextLine)
		{
			if(pt->visible)
			{
				int nline=0;
				int fint=m_lineat2d<0 ?pt->GetUsedLinesT():m_lineat2d+1;
				for(int r=m_lineat2d<0 ? 0 : m_lineat2d;r<fint;r++)
				{
					double z=-pt->GetPZ()[nline];
					double* x=pt->GetPX();double* y=pt->GetPY(r);
					glBegin( GL_LINE_STRIP );
					glColor3ub( OGL3D_RED(pt->color),OGL3D_GREEN(pt->color), OGL3D_BLUE(pt->color));	// Green
					int xinicio=m_lineat2d,xfin=m_lineat2d;
					for (xinicio=0;xinicio<pt->nPoints&&x[xinicio]<m_zoomx[0];xinicio++);
					for (xfin=pt->nPoints-1;xfin>0&&x[xfin]>m_zoomx[1];xfin--);
					for( ;xinicio<xfin;xinicio++)
					{
						glColor3f(y[xinicio]/0.015f*OGL3D_RED(pt->color),y[xinicio]/0.015f*OGL3D_GREEN(pt->color),
							y[xinicio]/0.015f*(float)OGL3D_BLUE(pt->color));	// Green
						glVertex3f( x[xinicio], y[xinicio],z );
					}
					glEnd();
			/*		if(m_lineat2d>=0)//indicador de posicion junto a leyenda
					{
						string txt;
						if(m_ejzhora==1)
						{
							txt=PasaFecha((long)(GetLine(nline)->GetPZ()[m_lineat2d]*1000.0f)+m_offsetz*1000);
							DrawText(txt,m_posXley-10*m_ancholetra,m_posYley-m_altletra,1);
						}
						else
						{
							std::ostringstream tmp;
							tmp<< std::fixed;
							tmp<<std::setprecision(precision);
							tmp<<GetLine(nline)->GetPZ()[m_lineat2d];

							DrawText(tmp.str(),ind-tmp.tellp()/2.0*m_fontWidth,m_zoomy[0]-m_fontHeight);
					}*/
					glColor3ub(OGL3D_RED(pt->color),OGL3D_GREEN(pt->color),OGL3D_BLUE(pt->color));
					for(int c=0;c<pt->m_puntos_interes.npuntos;c++)
					{
						glBegin( GL_POINTS );
						glVertex3f( pt->m_puntos_interes.ppuntosx[c], pt->m_puntos_interes.ppuntosy[c],z );
						glEnd(  );
					}

					nline++;
				}

			}
		}
		UnlockDataMutex();
	}
}

void OGLGraph3d::DrawAsPoligons(int posmax/*=-1*/,int posmin/*=-1*/)
{
	glEnable (GL_LINE_SMOOTH);
	glDisable (GL_LINE_STIPPLE);
	glPolygonMode(GL_FRONT_AND_BACK,m_renderizado);
	glLineWidth(OGL3D_LINE_WIDTH);
	CXYLineGL3D*pt;
	if (m_plines!=NULL)
	{
		LockDataMutex();
		int ilinea=0;
		for (pt=m_plines;pt!=NULL;pt=pt->nextLine)
		{
			if(pt->visible)
			{
				int fint=pt->GetUsedLinesT(),maxl=MaximoLocal(ilinea);
				maxl=maxl<0 ? 1: maxl;
				int minl=MinimoLocal(ilinea);
				minl=minl<0 ? 1: minl;
				float max=posmax>0 ? pt->GetPY(0)[posmax] : pt->GetPY(maxl/pt->nPoints)[maxl%pt->nPoints];
				float min=posmin>0 ? pt->GetPY(0)[posmin] : pt->GetPY(0)[minl];
				float total=m_colormanual>0 ? m_colormanual*(max-min) : max-min;
				struct strColorTable3D* yC;
				int nlinet=0;
				int xinicio=m_lineat2d,xfin=m_lineat2d;
				double* x=pt->GetPX();
				for (xinicio=0;xinicio<pt->nPoints&&x[xinicio]<this->m_zoomx[0];xinicio++);
				for (xfin=pt->nPoints-1;xfin>0&&x[xfin]>this->m_zoomx[1];xfin--);
#				ifdef COLORMAP3D
				if(m_colormaps[ilinea]!=NULL)
				{
					m_colormaps[ilinea]->SetMinMax(min,max);
#					ifdef OPENMP_GRAPHS3D
#					pragma omp parallel for
#					endif
					for(int r=0;r<pt->GetUsedLinesT();r++)//calculamos los colores
						m_colormaps[ilinea]->ValuesToColors(&pt->GetPY(r)[xinicio],xfin-xinicio+1,&pt->GetPYC(r)[xinicio],min,m_colormanual>0 ?m_colormanual : max);
				}
				else
#				endif
				{
#					ifdef OPENMP_GRAPHS3D
#					pragma omp parallel for
#					endif
					float red=OGL3D_RED(pt->color)/255.0f;
					float green=OGL3D_GREEN(pt->color)/255.0f;
					float blue=OGL3D_BLUE(pt->color)/255.0f;
					for(int r=0;r<fint;r++)//calculamos los colores
					{
						yC=pt->GetPYC(r);
						double *y=pt->GetPY(r);
						for(int c=xinicio;c<=xfin;c++)//calculamos los colores
						{
#							ifndef ESCALA_PROPORCIONAL
							yC[c].color[0]=(y[c]-min)/total* red;
							yC[c].color[1]=(y[c]-min)/total*green;
							yC[c].color[2]=(y[c]-min)/total* blue;
#							else
							yC[c].color[0]=red-1+(y[c]-min)/total* red;
							yC[c].color[1]=green-1+(y[c]-min)/total* green;
							yC[c].color[2]=green-1+(y[c]-min)/total* blue;
#							endif
						}
					}
				}
				for(int r=0 ;r<fint-1;r++)
				{
					double z=pt->GetPZ()[nlinet];
					double* y=pt->GetPY(r);
					double* y1=pt->GetPY(r+1);
					//glBegin(GL_QUAD_STRIP );
					//glBegin (GL_QUADS);
					glBegin(GL_TRIANGLE_STRIP );
					glShadeModel(GL_SMOOTH);
#					ifdef COLORMAP3D
					yC=pt->GetPYC(r);
					double z1=pt->GetPZ()[nlinet+1];
					struct strColorTable3D* yC1=pt->GetPYC(r+1);
					for(int r=xinicio;r<xfin;xfin-r>2? r+=2:r++)
					{
						glColor3f(yC[r].color[0],yC[r].color[1],yC[r].color[2] );
						glVertex3f( x[r], y[r],z );
						glColor3f(yC1[r].color[0],yC1[r].color[1],yC1[r].color[2] );
						glVertex3f( x[r], y1[r],z1 );
						glColor3f(yC[r+1].color[0],yC[r+1].color[1],yC[r+1].color[2] );
						glVertex3f( x[r+1], y[r+1],z );
						glColor3f(yC1[r+1].color[0],yC1[r+1].color[1],yC1[r+1].color[2] );
						glVertex3f( x[r+1], y1[r+1],z1 );

				/*		double ax = x[r+1]-x[r],
							ay = y[r+1]-y[r],
							by = y[r+1]-y[r],
							bz = z1-z,
							//====== Normal vector coordinates
							vx = ay*bz,
							vy = -bz*ax,
							vz = ax*by,
							//====== Normal vector length
							v  = float(sqrt(vx*vx + vy*vy + vz*vz));
						//====== Scale to unity
						vx /= v;
						vy /= v;
						vz /= v;
						//====== Set the normal vector
						glNormal3f (vx,vy,vz);*/
					}

					nlinet++;//contador de lineas de tiempo
#					endif
					glEnd();

				}
			}
			for(int c=0;c<pt->m_puntos_interes.npuntos;c++)
			{
					glBegin( GL_POINTS );
				glVertex3f( pt->m_puntos_interes.ppuntosx[c], pt->m_puntos_interes.ppuntosy[c],pt->m_puntos_interes.ppuntosz[c] );
				glEnd(  );
			}
			ilinea++;//contador de lineas de grafica
		}
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		UnlockDataMutex();
	}
}

void OGLGraph3d::DrawDraws()
{
	for(iterdrawing iter=m_mapdibujos.begin();iter!=m_mapdibujos.end();iter++)
	{
		drawingsstr tab=iter->second;
		double ONE_DEGREE = ( 3.14159265358979323846/180);
		double THREE_SIXTY = 2 *  3.14159265358979323846;
		double a,x,y;
		switch(tab.drawingType)
		{
		case CIRCUNFERENCE3D:
			glLoadIdentity();
			glEnable (GL_LINE_SMOOTH);
			glDisable (GL_LINE_STIPPLE);
			glLineWidth(OGL3D_LINE_WIDTH);
			glColor3ub( ((GLubyte*)&tab.colorpen)[0],((GLubyte*)&tab.colorpen)[1],((GLubyte*)&tab.colorpen)[2] );	// Green
			glBegin(GL_LINE_STRIP );
			for ( a=0; a<THREE_SIXTY; a+=ONE_DEGREE) {
				x = tab.tamano* (cos(a)) ;
				y = tab.tamano * (sin(a)) ;
				 glVertex2d(x, y);
			}
			glEnd();
			break;
		case CIRCLE3D:
			glColor3ub( ((GLubyte*)&tab.colorbrush)[0],((GLubyte*)&tab.colorbrush)[1],((GLubyte*)&tab.colorbrush)[2] );	// Green
			glLoadIdentity();
			glTranslatef(tab.posx,tab.posy,tab.posz);
			glutSolidSphere(tab.tamano,80,80);
			glLoadIdentity();
			break;
		}
	}
}

void OGLGraph3d::DrawManualMeasurement()
{
	double posx,posy;
	PixelsToWorld(NULL,&posx,&posy);

	glBegin( GL_LINE_LOOP );
	glColor3ub(OGL3D_RED(m_distancesColor),OGL3D_GREEN(m_distancesColor),OGL3D_BLUE(m_distancesColor) );	// amarillo
	glVertex3d( m_initialManualDistancePoint[0],m_initialManualDistancePoint[1], 0.1f );//pto inicial
	glVertex3d( m_initialManualDistancePoint[0],posy, 0.1f );//vertical
	glVertex3d(posx,posy, 0.1f );//horizontal
	glEnd();

	glColor3ub( 0,0,0 );
	string txtf=strprintf("%.3f",fabs(posy-m_initialManualDistancePoint[1]));

	if(posx>m_initialManualDistancePoint[0])//triangulo hacia derecha
	{
		DrawText(txtf,m_initialManualDistancePoint[0]-m_fontWidth*(txtf.length()+1),(posy-m_initialManualDistancePoint[1])/2+m_initialManualDistancePoint[1],0.2f,1,m_backgroundDistances);
		txtf=strprintf("%.3f",sqrt(pow(posx-m_initialManualDistancePoint[0],2)+pow(posy-m_initialManualDistancePoint[1],2)));//texto diagonal
		DrawText(txtf,(posx-m_initialManualDistancePoint[0])/2+m_initialManualDistancePoint[0]+m_fontWidth,(posy+m_initialManualDistancePoint[1])/2-(posy<m_initialManualDistancePoint[1]?0:m_fontHeight),0.2f,1,m_backgroundDistances);
	}
	else//triangulo hacia izda
	{
		DrawText(txtf,m_initialManualDistancePoint[0]+m_fontWidth,(posy-m_initialManualDistancePoint[1])/2+m_initialManualDistancePoint[1],0.2f,1,m_backgroundDistances);
		txtf=strprintf("%.3f",sqrt(pow(posx-m_initialManualDistancePoint[0],2)+pow(posy-m_initialManualDistancePoint[1],2)));//texto diagonal
		DrawText(txtf,fabs(posx-m_initialManualDistancePoint[0])/2+posx-m_fontWidth*(txtf.length()+1),(posy+m_initialManualDistancePoint[1])/2-(posy<m_initialManualDistancePoint[1]?0:m_fontHeight),0.2f,1,m_backgroundDistances);
	}

	string tstr;

	if(m_ejzhora)
	{
		tstr=PasaFecha((long)fabs(posx-m_initialManualDistancePoint[0])*1000);//texto horizontal
		DrawText(tstr,(posx-m_initialManualDistancePoint[0])/2+m_initialManualDistancePoint[0]-txtf.length()/2*m_fontWidth,posy+(posy<m_initialManualDistancePoint[1]?-m_fontHeight*1.3f:m_fontHeight/1.5f),0.2f,1,m_backgroundDistances);
	}
	else
	{
		txtf=strprintf("%.3f",fabs(posx-m_initialManualDistancePoint[0]));//texto horizontal
		DrawText(txtf,(posx-m_initialManualDistancePoint[0])/2+m_initialManualDistancePoint[0]-txtf.length()/2*m_fontWidth,posy+(posy<m_initialManualDistancePoint[1]?-m_fontHeight*1.3f:m_fontHeight/1.5f),0.2f,1,m_backgroundDistances);
	}
}

void OGLGraph3d::DibujaReferencias(GLdouble minescala[2],GLdouble maxescala[2],GLdouble paso[2])
{
	glEnable(GL_LINE_STIPPLE);//lineas de referencia discontinuas
	glLineStipple(1,0x0707  );
	glDisable (GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	glColor4ubv((GLubyte*)&m_fontColor);
	glEnable(GL_LINE_STIPPLE);//lineas de referencia discontinuas
	glLineStipple(5,0xAAAA  );
	glDisable (GL_LINE_SMOOTH);
	glLineWidth(0.4f);

	double posyaxis= 0< m_zoomy[0] || 0> m_zoomy[1] ? m_zoomy[0] : 0;
	double poszaxis= 0< m_zoomz[0] || 0> m_zoomz[1] ? m_zoomz[0] : 0;

	float ind;

	//x axis  plane
	if(paso[0]>0.000001)
		for (ind=minescala[0];ind<=maxescala[0];ind+=paso[0])
		{
			glBegin( GL_LINES );
				glVertex3f(m_zoomx[0],ind ,  poszaxis);
				glVertex3f(m_zoomx[1],ind, poszaxis  );
			glEnd();
		}

	//z axis
	if(paso[1]>0.000001)
		for (ind=minescala[1];ind<=m_zoomx[1];ind+=paso[1])
		{
			glBegin( GL_LINES );
			glVertex3f(ind, posyaxis,m_zoomz[0] );
			glVertex3f(ind, posyaxis,m_zoomz[1] );
			glEnd();
		}

	//y axis
	glLineWidth(1.0f);
	if(paso[1]>0.000001)
		for (ind=minescala[1];ind<=m_zoomx[1];ind+=paso[1])
		{
			glBegin( GL_LINES );
			glVertex3d(ind, m_zoomy[0] , poszaxis);
			glVertex3d(ind, m_zoomy[1] , poszaxis);
			glEnd();
		}
}

void OGLGraph3d::DrawZoomBox()
{
	CHARTS3D_POINTD pt=GetMousePoint();

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(5,0xAAAA  );
	glEnable (GL_LINE_SMOOTH);
	glLineWidth(3.5);
	glColor4ubv((GLubyte*)&m_fontColor);

	double x0, x1,y0,y1;

	PixelsToWorld(&m_initialZoomBoxPoint,&x0,&y0);
	PixelsToWorld(&pt,&x1,&y1);

	glBegin(GL_LINE_LOOP);
	glVertex2d(x0,y0  );
	glVertex2d(x1,y0  );
	glVertex2d(x1,y1 );
	glVertex2d( x0,y1 );
	glEnd();
}

void OGLGraph3d::DrawCurrentPosition()
{
	double posx,posy;

	CHARTS3D_POINTD pt=GetMousePoint();
	PixelsToWorld(&pt,&posx,&posy);
	if(posx<m_zoomx[0] || posx>m_zoomx[1] || posy<m_zoomy[0] || posy>m_zoomy[1])
		return;
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.5f);
	glColor3ub( OGL3D_REDC(m_currentPositionColor),OGL3D_GREEN(m_currentPositionColor),OGL3D_BLUE(m_currentPositionColor));
	double  x,y;
	PixelsToWorld(&pt,&x,&y);
	glBegin(GL_LINES);

	glVertex3d(m_zoomx[0],y ,0.6 );
	glVertex3d( m_zoomx[1],y ,0.6);
	glVertex3d(x,m_zoomy[0],0.6);
	glVertex3d( x,m_zoomy[1],0.6 );
	glEnd();

	string txt;

	if(m_ejzhora)
	 txt= strprintf("%s,%f",PasaFecha((long)x*1000).c_str(),y);
	else
	 txt= strprintf("%.3f ; %.3f",x,y);
	int txtlength=txt.length();
	if(m_zoomy[1]-y <m_fontHeight)//no se ve por encima
	{
		if(m_zoomx[1]-x>m_fontWidth*txtlength)//por la derecha
			DrawText(txt,x+m_fontWidth/2,y-m_fontHeight,0.6f,1,m_backgroundCurrentValue,0x000000,GLUT_BITMAP_HELVETICA_18);
		else//no se ve por la derecha
			DrawText(txt,x-m_fontWidth*txtlength,y-m_fontHeight,0.6f,1,m_backgroundCurrentValue,0x000000,GLUT_BITMAP_HELVETICA_18);
	}
	else//entra por encima
	{
		if(m_zoomx[1]-x>m_fontWidth*txtlength)//por la derecha
			DrawText(txt,x+m_fontWidth/2,y+m_fontHeight/2,0.6f,1,m_backgroundCurrentValue,0x000000,GLUT_BITMAP_HELVETICA_18);
		else//no se ve por la derecha
			DrawText(txt,x-m_fontWidth*txtlength,y+m_fontHeight/2,0.6f,1,m_backgroundCurrentValue,0x000000,GLUT_BITMAP_HELVETICA_18);
	}
}

void OGLGraph3d::DrawText( string txt,const double posx,const double posy,const float posz/*=0*/,int fondo/*=0*/ ,COLORREF colorfondo/*=0*/,COLORREF colorletra/*0*/,void* tipoletra/*=NULL*/)
{
	int colorini[4];

	if(fondo)
	{
		glGetIntegerv(GL_CURRENT_COLOR,(int*)&colorini);
		glBegin( GL_QUADS );	
		glColor4ubv((GLubyte*)&colorfondo);
		glVertex3d(posx , posy-m_fontHeight/5 ,posz-0.01f);
		glVertex3d(posx+m_fontWidth*txt.size() , posy-m_fontHeight/5 ,posz-0.01f );

		glVertex3d(posx +m_fontWidth*txt.size(), posy+m_fontHeight -m_fontHeight/5,posz-0.01f );
		glVertex3d( posx , posy+m_fontHeight-m_fontHeight/5 ,posz-0.01f );
		glEnd();
		glColor3ub( colorini[0],colorini[1],colorini[2] );
	}

  if((int)colorletra!=-1)
		glColor4ubv((GLubyte*)&colorletra);

	if(tipoletra==NULL)
		tipoletra=TIPO_LETRA;

	char* buf=(char*)txt.c_str();

	for(int c=txt.size();c>=0;c--)
	{
		double inc=c*m_fontWidth;
		double zscale=(m_zoomz[1]-m_zoomz[0])/(m_zoomx[1]-m_zoomx[0]);
		double posxc=posx+ inc*cos(RAD(m_angy));
		glRasterPos3d( posxc,posy ,posz-inc*sin(RAD(m_angy))*zscale );
    glutBitmapCharacter(tipoletra,buf[c]);
	}
}

void OGLGraph3d::DrawTextInPixels( string txt,const double posx,const double posy,const float posz/*=0*/,int fondo/*=0*/ ,COLORREF colorfondo/*=0*/,COLORREF colorletra/*0*/,void* tipoletra/*=NULL*/)
{
	int colorini[4];
	glGetIntegerv(GL_CURRENT_COLOR,(int*)&colorini);

	if(fondo)
	{
		glBegin( GL_QUADS );
		glColor4ubv((GLubyte*)&colorfondo);

		glVertex3d(posx , posy-OGL3D_FONT_HEIGHT/5 ,posz-0.01f);
		glVertex3d(posx+OGL3D_FONT_WIDTH*txt.size() , posy-OGL3D_FONT_HEIGHT/5 ,posz-0.01f );

		glVertex3d(posx +OGL3D_FONT_WIDTH*txt.size(), posy+OGL3D_FONT_HEIGHT -OGL3D_FONT_HEIGHT/5,posz-0.01f );
		glVertex3d( posx , posy+OGL3D_FONT_HEIGHT-OGL3D_FONT_HEIGHT/5 ,posz-0.01f );

		glEnd();
		glColor3ub( colorini[0],colorini[1],colorini[2] );
	}

	if((int)colorletra!=-1)
		glColor4ubv((GLubyte*)&colorletra);
	if(tipoletra==NULL)
		tipoletra=TIPO_LETRA;

	char* buf=(char*)txt.c_str();

	glRasterPos3d(posx,posy ,posz );
	for(unsigned int c=0;c<txt.size();c++)
	{		
		glutBitmapCharacter(tipoletra,buf[c]);
	}
	glColor3ub( colorini[0],colorini[1],colorini[2] );
}

void OGLGraph3d::DrawMaxValue(int pos)
{
	if(pos<0)
		return;
	string txt;
	int posx=pos < GetLine(m_focusLineIndex)->nPoints ? pos: pos %GetLine(m_focusLineIndex)->nPoints ;
	float x=GetLine(m_focusLineIndex)->GetPX()[posx];
	float y=GetLine(m_focusLineIndex)->GetPY(0)[pos];
	float z=GetLine(m_focusLineIndex)->GetPZ()[pos/GetLine(m_focusLineIndex)->nPoints];
	if(m_ejzhora)
		txt=strprintf("%f,%f,%s",x,y,PasaFecha((long)z*1000+this->m_offsetz*1000));
	else
		txt=strprintf("%f,%f",x,y);
#ifdef DIBUJAR_NEGATIVOS
	DrawText(txt,x+m_fontWidth/2,y+m_fontHeight/2,-z,m_angy>0?1:0,m_backgroundCurrentValue,GetLine(m_focusLineIndex)->color);
#else
	DrawText(txt,x+m_fontWidth/2,y+m_fontHeight/2,z,/*m_angy>0?1:*/0,m_backgroundCurrentValue,GetLine(m_focusLineIndex)->color);
#endif
}

///
///ZOOM
///
void OGLGraph3d::TopView()
{
	m_angx=-90;
	m_angy=0;
	double minx=0,maxx=100,miny=00,maxy=100,minz=-0,maxz=100;
	int nline;
	CXYLineGL3D *pt;

	SearchLimits(m_plines->GetPZ(),(long)m_plines->m_nUsedLinesT,&minz,&maxz);

	for ( nline=0,pt=m_plines;pt!=NULL;pt=pt->nextLine,nline++)
	{
		if(pt->visible)
		{
			double lowx,higx;
			SearchLimits(pt->GetPX(),pt->nPoints,&lowx,&higx);
			minx= minx< lowx ? minx : lowx;
			maxx= maxx< higx ? higx : maxx;

			double lowy,higy;
			for(int r=0;r<pt->GetUsedLinesT();r++)
			{
				SearchLimits(pt->GetPY(r),(long)pt->nPoints,&lowy,&higy);
				miny= miny< lowy ? miny : lowy;
				maxy= maxy < higy ? higy : maxy;
			}
		}
	}

	if(minx==DBL_MAX || maxx==-DBL_MAX||miny==DBL_MAX||maxy==-DBL_MAX||
		 minz==DBL_MAX ||maxz==-DBL_MAX)
		return;

	SetZoom(minx,maxx,miny,maxy,minz,maxz);
}

void OGLGraph3d::SetZoom(GLdouble minx, GLdouble maxx, GLdouble miny, GLdouble maxy,GLdouble minz, GLdouble maxz)
{
	m_angy=m_angy>360 ? m_angy-360:m_angy;
	m_angy=m_angy<-360 ? m_angy+360:m_angy;

	m_zoomx[0]=MIN(minx,maxx);
	m_zoomx[1]=MAX(minx,maxx);
	m_zoomy[0]=MIN(miny,maxy);
	m_zoomy[1]=MAX(miny,maxy);

	m_zoomz[0]=MIN(minz,maxz);
	m_zoomz[1]=MAX(minz,maxz);

	CalculateView();
	RePaint();
}

void OGLGraph3d::AutoZoom(float anglex,float angley)
{
	if (m_plines==NULL)
		return;
	m_angx=anglex;
	m_angy=angley;
	double minx=0,maxx=100,miny=00,maxy=100,minz=-0,maxz=100;
	int nline;
	CXYLineGL3D *pt;

	SearchLimits(m_plines->GetPZ(),(long)m_plines->m_nUsedLinesT,&minz,&maxz);

	for ( nline=0,pt=m_plines;pt!=NULL;pt=pt->nextLine,nline++)
	{
		if(pt->visible)
		{
			double lowx,higx;
			SearchLimits(pt->GetPX(),pt->nPoints,&lowx,&higx);
			minx= minx< lowx ? minx : lowx;
			maxx= maxx< higx ? higx : maxx;

			double lowy,higy;
			for(int r=0;r<pt->GetUsedLinesT();r++)
			{
				SearchLimits(pt->GetPY(r),(long)pt->nPoints,&lowy,&higy);
				miny= miny< lowy ? miny : lowy;
				maxy= maxy < higy ? higy : maxy;
			}
		}
	}

	if(minx==DBL_MAX || maxx==-DBL_MAX||miny==DBL_MAX||maxy==-DBL_MAX||
		 minz==DBL_MAX ||maxz==-DBL_MAX)
		return;

	SetZoom(minx,maxx,miny,maxy,minz,maxz);
}

void OGLGraph3d::SearchLimits(const double* buf,long ndat,double *min,double *max)
{
	*min=DBL_MAX;
	*max=-DBL_MAX;

	for(int i=0;i<ndat;i++)
	{
		if(buf[i]<*min)
			*min=buf[i];
		if(buf[i]>*max)
			*max=buf[i];		
	}
}

void OGLGraph3d::ZoomEjeY(int zDelta,GLdouble y)
{
	GLdouble total=m_zoomy[1]-m_zoomy[0];
	GLdouble med=(m_zoomy[1]+m_zoomy[0])/2;

	if(zDelta<0)	//zoom fuera
	{
		m_zoomy[0]-=total/2*(y-m_zoomy[0])/(med-m_zoomy[0]);
		m_zoomy[1]+=total/2*(m_zoomy[1]-y)/(total/2);
	}
	else
	{
		m_zoomy[0]+=total/4*(y-m_zoomy[0])/(med-m_zoomy[0]);
		m_zoomy[1]-=total/4*(m_zoomy[1]-y)/(total/2);
	}
	CalculateView();
}

void OGLGraph3d::ZoomEjeX(int zDelta, GLdouble x)
{

	GLdouble total=m_zoomx[1]-m_zoomx[0];
	GLdouble med=(m_zoomx[1]+m_zoomx[0])/2;
	if(zDelta<0)	//zoom fuera
	{
		m_zoomx[0]-=total/2*(x-m_zoomx[0])/(med-m_zoomx[0]);
		m_zoomx[1]+=total/2*(m_zoomx[1]-x)/(total/2);
	}
	else
	{
		m_zoomx[0]+=total/4*(x-m_zoomx[0])/(med-m_zoomx[0]);
		m_zoomx[1]-=total/4*(m_zoomx[1]-x)/(total/2);
	}
	CalculateView();
}

///
///CHART CALCULATION
///
int  OGLGraph3d::PixelsToWorld( const CHARTS3D_POINTD *ppoint,double* x,double*y,GLdouble*z/*=0*/)
{

	CHARTS3D_POINTD point;
	if(ppoint==NULL)//cojemos el raton
	{
		point=GetMousePoint();;
	}
	else
		point=CHARTS3D_POINTD(ppoint->x,ppoint->y);
	point.y=m_height-point.y;

	GLdouble modelMatrix[16];
	GLdouble xd,yd,zd;
	glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);

	GLdouble projMatrix[16];
	glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);

	int viewport[4];

	glGetIntegerv(GL_VIEWPORT,viewport);
	glReadPixels((int)*x,(int)*y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zd);

	gluUnProject((GLdouble)point.x,(GLdouble)point.y,zd,
		modelMatrix,		projMatrix,		viewport,	&xd, 	&yd,	&zd 	);

	*x=(float)xd;
	*y=(float)yd;

	if(z!=NULL)
		*z=(float)zd;
	return 1;
}

void OGLGraph3d::WorldToPixels( double x,double y,CHARTS3D_POINTD *point)
{
	BlockPaint(0);
	y=m_height-y;
	GLdouble modelMatrix[16];
	GLdouble xd,yd,zd;
	glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);
	GLdouble projMatrix[16];
	glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	gluProject(x,y,0,modelMatrix,projMatrix,viewport,&xd,&yd,&zd );
	point->x=(long)xd;
	point->y=(long)yd;
	unBlockPaint();
}

string OGLGraph3d::SearchClosePoint( CHARTS3D_POINTD )
{
	return "";
	//todo
	/*double * iniciox,*inicioy;
	GLdouble x,max,y;
	string salida="";
	if(m_nlines<=0)
		return salida;
	iniciox=this->GetLine(m_focusLineIndex)->GetPX();
	inicioy=GetLine(m_focusLineIndex)->GetPY();
	BlockPaint(0);
	PixelsToWorld(&pt,&x,&y);
	unBlockPaint();
	max=m_zoomx[1];
	GLdouble total=m_zoomy[1]-m_zoomy[0];
	total/=10;
	int npuntos=GetLine(m_focusLineIndex)->GetNPoints();
	for(int i=0;i<=npuntos-2;i++)
	{
		if(iniciox[i]>max)
			break ;
		if(iniciox[i]<=x&&iniciox[i+1]>=x)	// punto cerca de la grafica
		{
			if((fabs(inicioy[i]-y))<total)
				salida=GetTextFromPoint(iniciox[i],inicioy[i]);
			else
			{
				if((fabs(inicioy[i+1]-y))<total)
					salida=GetTextFromPoint(iniciox[i+1],inicioy[i+1]);
				else
				if((fabs(inicioy[i-1]-y))<total)
					salida=GetTextFromPoint(iniciox[i-1],inicioy[i-1]);
			}
			return salida;
		}
	}
	return salida;*/
}

string OGLGraph3d::GetTextFromPoint(double x,double y)//
{
 return strprintf("X=%.4f\nY=%.4f",x,y);
}

int OGLGraph3d::GetNextVisible()
{
	for(int n=0;n<m_nlines;n++)
		if (GetLine(n)->visible!=0)
			return n;
	return -1;
}

string OGLGraph3d::strprintf(const char* format,...)
{
	char out[400],*used;
	int ndat=strlen(format);

	if(ndat<200)
		used=new char[2*ndat];
	else
		used=out;

	va_list args;
	va_start (args, format);
	vsnprintf (used,256,format, args);
	va_end (args);

	string str=used;
	if(ndat<200)
		delete[] used;
	return str;
}

string OGLGraph3d::PasaFecha(long msegundos)
{
	string msg;

	msg=this->strprintf("%02d:%02d:%02d.%03d",msegundos/3600000,msegundos%3600000/60000,msegundos%3600000%60000/1000,msegundos%3600000%60000%1000);
	return msg;
}

void OGLGraph3d::CalculaLeyenda()
{
	if(m_nlines)
	{
		m_linesperline=(m_width-60)/(m_nlines*(m_widthLegend));
		m_linesperline=m_linesperline>0 ?m_linesperline:1;
		m_nlegend_lines=m_linesperline<m_nlines?m_nlines/m_linesperline:1;
		buttonAutozoom.ypos=m_nlegend_lines*OGL3D_FONT_HEIGHT+5;
		buttonAutozoomTop.ypos=m_nlegend_lines*OGL3D_FONT_HEIGHT+5;
		checkAutofix.ypos=m_nlegend_lines*OGL3D_FONT_HEIGHT+5;
	}
	m_freeSpaceXaxis=floor(OGL3D_FONT_HEIGHT*1.5+m_nlegend_lines*OGL3D_FONT_HEIGHT);
}
//
//
//
void OGLGraph3d::CalculateView()
{
	m_fontHeight=(m_zoomy[1]-m_zoomy[0]) * 1.4 / m_height*OGL3D_FONT_HEIGHT;
	m_fontWidth=OGL3D_FONT_WIDTH/(float)m_width * (m_zoomx[1]-m_zoomx[0]) * 1.4;

	//check view when rotating to increase/decrease view zone.
	//Pass y angle to +-90
	double angy90=m_angy>=90 ? m_angy - 90 : m_angy;
	if(m_angy <= -90)
		angy90=m_angy +90;

	double incppsx=((m_zoomx[1]-m_zoomx[0])/2)*ABS(sin(RAD(angy90*2)));

	double incppsz= ABS((m_zoomz[1]-m_zoomz[0])/4)*ABS(sin(RAD(m_angx*2)));
	double incppsz1=ABS((m_zoomz[1]-m_zoomz[0])/4)*ABS(sin(RAD(angy90*2)));

	incppsz=MAX(incppsz,incppsz1);

	double incppsy=((m_zoomy[1]-m_zoomy[0])/2)*ABS(sin(RAD((m_angx*2))));

	int charsx,charsy,charsz;
	CalculateMaxLenghtText(&charsx,&charsy,&charsz);

	m_zoomGLx[0]=m_zoomx[0]-incppsx-m_fontWidth*4;
	m_zoomGLx[1]=m_zoomx[1]+incppsx+m_fontWidth*4;

	m_zoomGLy[0]=m_zoomy[0]-incppsy-m_fontHeight*1.5;
	m_zoomGLy[1]=m_zoomy[1]+incppsy+m_fontHeight*1.5;

	double inczheigh=ABS(m_fontHeight*1.2*(m_zoomz[1]-m_zoomz[0])/(double)(m_zoomy[1]-m_zoomy[0])*cos(RAD(m_angy)));
	double inczwidth=ABS(m_fontWidth*(charsx+1)*(m_zoomz[1]-m_zoomz[0])/(double)(m_zoomx[1]-m_zoomx[0])*sin(RAD(m_angy)));
	m_zoomGLz[0]=m_zoomz[0]-incppsz-inczheigh-inczwidth;
	m_zoomGLz[1]=m_zoomz[1]+incppsz+inczheigh;

	//printf("y=%.1f  x=%.1f x(%.0f,%.0f) y(%.0f,%.0f) z(%.0f,%.0f) nearz(%.0f,%.0f)\n",m_angy,m_angx,
	//			 m_zoomGLx[0],m_zoomGLx[1],m_zoomGLy[0],m_zoomGLy[1],m_zoomGLz[0],m_zoomGLz[1],-m_zoomGLz[1], -m_zoomGLz[0]);
}

void OGLGraph3d::CalculateMaxLenghtText(int *x,int *y,int *z)
{
	std::ostringstream tmp;
	tmp<<m_zoomz[0];
	*z=tmp.tellp();
	tmp.str("");
	tmp.clear();
	tmp<<m_zoomz[1];
	*z=MAX((int)tmp.tellp(),*z);

	tmp.str("");
	tmp.clear();
	tmp<<m_zoomy[0];
	*y=tmp.tellp();
	tmp.str("");
	tmp.clear();
	tmp<<m_zoomy[1];
	*y=MAX((int)tmp.tellp(),*y);

	tmp.str("");
	tmp.clear();
	tmp<<m_zoomx[0];
	*x=tmp.tellp();
	string sd=tmp.str();
	tmp.clear();
	tmp.str("");
	tmp<<m_zoomx[1];
	sd=tmp.str();
	*x=MAX((int)tmp.tellp(),*x);

}

void OGLGraph3d::RotateFromPoint(glm::vec3 point,glm::vec3 ,double  )
{
	glm::mat4 translate = glm::translate(glm::mat4(1.f), glm::vec3((m_zoomx[1]-m_zoomx[0])/2, 0.f, 0.f));
	glm::vec4 vector(point.x,point.y,point.z,1);
	glm::vec4 transformedVector = translate * vector;
	if(transformedVector.w != 0)
		transformedVector/=transformedVector.w;
}

void OGLGraph3d::OptimunAxisScale(GLdouble minmaxIn[2],GLdouble stepIn,int nTipsIn,GLdouble* minOut,
																GLdouble* maxOut,GLdouble* stepOut,int* precision)
{
	GLdouble dif,paso,e;
	int ndigdec,ntips;

	dif=minmaxIn[1]-minmaxIn[0];
	paso=(stepIn>0) ? stepIn : (dif/nTipsIn);

	*minOut=minmaxIn[0];
	*maxOut=minmaxIn[1];
	*stepOut=paso;

	if (*minOut==*maxOut)
		return;
	ndigdec=(int) floor(log10(dif));
	e=(double) pow(10.0,ndigdec);

	*minOut=(double) ceil(*minOut/e)*e;
	*maxOut=(double) floor(*maxOut/e)*e;
	*stepOut=e;
	ntips=(int) ((*maxOut-*minOut)/e);

	while (ntips<5 && *stepOut>0.0f)
	{
		*stepOut/=2;
		ndigdec--;
		if (*minOut-*stepOut>=minmaxIn[0])
			*minOut-=*stepOut;
		if (*maxOut+*stepOut<=minmaxIn[1])
			*maxOut+=*stepOut;
		ntips=(int) ((*maxOut-*minOut)/ *stepOut)+1;
	}
	*precision=(int)-fmin(0,ndigdec);
}
int OGLGraph3d::MaximoLocal(int iline/*=-1*/, double *value/*=NULL*/)
{
	if (iline<0|| iline>m_nlines-1)
		return -1;

	int i=0,pos_max=-100;
	double * iniciox,*inicioy,valory=-100e32f;

	iniciox=GetLine(iline)->GetPX();

	int npoints=GetLine(iline)->nPoints;
	int nlinest=GetLine(iline)->GetUsedLinesT();

	for(int r=0;r<nlinest;r++)
	{
		inicioy=GetLine(iline)->GetPY(r);
		for(i=2;i<npoints;i++)
		{
			if(iniciox[i]>m_zoomx[1])
				break ;
			if(iniciox[i]>m_zoomx[0] &&iniciox[i] <m_zoomx[1] && inicioy[i]>valory )
			{
				pos_max=i+r*npoints;
				if(value!=NULL)
					*value=inicioy[i];
				valory=inicioy[i];
			}
		}
	}
	return pos_max;
}

int OGLGraph3d::MinimoLocal(int iline,double*value)
{
	if (iline<0||iline>m_nlines)
		return -1;

	int i=0,pos_max=-100;
	double * iniciox,*inicioy,valory=100e32f;

	iniciox=GetLine(iline)->GetPX();
	int npoints=GetLine(iline)->nPoints;
	int nlinest=GetLine(iline)->GetUsedLinesT();

	for(int r=0;r<nlinest;r++)
	{
		inicioy=GetLine(iline)->GetPY(r);
		for(i=2;i<npoints;i++)
		{
			if(iniciox[i]>m_zoomx[1])
				break ;
			if(iniciox[i]>m_zoomx[0] &&iniciox[i] <m_zoomx[1]&&inicioy[i]<valory )
			{
				pos_max=i+r*npoints;
				if(value!=NULL)
				*value=inicioy[i];
				valory=inicioy[i];
			}
		}
	}
	return pos_max;
}

void OGLGraph3d::SelectLine(double x,double y)
{
	m_focusLineIndex=-1;
	if(m_nlines<=0||m_plines==NULL)
		return ;

	LockDataMutex();
	int cont=0;

	GLdouble max=m_zoomx[1];
	CHARTS3D_POINTD pt(0,0);double distx,disty,disty1;

	PixelsToWorld(&pt,&distx,&disty);
	pt.y=20;//minimun distance 10px
	PixelsToWorld(&pt,&distx,&disty1);

	GLdouble total=fabs(disty1-disty);

	for (CXYLineGL3D *pt=m_plines;pt!=NULL;cont++,pt=pt->nextLine)
	{
		if(!pt->visible)
			continue;
		for(int zline=0;zline<pt->GetUsedLinesT() ; zline++)
		{
			double* iniciox=pt->GetPX();double* inicioy=pt->GetPY(zline);
			int npuntos=pt->nPoints;
			for(int i=0;i<=npuntos-1;i++)
			{
				if(iniciox[i]>max)//point visible
					break ;
				if(iniciox[i]<=x && iniciox[i+1]>=x)//between two points x axis
				{
					/*if((fabs(inicioy[i]-y))<total||//if distance y axis  cur point < 10 pixel-> focus
						 (fabs(inicioy[i+1]-y))<total||//if distance y axis  cur point+1 < 10 pixel-> focus
						 (fabs(inicioy[i-1]-y))<total)//if distance y axis  cur point-1 < 10 pixel-> focus
						 m_linea_focus=cont;*/
					double a=(inicioy[i]-inicioy[i+1])/(iniciox[i]-iniciox[i+1]);//adjust a rect
					double b=inicioy[i]-a*iniciox[i];
					double val= x*a+b;
					if(fabs(y-val)<total)//mouse point near rect
						m_focusLineIndex=cont;
				}
			}
		}
	}
	UnlockDataMutex();
}

double OGLGraph3d::GetMaximun()
{
	int ilinea=0;
	double maximun=-DBL_MAX;

	for (CXYLineGL3D *pt=m_plines;pt!=NULL;pt=pt->nextLine)
	{
		if(pt->visible)
		{
		//	int fint=pt->GetUsedLinesT();
			double max;
			MaximoLocal(ilinea,&max);
			maximun= maximun < max ? max : maximun;
		}
	}
	return maximun;
}

double OGLGraph3d::GetMinimun()
{
	int ilinea=0;
	double minimun=DBL_MAX;

	for (CXYLineGL3D *pt=m_plines;pt!=NULL;pt=pt->nextLine)
	{
		if(pt->visible)
		{
			double min;
			MinimoLocal(ilinea,&min);
			minimun= minimun > min ? min : minimun;
		}
	}
	return minimun;
}

///
///   SAVING IMAGES,GRAFS,DAT....
///
void OGLGraph3d::SaveCurrent(string dir)
{
/*archivo de grafica:*/
	//archivo de grafica:
	//POR LINEA
	struct datgraf3d datos;
	FILE* fid=fopen(dir.data(),"wb");
	if(fid==NULL)
		return;
	int i=sizeof(struct datgraf3d);
	fwrite(&i,sizeof(i),1,fid);//version
	fwrite(&m_nlines,sizeof(double),1,fid);	//7flotante 28 bytes
	fwrite(&m_zoomx[0],sizeof(double),1,fid);
	fwrite(&m_zoomx[1],sizeof(double),1,fid);
	fwrite(&m_zoomy[0],sizeof(double),1,fid);
	fwrite(&m_zoomy[1],sizeof(double),1,fid);
	fwrite(&m_zoomz[0],sizeof(double),1,fid);
	fwrite(&m_zoomz[1],sizeof(double),1,fid);
	fwrite(&m_angx,sizeof(double),1,fid);
	fwrite(&m_angy,sizeof(double),1,fid);

	for( i=0;i<m_nlines;i++)							//5 flotante 32bytes +16chars
	{
		memset(&datos,0,sizeof(datos));
		datos.nlinest=(double)GetLine(i)->GetNLinesT();
		datos.nusedlinest=(double)GetLine(i)->GetUsedLinesT();
		datos.color=GetLine(i)->color==0x000000 ? m_paleta[i] :GetLine(i)->color;
		datos.visible=GetLine(i)->visible;
		datos.nPoints=(double)GetLine(i)->nPoints;
		datos.npuntosinteres=GetLine(i)->m_puntos_interes.npuntos;
		strcpy(datos.nombre,GetLine(i)->lineName.data());
		fwrite(&datos,1,sizeof(datos),fid);
	}
	for(i=0;i<m_nlines;i++)							//[datos ejx][datos ejy][datos_ejez]
	{
		fwrite(GetLine(i)->GetPX(),sizeof(double),GetLine(i)->nPoints,fid);
		fwrite(GetLine(i)->GetPY(0),sizeof(double),GetLine(i)->GetUsedLinesT()*GetLine(i)->nPoints,fid);
		fwrite(GetLine(i)->GetPZ(),sizeof(double),GetLine(i)->GetUsedLinesT(),fid);
	}
	for(i=0;i<m_nlines;i++)							//puntos.x[],puntos.y[],puntos.z[]
	{
		fwrite(GetLine(i)->m_puntos_interes.ppuntosx,sizeof(double),GetLine(i)->m_puntos_interes.npuntos,fid);
		fwrite(GetLine(i)->m_puntos_interes.ppuntosy,sizeof(double),GetLine(i)->m_puntos_interes.npuntos,fid);
		fwrite(GetLine(i)->m_puntos_interes.ppuntosz,sizeof(double),GetLine(i)->m_puntos_interes.npuntos,fid);
	}
	fclose(fid);
}

void OGLGraph3d::Open(string dir)
{
	FILE* fid=fopen(dir.c_str(),"rb");
	if(fid==NULL)
	{
		printf("Error abriendo el archivo\n");
		return;
	}
	fseek(fid,0,SEEK_END);
	long size=ftell(fid);
	char*buf=new char[size];

	fseek(fid,0,SEEK_SET);
	fread(buf,1,size,fid);

	fclose(fid);
	unsigned int sizestructura=buf[0];
	if(sizestructura>sizeof(struct datgraf3d))
	{
		printf("Version de grafica mas reciente\n");
		delete []buf;
		return;
	}
	if(sizestructura<sizeof(struct datgraf3d))
	{
		printf("Version de grafica desconocida\n");
		delete []buf;
		return;
	}
	int nlines=(int)buf[sizeof(int)];
  struct datgraf3d datos;
	float*fin=(float*)&buf[9*sizeof(double)+sizeof(int)+nlines*(sizeof(struct datgraf3d))];
	RemoveLines(0);
	for(int i=0;i<nlines;i++)
	{
		memcpy(&datos,&buf[9*sizeof(double)+sizeof(int)+i*sizeof(struct datgraf3d)],sizestructura);
		AddLine((int)datos.nPoints,(int)datos.nlinest,datos.nombre,datos.color/*"C:\\jet.txt"*/,(bool)datos.visible);
		GetLine(i)->SetUsedLinesT((int)datos.nusedlinest);
		memcpy(GetLine(i)->GetPX(),fin,(size_t)datos.nPoints*sizeof(double));
		fin+=(int)datos.nPoints;
		memcpy(GetLine(i)->GetPY(0),fin,(size_t)(datos.nPoints*datos.nusedlinest*sizeof(float)));
		fin+=(int)(datos.nPoints*datos.nusedlinest);
		memcpy(GetLine(i)->GetPZ(),fin,(size_t)datos.nusedlinest*sizeof(double));
		fin+=(int)datos.nusedlinest;
	}
	for(int i=0;i<nlines;i++)
	{
		memcpy(GetLine(i)->m_puntos_interes.ppuntosx,fin,(size_t)sizeof(double)*GetLine(i)->m_puntos_interes.npuntos);
		fin+=GetLine(i)->m_puntos_interes.npuntos;
		memcpy(GetLine(i)->m_puntos_interes.ppuntosy,fin,(size_t)sizeof(double)*GetLine(i)->m_puntos_interes.npuntos);
		fin+=GetLine(i)->m_puntos_interes.npuntos;
		memcpy(GetLine(i)->m_puntos_interes.ppuntosz,fin,(size_t)sizeof(double)*GetLine(i)->m_puntos_interes.npuntos);
		fin+=GetLine(i)->m_puntos_interes.npuntos;
	}
	float *zoom=(float*)&buf[sizeof(int)+sizeof(float)];
	m_angx=zoom[6];
	m_angy=zoom[7];
	SetZoom(zoom[0],zoom[1],zoom[2],zoom[3],zoom[4],zoom[5]);
	m_focusLineIndex=GetNextVisible();
	delete []buf;

}

void OGLGraph3d::SaveDat(string dir)
{
	if(m_focusLineIndex==-1)
    return;
	FILE* fid=fopen(dir.data(),"wt");
	if(fid==NULL)
		return;
	double* ejex=GetLine(this->m_focusLineIndex)->GetPX(),
		*ejez=GetLine(this->m_focusLineIndex)->GetPZ();
	for(int r=0;r<GetLine(m_focusLineIndex)->nPoints;r++)//tantas filas como frecuencias
	{
			fprintf(fid,"%5.5f\t",ejex[r]);
			for(int c=0;c<GetLine(m_focusLineIndex)->GetUsedLinesT();c++)//tantas columnas como lineas de tiempo +2
				fprintf(fid,"%5.5f\t",GetLine(m_focusLineIndex)->GetPY(c)[r]);
			fprintf(fid,"\n");
	}
	for(int r=0;r<GetLine(this->m_focusLineIndex)->GetUsedLinesT();r++)//en la primera columna se anaden los tiempos del eje z (profundidad)
		fprintf(fid,"%5.5f\n",ejez[r]+(m_ejzhora ? m_offsetz : 0));//guardamos la hora en segundos del dia absolutos
	fclose(fid);
	return;
}

void OGLGraph3d::GetScreenShot(GLubyte * raw,int posx,int posy,int width,int heignt,int bitspixel)
{
	BlockPaint(0);
	glFlush(); glFinish();
	if(bitspixel==4*8)
		glReadPixels(posx,posy,width,heignt,GL_BGRA_EXT,GL_UNSIGNED_BYTE,raw);
	unBlockPaint();
}
///
///  DIALOG INTERFACE
///
void OGLGraph3d::OnMouseWheel(CHARTS3D_POINTD pt,unsigned int flags,int delta)
{//todo msg recalcular
	camera.IncrementCamPosition(0,0,delta*10);
	double posx,posy,posz=0;
	double posx1,posy1,posz1=1;

	PixelsToWorld(&pt,&posx,&posy,&posz);
	PixelsToWorld(&pt,&posx1,&posy1,&posz1);
	/* ecuacion recta=:  X=x1+t*vx
											 Y=y1+t*vy
											 Z=z1+t*vz
											 vx=x1-x,vy=y1-y,vz=z1-z;*/
	float cortex,t;

	if(posx!=posx1 && posy!=posy1)
	{
		t=-posy1/(posy1-posy);//corte con 0
		cortex=posx1+t*(posx1-posx);
	}
	else
		cortex=posx;

	posy=posy1+((m_plines[0].GetPZ()[m_plines[0].m_nUsedLinesT/2]-posz1)/(posz-posz1))*(posy1-posy);
	posy=posy>m_zoomy[0]&&posy<m_zoomy[1] ? posy : (m_zoomy[0]+m_zoomy[1])/2;

	if(flags == CONTROL_MODDIFIER )
		ZoomEjeX(delta,cortex);
	else
	{
		if(flags == SHIFT_MODDIFIER)//zoom eye y
			ZoomEjeY(delta,posy);
		else
		{
			if(flags & CONTROL_MODDIFIER && flags & SHIFT_MODDIFIER )
			{
				float inc=(m_zoomz[1]-m_zoomz[0])/10.0f;
				m_zoomz[1]-=delta<0 ? -inc : inc;
				m_zoomz[0]+=delta<0 ? -inc : inc;
			}
			else
			{
				ZoomEjeX(delta,cortex);
				ZoomEjeY(delta,posy);
			}
		}
	}

	SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0],m_zoomy[1],m_zoomz[0],m_zoomz[1]);
	RePaint();
}

void OGLGraph3d::OnKeyPress(chart3dkeys keypress, int modiffiers)
{
	double incr;

 switch(keypress)
 {
  case k_left3D:
		 incr=(ABS(m_zoomx[1]-m_zoomx[0]))/10.0f;
		SetZoom(m_zoomx[0]-incr,m_zoomx[1]-incr,m_zoomy[0],m_zoomy[1],m_zoomz[0],m_zoomz[1]);
   break;
  case k_right3D:
		incr=(ABS(m_zoomx[1]-m_zoomx[0]))/10.0f;
	 SetZoom(m_zoomx[0]+incr,m_zoomx[1]+incr,m_zoomy[0],m_zoomy[1],m_zoomz[0],m_zoomz[1]);
   break;
  case k_up3D:
	 if(modiffiers & SHIFT_MODDIFIER)
	 {
		 incr=(ABS(m_zoomz[1]-m_zoomz[0]))/10.0f;
		 SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0],m_zoomy[1],m_zoomz[0]+incr,m_zoomz[1]+incr);
	 }
	 else
	 {
		 incr=(ABS(m_zoomy[1]-m_zoomy[0]))/10.0f;
		 SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0]+incr,m_zoomy[1]+incr,m_zoomz[0],m_zoomz[1]);
	 }
   break;
  case k_down3D:
		if(modiffiers & SHIFT_MODDIFIER)
		{
			incr=(ABS(m_zoomz[1]-m_zoomz[0]))/10.0f;
			SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0],m_zoomy[1],m_zoomz[0]-incr,m_zoomz[1]-incr);
		}
		else
		{
			incr=(ABS(m_zoomy[1]-m_zoomy[0]))/10.0f;
			SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0]-incr,m_zoomy[1]-incr,m_zoomz[0],m_zoomz[1]);
		}
				break;
	default:
   break;
	}
	RePaint();
}

void OGLGraph3d::OnMouseMove(CHARTS3D_POINTD pt,unsigned int flags)
{
	if(m_drawManualDistance==1)
	{
		m_drawManualDistance=2;
	}

	if( flags & SHIFT_MODDIFIER && m_colormanual>=0)
	{
		//Percentage of max to asign color, 1 for all range.
		m_colormanual= 1-pt.y/(float)m_height;
	}

	if( flags & CONTROL_MODDIFIER)
	{
	 //todo tooltip
		return;
	}

	if(flags &MOUSE_LEFT_BUTTON )
	{
		if(m_oldCursorPosition.x!=pt.x)//GIRO EJE Y
		{
			int limit=180;
			if(((pt.x-m_oldCursorPosition.x) ))
				m_angy-=(pt.x-m_oldCursorPosition.x)*0.5f;
			//m_angy=m_angy>limit ? limit : (m_angy<-limit ? -limit : m_angy);
			if(ABS(m_angy)>limit)
				m_angy=m_angy>180 ? -180+(m_angy-180) : 180+ (m_angy+180);
		}
		if(m_oldCursorPosition.y!=pt.y)//GIRO EJE x
		{
			m_angx-=0.5*(pt.y-m_oldCursorPosition.y);
			m_angx= m_angx>90 ? 90 :m_angx;
			m_angx= m_angx<-90 ? -90 :m_angx;
		}
		float incx=((float)pt.x-m_oldCursorPosition.x)*(ABS(m_zoomx[1]-m_zoomx[0]))/(Width());
		float incy=(pt.y-m_oldCursorPosition.y)*(ABS(m_zoomy[1]-m_zoomy[0]))/(Height());
		camera.IncrementCamPosition(incx,incy,0);
		SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0],m_zoomy[1],m_zoomz[0],m_zoomz[1]);
		m_oldCursorPosition=pt;
		return;
	}

	if(flags & MOUSE_MID_BUTTON)
	{
		if( (ABS(pt.y-m_oldCursorPosition.y)<20) && (ABS(pt.x-m_oldCursorPosition.x)<20))
		{//pinchado se mueve la grafica con translacion
			if(m_oldCursorPosition.x!=pt.x)//MOVER EJE X
			{
				float inc=((float)pt.x-m_oldCursorPosition.x)*(ABS(m_zoomx[1]-m_zoomx[0]))/(Width());
				m_zoomx[0]-=inc;
				m_zoomx[1]-=inc;
			}

			if(m_oldCursorPosition.y!=pt.y)//MOVER EJE Y
			{
				float inc=(pt.y-m_oldCursorPosition.y)*(ABS(m_zoomy[1]-m_zoomy[0]))/(Height());
				m_zoomy[0]+=inc;
				m_zoomy[1]+=inc;
			}

			SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0],m_zoomy[1],m_zoomz[0],m_zoomz[1]);
		}
		camera.AddDirection((pt.x-m_oldCursorPosition.x)*0.5f,0.5*(pt.y-m_oldCursorPosition.y));
	}

	m_oldCursorPosition=pt;

	if(m_posaxisscroll||m_drawManualDistance||m_drawZoomBox||m_drawCurrentPos)
		RePaint();
}

void OGLGraph3d::OnLMousePress(CHARTS3D_POINTD pt,unsigned int flags)
{
	BlockPaint(0);
  double posx,posy;
	PixelsToWorld(&pt,&posx,&posy);
	unBlockPaint();
	if(buttonAutozoom.PointInbutton(pt.x,m_height-pt.y))
	{
		buttonAutozoom.SetPushed(1);
		AutoZoom(0,0);
		return;
	}

	if(buttonAutozoomTop.PointInbutton(pt.x,m_height-pt.y))
	{
		buttonAutozoomTop.SetPushed(1);
		TopView();
		return;
	}

	bool aux=checkAutofix.IsPushed();

	if(checkAutofix.PointInbutton(pt.x,m_height-pt.y))
	{
		checkAutofix.SetPushed(!aux);
		AutoZoom(-45,-45);
		return;

	}

	if(flags& SHIFT_MODDIFIER  )//
	{
		m_initialZoomBoxPoint=pt;
		m_drawZoomBox=1;
		m_drawManualDistance=0;
	}
	else
	{
		m_drawCurrentPos=1;
		m_drawZoomBox=0;
		m_oldCursorPosition=pt;
	}


	if(flags& CONTROL_MODDIFIER)//tecla control
  {
		m_drawManualDistance=0;
  }

	if(0 )//esta en la leyenda
  {
		int sel=(int)((-posy)/m_fontHeight);
		m_focusLineIndex=GetLine(sel)->visible ? sel : m_focusLineIndex;
		m_drawManualDistance=0;
  }

	SelectLine(posx,posy);
	RePaint();
}

void OGLGraph3d::OnLMouseRelease(CHARTS3D_POINTD pt,unsigned int flags)
{
	double posx,posy,posz;
	BlockPaint(0);
	PixelsToWorld(&pt,&posx,&posy,&posz);
	unBlockPaint();
	buttonAutozoom.SetPushed(0);
	buttonAutozoomTop.SetPushed(0);

	if(flags& SHIFT_MODDIFIER &&posx>m_zoomx[0] && posx<m_zoomx[1] && posy>m_zoomy[0] && posy<m_zoomy[1]&&m_drawZoomBox)//esta en la grafica)
  {
		double posx0,posy0,posz0=0;

		BlockPaint(0);
		PixelsToWorld(&m_initialZoomBoxPoint,&posx0,&posy0,&posz0);
		unBlockPaint();

		if(posx0!=posx && posy0 != posy  )
			SetZoom(posx0,posx,posy0,posy,m_zoomz[0],m_zoomz[1]);

		m_drawZoomBox=0;
  }
  else
		m_drawZoomBox=0;
	m_drawManualDistance=m_drawCurrentPos=0;
  RePaint();
}

void OGLGraph3d::OnMMousePress(CHARTS3D_POINTD pt,unsigned int )
{
  double posx,posy;

	BlockPaint(0);
	PixelsToWorld(&pt,&posx,&posy);
	unBlockPaint();
	m_oldCursorPosition=pt;
}

void OGLGraph3d::OnMMouseRelease(CHARTS3D_POINTD ,unsigned int )
{
	m_drawManualDistance=m_drawCurrentPos=0;
}

void OGLGraph3d::OnRMousePress(CHARTS3D_POINTD ,unsigned int )
{

}

void OGLGraph3d::OnRMouseRelease(CHARTS3D_POINTD pt,unsigned int flags)
{
  pt.x=pt.x+flags;
}

void OGLGraph3d::OnDoubleClic(CHARTS3D_POINTD pt,unsigned int )
{

  double posx,posy;

	BlockPaint(0);
	PixelsToWorld(&pt,&posx,&posy);
	unBlockPaint();

	if(posx>m_zoomx[0] && posx<m_zoomx[1] && posy>m_zoomy[0] && posy<m_zoomy[1]  && m_angy==0 && m_angx==0)//esta en la grafica)
  {
		m_drawManualDistance=1;
		m_initialManualDistancePoint[0]=posx;
		m_initialManualDistancePoint[1]=posy;
  }
	else
	{
			m_colormanual=m_colormanual>0 ? -10: 1;
	}
  if(posx>m_zoomx[0] && posy<m_zoomy[0] )//eje x
  {
    //todo ventana de asignacion de zoom
  }
  if(posx<m_zoomx[0] && posy>m_zoomy[0] )//eje Y
  {
    //todo ventana de asignacion de zoom
  }
}

void OGLGraph3d::OnResizeEvent(int width,int height)
{
	m_width=width;
	m_height=height;
	CalculaLeyenda();
	CalculateView();
	RePaint();
}

void OGLGraph3d::OnCloseEvent (  )
{
 // m_posdialogo=pos();
}

CHARTS3D_POINTD OGLGraph3d::GetPreviousCursorPosition() const
{
	return m_oldCursorPosition;
}

void OGLGraph3d::SetPreviousCursorPosition(const CHARTS3D_POINTD &pos_raton_ant)
{
	m_oldCursorPosition = pos_raton_ant;
}

