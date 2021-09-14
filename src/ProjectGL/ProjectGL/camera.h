#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

using namespace glm;

// ����������ƶ��Ķ���
enum Movement {
	FORWARD, BACKWARD, LEFTWARD, RIGHTWARD
};

// Ĭ�����������
const vec3 POS = vec3(0.0f, 0.0f, 0.0f);
const vec3 UP = vec3(0.0f, 1.0f, 0.0f);
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// �������
class Camera
{
public:
	// ���������
	vec3 _pos, _front, _up, _right, _worldup;
	// ŷ����
	float _yaw, _pitch;
	// �����ѡ��
	float _speed, _sensitivity, _zoom;

	// ���캯��
	Camera(vec3 pos = POS, vec3 up = UP, float yaw = YAW, float pitch = PITCH) :
		_speed(SPEED), _sensitivity(SENSITIVITY), _zoom(ZOOM)
	{
		_pos = pos;
		_worldup = up;
		_yaw = yaw;
		_pitch = pitch;
		updateCameraVectors();
	}

	// ��ȡ�۲����
	mat4 getViewMatrix()
	{
		return lookAt(_pos, _pos + _front, _up);
	}

	// �������ָ����������λ��
	void processKey(Movement m, float dt)
	{
		float v = _speed * dt;
		switch (m)
		{
		case FORWARD:
			_pos += _front * v;
			break;
		case BACKWARD:
			_pos -= _front * v;
			break;
		case LEFTWARD:
			_pos -= _right * v;
			break;
		case RIGHTWARD:
			_pos += _right * v;
			break;
		}
	}

	// ��������ƶ���������ת�ӽ�
	void processMouseMovement(float offsetX, float offsetY, GLboolean constraintPitch=true)
	{
		_yaw += offsetX * _sensitivity;
		_pitch += offsetY * _sensitivity;
		if (constraintPitch) {
			if (_pitch > 89.0f) {
				_pitch = 89.0f;
			}
			if (_pitch < -89.0f) {
				_pitch = -89.0f;
			}
		}
		updateCameraVectors();
	}

	// ���������֣����������ӽ�
	void processMouseScroll(float offset)
	{
		_zoom -= offset;
		if (_zoom < 1.0f) {
			_zoom = 1.0f;
		}
		if (_zoom > 45.0f) {
			_zoom = 45.0f;
		}
	}

	float getZoom() {
		return _zoom;
	}
	
private:
	void updateCameraVectors() {
		_front.x = cos(radians(_yaw)) * cos(radians(_pitch));
		_front.y = sin(radians(_pitch));
		_front.z = sin(radians(_yaw)) * cos(radians(_pitch));
		_front = normalize(_front);
		_right = normalize(cross(_front, _worldup));
		_up = normalize(cross(_right, _front));
	}
};
#endif