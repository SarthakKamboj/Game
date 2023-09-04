#include "renderer.h"
#include "glad/glad.h"
#include <vector>
#include "shared/networking.h"
#include "renderer/basic/shape_renders.h"
#include "fifo.h"
#include "constants.h"

// will add spritesheet renderers as well in the future

// extern fifo<TYPE_OF_MEMBER(res_data_t, snapshot_data), MAX_SNAPSHOT_BUFFER_SIZE> gameobject_saved_snapshots;

extern bool started_updates;
void render(application_t& app) {
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

    if (started_updates) {
 		draw_rectangle_renders();
	}

	SDL_GL_SwapWindow(app.window);
}