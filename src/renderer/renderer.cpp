#include "renderer.h"
#include <vector>


// will add spritesheet renderers as well in the future

void render(application_t& app) {
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

    draw_rectangle_renders();

	SDL_GL_SwapWindow(app.window);
}