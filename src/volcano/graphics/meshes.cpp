#include "meshes.hpp"

#include <array>
#include <random>

#include <loguru.hpp>

namespace volcano {
	namespace graphics {
		std::vector<vertex> make_grid(
			const unsigned int rows,
			const unsigned int cols
		) {
			std::vector<vertex> vertices;

			float half_width  = 0.5 * cols;
			float half_height = 0.5 * rows;

			glm::vec3 normal(0, 1, 0);

			std::random_device rd;
			std::default_random_engine randomizer(rd());
			std::uniform_real_distribution<> dist(0.3, 1);
			
			for(unsigned int row = 0; row < rows; row++) {
				for(unsigned int col = 0; col < cols; col++) {
					glm::vec4 color(
						dist(randomizer), dist(randomizer), dist(randomizer), 1
					);
			
					std::vector<vertex> quad {
						{
							{    col - half_width, 0,     row - half_height},
							normal, color,
						}, {
							{    col - half_width, 0, row + 1 - half_height},
							normal, color,
						}, {
							{col + 1 - half_width, 0,     row - half_height},
							normal, color,
						}, {
							{col + 1 - half_width, 0,     row - half_height},
							normal, color,
						}, {
							{    col - half_width, 0, row + 1 - half_height},
							normal, color,
						}, {
							{col + 1 - half_width, 0, row + 1 - half_height},
							normal, color,
						},
					};

					vertices.insert(vertices.end(), quad.begin(), quad.end());
				}
			}

			for(auto vertex: vertices) {
				LOG_F(ERROR,
					"vertex:\n"
					" - x: %f\n"
					" - y: %f\n"
					" - z: %f\n",
					vertex.position.x,
					vertex.position.y,
					vertex.position.z
				);
			}

			return vertices;
		}
		
		std::vector<vertex> make_linegrid(
			const unsigned int rows,
			const unsigned int cols
		) {
			std::vector<vertex> vertices;

			float half_width  = 0.5 * cols;
			float half_height = 0.5 * rows;

			for(int col = 0; col <= cols; col++) {
				std::vector<vertex> line {
					{
						{col - half_width, 0, -half_height},
						{0, 0, 0}, {0, 0, 0, 1}
					}, {
						{col - half_width, 0, rows - half_height},
						{0, 0, 0}, {0, 0, 0, 1}
					},
				};

				vertices.insert(vertices.end(), line.begin(), line.end());
			}

			for(int row = 0; row <= rows; row++) {
				std::vector<vertex> line {
					{
						{-half_width, 0, row - half_height},
						{0, 0, 0}, {0, 0, 0, 1}
					}, {
						{cols - half_width, 0, row - half_height},
						{0, 0, 0}, {0, 0, 0, 1}
					},
				};

				vertices.insert(vertices.end(), line.begin(), line.end());
			}

			return vertices;
		}
	}
}
