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
/** \file
  \brief Creates the behaviour of a button.
  \date 06-2017
  \author Marcos Perez*/

/// @ingroup graph3d

#ifndef FPVCAMERA_H
#define FPVCAMERA_H

#include <glm/vec3.hpp>
#include <glm/glm.hpp>

/** \class COGLGraph
    \brief Basic First Player View.*/
class FPVCamera
{
//****************************************** FUNCTIONS **************************************
//*******************************************************************************************

public:
	FPVCamera();
	~FPVCamera();
  /** \brief Update camera direction by absolute angle.
     \param xzangle
     \param yzangle	 */
	void UpdateCamDirection(float newxzangle, float newyzangle);
  /**  * \brief Update camera position by absolute position.
     \param newx
     \param newy
     \param newz	 */
	void UpdateCamPosition(float newx,float newy,float newz);
  /** \brief Add values to camera position.
     \param addx
     \param addy
     \param addz	 */
	void IncrementCamPosition(float addx,float addy,float addz);
  /** \brief Increment direction angles.
     \param incxzangle
     \param incyzangle	 */
	void AddDirection(float incxzangle, float incyzangle);

//****************************************** VARIABLES **************************************
//*******************************************************************************************
public:
  float xzAngle = 0.0f; //!< XZ angle.
  float yzAngle =0.f;   //!< YZ angle.
	glm::vec3 campos;				//!< Camera position.
	glm::vec3 camdir;				//!< Camera direction.
	glm::vec3 camright;			//!< Right vector to camera.
	glm::vec3 camdown;			//!< Down vector to camera.

};

#endif // FPVCAMERA_H
