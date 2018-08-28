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
	\brief OGLGraph implementation file.
	\date 05-2010
	\author Marcos Perez*/

#include <sstream>
#include <iomanip>
#include "oglgraph2d.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>//vnsprintf
#include <string.h>

#include <math.h>

vector<OGLGraph*> OGLGraph::m_pcharts;

OGLGraph::OGLGraph( )
{
	posdrawuserfnctionpointer=NULL;
	CommonInit();
	pthread_mutex_init (&m_dataMutex, NULL);
	if (sem_init(&m_paintSemaphore, 0, 1) == -1)
	{
		printf("error when globalsem initiation\n");
	}

}

OGLGraph::~OGLGraph()
{

  if (m_plines!=NULL)
  {
		RemoveLines(0);
		m_plines=NULL;
  }
	if(OGLGraph::m_pcharts.size()>0)
		OGLGraph::m_pcharts.pop_back();


}

bool OGLGraph::InitGraph( bool altocontraste,bool ejxhora)
{
	m_ejxhora=ejxhora;
	SetColors(altocontraste);
	FindOrigin();
	CalculaLeyenda();
	glClearColor(OGL_RED(m_backgroundColor)/255.0f,OGL_GREEN(m_backgroundColor)/255.0f,OGL_BLUE(m_backgroundColor)/255.0f, 0.0f);
	return 1;
}

void OGLGraph::CommonInit()
{
    m_nlines=0;
    m_plines=NULL;
    m_plines=NULL;
    m_nlegend_lines=1;
		m_freeSpaceXaxis=OGL_FONT_HEIGHT*1.5f+m_nlegend_lines;
		m_freeSpaceYaxis=OGL_FONT_WIDTH*7;
		m_linesperline=1;
    m_max=-1;
		m_widthLegend=0;
		m_drawZoomBox=0;
		m_posaxisscroll=0;
		m_posscroll=0;
		m_focusLineIndex=-1;
		m_drawManualDistance=0;
		m_barchart=0;
    m_nlegend_lines=1;
		m_drawCurrentPos=0;
  	m_zoomx[0]=m_zoomy[0]=0;
  	m_zoomx[1]=m_zoomy[1]=100;
    m_mapdibujos.clear();

		OGLGraph::m_pcharts.push_back(this);
}

///
///CONFIGURATION
///
void OGLGraph::SetGraphTitle(string tittle,string ejx/*=""*/,string ejy/*=""*/)
{
	 m_tittle=tittle;
	 m_xasisName=ejx;
	 m_yAxisName=ejy;
}

void OGLGraph::SetColors(int hightcontrast)
{
	m_backgroundColor=hightcontrast ? OGL_BACKGROUNDN :  OGL_BACKGROUND;
	m_fontColor=hightcontrast ?  OGL_FONTCOLORN : OGL_FONTCOLOR;

	m_backgroundcolormaxvalue=hightcontrast ? OGL_BACKGROUND_MAX_VALUEN : OGL_BACKGROUND_MAX_VALUE ;

	m_distancesColor=hightcontrast ? OGL_MANUAL_MEASURE_LINESN : OGL_MANUAL_MEASURE_LINES;
	m_backgroundDistances=hightcontrast ? OGL_BACKGROUND_MANUAL_MEASUREN : OGL_BACKGROUND_MANUAL_MEASURE;

	m_axisColor=hightcontrast ? OGL_AXIS_COLORN  : OGL_AXIS_COLOR;

	m_zoomAxisColor=hightcontrast ? OGL_AXIS_COLORN : OGL_AXIS_ZOOM_COLOR;
	m_backgroundLegendColor=hightcontrast ? OGL_BACKGROUND_LECEND_COLORN : OGL_BACKGROUND_LECEND_COLOR;
	m_zoomBoxColor=hightcontrast ? OGL_ZOOM_BOX_COLORN : OGL_ZOOM_BOX_COLOR;;
	m_auxPointsColor=hightcontrast ? OGL_SECONDARY_POINTS_COLORN :OGL_SECONDARY_POINTS_COLOR;

	m_backgroundCurrentValue=hightcontrast ? OGL_BACKGROUND_CURRENT_POSITIONN : OGL_BACKGROUND_CURRENT_POSITION;
	m_currentPositionColor=hightcontrast ? OGL_AXIS_ZOOM_COLORN : OGL_AXIS_ZOOM_COLOR;
}

COLORREF OGLGraph::currentPositionColor() const
{
	return m_currentPositionColor;
}

void OGLGraph::setCurrentPositionColor(const COLORREF &currentPositionColor)
{
	m_currentPositionColor = currentPositionColor;
}

COLORREF OGLGraph::auxPointsColor() const
{
	return m_auxPointsColor;
}

void OGLGraph::setAuxPointsColor(const COLORREF &auxPointsColor)
{
	m_auxPointsColor = auxPointsColor;
}

COLORREF OGLGraph::zoomBoxColor() const
{
	return m_zoomBoxColor;
}

void OGLGraph::setZoomBoxColor(const COLORREF &zoomBoxColor)
{
	m_zoomBoxColor = zoomBoxColor;
}

COLORREF OGLGraph::backgroundLegendColor() const
{
	return m_backgroundLegendColor;
}

void OGLGraph::setBackgroundLegendColor(const COLORREF &backgroundLegendColor)
{
	m_backgroundLegendColor = backgroundLegendColor;
}

COLORREF OGLGraph::zoomAxisColor() const
{
	return m_zoomAxisColor;
}

void OGLGraph::setZoomAxisColor(const COLORREF &zoomAxisColor)
{
	m_zoomAxisColor = zoomAxisColor;
}

COLORREF OGLGraph::axisColor() const
{
	return m_axisColor;
}

void OGLGraph::setAxisColor(const COLORREF &axisColor)
{
	m_axisColor = axisColor;
}

COLORREF OGLGraph::backgroundDistances() const
{
	return m_backgroundDistances;
}

void OGLGraph::setBackgroundDistances(const COLORREF &backgroundDistances)
{
	m_backgroundDistances = backgroundDistances;
}

COLORREF OGLGraph::distancesColor() const
{
	return m_distancesColor;
}

void OGLGraph::setDistancesColor(const COLORREF &distancesColor)
{
	m_distancesColor = distancesColor;
}

COLORREF OGLGraph::backgroundcolormaxvalue() const
{
	return m_backgroundcolormaxvalue;
}

void OGLGraph::setBackgroundcolormaxvalue(const COLORREF &backgroundcolormaxvalue)
{
	m_backgroundcolormaxvalue = backgroundcolormaxvalue;
}

COLORREF OGLGraph::fontColor() const
{
	return m_fontColor;
}

void OGLGraph::setFontColor(const COLORREF &fontColor)
{
	m_fontColor = fontColor;
}

COLORREF OGLGraph::backgroundCurrentValue() const
{
	return m_backgroundCurrentValue;
}

void OGLGraph::setBackgroundCurrentValue(const COLORREF &backgroundCurrentValue)
{
	m_backgroundCurrentValue = backgroundCurrentValue;
}

