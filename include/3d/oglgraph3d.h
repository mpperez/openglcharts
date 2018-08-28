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
	\brief Chart renderer.
	\date 05-2015
	\author Marcos Perez*/

/// @defgroup graph3d

/// @ingroup graph3d

/** \mainpage
3DChart implementation using OPengl and glut.\n
Class COGLGraph:\n
Renderer, creates a 3d chart with axis legend and tittles using OpenGL.\n
Class CXYLineGL:\n
Has the information about a line insithe the chart. One instance per line.\n

Class QOGLWindow:\n
Creates a window using Qt and containig the 3D chart inside it. Implements all the mouse/keyboard interaction using QT.

Class XOGLWindow:\n
Creates a window using X lib and containig the 3D chart inside it. Implements all the mouse/keyboard interaction using X interface.

Class WOGLWindow(TODO):\n
Creates a window using Windows MFC and containig the 3D chart inside it. Implements all the mouse/keyboard interaction using windows interface.


\b Creation for Qt interface: \n
Inside yourapp.h add:
\code
#include "Qglwindow.h"

//Add our chart dialog.
//QOGLWindow -> for QT interface
//XOGLWindow -> for X interface
//WOGLWindow -> for MFC interface

//Qt for this example.
QOGLWindow* m_oglwindow;

\endcode
And into our yourapp.cpp
\code

//Add glut initialization (one time only) into the constructor for example or main.
glutInit( &__argc, __argv );


	//We create a chart window.
	m_oglwindow=new QOGLWindow(NULL);

	//Add a window tittle.
	m_oglwindow->setWindowTitle("graphs");

	//Add a chart and axis tittles.
	m_oglwindow->SetGraphTittle("grsdfasdf","ejex","ejey");

	//Create the chart, define size popup and redraw time.
	m_oglwindow->CreateAndPosition(this,QRect(1000,0,500,500),1,50);

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

//Dont forget to delete.
//delete m_oglwindow;
\endcode

\b USE: \n

Once the chart is created we can update the data with:
\code
int valuesnumber = 32;//less or equal than line points
double newXvalues[valuesnumber];
double newYvalues[valuesnumber];
//fill values
m_oglwindow->UpdateLineValues(linenumber,newXvalues,newYvalues,valuesnumber)


Al dar a guardar se guarda un bitmap con la grafica actual o la grafica tal cual para poder abrirla con un visor.
Para fijar la grafica al dialogo padre hacer Ctrl+ boton primario raton.\n

\b ZOOMS: \n
Some zoom mode are impolemented: \n
Window: using mouse primary button and SHIFT  a zoom window could be selected.\n
Mouse weel: using mouse wheel the zoom is automated done inside and outside, if \n
is selected in one axis the zoom only is done over this axis.\n
Middle button: the visible window could be displaced using the middle button, and arrow keys.\n

\b MEASURE: \n
It is posible to measure by double click and moving.\n

\b SAVE \n
Current chart could be saved using secondary button and selcting save. Three different extensions could be selected:
.bmp : a image of the actual chart is saved.
.dat : current data are saved as .dat file.
.graf : the chart is saved using a defined file format.

At the begining of the file OGLGraph.h there are some defines with the aspect of the chart.
*/

#include "fpvcamera.h"
#include "charts3dwindowsbase.h"

using namespace std;
#include <semaphore.h>//API must be compiled with cc -pthread to link against the real-time library, librt.
#include <time.h>
#include <list>
#include <vector>
#include <stdio.h>
#include <map>
#include <string>
#include "xylinegl3d.h"
#include "oglbutton.h"

#include <GL/glut.h>

#define COLORMAP3D 	//!< INCLUYE CODIGO DE QUE PERMITE APLICAR EL COLOR DE UNA LISTA DE COLORES DE MATLAB

#include "colormapgl3d.h"

#define PUNTOS	GL_POINT									//!<	RENDERIZADO POR PUNTOS
#define LINEAS	GL_LINE										//!<	RENDERIZADO POR LINEAS
#define POLIGONOS	GL_FILL									//!<	RENDERIZADO POR POLIGONOS COLOREADOS


#ifndef	OPENGL3DCHARTS
#define OPENGL3DCHARTS
///////////////////_______CONFIGURACION____/////////////////////////////

