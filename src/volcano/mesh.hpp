#ifndef MESH_HPP
	#define MESH_HPP

#include <array>

#include <transform.hpp>
#include "renderer.hpp"

namespace volcano {
	class mesh {
		private:
			std::array<VkDescriptorSet, MAX_SYNC> descriptor_sets;
			std::array<struct buffer, MAX_SYNC> uniform_buffers;

			struct buffer vbo;
			int vert_count;

			std::array<kepler::transform_reference, MAX_SYNC> transforms;

		public:
			mesh(
				const std::array<VkDescriptorSet, MAX_SYNC> descriptor_sets,
				const std::array<struct buffer, MAX_SYNC> uniform_buffers,
				const struct buffer vbo,
				const int size,
				const kepler::transform_reference& transform
			);

			const VkDescriptorSet& get_descriptor_set(const int index);
			const struct buffer& get_uniform(const int index);
			VkBuffer_T *const *get_buffer();
			const int& get_size();
			kepler::transform_reference& get_transform(const int index);
	};
}

#endif
