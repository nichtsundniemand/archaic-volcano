#include "transform.hpp"

#include <loguru.hpp>

namespace kepler {
	void transform::set_position(const glm::vec3& position) {
		this->position = position;
		dirty = true;
	}

	void transform::set_orientation(const glm::quat& orientation) {
		this->orientation = orientation;
		dirty = true;
	}

	uint32_t transform::try_update() {
		if(dirty) {
			// Recalc matrices;
			LOG_F(INFO, "Recalculating transform matrix");
			model_matrix = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(orientation);

			dirty = false;
			version++;
		}

		return version;
	}

	const glm::mat4& transform::get_model_matrix() const {
		return model_matrix;
	}

	transform_reference::transform_reference(transform& transform)
	: ref(transform), version(0) {}

	bool transform_reference::try_update() {
		if(version < ref.try_update()) {
			version = ref.try_update();
			return true;
		}

		return false;
	}

	const glm::mat4& transform_reference::get_model_matrix() const {
		return ref.get_model_matrix();
	}
}
