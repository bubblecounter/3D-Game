#include "Cube.h"



Cube::Cube()
{
}


Cube::Cube(glm::vec3 position, float scale, int emojiNum, float speed)
{
	this->position = position;
	this->scale = scale;
	this->emojiNum = emojiNum;
	this->speed = speed;
	this->destroyed = false;
	this->bbox = AABB(position, scale);
}

Cube::~Cube()
{
}

void Cube::updateCubePosition(glm::vec3 position)
{
	this->position = position;
	this->bbox = AABB(position, scale);
}

bool Cube::collidesWith(Cube cube2)
{
	return(this->bbox.collidesWith(cube2.bbox));
}
bool Cube::collidesWith(AABB bbox)
{
	return(this->bbox.collidesWith(bbox));
}