#define TIPO_LETRA GLUT_BITMAP_HELVETICA_18 /*!<	FONT TYPE, AVAILABLE: \n(GLUT_BITMAP_8_BY_13, GLUT_BITMAP_9_BY_15 , GLUT_BITMAP_TIMES_ROMAN_10,GLUT_BITMAP_TIMES_ROMAN_24
																						GLUT_BITMAP_HELVETICA_10, GLUT_BITMAP_HELVETICA_12, GLUT_BITMAP_HELVETICA_18 )*/

#define OGL3D_FONT_WIDTH 10									//!<	FONT WIDTH
#define OGL3D_FONT_HEIGHT 18								//!<	FONT HEIGHT
#define OGL3D_TITTLE_HEIGHT OGL3D_FONT_HEIGHT*1.6f//!<  FREE SPACE OVER THE CHART USED BY TITTLE
#define OGL3D_LEFT_MARGIN 8									//!<	LEFT MARGIN
#define OGL3D_RIGHT_MARGIN 8								//!<	RIGHT MARGIN

///COLOR
#define OGL3D_BACKGROUND  0X000a0a0a0								//!<	BACKGROUND COLOR, GRAY
//#define FONDO  0X00fffffff									//!<	COLOR FONDO BLANCO
//#define FUENTE  0X00fffffff									//!<	COLOR FONDO BLANCO
#define OGL3D_FONTCOLOR 0X00000000									//!<	FONT COLOR BLACK
#define OGL3D_BACKGROUND_LECEND_COLOR 0X00AAAAAAA		//!<	BACKGROUND COLOR OF THE LEGEND
#define OGL3D_BACKGROUND_MAX_VALUE 0X00AAAAAAA			//!<	BACKGROUND MAX VALUE GRAY
#define OGL3D_MANUAL_MEASURE_LINES 0X0000FFFF				//!<	MANUAL MEASURE LINE COLOR YELLOW
#define OGL3D_BACKGROUND_MANUAL_MEASURE 0X00AAAAAAA	//!<	BAKCGROUND COLOR MANUAL MEASURE
#define OGL3D_AXIS_COLOR 0X00969669									//!<	AXIS COLOR
#define OGL3D_AXIS_ZOOM_COLOR 0X00000aa							//!<	AXIS ZOOM COLOR //NOT IMPLEMENTED
#define OGL3D_ZOOM_BOX_COLOR 0X00969600								//!<	ZOOM BOX COLOR
#define OGL3D_SECONDARY_POINTS_COLOR 0x00ae0284				//!<	AUXILIARY POINTS COLOR
#define OGL3D_CURRENT_POINT_AXIS_COLOR 0x00020AFF			//!<	CURRENT POSITION AXIS COLOR
#define OGL3D_BACKGROUND_CURRENT_POSITION 0X00AAAAAAA	//!<	BACKGROUND CURRENT POSITION

#define OGL3D_BACKGROUNDN  0X00000000								//!<	HIGHTCONTRAST BACKGROUND COLOR BLACK
#define OGL3D_FONTCOLORN 0X00ffffff									//!<	HIGHTCONTRAST FONT COLOR BLACK
#define OGL3D_BACKGROUND_LECEND_COLORN 0X00969696		//!<	HIGHTCONTRAST BACKGROUND COLOR OF THE LEGEND
#define OGL3D_BACKGROUND_MAX_VALUEN 0X000000				//!<	HIGHTCONTRAST BACKGROUND MAX VALUE
#define OGL3D_MANUAL_MEASURE_LINESN 0X0000FFFF			//!<	HIGHTCONTRAST MANUAL MEASURE LINE COLOR YELLOW
#define OGL3D_BACKGROUND_MANUAL_MEASUREN 0X00ffffff	//!<	HIGHTCONTRAST BAKCGROUND COLOR MANUAL MEASURE
#define OGL3D_AXIS_COLORN 0X00969669								//!<	HIGHTCONTRAST AXIS COLOR
#define OGL3D_AXIS_ZOOM_COLORN 0X00969600						//!<	HIGHTCONTRAST AXIS ZOOM COLOR //NOT IMPLEMENTED
#define OGL3D_ZOOM_BOX_COLORN 0X00969600						//!<	HIGHTCONTRAST OOM BOX COLOR
#define OGL3D_SECONDARY_POINTS_COLORN 0x00ae0284				//!<	HIGHTCONTRAST AUXILIARY POINTS COLOR
#define OGL3D_CURRENT_POINT_AXIS_COLORN 0x00020AFF			//!<	HIGHTCONTRAST CURRENT POSITION AXIS COLOR
#define OGL3D_BACKGROUND_CURRENT_POSITIONN 0X000AAAAAA	//!<	HIGHTCONTRAST BACKGROUND CURRENT POSITION

