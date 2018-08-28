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
	\brief xopengl3dcharts.cpp file.
	\date 05-2017
	\author Marcos Perez*/
#ifdef __linux__
	#define X_TARGET
#endif

#ifdef X_TARGET
#include "xopengl3dcharts.h"
#include <sys/time.h>

#define DOUBLE_CLICK_TIME_MS  350  //!< MS DELAY FOR DOUBLE CLICK DETECTION


int Opengl3DCharts::m_ngraphs=0;
Opengl3DCharts::Opengl3DCharts(void * parent)
{
	m_display=NULL;
	this->m_rootwd=(Window)parent;
	m_currgraph=m_ngraphs;
	m_movecursor=0;
	m_display=0;
	m_window=0;
	m_ngraphs++;
}
Opengl3DCharts::~Opengl3DCharts()
{
	void *res;
	pthread_join(m_windowthread,&res);//kill events thread
	XFreeCursor(m_display, m_movecursor);
	glXDestroyContext(m_display,m_glcontext);
	XDestroyWindow(m_display,m_window);
	XCloseDisplay(m_display);
	m_ngraphs--;

}

void Opengl3DCharts::InitXdisplay()
{
	m_display = XOpenDisplay(NULL);
	if(m_display == NULL)
	{
		printf("\n\tcannot connect to X server\n\n");
		return;
	}
	m_rootwd=DefaultRootWindow(m_display);

	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

	m_visualinfo=glXChooseVisual(m_display, 0, att);
	if(m_visualinfo==NULL)
	{
		printf("\n\tno appropiate visual found\n\n");
		return;
	}
	m_colormap = XCreateColormap(m_display, m_rootwd, m_visualinfo->visual, AllocNone);
}

void Opengl3DCharts::CreateWindow()
{
	if(m_display==0 || m_visualinfo==NULL)
	{
		return;
	}
	XSetWindowAttributes swa;
	swa.colormap = m_colormap;
	swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask | Button1MotionMask | Button2MotionMask |
			ButtonPressMask|ButtonPress|ButtonReleaseMask | PointerMotionMask; //KeymapStateMask

	m_window = XCreateWindow(m_display, m_rootwd, m_xposition, m_yposition,m_width,m_height, 0,
					m_visualinfo->depth, InputOutput, m_visualinfo->visual, CWColormap | CWEventMask, &swa);

	if(m_window==0)
		return;

	XAllowEvents(m_display,AsyncBoth,CurrentTime);
	XMapWindow(m_display, m_window);
	XStoreName(m_display, m_window, "X window");

	m_glcontext = glXCreateContext(m_display,m_visualinfo , NULL, GL_TRUE);
	if(m_glcontext==0)
		return;

	initializeGL();
	InitGraph();
}

void Opengl3DCharts::initializeGL()
{
	//glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glXMakeCurrent(m_display, m_window, m_glcontext);
	GLbyte *vers=(GLbyte*)glGetString(GL_VERSION);
	printf( "Initialized with OpenGL %s\n ", vers);
}
void Opengl3DCharts::CreateCursor()
{
	if(m_movecursor!=0)
		return;

	//m_movecursor=XCreateFontCursor(m_display, XC_fleur);
}

void Opengl3DCharts::SetWindowTitle(string tittle)
{
	if(m_display && m_window)
		XStoreName(m_display, m_window, tittle.data());
}

void Opengl3DCharts::RePaint()
{
	if(!TryBlockPaint())
	{
		printf("painteventwhilepainting\n");
		return;
	}
	if(m_display!=0 && 0!= m_window)
	{
		glXMakeCurrent(m_display, m_window, m_glcontext);
		RedrawChart();
		glXSwapBuffers(m_display, m_window);
	}
	unBlockPaint();
	//glDraw();
}
CHARTS3D_POINTD Opengl3DCharts::GetMousePoint()
{
	int root_x=0, root_y=0, w_x=0, w_y=0;
	unsigned int mask;
	Window root,window;
	int ret=XQueryPointer(m_display, m_window,
								&root, &window,
								&root_x, &root_y, &w_x, &w_y, &mask); //<--four
	if(ret)
		return CHARTS3D_POINTD(w_x,w_y);
	return CHARTS3D_POINTD(0,0);
}
bool Opengl3DCharts::CreateAndPosition(void *parent,  int x, int y,int width, int height, int popup, int )
{
	m_rootwd=(Window)parent;
	m_xposition=x;
	m_yposition=y;
	m_width=width;
	m_height=height;
	m_popup=popup;
	pthread_create(&m_windowthread,NULL,this->ThreadXWindow,this);
	return true;
}
void Opengl3DCharts::KeyPressEvent(XKeyEvent xkey)
{
	//int mod= xkey.state & ShiftMask;
	int modifiers=xkey.state & ShiftMask  ? SHIFT_MODDIFIER :0;
	modifiers+= xkey.state & ControlMask ? CONTROL_MODDIFIER : 0 ;
	switch (xkey.keycode)
	{
	case 113:
		OnKeyPress(k_left3D,modifiers);
		break;
	case 114:
		OnKeyPress(k_right3D,modifiers);
		break;
	case 116:
		OnKeyPress(k_down3D,modifiers);
		break;
	case 111:
		OnKeyPress(k_up3D,modifiers);
		break;
	}
	//pthis->AutoZoom();
	RePaint();
	//printf("keypress\n");
}

