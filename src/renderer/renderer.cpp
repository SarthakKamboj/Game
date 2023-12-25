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
		
		style_t panel_style;
		panel_style.display_dir = DISPLAY_DIR::HORIZONTAL;
		panel_style.float_val = FLOAT::END;
		panel_style.content_spacing = 20.f;
		push_style(panel_style);
		create_panel("main menu panel");
		pop_style();

		style_t container_style;
		container_style.display_dir = DISPLAY_DIR::VERTICAL;
		container_style.float_val = FLOAT::CENTER;
		container_style.content_spacing = 20.f;
		push_style(container_style);
		create_container(400, WINDOW_HEIGHT);
		pop_style();
		
		create_text("Platformer Game", TEXT_SIZE::TITLE);

		create_text("Play");
		if (app.clicked) {
			create_text("Settings");
		}
		create_text("Credits");
		create_text("Quit");

		end_container();
		end_panel();

		autolayout_hierarchy();

		render_ui();

	} else {
		// regular stuff
		draw_quad_renders(app);
	}


	SDL_GL_SwapWindow(app.window);
}