#define OGL3D_LINE_WIDTH 2.5f						//!<	LINE WIDTH 3D
#define OGL3D_BLOQUEADOR								//!<  PARA DIBUJAR COMO BLOQUEADOR DE ORDEN 0
#define OGL3D_MENU_PROCESAR							//!<  PARA GUARDAR GRAFICA Y LLAMAR A VISOR DE GRAFICAS

///////////////////_______CONFIGURACION____/////////////////////////////

#define OGL3D_XAXIS 1
#define OGL3D_YAXIS 2
#define OGL3D_RED(R) (((GLubyte*)&R)[0])		//!< GET RED COLOR FROM INT VALUE
#define OGL3D_GREEN(R) (((GLubyte *)&R)[1])	//!< GET GREEN COLOR FROM INT VALUE
#define OGL3D_BLUE(R) (((GLubyte *)&R)[2])	//!< GET BLUE COLOR FROM INT VALUE
#define OGL3D_REDC(R)  (R &0XFF)						//!< GET RED COLOR FROM CONST VALUE
#define OGL3D_GREENC(R) ((R>>8)& 0XFF)			//!< GET GREEN COLOR FROM CONST VALUE
#define OGL3D_BLUEC(R) (R>>16)							//!< GET BLUE COLOR FROM CONST VALUE

//#define OGL3D_PIXELSTOWORLD_X(R) 	((double)R-m_freeSpaceYaxis)/((double)m_width-m_freeSpaceYaxis)*(m_zoomGLx[1]-m_zoomGLx[0])+m_zoomGLx[0]									//!<	PASS FROM PIXELS TO 3D X AXIS
//#define OGL3D_PIXELSTOWORLD_Y(R)	((double)R-m_freeSpaceXaxis)/((double)(m_height-OGL3D_TITTLE_HEIGHT)-m_freeSpaceXaxis)*(m_zoomy[1]-m_zoomy[0])+m_zoomy[0]	//!<	PASS FROM PIXELS TO 3D Y AXIS

#ifndef LOG10
#define LOG10 log((double)10)
#endif

typedef enum{CIRCUNFERENCE3D=1,CIRCLE3D} enum_tipodibujos3d;

#ifndef DRAWINGS_STR
#define DRAWINGS_STR
/**
 * \brief The str_drawings struct contains information about auxiliary drawings that could be aded to a line as keypoints(circles and circunferences)
 */
struct str_drawings{
	enum_tipodibujos3d drawingType;	//!< Drawing type.
	float posx;						//!< X position.
	float posy;						//!< Y position.
	float posz;						//!< Z position visibility.
	float tamano;					//!< Draw size, depend on drawing type.
	COLORREF colorpen;				//!< Line color.
	COLORREF colorbrush;			//!< Fill color.
};

typedef struct str_drawings drawingsstr;
typedef map <string,drawingsstr> mapdrawing;
typedef map <string,drawingsstr>::iterator iterdrawing;
typedef pair <string,drawingsstr> pairdrawing;
#endif

/** \class COGLGraph
		\brief Dialogo principal de la grafica. Crea el dialogo y las ventanas OPenGL donde se pinta, cada linea es una instancia
		de la clase CXYLineGL.*/
class OGLGraph3d
{
public:
///****************************************** FUNCIONS ***************************************
///*******************************************************************************************

