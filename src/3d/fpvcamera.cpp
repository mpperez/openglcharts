#include "fpvcamera.h"
#include "math.h"


#define sind(x) (sin(x*M_PI/180));
#define cosd(x) (cos(x*M_PI/180));

FPVCamera::FPVCamera()
{
	campos=glm::vec3(0,10,5);
	glm::vec3(0,0,-1);
	UpdateCamDirection(20,0);
}

FPVCamera::~FPVCamera()
{

}

void FPVCamera::UpdateCamDirection(float newxzangle, float newyzangle)
{

	xzAngle=newxzangle>360 ? newxzangle-360 : (newxzangle<-360?  newxzangle+360 :newxzangle);
	yzAngle=newyzangle>90 ? 90 : (newyzangle<-90 ?  -90: newyzangle);
	camdir.x = sind(xzAngle );
	camdir.z = -cosd(xzAngle );
	camdir.y = -sind(yzAngle );
	camright=glm::cross(glm::normalize(camdir),glm::vec3(0,1,0));
	camdown=glm::cross(glm::normalize(camdir),camright);
	camdown=glm::normalize(camdown);
	camright=glm::normalize(camright);
	//printf("camvec(%f,%f,%f),up(%f,%f,%f)\n",camdir.x,camdir.y,camdir.z,down.x,down.y,down.z);
	//yzAngle=yzangle;
}
void FPVCamera::IncrementCamPosition(float addx,float addy,float addz)
{
	glm::vec3 right=glm::cross(glm::normalize(camdir),glm::vec3(0,1,0));
	glm::vec3 down=glm::cross(glm::normalize(camdir),right);
	down=glm::normalize(down);
	right=glm::normalize(right);

UpdateCamPosition(campos.x + addy* down.x + addx * right.x + camdir.x*addz,
							campos.y + addy * down.y+ camdir.y*addz,
							campos.z + addy * down.z + addx * right.z+ camdir.z*addz);

}

void FPVCamera::UpdateCamPosition(float newx,float newy,float newz)
{
	campos.x=newx;
	campos.y=newy;
	campos.z=newz;
}

void FPVCamera::AddDirection(float incxzangle, float incyzangle)
{
	UpdateCamDirection(xzAngle+incxzangle,yzAngle+incyzangle);
}
