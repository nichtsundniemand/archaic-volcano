#include "meshes.hpp"

#include <array>
#include <random>

#include <loguru.hpp>

namespace volcano {
	namespace graphics {
		std::vector<vertex> make_lineprism(std::vector<glm::vec3> shape, float height) {
			std::vector<vertex> vertices;

			std::random_device rd;
			std::default_random_engine randomizer(rd());
			std::uniform_real_distribution<> dist(0.3, 2);

			glm::vec3 shift(0, height, 0);
			glm::vec3 normal(0, 0, 0);
			glm::vec4 color(dist(randomizer), dist(randomizer), dist(randomizer), 1);

			for(unsigned int i = 0; i < shape.size(); i++) {
				vertices.push_back({
					shape[i],
					normal,
					color,
				});
				vertices.push_back({
					shape[(i + 1) % shape.size()],
					normal,
					color,
				});

				vertices.push_back({
					shape[i] + shift,
					normal,
					color,
				});
				vertices.push_back({
					shape[(i + 1) % shape.size()] + shift,
					normal,
					color,
				});

				vertices.push_back({
					shape[i],
					normal,
					color,
				});
				vertices.push_back({
					shape[i] + shift,
					normal,
					color,
				});
			}

			return vertices;
		}

		std::vector<glm::vec3> triangulate_quad(std::vector<glm::vec3> quad) {
			return std::vector<glm::vec3> {
				quad[0], quad[1], quad[3], quad[3], quad[1], quad[2]
			};
		}

		std::vector<vertex> make_cube() {
			std::vector<glm::vec3> points;

			std::vector<std::vector<glm::vec3>> faces {
				// left face
				triangulate_quad({
					{-0.5, -0.5, -0.5},
					{-0.5, -0.5,  0.5},
					{-0.5,  0.5,  0.5},
					{-0.5,  0.5, -0.5},
				}),
				// right face
				triangulate_quad({
					{ 0.5, -0.5, -0.5},
					{ 0.5,  0.5, -0.5},
					{ 0.5,  0.5,  0.5},
					{ 0.5, -0.5,  0.5},
				}),
				// bottom face
				triangulate_quad({
					{-0.5, -0.5, -0.5},
					{ 0.5, -0.5, -0.5},
					{ 0.5, -0.5,  0.5},
					{-0.5, -0.5,  0.5},
				}),
				// top face
				triangulate_quad({
					{-0.5,  0.5, -0.5},
					{-0.5,  0.5,  0.5},
					{ 0.5,  0.5,  0.5},
					{ 0.5,  0.5, -0.5},
				}),
				// back face
				triangulate_quad({
					{-0.5, -0.5, -0.5},
					{-0.5,  0.5, -0.5},
					{ 0.5,  0.5, -0.5},
					{ 0.5, -0.5, -0.5},
				}),
				// front face
				triangulate_quad({
					{-0.5, -0.5,  0.5},
					{ 0.5, -0.5,  0.5},
					{ 0.5,  0.5,  0.5},
					{-0.5,  0.5,  0.5},
				}),
			};

			for(auto face: faces) {
				points.insert(points.end(), face.begin(), face.end());
			}

			std::vector<vertex> vertices;
			for(auto point: points) {
				vertices.push_back({point + glm::vec3(0, 0.5, 0), {0, 0, 0}, {0.7, 0.7, 0.7, 1}});
			}
			return vertices;
		};

		std::vector<vertex> make_linecube() {
			std::vector<glm::vec3> square {
				{-0.5, 0, -0.5},
				{-0.5, 0,  0.5},
				{ 0.5, 0,  0.5},
				{ 0.5, 0, -0.5},
			};

			return make_lineprism(square, 1);
		}

		std::vector<vertex> make_linecylinder(unsigned int segments) {
			std::vector<glm::vec3> circle;
			for(unsigned int i = 0; i < segments; i++) {
				circle.push_back({
					0.5 * cos(2 * i * M_PI / segments),
					0,
					0.5 * sin(2 * i * M_PI / segments)
				});
			}

			return make_lineprism(circle, 1);
		}

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
