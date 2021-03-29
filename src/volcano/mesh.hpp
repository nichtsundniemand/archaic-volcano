#ifndef MESH_HPP
	#define MESH_HPP

#include "renderer.hpp"

namespace volcano {
	class mesh {
		private:
			struct buffer vbo;
			int vert_count;

		public:
			mesh(const struct buffer vbo, const int size);

			VkBuffer_T *const *get_buffer();
			const int& get_size();
	};
}

#endif
