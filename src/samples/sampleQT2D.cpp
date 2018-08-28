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

#include <QtGui>
#include <QApplication>
#include <GL/glut.h>
#include "opengl2dcharts.h"

int main(int argc, char *argv[])
{
    //XInitThreads();
    setvbuf(stdout,(char*)NULL,_IONBF,0);
    QApplication a(argc, argv);
		glutInit(&argc, argv);
		Opengl2DCharts *m_oglwindow=new Opengl2DCharts(NULL);

		//Add a window tittle.
    m_oglwindow->SetWindowTitle("Window title");

		//Add a chart and axis tittles.
    m_oglwindow->SetGraphTitle("chart title","x_axis","y_axis");

		//Create the chart, define size popup and redraw time.
		m_oglwindow->CreateAndPosition(NULL,1000,0,500,500,1,50);

		//Set chart zoom.
		m_oglwindow->SetZoom(0,500,0,5);

		//Add some data to our chart
		int pointsnumber=5000;

		//Add one line.
    m_oglwindow->AddLine(pointsnumber,"Line 1",0x0000ff00,1);

		//Add another line.
    m_oglwindow->AddLine(pointsnumber,"Line_2",0x00FF0000,1);

		//Fill the points.The space is created insithe the graph. Get it and fill.

		double *x=m_oglwindow->GetLine(0)->GetPX();
		double *y=m_oglwindow->GetLine(0)->GetPY();
		int np=5000;

		for(int i=0;i<np;i++)
		{
			x[i]=i;
			y[i]=sin(0.0005*i);
			m_oglwindow->GetLine(1)->GetPY()[i]=sin(0.005*i);
		}

    a.exec();
}
