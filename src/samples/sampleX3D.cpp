#include <GL/glut.h>
#include <math.h>
#include "opengl3dcharts.h"

int main(int argc, char *argv[])
{
		glutInit(&argc, argv);
		setvbuf(stdout,(char*)NULL,_IONBF,0);
		Opengl3DCharts *m_oglwindow3D=new Opengl3DCharts();

		//Add a window tittle.
		m_oglwindow3D->SetWindowTitle("graphs");

		//Add a chart and axis titles.
		m_oglwindow3D->SetGraphTitle("grsdfasdf","ejex","ejey");

		//Create the chart, define size popup and redraw time.
		m_oglwindow3D->CreateAndPosition(NULL,1000,0,500,500,1,50);

		//Set chart zoom.0
		m_oglwindow3D->SetZoom(-100,100,-200,200,-100,100);

		//Add some data to our chart
    int pointsnumber=500; // XY number of values
    int zlines=100; //depth, repeated xy values

		//Add one line.
		m_oglwindow3D->AddLine(pointsnumber,zlines,"Line 1",0x0000ff00,1);

		//Add another line.
		m_oglwindow3D->AddLine(pointsnumber,zlines,"Line 2",0x00ff0000,1);

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

    fflush(stdin);
		printf("Press x to exit\n");		

    while(1)
		{
			int ms=20;
			struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
			nanosleep(&ts, NULL);
		}

		//Dont forget to delete.
		delete m_oglwindow3D;
}
