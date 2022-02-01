#include "mesh.hpp"

#include <loguru.hpp>

namespace volcano {
	mesh::mesh(
		const struct buffer vbo,
		const int size,
		const kepler::transform_reference& transform
	)
	: vbo(vbo), vert_count(size), transforms({transform, transform, transform, transform}) {
		LOG_F(MAX, "Create new mesh (size=%d)", size);
	}

	VkBuffer_T *const *mesh::get_buffer() {
		return &vbo.buffer;
	}

	const int& mesh::get_size() {
		return vert_count;
	}

	kepler::transform_reference& mesh::get_transform(const int index) {
		return transforms[index];
	}
}
