#include "mesh.hpp"

#include <loguru.hpp>

namespace volcano {
	mesh::mesh(const struct buffer vbo, const int size)
	: vbo(vbo), vert_count(size) {
		LOG_F(MAX, "Create new mesh (size=%d)", size);
	}

	VkBuffer_T *const *mesh::get_buffer() {
		return &vbo.buffer;
	}

	const int& mesh::get_size() {
		return vert_count;
	}
}
