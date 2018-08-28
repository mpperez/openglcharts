/* (C) Copyright 2017 mpperez
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
	\brief wopengl3dcharts.cpp file.
	\date 05-2017
	\author Marcos Perez*/
#include "wopengl3dcharts.h"
#ifdef WIN_TARGET // TODO..............

#include "stdafx.h"
#include <windowsx.h>
#include <shlwapi.h>
#if (WINVER <0x500)
#error WINVER DEBE SER MAYOR O IGUAL A 0X500.		//para visibilidad
#endif


	#include <afxmt.h>
	#include <Winuser.h>
	#include "glut.h"
	#include <afxmt.h>


HACCEL QOGLWindow::m_hacelerator;
int COGLGraph::m_id_dialogos=5000;//!< Identificador del siguiente dialogo ES ACCESIBLE DESDE TODAS LAS INSTANCIAS
CWnd *QOGLWindow::m_Pdialogs[50];
HWND COGLGraph::m_hwndvisibilidad=NULL;		//!< Ventana de visibilidad.

QOGLWindow::QOGLWindow( void*pParent)
{
 InicializacionComun();
 m_pparent=(CWnd*)pParent;
 if(QOGLWindow::m_ndialogs==0)// inicializacion de los miembros static
 {
	 memset(QOGLWindow::m_pgraficas,0,sizeof(QOGLWindow::m_pgraficas));//inicializamos el vector de graficas
	 memset(m_teclado,0,sizeof(m_teclado));
	 m_teclado[0].fVirt=FVIRTKEY;m_teclado[0].key=VK_LEFT;m_teclado[0].cmd=ID_LEFT_KEY;
	 m_teclado[1].fVirt=FVIRTKEY;m_teclado[1].key=VK_RIGHT;m_teclado[1].cmd=ID_RIGHT_KEY;
	 m_teclado[2].fVirt=FVIRTKEY;m_teclado[2].key=VK_UP;m_teclado[2].cmd=ID_UP_KEY;
	 m_teclado[3].fVirt=FVIRTKEY;m_teclado[3].key=VK_DOWN;m_teclado[3].cmd=ID_DOWN_KEY;
	 m_teclado[4].fVirt=FCONTROL|FVIRTKEY;m_teclado[4].key=VK_F1;m_teclado[4].cmd=ID_VISUALIZAR;
	 m_hacelerator=CreateAcceleratorTable(m_teclado,5);
 }
 QOGLWindow::m_ndialogs++;
 COGLGraph::m_ngraficas++;
 QOGLWindow::m_Pdialogs[COGLGraph::m_ndialogs-1]=this;
}
void QOGLWindow::CreaTooltipW()
{
	DWORD flagext,flags;
	TOOLINFO ti={0};
#	if (_WIN32_WINNT <0x501)
	flagext=WS_EX_TOPMOST;
	flags=TTS_NOPREFIX | TTS_ALWAYSTIP;
#	else
	flags=TTS_NOFADE|	TTS_NOANIMATE|TTS_NOPREFIX | TTS_ALWAYSTIP;
	flagext=WS_EX_LAYERED|WS_EX_TOPMOST;
#	endif
	m_hwnd_tooltip= CreateWindowExA(flagext,TOOLTIPS_CLASS,NULL,flags,CW_USEDEFAULT,
									CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,m_hWnd,NULL,NULL,NULL );
	#if (_WIN32_WINNT >= 0x0501)
	::SetLayeredWindowAttributes(m_hwnd_tooltip,0x00ffffff, 255, LWA_COLORKEY | LWA_ALPHA);
	int err=SetWindowLongA(m_hwnd_tooltip, GWL_EXSTYLE,GetWindowLong(m_hwnd_tooltip, GWL_EXSTYLE) | WS_EX_LAYERED);
	ti.cbSize = TTTOOLINFOA_V1_SIZE;
#else
	ti.cbSize = sizeof(TOOLINFO);
#endif	ti.uFlags = TTF_TRACK;
	ti.hwnd = NULL;
	ti.hinst =::AfxGetApp()->m_hInstance;
	ti.uId = 0;	ti.lpszText = 0;ti.rect.left = 0;	ti.rect.top = 0;	ti.rect.right = 0;	ti.rect.bottom = 0;
	// SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW
	::SendMessage(m_hwnd_tooltip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
}
QOGLWindow::~QOGLWindow()
{
	if (m_hacelerator!=NULL&&QOGLWindow::m_ngraficas==0)
	{
		DestroyAcceleratorTable(m_hacelerator);
		m_hacelerator=NULL;
	}
	DestroyWindow();
}
BOOL QOGLWindow::CreateAndPosition(   void* pParentWnd,CRect pos,int popup)
{
	BOOL creado;
	WNDCLASS  m_clase_ventana;
	m_clase_ventana.style          = CS_VREDRAW|CS_DBLCLKS;
	m_clase_ventana.cbClsExtra     = 0;
	m_clase_ventana.cbWndExtra     = 0;
	m_clase_ventana.hInstance      = ::AfxGetApp()->m_hInstance;
	m_clase_ventana.hIcon          = NULL;
	m_clase_ventana.hCursor        = LoadCursor(NULL,IDC_ARROW);
	m_clase_ventana.hbrBackground  = (HBRUSH)COLOR_BTNSHADOW  ;
	m_clase_ventana.lpszMenuName   = NULL;
	m_clase_ventana.lpszClassName  = "GRAFICA_OPenGL";
	m_clase_ventana.lpfnWndProc    = WindowProc;
	int a;
	m_id_actual=m_id_dialogos;
	m_id_dialogos++;
	// registramos nuestra clase ventana con windows
	if(!RegisterClass(&m_clase_ventana))
	{
		a=	GetLastError();
	}
	creado=CreateEx(WS_EX_ACCEPTFILES ,"GRAFICA_OPenGL","GRAFICA",WS_CLIPSIBLINGS|WS_MINIMIZEBOX|WS_SYSMENU|WS_MAXIMIZEBOX|WS_CHILD|WS_CAPTION|WS_SIZEBOX|WS_VISIBLE,pos.left,pos.top,
			pos.left-pos.right,pos.bottom-pos.top,((CWnd*)pParentWnd)->m_hWnd,NULL);
	m_ancho=pos.right-pos.left;
	m_alto=pos.bottom-pos.top;
	if(creado)
	{
		Dialogodouble(popup);
		m_id_actual=COGLGraph::m_id_dialogos;
		COGLGraph::m_id_dialogos++;
		char valor[60];
		SetWindowPos(NULL,pos.left,pos.top,m_ancho,m_alto,SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
		sprintf(valor,"%3.4f",m_zoomx[0]);
		m_hwndeditzoommanual[0]=::CreateWindowA("EDIT",valor,WS_CHILD |WS_BORDER|ES_AUTOHSCROLL ,0,m_alto-20,60,20 ,m_hWnd,(HMENU)(6000),::AfxGetApp()->m_hInstance,NULL);
		sprintf(valor,"%3.4f",m_zoomx[1]);
		m_hwndeditzoommanual[1]=::CreateWindowA("EDIT",valor,WS_CHILD |WS_BORDER|ES_AUTOHSCROLL,m_ancho-60,m_alto-20,60,20 ,m_hWnd,(HMENU)(6001),::AfxGetApp()->m_hInstance,NULL);
		sprintf(valor,"%3.4f",m_zoomy[0]);
		m_hwndeditzoommanual[2]=::CreateWindowA("EDIT",valor,WS_CHILD |WS_BORDER|ES_AUTOHSCROLL,0,m_alto-40,60 ,20,m_hWnd,(HMENU)(6002),::AfxGetApp()->m_hInstance,NULL);
		sprintf(valor,"%3.4f",m_zoomy[1]);
		m_hwndeditzoommanual[3]=::CreateWindowA("EDIT",valor,WS_CHILD |WS_BORDER|ES_AUTOHSCROLL,0,0, 60,20,m_hWnd,(HMENU)(6003),::AfxGetApp()->m_hInstance,NULL);
	}
	else
	{
		ASSERT(0);
		return 0;
	}
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
			m_siempreencima=0;
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING|(m_siempreencima?MF_CHECKED:0x0) , 0x555, "Siempre encima");
	}
	CDC* cdc=GetDC();
	m_hdc=cdc->GetSafeHdc();
	GLuint l_PixelFormat = 0;
	BYTE nbits=32;
	static PIXELFORMATDESCRIPTOR l_Pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1,
				PFD_DRAW_TO_WINDOW + PFD_SUPPORT_OPENGL + PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA, nbits, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 16, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0};
	if(!(l_PixelFormat = ChoosePixelFormat(m_hdc, &l_Pfd)))
	{
		QString tx;
		tx.Format("No matching pixel format descriptor(Error OPenGL). \nArchivo:%s Linea:%d",__FILE__,__LINE__);
		throw std::runtime_error(tx.GetBuffer(0));
	}
	if(!SetPixelFormat(m_hdc, l_PixelFormat, &l_Pfd))
	{
		QString tx;
		tx.Format("Can't set the pixel format(Error OPenGL). \nArchivo:%s Linea:%d",__FILE__,__LINE__);
		throw std::runtime_error(tx.GetBuffer(0));
	}
	m_hrc=wglCreateContext(m_hdc);
	wglMakeCurrent(m_hdc, m_hrc);
	glClearDepth(1.0f);
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_TEXTURE_2D );
	glLineWidth( 2.0f );
	glPointSize( 1.0f );
	wglMakeCurrent(NULL,NULL);
	ReleaseDC(cdc);
	return creado;
}
void QOGLWindow::Draw()
{

	WINDOWINFO info;
	::GetWindowInfo(this->m_hWnd,&info);
	if(IsIconic()||(m_pparent->IsIconic()&&m_child==1)||!(info.dwStyle&WS_VISIBLE))
		return;
	CDC *cdc=GetDC();
	m_hdc=cdc->GetSafeHdc();
	int err=wglMakeCurrent(m_hdc, m_hrc);
	if(!err)
		return;
	Redibujar();
	SwapBuffers(m_hdc );
	ReleaseDC(cdc);
	wglMakeCurrent(NULL,NULL);
}
void QOGLWindow::MenuContextualW(CPoint point)
{
	ContextMenu();
}
int QOGLWindow::AceleratorsW(MSG* pMsg)
{
	if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
	{
		CRect rc;
		HWND hwn;
		for(int r=0;r<m_ngraficas;r++)
		{
			::GetWindowRect(m_pgraficas[r]->m_hWnd,&rc);//miramos si el raton esta sobre alguna ventana de grafica
			hwn=::WindowFromPoint(pMsg->pt);
			if(rc.PtInRect(pMsg->pt)&& hwn==m_Pdialogs[r]->m_hWnd)
			{
				int is=::TranslateAcceleratorA(m_Pdialogs[r]->m_hWnd, m_hacelerator, pMsg);
				is=GetLastError();
				if (m_hacelerator &&is )
					return TRUE;

			}
		}
	}
	return false;
}

