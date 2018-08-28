// ColorMap.cpp : implementation file
/*! \file
	\brief Archivo de implementacion las ColorMapGl3D.
	\date 05-2010
	\author Marcos Perez*/
#include "stdio.h"
#include <math.h>
#include "colormapgl3d.h"


/////////////////////////////////////////////////////////////////////////////
// CColorMapGL3D 
#define ABS(a) (a)>0?(a):-(a) 
CColorMapGL3D::CColorMapGL3D (const char* rutamap)
{
	nColors=0;
	colorTable=NULL;
	mini=0.0F; maxi=1.0F,
	startfromzero=false;
	if(rutamap==NULL)
		DefaultColorTable(16);
	else
		if(ReadColorTable(rutamap)==0)
			DefaultColorTable(16);
}

CColorMapGL3D::~CColorMapGL3D ()
{
	if (colorTable!=NULL)
	{
		delete []colorTable;
		colorTable=NULL;
	}
	colorTable=NULL;
}

int CColorMapGL3D::ReadColorTable(const char* filename)
{
	FILE* fid=fopen(filename,"r");
	int i;
	float r,g,b,v;

	char linea[1024];

	if (fid==NULL)
	{
		return 0;
	}
	if (colorTable!=NULL)
	{
		delete []colorTable;
		colorTable=NULL;
	}
	fgets(linea,1024,fid);
	sscanf(linea,"%d",&nColors);
	colorTable=new strColorTable3D[nColors];

	int nread;

	for (i=0;i<nColors;i++)
	{
		fgets(linea,1024,fid);
		nread=sscanf(linea,"%f%f%f%f",&r,&g,&b,&v);
		if(nread<3)
			return nColors;
		colorTable[i].color[0]=r;
		colorTable[i].color[1]=g;
		colorTable[i].color[2]=b;
	}
	fclose(fid);
	
	return nColors;
}

void CColorMapGL3D::DefaultColorTable(int i_ncolors)
{
	if (colorTable!=NULL)
	{
		delete [] colorTable;
		colorTable=NULL;
	}
	nColors=i_ncolors;
	colorTable=new strColorTable3D[nColors];

	int i;

	for (i=0;i<nColors;i++)
	{
		colorTable[i].color[0]=(float)i/(nColors-1);
		colorTable[i].color[1]=(float)i/(nColors-1);
		colorTable[i].color[2]=0.0f;
	}
}	
void CColorMapGL3D::SetMinMax(float i_min,float i_max)
{
	mini=i_min;
	maxi=i_max;
}

void CColorMapGL3D::setStartfromzero(bool value)
{
    startfromzero = value;
}

float CColorMapGL3D::ValuesToColors(float value,int rgb,float min,float max)
{
	float x;
	int ind1,ind2;
	float color;
	if (mini==maxi|| rgb>2)
	{
		return 0.0f;
	}
	x=(value-min )/(max-min);
	if (x<0.0F)
		x=0.0F;
	if (x>1.0F)
		x=1.0F;
	x*=nColors-1;
	ind1=int(floor(x));
	ind2=int(ceil(x));
	if (ind1==ind2)
	{
		color=colorTable[ind1].color[rgb];
	}
	else
	{
		float col=colorTable[ind1].color[rgb];
		color=col+(x-ind1)/(ind2-ind1)*(colorTable[ind2].color[rgb]-col);

	}
	return color;
}

void CColorMapGL3D::ValuesToColors(const double *values,const int nvalues,struct strColorTable3D* colors,double min,double max)
{
	float x;
	int ind1,ind2;

	if (mini==maxi)
	{
		return ;
	}
	for(int i=0;i<nvalues;i++)
	{
		x=(values[i]-min )/(max-min);
		if (x<0.0F)
			x=0.0F;
		if (x>1.0F)
			x=1.0F;
		x*=nColors-1;
		ind1=int(floor(x));
		ind2=int(ceil(x));
		if (ind1==ind2)
		{
			colors[i].color[2]=colorTable[ind1].color[2];
			colors[i].color[1]=colorTable[ind1].color[1];
			colors[i].color[0]=colorTable[ind1].color[0];
		}
		else
		{
			float col=colorTable[ind1].color[0];
			colors[i].color[0]=col+(x-ind1)/(ind2-ind1)*(colorTable[ind2].color[0]-col);
			col=colorTable[ind1].color[1];
			colors[i].color[1]=col+(x-ind1)/(ind2-ind1)*(colorTable[ind2].color[1]-col);
			col=colorTable[ind1].color[2];
			colors[i].color[2]=col+(x-ind1)/(ind2-ind1)*(colorTable[ind2].color[2]-col);
		}
	}
}

