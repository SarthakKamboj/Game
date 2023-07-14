#pragma once

#include "glm/glm.hpp"

struct vertex_t {
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 color = glm::vec3(0, 0, 0);
	glm::vec2 tex_coord = glm::vec2(0, 0);
};

vertex_t create_vertex(glm::vec3 position, glm::vec3 color, glm::vec2 tex_coord);