void QOGLWindow::RePaint()
{
	if(this->IsIconic()==0)
		Draw();
}
POINT QOGLWindow::GetMousePoint()
{
	POINT pt;
	CPoint ptw;
	GetCursorPos(&ptw);
	ScreenToClient(&ptw);
	pt.x=ptw.x;
	pt.y=ptw.y;
	return pt;
}
void QOGLWindow::Dialogodouble(int popup)
{
	if(popup)
	{
		ModifyStyle(WS_CHILD,WS_VISIBLE |WS_POPUP  ,SWP_DRAWFRAME);
		SetOwner(m_pparent);
		SetParent(GetDesktopWindow());
		m_child=0;
	}
	else
	{
		SetParent(m_pparent);
		SetOwner(m_pparent);
		ModifyStyle(WS_POPUP,WS_CHILD|WS_VISIBLE   ,SWP_DRAWFRAME);
		m_child=1;
	}
}
void QOGLWindow::CambiaBordeDlg()
{

	if(this->GetStyle()&WS_DLGFRAME)
		this->ModifyStyle(WS_CAPTION|WS_SIZEBOX,WS_BORDER  );
	else
		this->ModifyStyle( 0,WS_CAPTION|WS_SIZEBOX  ,SWP_DRAWFRAME);
}
void QOGLWindow::Mostrar()
{

}
BEGIN_MESSAGE_MAP(QOGLWindow, CWnd)
ON_WM_PAINT()
ON_WM_CLOSE()
ON_WM_SIZING()
ON_WM_RBUTTONDOWN()
ON_WM_MBUTTONDOWN()
ON_WM_MBUTTONUP()
ON_WM_MOUSEWHEEL()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_SIZE()
ON_WM_SYSCOMMAND()
ON_COMMAND(ID_LEFT_KEY, OnLeftKey)
ON_COMMAND(ID_RIGHT_KEY, OnRightKey)
ON_COMMAND(ID_DOWN_KEY, OnDownKey)
ON_COMMAND(ID_UP_KEY, OnUpKey)
ON_COMMAND(ID_VISUALIZAR, OnVisualizar)
ON_WM_LBUTTONDBLCLK()
ON_MESSAGE(WM_RECALCULAR, Recalcular)
ON_WM_DROPFILES()
END_MESSAGE_MAP()
void QOGLWindow::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: Agregue aqu�� su c��digo de controlador de mensajes o llame al valor predeterminado
if(nID==0x555)
	{
		SetWindowPos(m_siempreencima?&wndBottom :&wndTopMost,0,0,0,0,SWP_NOMOVE |SWP_NOSIZE  );
		SetWindowPos(&wndTop,0,0,0,0,SWP_NOMOVE |SWP_NOSIZE  );
		m_siempreencima=!m_siempreencima;
		CMenu* pSysMenu = GetSystemMenu(FALSE);
		if (pSysMenu != NULL)
			pSysMenu->CheckMenuItem(0x555,m_siempreencima?MF_CHECKED :MF_UNCHECKED );
	}
	CWnd::OnSysCommand(nID, lParam);
}

