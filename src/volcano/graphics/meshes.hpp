#ifndef MESHES_HPP
	#define MESHES_HPP

#include <vector>

#include "vertex.hpp"

namespace volcano {
	namespace graphics {
		std::vector<vertex> make_grid(
			const unsigned int rows,
			const unsigned int cols
		);

		std::vector<vertex> make_linegrid(
			const unsigned int rows,
			const unsigned int cols
		);
	}
}

#endif