COLORREF OGLGraph::backgroundColor() const
{
	return m_backgroundColor;
}

void OGLGraph::setBackgroundColor(const COLORREF &backgroundColor)
{
	m_backgroundColor = backgroundColor;
}

///
///ELEMENTS
///
int  OGLGraph::AddLine(int nPoints,string name,COLORREF color,bool visible)
{
	CXYLineGL** pt;

	LockDataMutex();

	for (pt=&m_plines;*pt!=NULL;pt=&((*pt)->nextLine));
	*pt=new CXYLineGL(nPoints,name,color,visible);
	m_nlines++;
	m_widthLegend=m_widthLegend<name.size()*OGL_FONT_WIDTH ? name.size()*OGL_FONT_WIDTH:m_widthLegend;

	CalculaLeyenda();
	FindOrigin();

  UnlockDataMutex();

	return m_nlines;
}

int  OGLGraph::GetnLines()
{
  return m_nlines;
}

CXYLineGL* OGLGraph::GetLine(int iLine)
{
	CXYLineGL** pt;
	int i;
	for (i=0,pt=&m_plines;i<iLine && (*pt!=NULL);i++,pt=&((*pt)->nextLine));
	return *pt;
}

CXYLineGL* OGLGraph::GetLines()
{
  return GetLine(0);
}

void OGLGraph::AddDraw(string identifier, enum_tipodibujos drawtype, float posx, float posy, float posz, float size, COLORREF colorline, COLORREF backgroundcolor)
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

void OGLGraph::DeleteDraw(string identifier)
{
	m_mapdibujos.erase(identifier);
}

void OGLGraph::DeleteAlldraws()
{
	m_mapdibujos.clear();
}

void OGLGraph::RemoveLines(int start)
{
	CXYLineGL *pt,**ptpt;
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

void OGLGraph::UpdateLineValues(int nline,double *Xvalues,double *Yvalues,int nvalues)
{
	if(nvalues<=0 || Yvalues==NULL || GetLine(nline)==NULL)
    return;
  LockDataMutex();

	double *y=GetLine(nline)->GetPY();
	double *x=GetLine(nline)->GetPX();

	int npoints=GetLine(nline)->GetNPoints();

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

	if(nvalues==npoints)
	{
		memcpy(y,Yvalues,npoints*sizeof(double));

		if(Xvalues!=NULL)
		memcpy(x,Xvalues,npoints*sizeof(double));
	}
  UnlockDataMutex();
}

///
///SYNCRO
///
void OGLGraph::LockDataMutex()
{
	pthread_mutex_lock( &m_dataMutex );
}

void OGLGraph::UnlockDataMutex()
{
	pthread_mutex_unlock( &m_dataMutex );
}

int OGLGraph::BlockPaint(int ms)
{
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

}

int OGLGraph::unBlockPaint()
{
 return !sem_post(&m_paintSemaphore);
}

int OGLGraph::TryBlockPaint()
{
	return !sem_trywait(&m_paintSemaphore);
}

///
///PAINTING
///
int  OGLGraph::RedrawChart()
{
	glClearColor(OGL_RED(m_backgroundColor)/255.0f,OGL_GREEN(m_backgroundColor)/255.0f,OGL_BLUE(m_backgroundColor)/255.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_INTERLACE_SGIX);
	GLdouble minescala[2],maxescala[2], paso[2];
  string format[2];

	//All window.
	DrawTittle();
	DibujarLeyenda();

	//All window but title.
	DrawAxis();
  DibujaTextoReferencias( minescala, maxescala,paso, format);

	//Drawing area
	DrawLines();
	DrawDraws();
	if(m_drawManualDistance==2)
	{
		DrawManualMeasurement();
	}

	DibujaReferencias(minescala, maxescala,paso);
	if(m_drawZoomBox)
	{
		DrawZoomBox();
	}
	if(m_drawCurrentPos)
	{
		DrawCurrentPosition();
	}

	if(m_focusLineIndex>=0 && m_nlines>0)
	{
	//	DibujaMaximo(m_max<0?MaximoLocal() : m_max);

	}
	if(posdrawuserfnctionpointer!=NULL)
	{
		posdrawuserfnctionpointer();
	}
	return 1;
}
void OGLGraph::SetExternalDrawingFunction(int (*functionptr) ())
{
	posdrawuserfnctionpointer=functionptr;
}

void OGLGraph::DrawTittle()
{
	glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
	glFrustum(0.0,0,0,(float)0, 0, 1);
  glMatrixMode(GL_MODELVIEW);
	glViewport(0,0,m_width,m_height );
	glOrtho(0,m_width ,0, m_height,-1,1);

	glColor3ub(200,200,200  );
	glBegin( GL_QUADS );

	//Title area.
  glVertex3f(0,m_height,OGL_MARGIN_Z_POSITION);
  glVertex3f(m_width,m_height,OGL_MARGIN_Z_POSITION);
  glVertex3f(m_width,m_height-OGL_TITTLE_HEIGHT,OGL_MARGIN_Z_POSITION);
  glVertex3f(0,m_height-OGL_TITTLE_HEIGHT ,OGL_MARGIN_Z_POSITION);

	//X axis area.
  glVertex3f(0,m_freeSpaceXaxis,OGL_MARGIN_Z_POSITION);
  glVertex3f(m_width,m_freeSpaceXaxis,OGL_MARGIN_Z_POSITION);
  glVertex3f(m_width,0,OGL_MARGIN_Z_POSITION);
  glVertex3f(0,0 ,OGL_MARGIN_Z_POSITION);

	//y axis area.
  glVertex3f(0,0,OGL_MARGIN_Z_POSITION);
  glVertex3f(m_freeSpaceYaxis,0,OGL_MARGIN_Z_POSITION);
  glVertex3f(m_freeSpaceYaxis,m_height,OGL_MARGIN_Z_POSITION);
  glVertex3f(0 ,m_height,OGL_MARGIN_Z_POSITION);

	//right margin
  glVertex3f(m_width-OGL_RIGHT_MARGIN,m_freeSpaceXaxis,OGL_MARGIN_Z_POSITION);
  glVertex3f(m_width-OGL_RIGHT_MARGIN,m_height,OGL_MARGIN_Z_POSITION);
  glVertex3f(m_width,m_height,OGL_MARGIN_Z_POSITION);
  glVertex3f(m_width ,m_freeSpaceXaxis,OGL_MARGIN_Z_POSITION);
	
	glEnd(  );
	
	glColor3ub(0,0,0 );

  DrawTextInPixels(m_tittle,m_width/2-m_tittle.size()/2.0*OGL_FONT_WIDTH,m_height-OGL_FONT_HEIGHT*1,OGL_MARGIN_Z_POSITION+0.1,0,0x00f0f0f0,0x00ff0000);
  DrawTextInPixels(m_yAxisName,OGL_FONT_WIDTH*2,m_height-OGL_FONT_HEIGHT*1.1,OGL_MARGIN_Z_POSITION+0.1);
  DrawTextInPixels(m_xasisName,m_width-OGL_FONT_WIDTH*(m_xasisName.size()),OGL_MARGIN_Z_POSITION+0.1);
}

void OGLGraph::DibujarLeyenda()
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
		CXYLineGL *pt;
		int nline;
		m_xLegendPosition=m_width/2-(m_linesperline<m_nlines?m_linesperline :m_nlines)*(m_widthLegend)/2;
		int line=0;//arriba a abajo
		int col=0;//columna
		for (nline=0,pt=m_plines;pt!=NULL;pt=pt->nextLine,nline++)
		{
      DrawTextInPixels(pt->lineName,m_xLegendPosition+ col*(m_widthLegend+20),4+m_nlegend_lines*OGL_FONT_HEIGHT-OGL_FONT_HEIGHT*(line+1),OGL_MARGIN_Z_POSITION+0.1,1,m_backgroundLegendColor,pt->color);
			col++;
			if(col>=m_linesperline)
			{
				col=0;
				line++;
			}
		}
	}
}

