#include "renderer.h"
#include "glad/glad.h"
#include <vector>
#include "renderer/basic/shape_renders.h"
#include "constants.h"
#include "ui/ui.h"
#include <iostream>

void render(application_t& app) {
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	static bool first = true;
	start_of_frame(first);
	first = false;

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
		create_container(0.7f, 1.f, WIDGET_SIZE::PARENT_PERCENT_BASED);
		pop_style();
		
		style_t title_container_style;
		title_container_style.display_dir = DISPLAY_DIR::VERTICAL;
		title_container_style.float_val = FLOAT::END;
		title_container_style.content_spacing = 10.f;
		push_style(title_container_style);
		create_container(1.f, 0.4f, WIDGET_SIZE::PARENT_PERCENT_BASED);
		create_text("Platformer Game", TEXT_SIZE::TITLE);
		end_container();
		pop_style();

		style_t options_style;
		options_style.display_dir = DISPLAY_DIR::VERTICAL;
		options_style.float_val = FLOAT::START;
		options_style.content_spacing = 10.f;
		push_style(options_style);
		create_container(1.f, 0.6f, WIDGET_SIZE::PARENT_PERCENT_BASED);

		style_t btn_style;
		btn_style.background_color = create_color(48, 128, 255);
		push_style(btn_style);
		if (create_button("Play")) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = 1;
		}
		create_text("Settings");
		create_text("Credits");
		if (create_button("Quit")) {
			app.running = false;
		}
		pop_style();
		end_container();
		pop_style();

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
