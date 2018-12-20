#pragma once

#include "Common.h"
#include <vector>

using namespace glm;
using namespace std;

// default camera values
const float PITCH = 0.0f;
const float YAW = -90.0f;
const float CMERA_SPEED = 0.1f;

class  Camera
{
public:
	// camera attributes
	vec3 Position;
	vec3 Front;
	vec3 Up;
	vec3 Right;

	// Euler angles
	float Pitch;
	float Yaw;

	// timing
	float delta_time;
	float last_frame;

	// constructor
	Camera(vec3, vec3, vec3);

	// delta time
	void get_delta_time();

	// movement
	void camera_movement(unsigned char, int, int);

	// return view matrix
	mat4 get_view_matrix();

private:

};

// constructor
 Camera:: Camera(vec3 position = vec3(0.0f), vec3 front = vec3(0.0f, 0.0f, -1.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f))
{
	 Position = position;
	 Front = normalize(front);
	 Up = normalize(up);
	 Right = normalize(cross(front, up));

	 delta_time = 0.0f;
	 last_frame = 0.0f;
}

// delta time
void Camera::get_delta_time()
{
	float current_frame = glutGet(GLUT_ELAPSED_TIME);
	delta_time = current_frame - last_frame;
	last_frame = current_frame;
}

// movement
void Camera::camera_movement(unsigned char key, int x, int y)
{
	if (key == 'w')
		Position += delta_time * CMERA_SPEED * Front;
	if (key == 's')
		Position -= delta_time * CMERA_SPEED * Front;
	if (key == 'a')
		Position -= delta_time * CMERA_SPEED * Right;
	if (key == 'd')
		Position += delta_time * CMERA_SPEED * Right;
}

// return view matrix
 mat4 Camera::get_view_matrix()
{
	 return lookAt(Position, Position + Front, Up);
}