void OGLGraph::DrawAxis()
{
	glViewport(0,0,m_width-OGL_RIGHT_MARGIN,m_height- OGL_TITTLE_HEIGHT);//trozo de ventana
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(m_posorigx, m_zoomx[1],m_posorigy, m_zoomy[1],-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable (GL_LINE_SMOOTH);
	glDisable (GL_LINE_STIPPLE);
	glLineWidth(3);
	glBegin( GL_LINES );

	glColor3ub( OGL_RED(m_axisColor),OGL_GREEN(m_axisColor),OGL_BLUE(m_axisColor) );
  glVertex3d(m_zoomx[0],m_zoomy[0], OGL_AXIS_Z_POSITION);		//ejex
  glVertex3d(m_zoomx[1],m_zoomy[0], OGL_AXIS_Z_POSITION );
  glVertex3d(m_zoomx[0],m_zoomy[0], OGL_AXIS_Z_POSITION);		//ejey
  glVertex3d(m_zoomx[0], m_zoomy[1], OGL_AXIS_Z_POSITION );
	glEnd();

  if(m_posaxisscroll)//NOT USED DELETE
	{
		double x,y;
		this->PixelsToWorld(NULL,&x,&y);
		glColor3ub( OGL_RED(m_zoomAxisColor),OGL_GREEN(m_zoomAxisColor),OGL_BLUE(m_zoomAxisColor) );
		glBegin( GL_POINTS );
    glVertex3d(m_ptoMejesinicial[0],m_ptoMejesinicial[1], OGL_AXIS_Z_POSITION+0.1f );		//puntos final e inicial
		if(m_posaxisscroll==OGL_YAXIS)
      glVertex3d(m_ptoMejesinicial[0],y, OGL_AXIS_Z_POSITION+0.1f );
		else	
      glVertex3d(x,m_ptoMejesinicial[1], OGL_AXIS_Z_POSITION+0.1f );
		glEnd();
		glBegin( GL_LINES );
		glVertex3d(m_ptoMejesinicial[0],m_ptoMejesinicial[1], 0.2f );	//linea paralela al eje
		if(m_posaxisscroll==OGL_YAXIS)
      glVertex3d(m_ptoMejesinicial[0],y, OGL_AXIS_Z_POSITION+0.1f);
		else	
      glVertex3d(x,m_ptoMejesinicial[1], OGL_AXIS_Z_POSITION+0.1f);
		glEnd();
	}
}
void OGLGraph::DibujaReferencias(GLdouble minescala[2],GLdouble maxescala[2],GLdouble paso[2])
{
	glEnable(GL_LINE_STIPPLE);//lineas de referencia discontinuas
	glLineStipple(1,0x0707  );
	glDisable (GL_LINE_SMOOTH);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	glColor3ub(OGL_RED(m_fontColor),OGL_GREEN(m_fontColor),OGL_BLUE(m_fontColor) );
	glLineWidth(1.0f);
	GLdouble ind;
  GLdouble zpos=OGL_REFERENCES_Z_POSITION;
	//horizontals.
	for (ind=minescala[0];ind<=maxescala[0];ind+=paso[0])
	{
		glBegin( GL_LINES );
    glVertex3d((m_zoomx[0]),ind , zpos );
    glVertex3d(m_zoomx[1],ind, zpos );
		glEnd();
	}

	//verticals
	for (ind=minescala[1];ind<=m_zoomx[1];ind+=paso[1])
	{
		glBegin( GL_LINES );
		glVertex2d(ind, m_zoomy[0] );
		glVertex2d(ind, m_zoomy[1] );
		glEnd();
	}
}

void OGLGraph::DibujaTextoReferencias(GLdouble minescala[2],GLdouble maxescala[2],GLdouble paso[2],string [2])
{
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(5,0xAAAA  );
	glDisable (GL_LINE_SMOOTH);
	glLineWidth(0.4f);

  GLdouble zpos=OGL_REFERENCES_Z_POSITION;
  double ind;

  glColor3ub(OGL_RED(m_fontColor),OGL_GREEN(m_fontColor),OGL_BLUE(m_fontColor)  );
  int precision;

  OptimunAxisScale(m_zoomy,-1,6,&minescala[0],&maxescala[0],&paso[0],&precision);

  double espacio_ejY=0;
	for (ind=minescala[0];ind<=maxescala[0];ind+=paso[0])
	{
		std::ostringstream tmp;
		tmp << std::setprecision(OGL_DECIMAL_PRECISSION) << ind;
		string sd=tmp.str();
		int ncaract=tmp.tellp();
    DrawText(sd,m_zoomx[0]-m_fontWidth/2- ncaract*m_fontWidth,ind-m_fontHeight/4,zpos);
		espacio_ejY=espacio_ejY>ncaract*OGL_FONT_WIDTH ?espacio_ejY:ncaract*OGL_FONT_WIDTH;

	}

	//recalculate y axis space for text.
	if(m_freeSpaceYaxis!=espacio_ejY+OGL_LEFTMARGIN)
	{
		m_freeSpaceYaxis=espacio_ejY+OGL_LEFTMARGIN;
		FindOrigin();
	}

	OptimunAxisScale(m_zoomx,-1,6,&minescala[1],&maxescala[1],&paso[1],&precision);
	for (ind=minescala[1];ind<=m_zoomx[1];ind+=paso[1])
	{
		if(m_ejxhora)
		{
			string txtstr=PasaFecha((long)ind*1000);
      DrawText(txtstr,ind-txtstr.size()/2.0*m_fontWidth,m_zoomy[0]-m_fontHeight,zpos);

		}
		else
		{
			std::ostringstream tmp;
			tmp<<ind;
      DrawText(tmp.str(),ind-tmp.tellp()/2.0*m_fontWidth,m_zoomy[0]-m_fontHeight,zpos);
		}
	}
}

void OGLGraph::DrawLines()
{
	glViewport((int)m_freeSpaceYaxis,(int)m_freeSpaceXaxis,(int)m_width-(int)m_freeSpaceYaxis-OGL_RIGHT_MARGIN,floor(m_height-m_freeSpaceXaxis-OGL_TITTLE_HEIGHT) );//trozo de ventana
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(m_zoomx[0], m_zoomx[1],m_zoomy[0], m_zoomy[1],-1,1);//trozo mundo real
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable (GL_LINE_SMOOTH);
	glDisable (GL_LINE_STIPPLE);
	glLineWidth(OGL_LINE_WIDTH);
	CXYLineGL*pt;

	if (m_plines!=NULL)
	{
		LockDataMutex();
		int sel=0;
		for (pt=m_plines;pt!=NULL;pt=pt->nextLine)
		{
			if(pt->visible)
			{
				double* x=pt->GetPX();double* y=pt->GetPY();
				glBegin( GL_LINE_STRIP );
				glColor3ub( ((GLubyte*)&pt->color)[0],((GLubyte*)&pt->color)[1],((GLubyte*)&pt->color)[2] );	// Green
				int visible=0;
				for(visible=0;x[visible]<this->m_zoomx[0]&&visible<pt->GetUsedPoints()-1;visible++);
				visible= visible==0 ? visible : visible-1;
        float z=sel==m_focusLineIndex ? OGL_SELECTED_DATALINES_Z_POSITION: OGL_DATALINES_Z_POSITION;
				int used=pt->GetUsedPoints();
				for(int r=visible;r<used-1;r++)
				{
					glVertex3d( x[r], y[r],z );
#						ifdef OGL_BLOQUEADOR
					if(m_barchart)	//dibujar escalones del bloqueador
						glVertex3d( x[r], y[r+1],z );
#						endif
					if(x[r]>this->m_zoomx[1]&& x[r+1]>this->m_zoomx[1])
						break;
				}
				glVertex3d( x[pt->GetUsedPoints()-1], y[pt->GetUsedPoints()-1],sel==m_focusLineIndex ? 0.05f: 0 );
				glEnd();
				glPointSize(10.0f);

				if(sel==m_focusLineIndex)
				{
					glBegin( GL_POINTS );
					for(int r=visible;r<pt->GetUsedPoints()-1;r++)
            glVertex3d( x[r], y[r], OGL_SELECTED_DATALINES_Z_POSITION );
					glEnd(  );
				}

				for(int c=0;c<pt->m_auxPoints.npuntos;c++)
				{
					glBegin( GL_POINTS );
          glVertex3d( pt->m_auxPoints.ppuntosx[c], pt->m_auxPoints.ppuntosy[c], z );
					glEnd(  );
				}
			}
			sel++;
		}
		UnlockDataMutex();
	}
}

void OGLGraph::DrawDraws()
{
	for(iterdrawing iter=m_mapdibujos.begin();iter!=m_mapdibujos.end();iter++)
	{
		drawingsstr tab=iter->second;
		double ONE_DEGREE = ( 3.14159265358979323846/180);
		double THREE_SIXTY = 2 *  3.14159265358979323846;
		double a,x,y;
		switch(tab.drawingType)
		{
		case CIRCUNFERENCE:
			glLoadIdentity();
			glEnable (GL_LINE_SMOOTH);
			glDisable (GL_LINE_STIPPLE);
			glLineWidth(OGL_LINE_WIDTH);
			glColor3ub( ((GLubyte*)&tab.colorpen)[0],((GLubyte*)&tab.colorpen)[1],((GLubyte*)&tab.colorpen)[2] );	// Green
			glBegin(GL_LINE_STRIP );
			for ( a=0; a<THREE_SIXTY; a+=ONE_DEGREE) {
				x = tab.tamano* (cos(a)) ;
				y = tab.tamano * (sin(a)) ;
         glVertex3d(x, y,OGL_DATALINES_Z_POSITION);
			}
			glEnd();
			break;
		case CIRCLE:
			glColor3ub( ((GLubyte*)&tab.colorbrush)[0],((GLubyte*)&tab.colorbrush)[1],((GLubyte*)&tab.colorbrush)[2] );	// Green
			glLoadIdentity();
			glTranslatef(tab.posx,tab.posy,tab.posz);
			glutSolidSphere(tab.tamano,80,80);
			glLoadIdentity();
			break;
		}
	}
}

void OGLGraph::DrawManualMeasurement()
{
	double posx,posy;
	PixelsToWorld(NULL,&posx,&posy);
	glBegin( GL_LINE_LOOP );
  GLdouble zpos=OGL_MANUALRULER_Z_POSITION;
	glColor3ub(OGL_RED(m_distancesColor),OGL_GREEN(m_distancesColor),OGL_BLUE(m_distancesColor) );
  glVertex3d( m_initialManualDistancePoint[0],m_initialManualDistancePoint[1], zpos );
  glVertex3d( m_initialManualDistancePoint[0],posy, zpos );
  glVertex3d(posx,posy,zpos );
	glEnd();
	glColor3ub( 0,0,0 );
  string txtf=strprintf("%.3f",fabs(posy-m_initialManualDistancePoint[1]));

	if(posx>m_initialManualDistancePoint[0])
	{
    DrawText(txtf,m_initialManualDistancePoint[0]-m_fontWidth*(txtf.length()+1),(posy-m_initialManualDistancePoint[1])/2+m_initialManualDistancePoint[1],zpos,1,m_backgroundDistances);
		txtf=strprintf("%.3f",sqrt(pow(posx-m_initialManualDistancePoint[0],2)+pow(posy-m_initialManualDistancePoint[1],2)));//texto diagonal
    DrawText(txtf,(posx-m_initialManualDistancePoint[0])/2+m_initialManualDistancePoint[0]+m_fontWidth,(posy+m_initialManualDistancePoint[1])/2-(posy<m_initialManualDistancePoint[1]?0:m_fontHeight),zpos,1,m_backgroundDistances);


	}
	else//triangulo hacia izda
	{

		DrawText(txtf,m_initialManualDistancePoint[0]+m_fontWidth,(posy-m_initialManualDistancePoint[1])/2+m_initialManualDistancePoint[1],0.2f,1,m_backgroundDistances);
		txtf=strprintf("%.3f",sqrt(pow(posx-m_initialManualDistancePoint[0],2)+pow(posy-m_initialManualDistancePoint[1],2)));//texto diagonal
		DrawText(txtf,fabs(posx-m_initialManualDistancePoint[0])/2+posx-m_fontWidth*(txtf.length()+1),(posy+m_initialManualDistancePoint[1])/2-(posy<m_initialManualDistancePoint[1]?0:m_fontHeight),0.2f,1,m_backgroundDistances);

	}

	string tstr;

	if(m_ejxhora)
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

void OGLGraph::DrawZoomBox()
{
	CHARTS_POINTD pt=GetMousePoint();

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(5,0xAAAA  );
	glEnable (GL_LINE_SMOOTH);
	glLineWidth(3.5);
	glColor3ub( OGL_RED(m_zoomBoxColor),OGL_GREEN(m_zoomBoxColor),OGL_BLUE(m_zoomBoxColor));
	double x0, x1,y0,y1;
  GLdouble zpos= OGL_MOUSEINTERACT_Z_POSITION;
	PixelsToWorld(&m_initialZoomBoxPoint,&x0,&y0);
	PixelsToWorld(&pt,&x1,&y1);
	glBegin(GL_LINE_LOOP);
  glVertex3d(x0,y0, zpos  );
  glVertex3d(x1,y0, zpos  );
  glVertex3d(x1,y1, zpos );
  glVertex3d( x0,y1, zpos );
	glEnd();
}

void OGLGraph::DrawCurrentPosition()
{
	double posx,posy;

	CHARTS_POINTD pt=GetMousePoint();
	PixelsToWorld(&pt,&posx,&posy);
	if(posx<m_zoomx[0] || posx>m_zoomx[1] || posy<m_zoomy[0] || posy>m_zoomy[1])
		return;
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.5f);
	glColor3ub( OGL_REDC(m_currentPositionColor),OGL_GREEN(m_currentPositionColor),OGL_BLUE(m_currentPositionColor));
	double  x,y;
	PixelsToWorld(&pt,&x,&y);
	glBegin(GL_LINES);
  GLdouble zpos= OGL_MOUSEINTERACT_Z_POSITION;
  glVertex3d(m_zoomx[0],y ,zpos );
  glVertex3d( m_zoomx[1],y ,zpos);
  glVertex3d(x,m_zoomy[0],zpos);
  glVertex3d( x,m_zoomy[1],zpos );
	glEnd();
	//TRACE("%f %f\n",x,y);
	string txt;
	if(m_ejxhora)
   txt= strprintf("%s,%f",PasaFecha((long)x*1000).c_str(),y);
	else
	 txt= strprintf("%.3f ; %.3f",x,y);
	int txtlength=txt.length();
	if(m_zoomy[1]-y <m_fontHeight)//no se ve por encima
	{
		if(m_zoomx[1]-x>m_fontWidth*txtlength)//por la derecha
      DrawText(txt,x+m_fontWidth/2,y-m_fontHeight,zpos,1,m_backgroundCurrentValue,0x000000,GLUT_BITMAP_HELVETICA_18);
		else//no se ve por la derecha
      DrawText(txt,x-m_fontWidth*txtlength,y-m_fontHeight,zpos,1,m_backgroundCurrentValue,0x000000,GLUT_BITMAP_HELVETICA_18);
	}
	else//entra por encima
	{
		if(m_zoomx[1]-x>m_fontWidth*txtlength)//por la derecha
      DrawText(txt,x+m_fontWidth/2,y+m_fontHeight/2,zpos,1,m_backgroundCurrentValue,0x000000,GLUT_BITMAP_HELVETICA_18);
		else//no se ve por la derecha
      DrawText(txt,x-m_fontWidth*txtlength,y+m_fontHeight/2,zpos,1,m_backgroundCurrentValue,0x000000,GLUT_BITMAP_HELVETICA_18);
	}
}

void OGLGraph::DrawText( string txt,const double posx,const double posy,const float posz/*=0*/,int fondo/*=0*/ ,COLORREF colorfondo/*=0*/,COLORREF colorletra/*0*/,void* tipoletra/*=NULL*/)
{
	int colorini[4];
	//colorfondo=0x00ff0000;
	if(fondo)
	{
		glGetIntegerv(GL_CURRENT_COLOR,(int*)&colorini);
		glBegin( GL_QUADS );	
		glColor3ub( OGL_RED(colorfondo),OGL_GREEN(colorfondo),OGL_BLUE(colorfondo) );
		glVertex3d(posx , posy-m_fontHeight/5 ,posz-0.01f);
		glVertex3d(posx+m_fontWidth*txt.size() , posy-m_fontHeight/5 ,posz-0.01f );

		glVertex3d(posx +m_fontWidth*txt.size(), posy+m_fontHeight -m_fontHeight/5,posz-0.01f );
		glVertex3d( posx , posy+m_fontHeight-m_fontHeight/5 ,posz-0.01f );
		glEnd();
		glColor3ub( colorini[0],colorini[1],colorini[2] );
	}
  if((int)colorletra!=-1)
		glColor3ub( OGL_RED(colorletra),OGL_GREEN(colorletra),OGL_BLUE(colorletra) );
	if(tipoletra==NULL)
		tipoletra=OGL_TIPO_LETRA;
	char* buf=(char*)txt.c_str();

	for(int c=txt.size();c>=0;c--)
	{
		glRasterPos3d(posx+ c*m_fontWidth,posy ,posz );
    glutBitmapCharacter(tipoletra,buf[c]);
	}
}

void OGLGraph::DrawTextInPixels( string txt,const double posx,const double posy,const float posz/*=0*/,int fondo/*=0*/ ,COLORREF colorfondo/*=0*/,COLORREF colorletra/*0*/,void* tipoletra/*=NULL*/)
{
	int colorini[4];
	glGetIntegerv(GL_CURRENT_COLOR,(int*)&colorini);
	//colorfondo=0x00ff0000;
	if(fondo)
	{
		glBegin( GL_QUADS );
		glColor3ub( OGL_RED(colorfondo),OGL_GREEN(colorfondo),OGL_BLUE(colorfondo) );
		glVertex3d(posx , posy-OGL_FONT_HEIGHT/5 ,posz-0.01f);
		glVertex3d(posx+OGL_FONT_WIDTH*txt.size() , posy-OGL_FONT_HEIGHT/5 ,posz-0.01f );

		glVertex3d(posx +OGL_FONT_WIDTH*txt.size(), posy+OGL_FONT_HEIGHT -OGL_FONT_HEIGHT/5,posz-0.01f );
		glVertex3d( posx , posy+OGL_FONT_HEIGHT-OGL_FONT_HEIGHT/5 ,posz-0.01f );
		glEnd();
		glColor3ub( colorini[0],colorini[1],colorini[2] );
	}
  if((int)colorletra!=-1)
		glColor3ub( OGL_RED(colorletra),OGL_GREEN(colorletra),OGL_BLUE(colorletra) );
	if(tipoletra==NULL)
		tipoletra=OGL_TIPO_LETRA;
	char* buf=(char*)txt.c_str();
  glRasterPos3d(posx,posy ,posz );
	for(unsigned int c=0;c<txt.size();c++)
	{		
		glutBitmapCharacter(tipoletra,buf[c]);
   //glutStrokeCharacter(GLUT_STROKE_ROMAN,buf[c]);
	}
	glColor3ub( colorini[0],colorini[1],colorini[2] );
}

void OGLGraph::DrawMaxValue(int pos)
{
	if(pos<0)
		return;

	float x=GetLine(m_focusLineIndex)->GetPX()[pos];
	float y=GetLine(m_focusLineIndex)->GetPY()[pos];
  string txt=this->strprintf("%f,%f",x,y);
	DrawText(txt,x+m_fontWidth/2,y+m_fontHeight/2,0.6f,1,m_backgroundCurrentValue,GetLine(m_focusLineIndex)->color);
}

///
///ZOOM
///
void OGLGraph::SetZoom(GLdouble minx, GLdouble maxx, GLdouble miny, GLdouble maxy)
{
	double val=ABS(fmax(minx,maxx)-fmin(minx,maxx));
	if(val > OGL_MIN_DEFINITION)
	{
		m_zoomx[0]=fmin(minx,maxx);
		m_zoomx[1]=fmax(minx,maxx);
	}

	val=ABS(fmax(miny,maxy)-fmin(miny,maxy));
	if(val > OGL_MIN_DEFINITION)
	{
	m_zoomy[0]=fmin(miny,maxy);
	m_zoomy[1]=fmax(miny,maxy);
	}

	FindOrigin();
	CalculaLeyenda();
  RePaint();
}

void OGLGraph::AutoZoom()
{
	if (m_plines==NULL)
		return;
	float minx=5e6,maxx=-5e6,miny=5e6,maxy=-5e6;
	int nline;
	CXYLineGL *pt;
	for ( nline=0,pt=m_plines;pt!=NULL;pt=pt->nextLine,nline++)
	{
		if(pt->visible)
		{
      double mix=5e6,max=-5e6,miy=5e6,may=-5e6;
			SearchLimits(pt->GetPX(),pt->GetUsedPoints(),&mix,&max);
			minx=	mix!=5e6 && minx>mix ? mix :minx;
			maxx=	max!=-5e6 && maxx<max ? max :maxx;
			SearchLimits(pt->GetPY(),pt->GetUsedPoints(),&miy,&may);
			miny=	miy!=5e6 && miny>miy ? miy :miny;
			maxy=	may!=-5e6 && maxy<may ? may :maxy;
		}
	}
	if((maxx-minx<1e-10)||(maxy-miny<1e-10))
		return;
	if(miny==maxy)
		{
			maxy+=miny/5;
			miny-=miny/5;
		}
	SetZoom(minx,maxx,miny,maxy);
}

void OGLGraph::SearchLimits(const double* buf,long ndat,double *min,double *max)
{
	*min=50e10f;
	*max=-50e20f;
	for(int i=0;i<ndat;i++)
	{
		if(buf[i]<*min)
			*min=buf[i];
		if(buf[i]>*max)
			*max=buf[i];		
	}
}

void OGLGraph::ZoomEjeY(int zDelta,GLdouble y)
{
	GLdouble total=m_zoomy[1]-m_zoomy[0];
	GLdouble med=(m_zoomy[1]+m_zoomy[0])/2;

	if(zDelta<0)	//zoom fuera
	{
		SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0]-total/2*(y-m_zoomy[0])/(med-m_zoomy[0])
				,m_zoomy[1]+total/2*(m_zoomy[1]-y)/(total/2));
	}
	else
	{
		SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0]+total/4*(y-m_zoomy[0])/(med-m_zoomy[0]),
				m_zoomy[1]-total/4*(m_zoomy[1]-y)/(total/2));
	}
}

