#ifndef MESH_LOADER_HPP
	#define MESH_LOADER_HPP

#include <span>
#include <vector>

#include "graphics/vertex.hpp"

namespace volcano {
	class mesh_loader {
		private:
			void *mapped_file;
			int mapping_size;

			std::span<glm::vec3> position_data;
			std::span<glm::vec3> normal_data;
			std::span<uint16_t> index_data;

			std::vector<graphics::vertex> vertex_data;

		public:
			struct file_layout {
				int buffer_size;

				int vertex_start;
				int vertex_count;

				int normal_start;
				int normal_count;

				int index_start;
				int index_count;
			};

			mesh_loader(const char *filename, const struct file_layout& layout);
			~mesh_loader();

			const std::vector<graphics::vertex>& vertices();
	};
}

#endif