  /** \brief Constructor.
			\param pParent: Pointer to parent.*/
	OGLGraph3d();
	~OGLGraph3d();//!< Destructor
  /** \brief Init graph opengl
      @param altocontraste Hight contrast.
      @param ejxhora Hours in ejx.
      @return  */
	bool InitGraph( bool altocontraste=0, bool ejxhora=0);
	/** \brief Common inicialization to 0*/
	void CommonInit();
//
//CONFIGURATION
//
	/** \brief Sets chart and axis titles.
			\param titulo Tittle.
			\param ejx X axis name.
			\param ejy Y axis name.*/
	void SetGraphTitle(string tittle, string ejx="", string ejy="");
	/** \brief Set all chart colors backgrounds, legends....
			\param hightcontrast true to set hight contrast .*/
	void SetColors(int hightcontrast);
  /** \brief Width
  @return Curent width*/
  int Width(){return m_width;}
  /**  \brief Height
       @return  Current height.  */
  int Height(){return m_height;}
//
//ELEMENTS
//
	/** \brief Ads new line to chart.
			\param nPoints Number of points.
			\param name Line name.
			\param color Line color.
			\param visible Visibility 1 for visible.
			\return Total line number.*/
	int AddLine(int nPoints,int zLines,string name,COLORREF color,bool visible);
#ifdef COLORMAP3D
	/** \brief Anade una nueva linea a la grafica con colormap.
			\param nPoints Numero de puntos de la linea.
			\param zLines Numero de lineas en el eje del tiempo.
			\param name Nombre de la linea.
			\param rutamap Ruta del archivo de colores.
			\param visible Visibilidad inicial de la linea 1 visible, 0 oculta.
			\return Numero total de lineas.*/
	int AddLine(int nPoints,int zLines,string name,string rutamap,bool visible);
#endif
  /** \brief Gets number of lines.
     \return	 */
	int GetnLines();
	/** \brief Gets a line.
			\param iLine Line number.
			\return Pointer to line or NULL.*/
	CXYLineGL3D* GetLine(int iLine);
  /** \brief Gets pointer to line 0.
      @return	 */
	CXYLineGL3D* GetLines();
	/** \brief Remove lines from start to end.
			\param start First line to erase.*/
  void RemoveLines(int start);
	/** \brief Add an auxiliary drawing circle or circunference....
			\param identifier Draw identifier.
			\param drawtype Drawing ty pe circle or circunference...
			\param posx X position.
			\param posy Y position.
			\param posz Z position.
			\param size Size depends on drawing.
			\param linecolor Line color.
			\param brushcolor Brush color.*/
	void AddDraw(string identifier,enum_tipodibujos3d drawtype,float posx,float posy,float posz,float size,COLORREF linecolor,COLORREF brushcolor);
	/** \brief Remove an auxiliary drawing....
			\param identifier Drawing identifier.*/
	void DeleteDraw(string identifier);
	/** \brief Remove all drawings.*/
	void DeleteAlldraws();
  /** \brief Update the points of a line. Doesnt repaint itself!!
     @param nline Line to modify
     @param Xvalues Pointerto Xvalues Null if not necessary to upload.
     @param Yvalues Pointerto yvalues.
     @param nvalues Number of values   */
	void UpdateLineValues(int nline,double *Xvalues, double *Yvalues, int nvalues);
  /** \brief Create static buttons.	 */
	void CreateButtons();
  /** \brief Get previous cursor positon.
     \return  point.	 */
	CHARTS3D_POINTD GetPreviousCursorPosition() const;
  /** \brief Set previous cursor positon.
     \param GetPreviousCursorPosition point.	 */
	void SetPreviousCursorPosition(const CHARTS3D_POINTD &GetPreviousCursorPosition);
//
//SYNCRO
//
	/** \brief Lock data mutex for data manipulation.  */
  void LockDataMutex();
	/** \brief Unlock data mutex after data manipulation.  */
  void UnlockDataMutex();
  /** \brief Try get paint semaphore for ms milisedonds.
     \param ms Miliseconds.
     \return	 */
	int BlockPaint(int ms);
	/**
   * \brief Unget paint semaphore.
     \return	 */
	int unBlockPaint();
	/**
   * \brief Tryget paint semaphore.
     \return	 */
	int TryBlockPaint();
//
//PAINTING
//
	/** \brief Redraw the chart.
		* \return 1 if redraws 0 if not.*/
	int  RedrawChart();
	/**
   * \brief Set external drawing function, this function will be called at the end of main drawing function.
	 *   \code static int myfunc(); \code	 */
	void SetExternalDrawingFunction(int (*functionptr) ());
	/**
   * \brief Update FPV camera.	 */
	void UpdateCamera();
	/**
   * \brief Draw a box outsithe graph.	 */
	void DrawGraphBox();
	void UpdateView();
private:
	/** \brief Draw chart tittle.*/
	void DrawTittle();
	/** \brief Draw the lines legend*/
	void DibujarLeyenda();
	/**
   * \brief Draw the buttons.
	 */
	void DrawButtons();
	/** \brief Draw XY axis.*/
	void DrawAxis();
	/** \brief Draw the reference text in  x [1] e y [2].
			\param minescala First reference value.
			\param maxescala Last reference value.
			\param paso Reference step.
			\param format Gets the text output format similar to printf.*/
	void DibujaTextoReferencias(GLdouble minescala[2], GLdouble maxescala[2], GLdouble paso[2], string format[]);
	/** \brief Draw all the lines.*/
	void DrawAsLines();
	/** \brief Dibuja los poligonos de las lineas de todas las graficas.
			\param posmax Posicion del punto maximo, para interpolar color.
			\param posmin Posicion del punto minimo, para interpolar color.*/
	void DrawAsPoligons(int posmax=-1,int posmin=-1);
	/** \brief Draw auxiliary drawings*/
	void DrawDraws();
	/** \brief Draw manual measurement between two points.*/
	void DrawManualMeasurement();
	/** \brief Draw reference lines.
			\param minescala First reference value.
			\param maxescala Last reference value.
			\param paso Reference step.*/
	void DibujaReferencias(GLdouble minescala[2], GLdouble maxescala[2], GLdouble paso[2]);
	/** \brief Draw the zoom box.*/
	void DrawZoomBox();
	/** \brief Draw current mouse position.*/
	void DrawCurrentPosition();
	/** \brief Draw text in world coordinates.
       \param txt Text.
       \param posx X position .
       \param posy Y position .
       \param posz z position .
       \param fondo True if it is neccesary to draw background.
       \param colorfondo Background color.
       \param colorletra Font color.
       \param tipoletra Font type from glut.	*/
	void DrawText( string txt,const GLdouble posx,const GLdouble posy,const float posz=0,
            int fondo=0 ,COLORREF colorfondo=0,COLORREF colorletra=-1,void* tipoletra=NULL);
	/** \brief Draw text iparentn pixels coordinates.
			 \param txt Text.
			 \param posx X position .
			 \param posy Y position .
			 \param posz z position .
			 \param fondo True if it is neccesary to draw background.
			 \param colorfondo Background color.
			 \param colorletra Font color.
			 \param tipoletra Font type from glut.	*/
	void DrawTextInPixels( string txt,const GLdouble posx,const GLdouble posy,const float posz=0,
              int fondo=0 ,COLORREF colorfondo=0,COLORREF colorletra=-1,void* tipoletra=NULL);
	/** \brief Draw the max value  of the focus line.
			\param pos Max value index in focus line.*/
	void DrawMaxValue(int pos);
//
//ZOOM
//
public:
  /** \brief Sets top view. */
	void TopView();
	/** \brief Set visible window to new values.
			\param minx Min x value.
			\param maxx Max x value.
			\param miny Min y value.
			\param maxy Max y value.
			\param minz Min z value.
			\param maxz Max z value.*/
	void SetZoom(GLdouble minx, GLdouble maxx, GLdouble miny, GLdouble maxy, GLdouble minz, GLdouble maxz);
   /** \brief AutoZoomSearch for all visible points and set the zoom acording.
     \param anglex Angle over x axis.
     \param angley Angle over y axis.   */
  void AutoZoom(float anglex, float angley);
	/** \brief Search for max and min values into a buffer from initial max and min values.
			\param buf Buffer.
			\param ndat Number of data.
			\param min Min initial value and minimun return.
			\param max Max initial value and max return.			*/
	void SearchLimits(const double *buf, long ndat, double *min, double *max);
	/** \brief Y axis zoom.
			\param zDelta: Zoom direction <0 outside >0 inside.
			\param y: Zoom center point in world coordinates.	*/
	void ZoomEjeY(int zDelta, GLdouble y);
	/** \brief X axis zoom.
			\param zDelta: Zoom direction <0 outside >0 inside.
			\param x: Zoom center point in world coordinates.	*/
	void ZoomEjeX(int zDelta,GLdouble x);
	/** \brief Translate from pixel coordinate to internal world coordinate.
			\param point Point in pixels, if null uses mouse current position.
			\param x Return x in world coordinates.
			\param y Return y in world coordinates.
			\return 1 if no error, 0 if errror.*/
	int PixelsToWorld( const CHARTS3D_POINTD *point,GLdouble* x,GLdouble*y,GLdouble*z=NULL);
//
//CHART CALCULATION
//
private:
  /** \brief Calculate 3d zoom view to prevent drawing troubles.	 */
	void CalculateView();
	/** \brief Calculate legend size and position in world coordinates.*/
  void CalculaLeyenda();
	/** \brief Translate from world coordinates to pixel coordinates.
			\param x Xvalue.
			\param y y value.
			\param point Return the point in pixels coordinates.*/
	void WorldToPixels( GLdouble x,GLdouble y,CHARTS3D_POINTD *point);
	/** \brief Search for a close point in selected line.
			\param pt: Point in world coordinates.
			\return Point parsed in text.*/
	string SearchClosePoint(CHARTS3D_POINTD pt);
	/** \brief Parse a point into text.
			\param x X value.
			\param y Y value.
			\return Text.*/
	string GetTextFromPoint(double x,double y);
	/** \brief Find first visible line.
			\return Line number or -1 if no visible line.*/
	int	GetNextVisible();
  /** \brief Printf like format function over strings.
     \param format Format like printf options.
     \return String Format string or empty string.	 */
	string strprintf(const char *format...);
  /** \brief Calculate max text lenght per axis(characters).
     \param x Lengh in x axis.
     \param y Lengh in y axis.
     \param z Lengh in z axis.	 */
	void CalculateMaxLenghtText(int *x,int *y,int *z);
	/** \brief Return from long to time, uses long to store the total miliseconds of a day, return HH:MM:SS.ms.
			 \param msegundos Miliseconds from 00:00:00.
			 \return String with the hour.*/
  string PasaFecha(long msegundos);
	void RotateFromPoint(glm::vec3 point, glm::vec3 normal, double angle );
  /** \brief Calculates the references.
			\param minmaxIn Chart with limits.
			\param stepIn Init step.
			\param nTipsIn Minimun reference number.
			\param minOut Return first reference.
			\param maxOut Return last reference.
			\param stepOut Reference step.
			\param fmtOut Text output format similar to printf.*/
	void OptimunAxisScale(GLdouble minmaxIn[2], GLdouble stepIn, int nTipsIn, GLdouble* minOut, GLdouble* maxOut, GLdouble* stepOut, int *precision);
	/** \brief Search for the local(visible) maximun into the focus line or line pased by index.
			\return Max point index.*/
	int MaximoLocal(int iline, double *value=NULL);
	/** \brief Search for the local(visible) manimun into the focus line or line pased by index.
			\return Min point index.*/
	int MinimoLocal(int iline, double *value=NULL);
  /** \brief Check if the point is near a line and select it with focus(draw the points)
	 * \param x x position in real world.
	 * \param y y position in real world.	 */
	void SelectLine(double x,double y);
  /** \brief Gets maximun value of visible lines.
     \return	 */
	double GetMaximun();
  /** \brief Gets minimun value of visible lines.
     \return	 */
	double GetMinimun();
//
//   SAVING IMAGES,GRAFS,DAT....
//
public:
	/** \brief Store a chart.
			\param dir: Complete path of the file.*/
	void SaveCurrent(string dir);
	/** \brief Open a chart.
			\param dir Complete path of the file.*/
	void Open(string dir);
	/** \brief Store a chart values like .dat file [xaxis][line1][xaxis][line2][xaxis][line3]....
	\param dir Complete path of the file.*/
  void SaveDat(string dir);
  /** \brief Get current draw rectangle
       \param raw Data Buffer initialized.
       \param posx X rectangle position.
       \param posy Y rectangle position
       \param width width.
       \param heignt height.
       \param bitspixel	 */
	void GetScreenShot(GLubyte * raw,int posx,int posy,int width,int heignt,int bitspixel);
//
//   DIALOG INTERFACE
//
	void OnMouseWheel(CHARTS3D_POINTD pt,unsigned int flags,int delta);
	void OnKeyPress(chart3dkeys keypress, int modiffiers);
	void OnMouseMove(CHARTS3D_POINTD pt,unsigned int flags);
	void OnLMousePress(CHARTS3D_POINTD pt,unsigned int flags);
	void OnLMouseRelease(CHARTS3D_POINTD pt,unsigned int flags);
	void OnMMousePress(CHARTS3D_POINTD pt, unsigned int);
	void OnMMouseRelease(CHARTS3D_POINTD , unsigned int);
	void OnRMousePress(CHARTS3D_POINTD, unsigned int);
	void OnRMouseRelease(CHARTS3D_POINTD pt,unsigned int flags);
	void OnDoubleClic(CHARTS3D_POINTD pt,unsigned int flags);
  void OnResizeEvent(int width, int height);
  void OnCloseEvent ();
//
// FILLED BY WINDOW CONTAINER
//