void OGLGraph::ZoomEjeX(int zDelta, GLdouble x)
{

	GLdouble total=m_zoomx[1]-m_zoomx[0];
	GLdouble med=(m_zoomx[1]+m_zoomx[0])/2;
	if(zDelta<0)	//zoom fuera
	{
		SetZoom(m_zoomx[0]-total/2*(x-m_zoomx[0])/(med-m_zoomx[0]),m_zoomx[1]+total/2*(m_zoomx[1]-x)/(total/2),m_zoomy[0],m_zoomy[1]);
	}
	else
	{
		SetZoom(m_zoomx[0]+total/4*(x-m_zoomx[0])/(med-m_zoomx[0]),m_zoomx[1]-total/4*(m_zoomx[1]-x)/(total/2),m_zoomy[0],m_zoomy[1]);
	}
}

///
///CHART CALCULATION
///
void OGLGraph::FindOrigin()
{
	m_freeSpaceXaxis=floor(OGL_FONT_HEIGHT*1.5+m_nlegend_lines*OGL_FONT_HEIGHT);
	m_posorigx=OGL_PIXELSTOWORLD_X(2*m_freeSpaceYaxis);
	m_posorigx=m_zoomx[0]-(m_posorigx-m_zoomx[0]);//pto inferior izquierdo

	m_posorigy=OGL_PIXELSTOWORLD_Y(2.0*(m_freeSpaceXaxis));
	m_posorigy=m_zoomy[0]-(m_posorigy-m_zoomy[0]);

	m_fontWidth=OGL_PIXELSTOWORLD_X(OGL_FONT_WIDTH+m_freeSpaceYaxis);
	m_fontWidth-=m_zoomx[0];

	m_fontHeight=	OGL_PIXELSTOWORLD_Y(OGL_FONT_HEIGHT+m_freeSpaceXaxis);
	m_fontHeight-=m_zoomy[0];
}

