#include "mesh_loader.hpp"

#include <fcntl.h>
#include <sys/mman.h>

#include <loguru.hpp>

namespace volcano {
	mesh_loader::mesh_loader(const char *filename, const struct file_layout& layout)
	: mapping_size(layout.buffer_size) {
		int fd = open(filename, O_CLOEXEC);
		mapped_file = mmap(nullptr, layout.buffer_size, PROT_READ, MAP_PRIVATE, fd, 0);

		position_data = {
			(glm::vec3 *)(mapped_file + layout.vertex_start),
			layout.vertex_count
		};
		normal_data = {
			(glm::vec3 *)(mapped_file + layout.normal_start),
			layout.normal_count
		};
		index_data = {
			(uint16_t *)(mapped_file + layout.index_start),
			layout.index_count
		};

		for(auto index: index_data) {
			LOG_F(MAX, "index: %d", index);

			vertex_data.push_back({
				.position = position_data[index],
				.normal   = normal_data[index],
				.color    = {0, 0, 0, 1},
			});
		}
	}

	mesh_loader::~mesh_loader() {
		munmap(mapped_file, mapping_size);
	}

	const std::vector<graphics::vertex>& mesh_loader::vertices() {
		return vertex_data;
	}
}