void QOGLWindow::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Agregue aqu�� su c��digo de controlador de mensajes
	// No llama a CWnd::OnPaint() para mensajes de pintura
	RePaint();
}
void QOGLWindow::OnClose()
{
	OnCloseEvent();
}
void QOGLWindow::OnSizing(UINT fwSide, LPRECT pRect)
{
	CRect rct;
	GetClientRect(rct);
	m_ancho=rct.Width();
	m_alto=rct.Height();
	OnResizeEvent();//TODO
}
LRESULT CALLBACK QOGLWindow::WindowProc(HWND hwnd,UINT Mensaje,WPARAM wparam,LPARAM lparam)
{
	return (::DefWindowProc(hwnd,Mensaje,wparam,lparam));
}
BOOL QOGLWindow::PreTranslateMessage(MSG* pMsg)
{
	if(Acelerators(pMsg))
		return TRUE;
	return CWnd::PreTranslateMessage(pMsg);
}
afx_msg void QOGLWindow::OnLeftKey()
{
	 OnKeyPress(k_left);
}
afx_msg void QOGLWindow::OnRightKey()
{
	 OnKeyPress(k_right);
}
afx_msg void QOGLWindow::OnUpKey()
{
	 OnKeyPress(k_up);
}
afx_msg void QOGLWindow::OnDownKey()
{
	 OnKeyPress(k_down);
}
void QOGLWindow::OnLButtonDblClk(UINT nFlags, CPoint point)//ejes auxiliares
{
	POINT point(point.x(),point->pos().y());
	OnDoubleClic(point,flags);
	CWnd::OnLButtonDblClk(nFlags, point);
}
void QOGLWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
	POINT point(point.x(),point->pos().y());
	OnLMousePress(point,nFlags);
	CWnd::OnLButtonDown(nFlags, point);
}
void QOGLWindow::OnLButtonUp(UINT nFlags, CPoint point)
{
	POINT point(point.x(),point->pos().y());
	OnLMouseRelease(point,nFlags);
	keys key= nFlags& Qt::ControlModifier ? k_control:  0;
	key= key|	||  flags& Qt::ShiftModifie ? k_shift: 0;
	CWnd::OnLButtonUp( nFlags|point);
}
void QOGLWindow::OnRButtonDown(UINT nFlags, CPoint point)
{
	POINT point(point.x(),point->pos().y());
	OnRMousePress(point,nFlags);
	CWnd::OnRButtonDown(nFlags, point);
}
void QOGLWindow::OnMButtonDown(UINT nFlags, CPoint point)
{
	POINT point(point.x(),point->pos().y());
	OnRMousePress(point,nFlags);
	CWnd::OnMButtonDown(nFlags, point);
}
void QOGLWindow::OnMButtonUp(UINT nFlags, CPoint point)
{
	POINT point(point.x(),point->pos().y());
	OnMMouseRelease(point,nFlags);
	CWnd::OnMButtonUp(nFlags, point);
}
BOOL QOGLWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	POINT point(point.x(),point->pos().y())
	OnMouseWheel(point,nFlags,zDelta);
}
void QOGLWindow::OnSize(UINT nType, int cx, int cy)
{
	m_ancho=cx;
	m_alto=cy;
	OnResize();
}
void QOGLWindow::OnMouseMove(UINT nFlags, CPoint point)
{
	POINT point(point.x(),point->pos().y());
	OnMouseMoveEvent(point,nFlags);
}