int  OGLGraph::PixelsToWorld( const CHARTS_POINTD *ppoint,double* x,double*y)
{

	CHARTS_POINTD point;
	if(ppoint==NULL)//cojemos el raton
	{
		point=GetMousePoint();;
	}
	else
		point=CHARTS_POINTD(ppoint->x,ppoint->y);
	point.y=m_height-point.y;
	GLdouble modelMatrix[16];
	GLdouble xd,yd,zd;
	glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);
	GLdouble projMatrix[16];
	glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	gluUnProject((GLdouble)point.x,(GLdouble)point.y,	0,
		modelMatrix,		projMatrix,		viewport,	&xd, 	&yd,	&zd 	);
	*x=xd;
	*y=yd;
	return 1;
}

void OGLGraph::WorldToPixels( double x,double y,CHARTS_POINTD *point)
{
	//BlockPaint(0);
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

string OGLGraph::SearchClosePoint( CHARTS_POINTD pt)
{
	double * iniciox,*inicioy;
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
	return salida;
}

string OGLGraph::GetTextFromPoint(double x,double y)//
{
 return strprintf("X=%.4f\nY=%.4f",x,y);
}

int  OGLGraph::GetNextVisible()
{
	for(int n=0;n<m_nlines;n++)
		if (GetLine(n)->visible!=0)
			return n;
	return -1;
}

string OGLGraph::strprintf(const char* format,...)
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

string OGLGraph::PasaFecha(long msegundos)
{
	string msg;
	msegundos++;
//	msg.Format("%02d:%02d:%02d.%03d",msegundos/3600000,msegundos%3600000/60000,msegundos%3600000%60000/1000,msegundos%3600000%60000%1000);
	//msg.Format("%02d:%02d:%02d",msegundos/3600000,msegundos%3600000/60000,msegundos%3600000%60000/1000,msegundos%3600000%60000%1000);
	return msg;
}

void OGLGraph::CalculaLeyenda()
{
	if(m_nlines)
	{
		m_linesperline=(m_width-60)/(m_nlines*(m_widthLegend));
		m_linesperline=m_linesperline>0 ?m_linesperline:1;
		m_nlegend_lines=m_linesperline<m_nlines?m_nlines/m_linesperline:1;
	}
}

void OGLGraph::OptimunAxisScale(GLdouble minmaxIn[2],GLdouble stepIn,int nTipsIn,GLdouble* minOut,
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
	//char txt[90];
 // sprintf(txt,"%%.%df",(int)-fmin(0,ndigdec));
 //   *fmtOut=txt;
	*precision=(int)-fmin(0,ndigdec);
}

int OGLGraph::MaximoLocal()
{
	if (m_focusLineIndex<0)
		return -1;
	int i,pos_max=-100;
//	GLdouble pendiente=(m_zoomy[1]-m_zoomy[0])/10;
	double * iniciox,*inicioy,valory=-100e32f;
	iniciox=GetLine(m_focusLineIndex)->GetPX();
	inicioy=GetLine(m_focusLineIndex)->GetPY();
	float valx,valy;
	for(i=2;i<GetLine(m_focusLineIndex)->GetNPoints();i++)
	{
		valx=iniciox[i];
		valy= inicioy[i];
		if(valx>m_zoomx[1])
			return pos_max ;
		if(valx>m_zoomx[0]&&valx<m_zoomx[1]&&
			valy>valory /*&& inicioy[i-1]-inicioy[i]>pendiente &&inicioy[i-1]-inicioy[i-2]>pendiente*/)
		{
			pos_max=i/*-1*/;
			valory=valy;//inicioy[i/*-1*/];
		}
	}
	return pos_max;
}

void OGLGraph::SelectLine(double x,double y)
{
	m_focusLineIndex=-1;
	if(m_nlines<=0||m_plines==NULL)
		return ;
	LockDataMutex();
	int cont=0;
	GLdouble max=m_zoomx[1];
	CHARTS_POINTD pt(0,0);double distx,disty,disty1;
	PixelsToWorld(&pt,&distx,&disty);
	pt.y=20;//minimun distance 10px
	PixelsToWorld(&pt,&distx,&disty1);
	GLdouble total=fabs(disty1-disty);
	for (CXYLineGL *pt=m_plines;pt!=NULL;cont++,pt=pt->nextLine)
	{
		if(!pt->visible)
			continue;
		double* iniciox=pt->GetPX();double* inicioy=pt->GetPY();
		int npuntos=GetLine(m_focusLineIndex)->GetNPoints();
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
	UnlockDataMutex();
}

///
///   SAVING IMAGES,GRAFS,DAT....
///
void OGLGraph::SaveCurrent(string dir)
{
/*archivo de grafica:*/
	double color,ndatlinea,visible,nlines=(double)m_nlines;		//numero de lineas
	double ejxini=(double)m_zoomx[0],ejxfin=(double)m_zoomx[1],ejyini=(double)m_zoomy[0],ejyfin=(double)m_zoomy[1];//zona visible
  /*POR LINEA*/
  char nombrelinea[16];	//nombre de la linea
  COLORREF colora;
  //DATOS EJEX Y DATOS LINEAS
  FILE* fid=fopen(dir.c_str(),"wb");
  if(fid==NULL)
    return;
	fwrite(&nlines,sizeof(double),1,fid);	//5double 20 bytes
	fwrite(&ejxini,sizeof(double),1,fid);
	fwrite(&ejxfin,sizeof(double),1,fid);
	fwrite(&ejyini,sizeof(double),1,fid);
	fwrite(&ejyfin,sizeof(double),1,fid);
  int i;
  for( i=0;i<nlines;i++)							//24bytes
  {
		ndatlinea=(double)GetLine(i)->GetNPoints();
		fwrite(&ndatlinea,sizeof(double),1,fid);
		ndatlinea=(double)GetLine(i)->GetUsedPoints();
		fwrite(&ndatlinea,sizeof(double),1,fid);
    colora=this->GetLine(i)->color;
    memcpy(&color,&colora,sizeof(COLORREF));
		fwrite(&color,sizeof(double),1,fid);
    visible=GetLine(i)->visible;
		fwrite(&visible,sizeof(double),1,fid);
    strcpy(nombrelinea,GetLine(i)->lineName.c_str());
    fwrite(nombrelinea,sizeof(char),16,fid);
  }
  for(i=0;i<nlines;i++)							//[datos ejx][datos ejy]
  {
		fwrite(GetLine(i)->GetPX(),sizeof(double),GetLine(i)->GetUsedPoints(),fid);
		fwrite(GetLine(i)->GetPY(),sizeof(double),GetLine(i)->GetUsedPoints(),fid);
  }
  fclose(fid);
}

void OGLGraph::Open(string dir)
{
   char *buf;
  FILE* fid=fopen(dir.c_str(),"wb");
  if(fid==NULL)
    return;
  fseek(fid,0,SEEK_END);
	long size=ftell(fid);

  buf=new char[size];
	fseek(fid,0,SEEK_SET);
	fread(buf,1,size,fid);
  fclose(fid);
  int nlines=(int)*(float*)&buf[0];
  float*fin=(float*)&buf[20+nlines*32];
  RemoveLines(0);
  for(int i=0;i<nlines;i++)
  {
    float * dat=(float*)&buf[20+i*32];
    AddLine((int)dat[0],(char*)&dat[4],*(COLORREF*)&dat[2],(int)dat[3]);
    GetLine(i)->SetUsedPoints((int)dat[1]);
    memcpy(GetLine(i)->GetPX(),fin,(size_t)dat[1]*sizeof(float));
    fin+=(int)dat[1];
    memcpy(GetLine(i)->GetPY(),fin,(size_t)dat[1]*sizeof(float));
    fin+=(int)dat[1];

  }
  float *zoom=(float*)&buf[4];
	SetZoom(zoom[0],zoom[1],zoom[2],zoom[3]);
	m_focusLineIndex=GetNextVisible();
  delete []buf;
}

void OGLGraph::SaveDat(string dir)
{
	if(m_focusLineIndex==-1)
    return;
  FILE* fid=fopen(dir.c_str(),"wt");
  if(fid==NULL)
    return;
  int	maxpoints=0;
  for(int r=0;r<m_nlines;r++)
    maxpoints=maxpoints>GetLine(r)->GetUsedPoints() ? maxpoints :GetLine(r)->GetUsedPoints();

  for(int r=0;r<maxpoints;r++)//una fila por cada punto
  {
    for(int c=0;c<m_nlines;c++)  //dos  columnas (x,y) por cada linea de grafica
    {
      if(GetLine(c)->GetUsedPoints()>c)//miramos si hay datos
        fprintf(fid,"%5.5f\t%5.5f\t",GetLine(c)->GetPX()[r],GetLine(c)->GetPY()[r]);
      else
        fprintf(fid," \t \t");
    }
    fprintf(fid,"\n");//fin de linea
  }
  fclose(fid);
  return;
}

void OGLGraph::GetScreenShot(GLubyte * raw,int posx,int posy,int width,int heignt,int bitspixel)
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
void OGLGraph::OnMouseWheel(CHARTS_POINTD pt,unsigned int flags,int delta)
{//todo msg recalcular
  flags++;
  double posx,posy;
	BlockPaint(0);
	PixelsToWorld(&pt,&posx,&posy);
	unBlockPaint();
  if(posx>m_zoomx[0] && posx<m_zoomx[1] && posy>m_zoomy[0] && posy<m_zoomy[1] )//esta en la grafica
  {
		ZoomEjeY(delta, posy);
		ZoomEjeX(delta, posx);
  }
  if(posx>m_zoomx[0] && posy<m_zoomy[0] )//esta en ejeX
		ZoomEjeX(delta, posx);
  if(posx<m_zoomx[0] && posy>m_zoomy[0] )//esta en ejeY
		ZoomEjeY(delta, posy);
	SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0],m_zoomy[1]);
  RePaint();
}

