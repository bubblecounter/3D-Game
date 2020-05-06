#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class AABB
{
public:
	glm::vec3 bottomLeftBack;
	glm::vec3 TopRightFront;
	AABB(glm::vec3 center, float scale);
	AABB(glm::vec3 bottomLeftBack, glm::vec3 TopRightFront);
	AABB();
	~AABB();
	bool collidesWith(AABB box2);
	bool linearCollision(float min1,float max1, float min2, float max2);
};

