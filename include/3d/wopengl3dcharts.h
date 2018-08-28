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
	\brief wopengl3dcharts.h file.
	\date 05-2017
	\author Marcos Perez*/

/// @ingroup graph3d

#ifndef WOPENGL3DCHARTS_H
#define WOPENGL3DCHARTS_H



#ifdef WIN_TARGET // TODO..............


#error WINDOWS SUPORT IS NOT STARTED/TESTED/ENDED/
class WOGLWindow: public CWnd,public ChartsWindowBase,
{
///****************************************** FUNCIONES **************************************
///*******************************************************************************************
///CREACION
public:
/** \brief Constructor.
		\param pParent: Puntero a la clase padre.*/
QOGLWindow(void* pParent);
/** \brief Crea el cuadro de dialogo y lo posiciona.
		\param pParentWnd Puntero al dialogo padre (sobre el que se dibuja), si es un dialogo pertenece a el, si es el escritorio es una ventana a parte.
		\param pos Posicion y tamano del dialogo.
		\param popup Indica si es popup(ventana a parte) o child(insertado en otro dialogo).
		\return 0 si no se pudo crear el dialogo.*/
BOOL CreateAndPosition(  void* pParentWnd,CRect pos,int popup);
/** \brief Crea la tooltip que muestra el punto cercano al mouse.*/
void CreaTooltipW();
/** \brief Redibuja todo.*/
void Draw();
/** \brief Crea elmenu contextual del boton secundario.
		\param point Punto donde se muestra.*/
void MenuContextualW(CPoint point);
/** \brief Recoge el mensaje de aceleradores.
		\param pMsg Mensaje .
		\return 1 si el mensaje es para alguna grafica 0 si no.*/
static int AceleratorsW(MSG* pMsg);
/** \brief Reenvia el mensaje de zoom por scrool a la grafica correspondiente.
		\param nFlags Parametro nFlags del mensaje OnMouseWheel().
		\param zDelta Parametro zDelta del mensaje OnMouseWheel().
		\param pt Parametro pt del mensaje OnMouseWheel ().
		\return 1 si se reenvia el mensaje 0 si no.*/
static int ZoomWheel(UINT nFlags, short zDelta, CPoint pt);
/** \brief Para modificar la visibilidad de las graficas.
		\param parent Define el padre del dialogo .
		\param visible Si se debe ver o no .	*/
static void VisibilidadW(HWND parent,int visible);
/** \brief Crea una ventana HWND con los parametros.
		\param messages Manejador de mensajes.
		\param dwExStyle Estilos extendidos.
		\param lpClassName Nombre de la clase de la ventana.
		\param lpWindowName Titulo de la ventana.
		\param dwStyle Estilos de ventana.
		\param X Esquina sup izqda.
		\param Y Esquina sup izda.
		\param nWidth Ancho.
		\param nHeight alto.
		\param lpParam lParam.
		\param parent Padre.
		\return 1 si se reenvia el mensaje 0 si no.*/
static HWND VentanaAux(MSG *messages,DWORD dwExStyle,char* lpClassName,char* lpWindowName,
												 DWORD dwStyle,int X,int Y,int nWidth,int nHeight,HWND parent,LPVOID lpParam=NULL);
/** \brief Mensajes de dialogo de visibilidad.*/
static	LRESULT CALLBACK WindowProcedure2D(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
///guardar
/** \brief Crea el dialogo de guardar como del sistema.
		\return 1 si se guardo la grafica, 0 si se cancelo.*/
protected:
//{{AFX_MSG
DECLARE_MESSAGE_MAP()
static UINT_PTR CALLBACK OFNHookProcOldStyle(HWND hdlg, UINT uiMsg,WPARAM wParam, LPARAM lParam);
/** \brief Cerrar dialogo y liberar memoria.*/
afx_msg void OnClose();
/** \brief Calcular tamano, posicion de zoom total y tamanos de letra.*/
afx_msg void OnSizing(UINT fwSide, LPRECT pRect);

static LRESULT CALLBACK WindowProc(HWND hwnd,UINT Mensaje,WPARAM wparam,LPARAM lparam);
/** \brief Boton secundario, craear menu contextual.*/
afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
/** \brief Zoom en los ejes y movimiento de la grafica.*/
afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
/** \brief Fin de zoom en los ejes y fin de movimiento de la grafica.*/
afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
/** \brief Boton primario: distancias,si SHIFT zoom ventana y si CONTROL fijar dialogo.*/
afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
/** \brief Boton primario: fin distancias,si SHIFT fin zoom ventana .*/
afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
/** \brief Boton central: si CONTROL mostrar tooltip con medida y si hay zoom activo redibujar .*/
afx_msg void OnMouseMove(UINT nFlags, CPoint point);
/** \brief Doble clic en boton primario: muestra un punto con los ejes de referencia.*/
afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
/** \brief Ventana maximizada, llama a OnSizing().*/
afx_msg void OnSize(UINT nType, int cx, int cy);
/** \brief Lama a Draw().*/
afx_msg void OnPaint();
/** \brief Eventos en barra del sistema, cambia opcion de siempre visible.*/
afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
public:
/** \brief Eventos del scrool, hace zooms sobre la grafica o los ejes.*/
afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	/** \brief Funcion llamada cuando se presiona la tecla  izquierda*/
afx_msg void OnLeftKey();
/** \brief Funcion llamada cuando se presiona la tecla  derecha*/
afx_msg void OnRightKey();
/** \brief Funcion llamada cuando se presiona la tecla  arriba*/
afx_msg void OnUpKey();
/** \brief Funcion llamada cuando se presiona la tecla  abajo*/
afx_msg void OnDownKey();
/** \brief Funcion llamada cuando se presiona CTRL+F1*/
afx_msg void OnVisualizar();
/** \brief Para adquirir aceleradores de teclado*/
virtual BOOL PreTranslateMessage(MSG* pMsg);
/** \brief Para recalcular graficas si se modifica el zoom a mano.*/
LRESULT Recalcular(WPARAM wParam,LPARAM lParam);
/** \brief Abrir una grafica arrastrada a la ventana.*/
afx_msg void OnDropFiles(HDROP hDropInfo);
//}}AFX_MSG
///****************************************** VARIABLES **************************************
///*******************************************************************************************
private:
CWnd *m_pparent;					//!< Puntero a clase padre.
static int m_id_dialogos;	//!< Variable estatica con el siguiente identificador de los dialogos.
HWND m_hwnd_tooltip;			//!< Ventana de la tooltip.
ACCEL m_teclado[8];               //!< Tabla de eventos de teclado.
CMenu m_menucontextual;           //!< Menu contextual del boton secundario.
static HACCEL m_hacelerator;			//!< Puntero a aceleradores.
static m_Pdialogs[50];            //!< For sending key events to every window.
///VARIABLES DE OPENGL
HGLRC m_hrc;							//!< Puntero a dispositivo de renderizado;
HDC m_hdc;								//!< Puntero a dispositivo de ventana;

#endif // WOPENGL3DCHARTS_H

#endif