void COGLGraph::MenuContextualW(CPoint point)
{
	m_menucontextual.CreatePopupMenu();
	int id=m_id_actual+100;
	int iLine;
	CXYLineGL *pt;
	ClientToScreen(&point);
	for (iLine=0, pt=m_plines;pt!=NULL;pt=pt->nextLine,iLine++)
	{
		m_menucontextual.AppendMenu(MF_STRING|(pt->visible==0 ?MF_UNCHECKED:MF_CHECKED),id+iLine,pt->lineName);
	}
	m_menucontextual.AppendMenuA(MF_SEPARATOR);
	m_menucontextual.AppendMenuA(MF_STRING,id+iLine,"Guardar");
	m_menucontextual.AppendMenuA(MF_STRING,id+iLine+1,m_child==1?"Desacoplar":"Acoplar");
	m_menucontextual.AppendMenuA(MF_STRING,id+iLine+2,"AutoZoom");
#ifdef MENU_PROCESAR
	m_menucontextual.AppendMenuA(MF_STRING,id+iLine+3,"Procesar");
#endif

	CMenu submenu;
	submenu.CreatePopupMenu();
	for (iLine=0, pt=m_plines;pt!=NULL;pt=pt->nextLine,iLine++)
		submenu.AppendMenu(MF_STRING,id+iLine+20,pt->lineName);
	MENUITEMINFO info;
	memset(&info,0,sizeof(MENUITEMINFO));
	info.cbSize=sizeof(MENUITEMINFO);
	info.fMask=MIIM_SUBMENU|MIIM_ID|MIIM_FTYPE|MIIM_STRING;
//#include "../resource.h"
	//info.hbmpItem=LoadBitmap(AfxGetResourceHandle(),MAKEINTRESOURCE(IDB_BITMAP0));
	info.hSubMenu=submenu.m_hMenu;
	info.wID=id+19;
	info.dwTypeData="Color";
	info.cch=5;
	int i=m_menucontextual.InsertMenuItemA(id+iLine+4,&info,1);

	i=GetLastError();

	m_menucontextual.SetDefaultItem(-1);
	int b=0;
		b=m_menucontextual.TrackPopupMenu(TPM_LEFTBUTTON |TPM_NONOTIFY | TPM_RETURNCMD,point.x,point.y,this,NULL);
	b-=id;
	switch(b)
	{
	case(-1):
		break;
	default:
		if (b<iLine&&b>=0)
		{
			this->GetLine(b)->visible=!this->GetLine(b)->visible;
			if(m_linea_focus==b&&GetLine(b)->visible==0)
				m_linea_focus=GetNextVisible();
			if(m_linea_focus<0&&GetLine(b)->visible!=0)
				m_linea_focus=b;
		}
		if(b==iLine)
		{
			SaveAs();
			break;
		}
		if(b==iLine+1)//acoplar
			Dialogodouble(m_child);
		if(b==iLine+2)//cambiar escalas
			AutoZoom();
		Draw();
#ifdef MENU_PROCESAR
		if(b==iLine+3)	//procesar
		{
			char tmp[150];
			GetTempPathA(150,tmp);
			QString ruta=tmp;
			char rutavisor[MAX_PATH];
			ruta+="temp.graf";
			GuardaGrafica(ruta);
#ifdef RUTA_VISOR
			sprintf(rutavisor,"%s",RUTA_VISOR);
#else
			GetModuleFileName(GetModuleHandle(NULL),rutavisor,MAX_PATH);
			PathRemoveFileSpec((LPSTR)rutavisor);
			strcat(rutavisor,"\\visor graficas.exe");
#endif
			HINSTANCE o=ShellExecute(NULL,"open",rutavisor,ruta,ruta,SW_SHOW);
		}
		if(b>=20&&	b<20+COGLGraph::m_nlines)	//cambiar color
		{
			ElegirColor(&GetLine(b-20)->color);
			Draw();
		}
		//ERROR_PATH_NOT_FOUND
#endif
	break;
	}
	m_menucontextual.DestroyMenu();
	submenu.DestroyMenu();
}
HWND COGLGraph::VentanaAux(MSG *messages,DWORD dwExStyle,char* lpClassName,char* lpWindowName,
													 DWORD dwStyle,int X,int Y,int nWidth,int nHeight,HWND parent,LPVOID lpParam)
{
	WNDCLASSEX wincl;        // Data structure for the windowclass
	// The Window structure
	wincl.hInstance = ::AfxGetApp()->m_hInstance;;
	wincl.lpszClassName =lpClassName;
	wincl.lpfnWndProc = WindowProcedure2D;     // This function is called by windows
	wincl.style = CS_DBLCLKS;                 // Catch double-clicks
	wincl.cbSize = sizeof(WNDCLASSEX);

	// Use default icon and mouse-pointer
	wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.hbrBackground=(HBRUSH)COLOR_WINDOW ;//COLOR_BTNSHADOW
	wincl.lpszMenuName = NULL; // No menu
	wincl.cbClsExtra = 0;                      // No extra bytes after the window class
	wincl.cbWndExtra = 0;                      // structure or the window instance
	//wincl.hbrBackground = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
	RegisterClassEx(&wincl);
	HWND ventana = CreateWindowExA(0, lpClassName, lpWindowName,dwStyle,X,Y,//WS_EX_TOPMOST
		nWidth, nHeight, NULL,NULL,::AfxGetApp()->m_hInstance,NULL);
	::ShowWindow(ventana, SW_SHOW);
	return ventana;
}
LRESULT COGLGraph::Recalcular(WPARAM wParam,LPARAM lParam)
{
	CalculaOrig();
	CalculaLeyenda();
	Draw();
	return 1;
}
void COGLGraph::VisibilidadW(HWND parent,int visible)
{
	if(m_hwndvisibilidad !=NULL)
		return;
	MSG messages;
	m_hwndvisibilidad=VentanaAux(&messages,WS_EX_TOPMOST, "Visibilidad2D", "Visibilidad 2D",
		WS_CLIPSIBLINGS|WS_SYSMENU|WS_SIZEBOX,CW_USEDEFAULT, CW_USEDEFAULT,
		110, m_ngraficas*20+40,parent) ;
	char caption[80];
	int maxwith=0;
	for(int r=0;r<m_ngraficas;r++)
	{
		m_pgraficas[r]->GetWindowText(caption,80);
		m_hwndButtonvisibilidad[r]=::CreateWindowA("BUTTON",caption,WS_CHILD | WS_VISIBLE|BS_AUTOCHECKBOX,10,
			5+r*20,9+ 9.5f*(int)strlen(caption), 16, m_hwndvisibilidad,(HMENU)(8000+r),::AfxGetApp()->m_hInstance,NULL);
		maxwith=(int)maxwith>9+ 9.5f*(int)strlen(caption) ? maxwith :9+ 9.5f*(int)strlen(caption);
		::SetWindowPos(m_hwndvisibilidad,NULL,0,0,maxwith+16,m_ngraficas*20+40,SWP_NOMOVE);
		WINDOWINFO info;
		::GetWindowInfo(m_pgraficas[r]->m_hWnd,&info);
		if(info.dwStyle&WS_VISIBLE)
			Button_SetCheck(m_hwndButtonvisibilidad[r],1);
	}
	while(GetMessage(&messages, NULL, 0, 0))
	{
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}
	m_hwndvisibilidad =NULL;
	return ;
}
void COGLGraph::OnVisualizar()//ejes auxiliares
{
	Visibilidad(this->m_hWnd,0);
}
////
///   SAVING IMAGES,GRAFS,DAT....
///
bool COGLGraph::SaveAs()
{
#ifndef OLDSTYLESAVEAS
	char lpszDefExt[MAX_PATH] ;
	strcpy(lpszDefExt,"");
	char lpszFileName[MAX_PATH] ;
	strcpy(lpszFileName,"");
	char lpszFilter[MAX_PATH]="Grafica(*.graf)|*.graf|bmp files (*.bmp)|*.bmp|Dat (*.dat)|*.dat|" ;
	CFileDialog archivo(FALSE,NULL,NULL,0x10000000 | OFN_OVERWRITEPROMPT,lpszFilter,this);

	OPENFILENAME &of=archivo.m_ofn;
	of.lStructSize      = sizeof (OPENFILENAME);
	if(archivo.DoModal()==IDCANCEL)
			return 0;
	QString direccion=archivo.GetPathName();
	direccion.Truncate((of.nFileExtension-1)>0 ?(of.nFileExtension-1): direccion.GetLength() );
#else
	TCHAR dirfin[MAX_PATH] = TEXT("");
	TCHAR dirini[MAX_PATH] = TEXT("C:\\");
	OPENFILENAME of = { sizeof(OPENFILENAME), this->m_hWnd, NULL,

	TEXT("graf files\0*.graf\0bmp files\0*.bmp\0dat files\0*.dat\0\0"), NULL,
	0, 1, dirfin, MAX_PATH, NULL, 0, dirini ,
	TEXT("Exportar gr��fica"),
	OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST |  OFN_ENABLEHOOK,
	0, 0, TEXT(".graf"), 0, NULL, NULL };
	of.lpfnHook=OFNHookProcOldStyle;
	if( TRUE != GetSaveFileName( &of) )
		return 0;
	QString direccion=dirfin;
if(direccion.Find("..")>=0)//a veces devuelve 2 puntos en la extension???
		direccion.Truncate(direccion.Find(".."));
	else
		direccion.Truncate(of.nFileExtension-1);
#endif
	switch(of.nFilterIndex)//graf
	{
	case 1:
			direccion+=".graf";
		GuardaGrafica(direccion);
		break;
	case 2:
		direccion+=".bmp";
		GuardaImagen(direccion);
		break;
	case 3:
		direccion+=".dat";
		SaveDat(direccion);
		break;
	}
	return 1;
}
void COGLGraph::GuardaImagen( string direccion)
{

	int sel=0;CDC * cdc;
	if (wglGetCurrentContext() ==NULL)
	{
		cdc=GetDC();
		m_hdc=cdc->GetSafeHdc();
		int err=wglMakeCurrent(m_hdc, m_hrc);
		sel=1;
	}
	int ancho=m_ancho;//graphrect.right-graphrect.left;
	int alto=m_alto;//graphrect.bottom-graphrect.top;
	char *raw=new  char [ancho*alto*4*sizeof(char)];
	glFlush(); glFinish();
	glReadPixels(0,0,ancho,alto,GL_BGRA_EXT,GL_UNSIGNED_BYTE,raw);
	SaveBitmapToFile( (GLubyte*)raw, ancho,alto,4*8,direccion );
	delete []raw;
	if(sel)
	{
		wglMakeCurrent(NULL,NULL);
	ReleaseDC(cdc);
	}
}
void COGLGraph::SaveBitmapToFile( GLubyte* pBitmapBits, long lWidth, long lHeight,unsigned int wBitsPerPixel, string lpszFileName )
{//SaveBitmapToFile( GLubyte* pBitmapBits, LONG lWidth, LONG lHeight,WORD wBitsPerPixel, LPCTSTR lpszFileName )
	BITMAPINFOHEADER bmpInfoHeader = {0};
	// Set the size
	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	// Bit count
	bmpInfoHeader.biBitCount = wBitsPerPixel;
	// Use all colors
	bmpInfoHeader.biClrImportant = 0;
	// Use as many colors according to bits per pixel
	bmpInfoHeader.biClrUsed = 0;
	// Store as un Compressed
	bmpInfoHeader.biCompression = BI_RGB;
	// Set the height in pixels
	bmpInfoHeader.biHeight = lHeight;
	// Width of the Image in pixels
	bmpInfoHeader.biWidth = lWidth;
	// Default number of planes
	bmpInfoHeader.biPlanes = 1;
	// Calculate the image size in bytes
	bmpInfoHeader.biSizeImage = lWidth* lHeight * (wBitsPerPixel/8);

	BITMAPFILEHEADER bfh = {0};
	// This value should be values of BM letters i.e 0��4D42
	// 0��4D = M 0��42 = B storing in reverse order to match with endian
	bfh.bfType=0x4D42;
	/// or    bfh.bfType = ��B��+(��M�� << 8);
	// <<8 used to shift ��M�� to end
	// Offset to the RGBQUAD
	bfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
	// Total size of image including size of headers
	bfh.bfSize = bfh.bfOffBits + bmpInfoHeader.biSizeImage;
	// Create the file in disk to write
	HANDLE hFile = CreateFile( lpszFileName,GENERIC_WRITE, 0,NULL,
														 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
	if( !hFile ) // return if error opening file
	{
			return;
	}
	DWORD dwWritten = 0;
	// Write the File header
	WriteFile( hFile, &bfh, sizeof(bfh), &dwWritten , NULL );
	// Write the bitmap info header
	WriteFile( hFile, &bmpInfoHeader, sizeof(bmpInfoHeader), &dwWritten, NULL );
	// Write the RGB Data
	WriteFile( hFile, pBitmapBits, bmpInfoHeader.biSizeImage, &dwWritten, NULL );
	// Close the file handle
	CloseHandle( hFile );
}




#endif
