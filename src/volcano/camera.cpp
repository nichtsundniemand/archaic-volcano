#include "camera.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <loguru.hpp>

namespace volcano {
	camera::camera(float fov, float aspect, float z_near, float z_far)
	: eye(glm::vec3(0, 0, 0)), target(glm::vec3(0, 0, 1)), up(glm::vec3(0, 1, 0)),
	  clipmatrix(glm::perspective(fov, aspect, z_near, z_far)) {
		LOG_SCOPE_F(MAX, "Create new camera");
		LOG_F(MAX, "fov: %f", fov);
		LOG_F(MAX, "aspect: %f", aspect);
		LOG_F(MAX, "z_near: %f", z_near);
		LOG_F(MAX, "z_far: %f", z_far);

		cameramatrix       = clipmatrix * glm::lookAt(eye, target, up);
		cameramatrix_dirty = false;
	}

	void camera::set_eye(const glm::vec3& eye) {
		this->eye          = eye;
		cameramatrix_dirty = true;
	}

	void camera::set_target(const glm::vec3& target) {
		this->target       = target;
		cameramatrix_dirty = true;
	}

	bool camera::update_matrix() {
		if(!cameramatrix_dirty) {
			return false;
		}

		cameramatrix       = clipmatrix * glm::lookAt(eye, target, up);
		cameramatrix_dirty = false;

		return true;
	}

	const glm::mat4& camera::get_matrix() {
		return cameramatrix;
	}
}
