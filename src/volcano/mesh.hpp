#ifndef MESH_HPP
	#define MESH_HPP

#include <array>

#include <transform.hpp>
#include "renderer.hpp"

namespace volcano {
	class mesh {
		private:
			struct buffer vbo;
			int vert_count;

			std::array<kepler::transform_reference, MAX_SYNC> transforms;

		public:
			mesh(
				const struct buffer vbo,
				const int size,
				const kepler::transform_reference& transform
			);

			VkBuffer_T *const *get_buffer();
			const int& get_size();
			kepler::transform_reference& get_transform(const int index);
	};
}

#endif
