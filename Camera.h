#pragma once

// #include "Common.h"
#include <vector>
#define WINDOW_BOUNDARY 0.05

using namespace glm;
using namespace std;

// default camera values
const float PITCH = 0.0f;
const float YAW = -90.0f;	// view at (0, 0, -1) by default 
const float CAMERA_SPEED = 0.1f;
const float MOUSE_SENSITIVITY = 0.5f;
const float FOV = 45.0f;
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 1000.0f;

class  Camera
{
public:
	// camera attributes
	// Front, Up, and Right are perpendicular to each other. World_up may not be perpendicular.
	vec3 Position;
	vec3 Front;
	vec3 Up;
	vec3 Right;
	vec3 World_up;

	// Euler angles
	float Pitch;
	float Yaw;

	// zoom
	float fov;	// field of view
	float aspect_w;	// width
	float aspect_h; // height
	float near_plane;
	float far_plane;

	// record the last mouse position
	float last_x;
	float last_y;

	// option
	bool first_mouse;
	float camera_speed;
	float mouse_sensitivity;
	// timing
	float delta_time;
	float last_frame;

	// constructor
	Camera(vec3 position, vec3 up, float width, float height);

	// delta time
	void get_delta_time();
	// movement
	void camera_movement(unsigned char key);
	// rotation
	void camera_rotation(float current_x, float current_y);
	// zoom
	void camera_zoom(int dir);	// change fov

	// return view matrix
	mat4 get_view_matrix();
	// return projection matrix
	mat4 get_proj_matrix();

private:
	// calculates the front vector from the updated Eular angles 
	void update_camera_vector();
};

// constructor
 Camera:: Camera(vec3 position = vec3(0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float width = 600.0f, float height = 600.0f)
{
	 // camera attrubutes
	 Position = position;
	 World_up = up;

	 // angles
	 Pitch = PITCH;
	 Yaw = YAW;

	 // zoom
	 fov = FOV;
	 aspect_w = width;
	 aspect_h = height;
	 near_plane = NEAR_PLANE;
	 far_plane = FAR_PLANE;

	 update_camera_vector();

	 // option
	 first_mouse = true;
	 camera_speed = CAMERA_SPEED;
	 mouse_sensitivity = MOUSE_SENSITIVITY;
	 // timing
	 delta_time = 0.0f;
	 last_frame = 0.0f;
}

// delta time
void Camera::get_delta_time()
{
	float current_frame = (float)glutGet(GLUT_ELAPSED_TIME);
	delta_time = current_frame - last_frame;
	last_frame = current_frame;
}

// movement
void Camera::camera_movement(unsigned char key)
{
	float final_speed = delta_time * camera_speed;
	if (key == 'w')
		Position += final_speed * Front;
	if (key == 's')
		Position -= final_speed * Front;
	if (key == 'a')
		Position -= final_speed * Right;
	if (key == 'd')
		Position += final_speed * Right;
}

// rotation
void Camera::camera_rotation(float current_x, float current_y)
{
	if (first_mouse)
	{
		last_x = current_x;
		last_y = current_y;
		first_mouse = false;
	}

	float x_offset = current_x - last_x;
	float y_offset = last_y - current_y;	// e.g. in a window, y is 0 to 600 from top to bottom.
	last_x = current_x;
	last_y = current_y;

	// float x_offset = current_x - aspect_w / 2;
	// float y_offset = aspect_h / 2 - current_y;

	x_offset *= mouse_sensitivity;
	y_offset *= mouse_sensitivity;

	Pitch += y_offset;
	Yaw += x_offset;

	if (Pitch > 89.0f)
		Pitch = 89.0f;
	if (Pitch < -89.0f)
		Pitch = -89.0f;

	update_camera_vector();

	// boundary condition
	if (current_x < aspect_w * WINDOW_BOUNDARY || current_x > aspect_w * (1 - WINDOW_BOUNDARY) ||
		current_y < aspect_h * WINDOW_BOUNDARY || current_y > aspect_h * (1 - WINDOW_BOUNDARY))
	{
		glutWarpPointer((int)(aspect_w / 2), (int)(aspect_h / 2));
		last_x = aspect_w / 2;
		last_y = aspect_h / 2;
	}
}

// zoom
void Camera::camera_zoom(int dir)
{
	if (fov >= 1.0f && fov <= 60.0f)
		fov -= dir;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 60.0f)
		fov = 60.0f;
}

// return view matrix
 mat4 Camera::get_view_matrix()
{
	 return lookAt(Position, Position + Front, Up);
}

// return projection matrix
mat4 Camera::get_proj_matrix()
{
	return perspective(radians(fov), aspect_w / aspect_h, near_plane, far_plane);
}
 
// calculates the front vector from the updated Eular angles 
void Camera::update_camera_vector()
{
	// Front
	vec3 front;
	float P = radians(Pitch);
	float Y = radians(Yaw);
	front.x = cos(P) * cos(Y);
	front.y = sin(P);
	front.z = cos(P) * sin(Y);
	Front = normalize(front);

	// Right
	Right = normalize(cross(Front, World_up));

	// Up
	Up = normalize(cross(Right, Front));
}