void OGLGraph::OnKeyPress(chartkeys keypress)
{
 GLdouble incr;
 switch(keypress)
 {
  case k_left:
    incr=(fabs(m_zoomx[1]-m_zoomx[0]))/10;
		SetZoom(m_zoomx[0]-incr,m_zoomx[1]-incr,m_zoomy[0],m_zoomy[1]);
   break;
  case k_right:
    incr=(fabs(m_zoomx[1]-m_zoomx[0]))/10;
		SetZoom(m_zoomx[0]+incr,m_zoomx[1]+incr,m_zoomy[0],m_zoomy[1]);
   break;
  case k_up:
    incr=(fabs(m_zoomy[1]-m_zoomy[0]))/10;
		SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0]+incr,m_zoomy[1]+incr);
   break;
  case k_down:
    incr=(fabs(m_zoomy[1]-m_zoomy[0]))/10;
		SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0]-incr,m_zoomy[1]-incr);
   break;
 default:
   break;
 }
 RePaint();
}

void OGLGraph::OnMouseMove(CHARTS_POINTD pt,unsigned int flags)
{
	if(m_drawManualDistance==1)
	{
		m_drawManualDistance=2;
	}
    if( flags & CONTROL_MODDIFIER)
    {
      //todo tooltip
    }
    else
    {//mover toda grafica con boton central
			if(flags & MOUSE_MID_BUTTON && !m_posaxisscroll )
      {
        GLdouble posx,posy,posxant,posyant;
				PixelsToWorld(&pt,&posx,&posy);
				PixelsToWorld(&m_oldCursorPosition,&posxant,&posyant);
				if(m_oldCursorPosition.x!=pt.x)//MOVER EJE X
        {
          GLdouble inc=posx-posxant;
          m_zoomx[0]-=inc;//PIXELSAMEDIDA_X(inc);
          m_zoomx[1]-=inc;//PIXELSAMEDIDA_X(inc);
        }
				if(m_oldCursorPosition.y!=pt.y)//MOVER EJE Y
        {
          GLdouble inc=posy-posyant;
          m_zoomy[0]-=inc;//PIXELSAMEDIDA_Y(inc);
          m_zoomy[1]-=inc;//PIXELSAMEDIDA_Y(inc);
        }
				SetZoom(m_zoomx[0],m_zoomx[1],m_zoomy[0],m_zoomy[1]);
				m_oldCursorPosition=pt;
				return;
      }
      else
      {
        //todo desactivar tooltip
      }
    }
		m_oldCursorPosition=pt;
		if(m_posaxisscroll||m_drawManualDistance||m_drawZoomBox||m_drawCurrentPos)
			RePaint();
}

