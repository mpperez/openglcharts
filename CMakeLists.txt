cmake_minimum_required(VERSION 2.9.1)
option(BUILD_QT     "Build usin Qt"             ON )
option(BUILD_SAMPLES   "Build samples"             ON )
#install
SET(INSTALL_PATH "/usr/local/")

if(BUILD_QT)
	add_definitions(-DUSE_QT)
	set(TARGET_VERSION "USE_QT")
else()
	add_definitions(-DUSE_X)
	set(TARGET_VERSION "USE_X")
endif()


SET(TARGETLIBRARYNAME2D "opengl2dcharts")
SET(TARGETLIBRARYNAME3D "opengl3dcharts")

add_subdirectory (src)
include_directories (./include/)
include_directories (./src/)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/./bin CACHE PATH "Directory for Executables")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#2d module
include_directories (./include/2d)
include_directories (./src/2d)
#Generating target dialog information.
configure_file(templates/opengl2dchartsbuildinfo.h.in opengl2dchartsbuildinfo.h)

#Samples
if(BUILD_SAMPLES)
message("Configuring 2D sample.")
	if(BUILD_QT)
		SET(SRC2D ./src/samples/sampleQT2D.cpp)
		SET(TARGETINSTALL include/2d/qopengl2dcharts.h)
	else()
		SET(SRC2D ./src/samples/sampleX2D.cpp)
		SET(TARGETINSTALL include/2d/xopengl2dcharts.h)
	endif()#BUILD_WINDOWS...

	ADD_EXECUTABLE(sample2d ${SRC2D})
	target_link_libraries(sample2d ${TARGETLIBRARYNAME2D})
endif()#BUILD_SAMPLES


install(FILES  include/2d/opengl2dcharts.h  ${CMAKE_CURRENT_BINARY_DIR}/opengl2dchartsbuildinfo.h ${TARGETINSTALL} DESTINATION "${INSTALL_PATH}/include")

#3d module
include_directories (./include/3d)
include_directories (./src/3d)
#Generating target dialog information.
configure_file(templates/opengl3dchartsbuildinfo.h.in opengl3dchartsbuildinfo.h)

#Samples
if(BUILD_SAMPLES)
message("Configuring 3D sample.")
	if(BUILD_QT)
		SET(SRC3D ./src/samples/sampleQT3D.cpp)
		SET(TARGETINSTALL include/3d/qopengl3dcharts.h)
	else()
		SET(SRC3D ./src/samples/sampleX3D.cpp)
		SET(TARGETINSTALL include/3d/xopengl3dcharts.h)
	endif()#BUILD_WINDOWS...

	ADD_EXECUTABLE(sample3d ${SRC3D})
	target_link_libraries(sample3d ${TARGETLIBRARYNAME3D})
endif()#BUILD_SAMPLES


install(FILES  include/2d/opengl3dcharts.h  ${CMAKE_CURRENT_BINARY_DIR}/opengl3dchartsbuildinfo.h ${TARGETINSTALL} DESTINATION "${INSTALL_PATH}/include")




