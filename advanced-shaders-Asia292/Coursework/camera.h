#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <iostream>
#include <fstream>
#include <GLEW/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iomanip>

// Directions of movement
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Camera movement
const GLfloat YAW = 0.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 3.0f;
const GLfloat MOUSE_SENSITIVTY = 0.05f;
const GLfloat MOUSE_ZOOM = 45.0f;
const float  MAX_PITCH_ANGLE = 89.0f;

class Camera
{
public:
	Camera(glm::vec3 pos = glm::vec3(0.0, 0.0, 2.0),
		glm::vec3 up = glm::vec3(0.0, 1.0, 0.0),
		GLfloat yaw = YAW, GLfloat pitch = PITCH) 
		:position(pos), forward(0.0, 0.0, -1.0), viewUp(up),
		moveSpeed(SPEED), mouse_zoom(MOUSE_ZOOM), mouse_sensitivity(MOUSE_SENSITIVTY),
		yawAngle(yaw), pitchAngle(pitch)
	{
		this->updateCameraVectors();
	}
public:
	// Return view transformation matrix
	glm::mat4 getViewMatrix()
	{
		return glm::lookAt(this->position, this->position + this->forward, this->viewUp);
	}

	// Processes keyboard events and moves camera
	void handleKeyPress(Camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->moveSpeed * deltaTime;
		switch (direction)
		{
		case FORWARD:
			this->position += this->forward * velocity;
			break;
		case BACKWARD:
			this->position -= this->forward * velocity;
			break;
		case LEFT:
			this->position -= this->side * velocity;
			break;
		case RIGHT:
			this->position += this->side * velocity;
			break;
		default:
			break;
		}
	}
	// Processes mouse movement
	void handleMouseMove(GLfloat xoffset, GLfloat yoffset)
	{
		
		xoffset *= this->mouse_sensitivity;
		yoffset *= this->mouse_sensitivity;

		this->pitchAngle += yoffset;
		this->yawAngle += xoffset;

		this->normalizeAngle();
		this->updateCameraVectors();
	}

	// Processes mouse wheel scroll
	void handleMouseScroll(GLfloat yoffset)
	{
		if (this->mouse_zoom >= 1.0f && this->mouse_zoom <= MOUSE_ZOOM)
			this->mouse_zoom -= this->mouse_sensitivity * yoffset;
		if (this->mouse_zoom <= 1.0f)
			this->mouse_zoom = 1.0f;
		if (this->mouse_zoom >= 45.0f)
			this->mouse_zoom = 45.0f;
	}

	// Constrain pitch and yaw
	void normalizeAngle()
	{
		if (this->pitchAngle > MAX_PITCH_ANGLE)
			this->pitchAngle = MAX_PITCH_ANGLE;
		if (this->pitchAngle < -MAX_PITCH_ANGLE)
			this->pitchAngle = -MAX_PITCH_ANGLE;
		if (this->yawAngle < 0.0f)
			this->yawAngle += 360.0f;
	}

	// Calculates forward and side vectors
	void updateCameraVectors()
	{
		glm::vec3 forward;
		forward.x = -sin(glm::radians(this->yawAngle)) * cos(glm::radians(this->pitchAngle));
		forward.y = sin(glm::radians(this->pitchAngle));
		forward.z = -cos(glm::radians(this->yawAngle)) * cos(glm::radians(this->pitchAngle));
		this->forward = glm::normalize(forward);
		
		glm::vec3 side;
		side.x = cos(glm::radians(this->yawAngle));
		side.y = 0;
		side.z = -sin(glm::radians(this->yawAngle));
		this->side = glm::normalize(side);
	}

public:
	glm::vec3 forward,up, side, viewUp, position; // Camera properties
	GLfloat yawAngle, pitchAngle; // Euler angle
	GLfloat moveSpeed, mouse_sensitivity, mouse_zoom; // Camera options
};

#endif