void OGLGraph::OnLMousePress(CHARTS_POINTD pt,unsigned int flags)
{
	BlockPaint(0);
  double posx,posy;
	PixelsToWorld(&pt,&posx,&posy);
	unBlockPaint();
	if(posx>m_zoomx[0] && posx<m_zoomx[1] && posy>m_zoomy[0] && posy<m_zoomy[1])//esta en la grafica
	{
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

    }
  }
  if(flags& CONTROL_MODDIFIER)//tecla control
  {
		m_drawManualDistance=0;
  }
	if(posx>m_xLegendPosition && posx<m_xLegendPosition+m_legendWidth && posy<m_yLegendPosition && posy>m_yLegendPosition-m_legendHeight )//esta en la leyenda
  {
		int sel=(int)((m_yLegendPosition-posy)/m_fontHeight);
		m_focusLineIndex=GetLine(sel)->visible ? sel : m_focusLineIndex;
		m_drawManualDistance=0;
  }
	SelectLine(posx,posy);
	RePaint();
}

void OGLGraph::OnLMouseRelease(CHARTS_POINTD pt,unsigned int flags)
{
  double posx,posy;
	BlockPaint(0);
	PixelsToWorld(&pt,&posx,&posy);
	unBlockPaint();
	if(flags& SHIFT_MODDIFIER &&posx>m_zoomx[0] && posx<m_zoomx[1] && posy>m_zoomy[0] && posy<m_zoomy[1]&&m_drawZoomBox)//esta en la grafica)
  {
    double posx0,posy0;
		BlockPaint(0);
		PixelsToWorld(&m_initialZoomBoxPoint,&posx0,&posy0);
		unBlockPaint();
		if(posx0!=posx && posy0 != posy )
			SetZoom(posx0,posx,posy0,posy);
		m_drawZoomBox=0;
  }
  else
		m_drawZoomBox=0;
	m_drawManualDistance=m_drawCurrentPos=0;
  RePaint();
}

