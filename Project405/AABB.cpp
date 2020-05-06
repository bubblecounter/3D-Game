#include "AABB.h"
#include <math.h>  
 //constructor for cube objects
AABB::AABB(glm::vec3 center, float scale)
{
	bottomLeftBack = center - glm::vec3(scale*0.35f);
	TopRightFront = center + glm::vec3(scale*0.35f);
}

AABB::AABB(glm::vec3 bottomLeftBack, glm::vec3 TopRightFront)
{
	this->bottomLeftBack = bottomLeftBack;
	this->TopRightFront = TopRightFront;
}
AABB::AABB()
{
}

AABB::~AABB()
{
}

bool AABB::collidesWith(AABB box2)
{
	bool xIntersection = linearCollision(bottomLeftBack.x, TopRightFront.x, box2.bottomLeftBack.x, box2.TopRightFront.x);
	bool yIntersection = linearCollision(bottomLeftBack.y, TopRightFront.y, box2.bottomLeftBack.y, box2.TopRightFront.y);
	bool zIntersection = linearCollision(bottomLeftBack.z, TopRightFront.z, box2.bottomLeftBack.z, box2.TopRightFront.z);
	if (xIntersection && yIntersection && zIntersection) {
		return true;
	}
	return false;
}

bool AABB::linearCollision(float min1, float max1, float min2, float max2)
{
	if (max2 <= min1 || max1 <= min2) {
		return false;
	}
	return true;
}
