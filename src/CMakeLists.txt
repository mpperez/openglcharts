cmake_minimum_required(VERSION 2.9.1)

if(BUILD_QT)
	message("Building usin Qt windows.")

	find_package(Qt5Widgets REQUIRED)
	find_package(Qt5Core REQUIRED)
	find_package(Qt5Gui REQUIRED)
	find_package(Qt5OpenGL REQUIRED)

	include_directories (../include/3d)
	include_directories (../include/2d)

	set(CMAKE_AUTOUIC ON)
	#set(CMAKE_AUTOMOC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	set (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../lib CACHE PATH "Directory for Libraries")

	#2D
	SET ( LIBSRC2D 2d/xylinegl.cpp 2d/oglgraph2d.cpp 2d/qopengl2dcharts.cpp )

	qt5_wrap_cpp(moc_sources2d ../include/2d/qopengl2dcharts.h)

	ADD_LIBRARY(${TARGETLIBRARYNAME2D} SHARED ${LIBSRC2D} ${moc_sources2d})

	target_link_libraries(${TARGETLIBRARYNAME2D}  glut GLU X11 Qt5::Widgets Qt5::Gui Qt5::Core Qt5::OpenGL)

	#3D
	SET ( LIBSRC3D 3d/xylinegl3d.cpp 3d/oglgraph3d.cpp 3d/qopengl3dcharts.cpp 3d/colormapgl3d.cpp 3d/fpvcamera.cpp 3d/oglbutton.cpp)

	qt5_wrap_cpp(moc_sources3d ../include/3d/qopengl3dcharts.h)

	ADD_LIBRARY(${TARGETLIBRARYNAME3D} SHARED ${LIBSRC3D} ${moc_sources3d})

	target_link_libraries(${TARGETLIBRARYNAME3D}  glut GLU X11 Qt5::Widgets Qt5::Gui Qt5::Core Qt5::OpenGL)


else()

	message("Building usin X window system.")

	include_directories (../include/3d)
	include_directories (../include/2d)

	set(CMAKE_AUTOUIC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)

	set (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../lib CACHE PATH "Directory for Libraries")
	#2D
	message("Building 2D lib.")
	SET ( LIBSRC2D 2d/xylinegl.cpp  2d/oglgraph2d.cpp 2d/xopengl2dcharts.cpp)

	ADD_LIBRARY (${TARGETLIBRARYNAME2D} SHARED ${LIBSRC2D})
	#ADD_LIBRARY ( v4lcamera SHARED ${LIBSRC})
	target_link_libraries(${TARGETLIBRARYNAME2D}  glut GLU X11 )
	
	#3D
	message("Building 3D lib.")
	SET ( LIBSRC3D 3d/xylinegl3d.cpp 3d/oglgraph3d.cpp 3d/xopengl3dcharts.cpp 3d/colormapgl3d.cpp 3d/fpvcamera.cpp 3d/oglbutton.cpp)

	ADD_LIBRARY (${TARGETLIBRARYNAME3D} SHARED ${LIBSRC3D})
	target_link_libraries(${TARGETLIBRARYNAME3D}  glut GLU X11 )

endif()

install(TARGETS ${TARGETLIBRARYNAME2D}  DESTINATION "${INSTALL_PATH}/lib")
install(TARGETS ${TARGETLIBRARYNAME3D}  DESTINATION "${INSTALL_PATH}/lib")