void Opengl3DCharts::MouseEvent(XButtonEvent xbutton, butevent type)
{
	unsigned int modifiers=0;//ShiftMask ControlMask
	static struct timeval   clockdoublelick;
	struct timeval clockdoublelickaux;
	double diff;

	if(xbutton.state & ShiftMask)
		modifiers+=SHIFT_MODDIFIER;

	if(xbutton.state & ControlMask)
		modifiers+=CONTROL_MODDIFIER;

	switch (type)
	{
		case  PRESSED:
			switch (xbutton.button)
			{
			case 1:
        //time from last press double click
        gettimeofday(&clockdoublelickaux, NULL);
        diff=(clockdoublelickaux.tv_sec-clockdoublelick.tv_sec)*1000+
            (clockdoublelickaux.tv_usec-clockdoublelick.tv_usec)/1000;
        if(diff < DOUBLE_CLICK_TIME_MS)
					OnDoubleClic(CHARTS3D_POINTD(xbutton.x,xbutton.y),modifiers);
        else
					OnLMousePress(CHARTS3D_POINTD(xbutton.x,xbutton.y),modifiers);

        clockdoublelick=clockdoublelickaux;
				break;
			case 3:
				OnRMousePress(CHARTS3D_POINTD(xbutton.x,xbutton.y),modifiers);
				break;
			case 2:
				XDefineCursor(m_display,m_window , m_movecursor);
				OnMMousePress(CHARTS3D_POINTD(xbutton.x,xbutton.y),modifiers);
				break;
			case 4:
				OnMouseWheel(CHARTS3D_POINTD(xbutton.x,xbutton.y),modifiers,1);
				break;
			case 5:
				OnMouseWheel(CHARTS3D_POINTD(xbutton.x,xbutton.y),modifiers,-1);
				break;
			}
			break;
		case RELEASED:
			switch (xbutton.button)
			{
				case 1:
					OnLMouseRelease(CHARTS3D_POINTD(xbutton.x,xbutton.y),modifiers);
				 break;
				case 3:
					OnRMouseRelease(CHARTS3D_POINTD(xbutton.x,xbutton.y),modifiers);
					break;
				case 2:
					OnMMouseRelease(CHARTS3D_POINTD(xbutton.x,xbutton.y),modifiers);
					XUndefineCursor(m_display, m_window);
				break;
			}
			break;
		default:
			break;

	}
}

void Opengl3DCharts::MouseMoveEvent(XMotionEvent xbuttonmask)
{
	int flags=0;

	if(xbuttonmask.state & ShiftMask)
		flags+= SHIFT_MODDIFIER;

	if(xbuttonmask.state & Button1Mask)
	{
		flags+=MOUSE_LEFT_BUTTON;
	}

	if(xbuttonmask.state & Button2Mask)
	{
		flags+=MOUSE_MID_BUTTON;
	}

	OnMouseMove(CHARTS3D_POINTD(xbuttonmask.x,xbuttonmask.y),flags);
}

void Opengl3DCharts::SetWindowSize(int width,int height)
{
	if(m_display && m_window)
		XResizeWindow(m_display,m_window,width,height);
}
void Opengl3DCharts::SetWindowPosition(int x,int y)
{
	if(m_display && m_window)
		XMoveWindow(m_display,m_window,x,y);
}

//
//THREADS
//
void* Opengl3DCharts::ThreadXWindow(void* param)
{

	Opengl3DCharts* pthis=(Opengl3DCharts*)param;
	char name[40];
	sprintf(name,"Xogl_th%d",pthis->m_currgraph);
	pthread_setname_np(pthis->m_windowthread,name);
	pthis->InitXdisplay();
	pthis->CreateWindow();
	if(pthis->m_movecursor==0)
		pthis->CreateCursor();
	Atom  wm_protocols=0;
	Atom  wm_delete_window;
	if(pthis->m_display!=0 && pthis->m_window!=0)
	{
		wm_protocols = XInternAtom(pthis->m_display, "WM_PROTOCOLS", False);
		wm_delete_window = XInternAtom(pthis->m_display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(pthis->m_display, pthis->m_window, &wm_delete_window, 1);
	}
	XEvent xev;
	long mask;
	while(1)
	{
		XNextEvent(pthis->m_display, &xev);
		mask=PointerMotionMask;
		if(xev.type==MotionNotify)//grab last motion event
			while(XCheckWindowEvent(pthis->m_display,pthis->m_window ,mask,&xev));
		switch(xev.type)
		{
			case  Expose:
				pthis->RePaint();
			break;
			case  ConfigureNotify://ResizeRequest:
				pthis->OnResizeEvent(xev.xconfigure.width, xev.xconfigure.height);
			break;
			case KeyPress:
				pthis->KeyPressEvent(xev.xkey);
			break;
			case ButtonPress:
				pthis->MouseEvent(xev.xbutton,PRESSED);
			break;
			case ButtonRelease:
				pthis->MouseEvent(xev.xbutton,RELEASED);
			break;
			case MotionNotify:
				pthis->MouseMoveEvent(xev.xmotion);
			break;
		case ClientMessage:
						if (xev.xclient.message_type == wm_protocols &&
								xev.xclient.data.l[0] == (int)wm_delete_window)  {
							//XDestroyWindow(dpy, win);
							//XCloseDisplay(dpy);
							//exit(0);
							XUnmapWindow(pthis->m_display,pthis->m_window);
						}
						break;
		}
	}
}
#endif //X_TARGET