void OGLGraph::OnMMousePress(CHARTS_POINTD pt,unsigned int )
{
  double posx,posy;
	BlockPaint(0);
	PixelsToWorld(&pt,&posx,&posy);
	unBlockPaint();
  //if(flags & MOUSE_MID_BUTTON)
	m_oldCursorPosition=pt;
}

void OGLGraph::OnMMouseRelease(CHARTS_POINTD ,unsigned int )
{
	m_drawManualDistance=m_drawCurrentPos=0;
}

void OGLGraph::OnRMousePress(CHARTS_POINTD ,unsigned int )
{

}

void OGLGraph::OnRMouseRelease(CHARTS_POINTD pt,unsigned int flags)
{
  pt.x=pt.x+flags;
}

void OGLGraph::OnDoubleClic(CHARTS_POINTD pt,unsigned int flags)
{
  flags++;
  double posx,posy;
	BlockPaint(0);
	PixelsToWorld(&pt,&posx,&posy);
	unBlockPaint();
  if(posx>m_zoomx[0] && posx<m_zoomx[1] && posy>m_zoomy[0] && posy<m_zoomy[1] )//esta en la grafica)
  {
		m_drawManualDistance=1;
		m_initialManualDistancePoint[0]=posx;
		m_initialManualDistancePoint[1]=posy;
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

void OGLGraph::OnResizeEvent(int width,int height)
{
	m_width=width;
	m_height=height;
	CalculaLeyenda();
	FindOrigin();
	RePaint();
}

void OGLGraph::OnCloseEvent (  )
{
 // m_posdialogo=pos();
}

CHARTS_POINTD OGLGraph::GetPreviousCursorPosition() const
{
	return m_oldCursorPosition;
}

void OGLGraph::SetPreviousCursorPosition(const CHARTS_POINTD &pos_raton_ant)
{
	m_oldCursorPosition = pos_raton_ant;
}