  /**
   * \brief Get current mouse point.   */
	virtual CHARTS3D_POINTD GetMousePoint(){return CHARTS3D_POINTD();};
  /** \brief Ask for repaint.   */
  virtual void RePaint(){};
	/** \brief Creates the dialog and positioning it.
			\param Parent Parent.
			\param width  Window width.
			\param height Window height.
			\param x Window x position.
			\param y Window y position.
			\param popup Trure if popup false if inside parent.
			\param mspaint Miliseconsfor automatic repainting 0 for manual repainting.
			\return 0 if error.*/
	bool CreateAndPosition(void */*parent*/, int /*x*/, int /*y*/, int /*width*/, int /*height*/, int /*popup*/, int /*mspaint*/){return false;};

//****************************************** VARIABLES **************************************
//*******************************************************************************************

//
//DIALOG CONTROL
//
private:
	int m_width;					//!< Width in pixels.(Updated into OnSizing()).
	int m_height;					//!< Height in pixels.(Updated into OnSizing()).
	int m_child;					//!< True if child false if popup.
public:
	static vector<OGLGraph3d*>m_pcharts;   //!< List of all charts.

//
//BUTTONS
//
	OGLButton buttonAutozoom;			//!< Button autozoom.
	OGLButton buttonAutozoomTop;	//!< Button autozoom from top view.
	OGLButton checkAutofix;				//!< Check autofix.
//
//VARIABLES DE VISUALIZACION
//
private:
	unsigned int m_widthLegend;	//!< Width of the legend.
	int m_linesperline;					//!< Number of linenames per line of legend.
	int m_nlegend_lines;				//!< Number of lines of the legend.(Depends on width and number of lines of chart)
	GLdouble m_freeSpaceXaxis;	//!< Free space in pixels used by legend.
	GLdouble m_freeSpaceYaxis;	//!< Fre space in pixels used by y values
	GLdouble m_xLegendPosition;	//!< Legend x position in world coordinates.
	GLdouble m_fontWidth;				//!< Font width in world coordinates.
	GLdouble m_fontHeight;			//!< Font height in world coordinates.
public:
	string m_tittle;						//!< Chart tittle.
	string m_xasisName;					//!< X axis name.
	string m_yAxisName;					//!< Y axis name.
	int m_focusLineIndex;				//!< Focus line index.
	COLORREF m_backgroundColor;	//!< Background color.
	COLORREF m_fontColor;				//!< Font color.
	COLORREF m_backgroundcolormaxvalue;	//!< Background color of max value.
	COLORREF m_distancesColor;			//!< Manual measure line color.
	COLORREF m_backgroundDistances;	//!< Background color of manual measure.
	COLORREF m_axisColor;						//!< Axis color.
	COLORREF m_zoomAxisColor;				//!< Zoom axismode color.
	COLORREF m_backgroundLegendColor;	//!< Legend background color.
	COLORREF m_zoomBoxColor;					//!< Zoom box color.
	COLORREF m_auxPointsColor;				//!< Auxiliary points color.
	COLORREF m_currentPositionColor;	//!< Current position axis color.
	COLORREF m_backgroundCurrentValue;//!< Background color current position.
	int m_ejzhora;										//!< X axis contanins total miliseconds of day to time hour axis.
	mapdrawing m_mapdibujos;
	CHARTS3D_POINTD m_oldCursorPosition;				//!< Old position of the cursor.
protected:
	sem_t  m_paintSemaphore;  //!< Paint  semaphore.
	int m_renderizado;				//!< Opcion de dibujar poligonos, lineas o puntos.
#ifdef COLORMAP3D
	CColorMapGL3D* m_colormaps[20];//!< Puntero a las tablas de color de cada linea.
#endif
	int (*posdrawuserfnctionpointer) ();		//!< Auxiliar to draw user funcions after local drawing is made.
//
//VARIABLES PARA ZOOMS
//
private:
	bool m_posscroll;										//!< True if moving using central mouse buton.
	GLdouble m_middleBtInitialPoint[2];	//!< Middle button initial point when moving, in world coordinates.
	int m_posaxisscroll;								//!< Zoom using mouse wheel one axis.
	GLdouble m_ptoMejesinicial[2];			//!< Initial point for zoom in one axis, world coordinates.
	int m_drawManualDistance;						//!< 1 if double click, 2 if it is necesary to draw manual measure.
	GLdouble m_initialManualDistancePoint[2];	//!< Initial manual measure point in world coordinates.
	int	m_drawZoomBox;							//!< True if it is necesary to draw zooom box.
	CHARTS3D_POINTD m_initialZoomBoxPoint;		//!< Initial point for zoom box, world coordinates.
	int m_drawCurrentPos;						//!< True if it is necesary to draw current position.
	int m_lineat2d;				//!< Solo se dibuja una linea de tiempo para verla en detalle.
	GLdouble m_zoomx[2];				//!< Visibility in x axis.
	GLdouble m_zoomy[2];				//!< Visibility in y axis.
	GLdouble m_zoomz[2];			//!< Visibilidad del eje z.Puntos que se dibujan.
	GLdouble m_zoomGLx[2];		//!< Visibilidad del eje x en OPengl, teniendo en cuenta giros para ver la grafica.
	GLdouble m_zoomGLy[2];		//!< Visibilidad del eje y en OPengl, teniendo en cuenta giros para ver la grafica.
	GLdouble m_zoomGLz[2];		//!< Visibilidad del eje y en OPengl, teniendo en cuenta giros para ver la grafica.
	GLdouble m_angx;					//!< Angulo de rotacion del eje x.
	GLdouble m_angy;					//!< Angulo de rotacion del eje y.
	int m_maxReferenceLenght;	//!< Max mumber of letters in axis.
	int	m_zoom_ventana;				//!< Indica que el zoom por venana no se termino aun.
	int m_posejesscroll;			//!< Zoom en los ejes con el boton central del raton.
	float	m_colormanual;	//!< Distribucion colores manual 0a 1 (si >0 contiene el valor maximo para calcular color  raton +SHIFT)
	int m_distancias;					//!< Indica que hay que dibujar las lineas de referencia de medida de distancias.
	long m_offsetz;				//!< Offset en ms para anadir al tiempo de escala y dibujar en 0 si son horas se ponen segundos().

//
//CAMERA
//
	FPVCamera camera;
//
//LINE CONTROL
//
public:
	int m_nlines;					//!< Number of lines into the chart.
	CXYLineGL3D* m_plines;	//!< Pointer to lines.
	int m_max;						//!< Index to max value, if <0 the local max is searched.
	bool m_searchMaxValue;				//!< True if max value should be drown.
	pthread_mutex_t m_dataMutex;	//!< Data accesing buffer mutex.
};


#ifndef __PALETA
#define __PALETA
	/** \brief Auxiliary colors.*/
	const int m_paleta[]={ 0x000000ff,0x0000ff00,0x00ff0000,0x00ff8c00,0x003a5fcd,0x00FF00FF,0x00ffff00,	0x009400d3,	0x00FFFF};
	#endif //__PALETA
#endif	// OGLGRAPH2D

#ifndef __DATGRAF3D
#define __DATGRAF3D
/** \struct datgraf
    \brief Raw structure written at the begining of the file with information.*/
struct datgraf3d{
	double nPoints=0;			//<! Number of points of every line of time.
	double  nlinest=0;			//<! Time lines total.
	double nusedlinest=0.0;	//<! Time lines used.
	char nombre[16]="";			//<! Name.
	COLORREF color=0;				//<! Color.
	float visible=0;				//<! Visibility.
	int npuntosinteres=0;		//<! Points of interest.
};

#endif
