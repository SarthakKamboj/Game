#include "renderer.h"
#include "glad/glad.h"
#include <vector>
#include "renderer/basic/shape_renders.h"
#include "constants.h"

void render(application_t& app) {
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

 	draw_quad_renders(app);
	draw_text("hellooooooooo", glm::vec2(250, WINDOW_HEIGHT - 50));

	SDL_GL_SwapWindow(app.window);
}
