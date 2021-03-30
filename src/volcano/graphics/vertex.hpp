#ifndef VERTEX_HPP
	#define VERTEX_HPP

#include <glm/glm.hpp>

namespace volcano {
	namespace graphics {
		struct vertex {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec4 color;
		};
	}
}

#endif
