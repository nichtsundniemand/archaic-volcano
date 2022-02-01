#include "mesh.hpp"

#include <loguru.hpp>

namespace volcano {
	mesh::mesh(
		const std::array<VkDescriptorSet, MAX_SYNC> descriptor_sets,
		const std::array<struct buffer, MAX_SYNC> uniform_buffers,
		const struct buffer vbo,
		const int size,
		const kepler::transform_reference& transform
	)
	: descriptor_sets(descriptor_sets), uniform_buffers(uniform_buffers),
	  vbo(vbo), vert_count(size), transforms({transform, transform, transform, transform}) {
		LOG_F(MAX, "Create new mesh (size=%d)", size);
	}

	const VkDescriptorSet& mesh::get_descriptor_set(const int index) {
		return descriptor_sets[index];
	}

	const struct buffer& mesh::get_uniform(const int index) {
		return uniform_buffers[index];
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
