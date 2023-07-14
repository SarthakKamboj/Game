#include "conversion.h"
#include "constants.h"

namespace conversion {
	glm::vec2 ndc_coord_to_window_coord(const glm::vec2& ndc_coord) {
		glm::vec2 ndc_0_to_1 = (ndc_coord + 1.0f) * 0.5f;
		return ndc_0_to_1 * glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT);
	}

	glm::vec2 ndc_size_to_window_size(const glm::vec2& ndc_size) {
		return ndc_size * (glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT) / 2.f);
	}

	glm::vec2 screen_coord_to_ndc_coord(const glm::vec2& screen_coord) {
		glm::vec2 normalized_screen = screen_coord / glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT);
		glm::vec2 ndc = (normalized_screen * 2.0f) - 1.0f;
		return glm::vec2(ndc.x, ndc.y);
	}
}