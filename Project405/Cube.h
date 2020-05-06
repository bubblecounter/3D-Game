#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "AABB.h"
class Cube
{
public:
	glm::vec3 position; // center of cube
	int emojiNum;
	float scale;
	float speed;
	bool destroyed; // whether player touvhed it
	AABB bbox; // axis alligned bounding box of cube for collision detection
	
	Cube();
	Cube(glm::vec3 position, float scale, int emojiNum, float speed);
	~Cube();
	void updateCubePosition(glm::vec3 position);
	bool collidesWith(Cube cube2);
	bool collidesWith(AABB bbox);
};

