#include "renderer.h"
#include "glad/glad.h"
#include <vector>
#include "renderer/basic/shape_renders.h"
#include "constants.h"

void render(application_t& app) {
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	// main menu
	if (app.scene_manager.cur_level == 0) {
		draw_text("Platformer Game", glm::vec2(250, (WINDOW_HEIGHT/2) + 150));
		draw_text("Play", glm::vec2(250, (WINDOW_HEIGHT/2) + 100));
		draw_text("Quit", glm::vec2(250, (WINDOW_HEIGHT/2) + 50));
	} else {
		// regular stuff
		draw_quad_renders(app);
	}


	SDL_GL_SwapWindow(app.window);
}
