#ifndef CAMERA_HPP
	#define CAMERA_HPP

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace volcano {
	class camera {
		private:
			glm::vec3 eye;
			glm::vec3 target;
			glm::vec3 up;

			glm::mat4 clipmatrix;

			bool cameramatrix_dirty;
			glm::mat4 cameramatrix;

		public:
			camera(float fov, float aspect, float z_near, float z_far);

			void set_eye(const glm::vec3& eye);
			void set_target(const glm::vec3& target);

			bool update_matrix();
			const glm::mat4& get_matrix();
	};
}

#endif
