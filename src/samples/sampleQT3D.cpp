/* sampleQT.cpp: Qt sample for Opengl3DCharts usage.
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
along with this library;*/

#include <QtGui>
#include <QApplication>
#include <GL/glut.h>

#include "opengl3dcharts.h"
int main(int argc, char *argv[])
{
    //XInitThreads();
    setvbuf(stdout,(char*)NULL,_IONBF,0);
    QApplication a(argc, argv);
		glutInit(&argc, argv);
		Opengl3DCharts*m_oglwindow3D=new Opengl3DCharts(NULL);

		//Add a window tittle.
    m_oglwindow3D->setWindowTitle("Dialog title");

		//Add a chart and axis titles.
    m_oglwindow3D->SetGraphTitle("chart title","x_axis","y_axis");

		//Create the chart, define size popup and redraw time.
		m_oglwindow3D->CreateAndPosition(NULL,1000,0,500,500,1,50);

		//Set chart zoom.0
		m_oglwindow3D->SetZoom(-100,100,-200,200,-100,100);

		//Add some data to our chart
		int pointsnumber=500;
		int zlines=100;

    //Add one line.
    m_oglwindow3D->AddLine(pointsnumber,zlines,"Line 1",0x0000fff,1);

    //Add another line.
    m_oglwindow3D->AddLine(pointsnumber,zlines,"Line 2",0x00ffff00,1);

    //Fill the points.The space is reserved into the library. Get it and fill.
    //Fill z values
    double *z=m_oglwindow3D->GetLine(0)->GetPZ();//inicializar eje z
    double *z1=m_oglwindow3D->GetLine(1)->GetPZ();
    for(int b=0;b<zlines;b++)
      z[b]=z1[b]=b;

    //Fill X values, are the same for the z lines.
    double *x=m_oglwindow3D->GetLine(0)->GetPX();
    double *x1=m_oglwindow3D->GetLine(1)->GetPX();
    for(int b=0;b<pointsnumber;b++)
      x[b]=x1[b]=b;

    //Fill Y values, height
    for(int i=0;i<zlines;i++)
    {
      double *y=m_oglwindow3D->GetLine(0)->GetPY(i);
      double *y1=m_oglwindow3D->GetLine(1)->GetPY(i);
      for(int b=0;b<pointsnumber;b++)
      {
        y[b]=100*sin(0.005*b);
        y1[b]=70*cos(0.05*b);
      }
    }

    //Force to use filled number of lines.
    m_oglwindow3D->GetLine(0)->SetUsedLinesT(zlines);
    m_oglwindow3D->GetLine(1)->SetUsedLinesT(zlines);

    // Note  we have all acces to qt staff of the dialog.
    a.exec();
}
