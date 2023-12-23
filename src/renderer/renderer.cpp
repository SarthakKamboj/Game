#include "renderer.h"
#include "glad/glad.h"
#include <vector>
#include "renderer/basic/shape_renders.h"
#include "constants.h"
#include "ui/ui.h"

void render(application_t& app) {
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	start_of_frame();

	// main menu
	if (app.scene_manager.cur_level == 0) {
		// style_t style;
		// style.center_x = true;
		// style.center_y = true;
		// style.content_spacing = 20;
		create_panel("main menu panel");
		// style.center_x = false;
		// style.center_y = false;
		// style.text_size = TEXT_SIZE::TITLE;
		// style.margins = glm::vec2(40);
		create_text("Platformer Game");

		// style.text_size = TEXT_SIZE::REGULAR;
		// style.margins = glm::vec2(0);

		create_text("Play");
		if (app.clicked) {
			create_text("Settings");
		}
		create_text("Credits");
		create_text("Quit");

		end_panel();

		autolayout_hierarchy();

		// add_text_to_panel(panel_handle, t1);
		// add_text_to_panel(panel_handle, t2);
		// add_text_to_panel(panel_handle, t3);
		// add_text_to_panel(panel_handle, t4);

		render_ui();
	} else {
		// regular stuff
		draw_quad_renders(app);
	}


	SDL_GL_SwapWindow(app.window);
}
