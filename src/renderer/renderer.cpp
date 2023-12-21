#include "renderer.h"
#include "glad/glad.h"
#include <vector>
#include "renderer/basic/shape_renders.h"
#include "constants.h"
#include "ui/ui.h"

void render(application_t& app) {
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	// main menu
	if (app.scene_manager.cur_level == 0) {
		style_t style;
		style.center_x = true;
		style.center_y = true;
		style.content_spacing = 20;
		int panel_handle = create_panel(style);
		style.center_x = false;
		style.center_y = false;
		style.text_size = TEXT_SIZE::TITLE;
		text_t t1 = create_text("Platformer Game", style);
		style.text_size = TEXT_SIZE::REGULAR;

		text_t t2 = create_text("Play", style);
		text_t t3 = create_text("Settings", style);
		text_t t4 = create_text("Quit", style);

		add_text_to_panel(panel_handle, t1);
		add_text_to_panel(panel_handle, t2);
		add_text_to_panel(panel_handle, t3);
		add_text_to_panel(panel_handle, t4);

		render_ui();
	} else {
		// regular stuff
		draw_quad_renders(app);
	}


	SDL_GL_SwapWindow(app.window);
}
