#include "renderer.h"
#include "glad/glad.h"
#include <vector>
#include "renderer/basic/shape_renders.h"
#include "constants.h"
#include "ui/ui.h"
#include <iostream>
#include "utils/io.h"

void render(application_t& app) {
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

	// main menu
	if (app.scene_manager.cur_level == MAIN_MENU_LEVEL) {
		
		static bool first = true;
		static int tex_handle = -1;
		start_of_frame(first);
		if (first) {
			char resources_path[256]{};
			io::get_resources_folder_path(resources_path);
			char title_img_path[256]{};
			sprintf(title_img_path, "%s\\%s\\character\\idle\\0001.png", resources_path, ART_FOLDER);
			tex_handle = create_texture(title_img_path , 1);
			first = false;
		}
		
		glm::vec3 dark_blue = create_color(1, 29, 45);
		
		style_t panel_style;
		panel_style.display_dir = DISPLAY_DIR::VERTICAL;
		panel_style.float_val = FLOAT::END;
		push_style(panel_style);
		create_panel("main menu panel");
		pop_style();

#if 1

		float main_section_height_percent = 0.85f;

		style_t main_section_style;
		main_section_style.background_color = create_color(255, 255, 255);
		main_section_style.content_spacing = 20;
		main_section_style.display_dir = DISPLAY_DIR::VERTICAL;
		main_section_style.float_val = FLOAT::CENTER;
		push_style(main_section_style);
		create_container(1.f, main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED);

		style_t title_container;
		title_container.display_dir = DISPLAY_DIR::HORIZONTAL;
		title_container.float_val = FLOAT::CENTER;
		title_container.content_spacing = 50;
		push_style(title_container);
		create_container(1.f, 0.f, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::FIT_CONTENT);

		create_image_container(tex_handle, app.window_height * 0.5f, app.window_height * 0.5f, WIDGET_SIZE::PIXEL_BASED, WIDGET_SIZE::PIXEL_BASED);

		style_t text_container_style;
		text_container_style.display_dir = DISPLAY_DIR::VERTICAL;
		text_container_style.float_val = FLOAT::CENTER;
		push_style(text_container_style);
		create_container(0.f, 1.f, WIDGET_SIZE::FIT_CONTENT, WIDGET_SIZE::PARENT_PERCENT_BASED);
		pop_style();

		style_t text_style;
		text_style.color = dark_blue;
		push_style(text_style);
		create_text("Night Run", TEXT_SIZE::TITLE);
		pop_style();

		end_container();

		end_container();
		pop_style();

		end_container();
		pop_style();

		style_t bottom_bar;
		bottom_bar.background_color = dark_blue;
		bottom_bar.content_spacing = 20;
		bottom_bar.display_dir = DISPLAY_DIR::HORIZONTAL;
		bottom_bar.float_val = FLOAT::SPACED_OUT;
		push_style(bottom_bar);
		create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED);

		style_t options_btn_style;
		options_btn_style.background_color = create_color(255, 255, 255);
		options_btn_style.border_radius = 10.f;
		push_style(options_btn_style);
		create_button("Settings");
		create_button("Quit");
		pop_style();

		end_container();
		pop_style();
#else
		style_t container_style;
		container_style.display_dir = DISPLAY_DIR::VERTICAL;
		container_style.float_val = FLOAT::START;
		push_style(container_style);
		create_container(0.7f, 1.f, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED);
		pop_style();
		
		float container_height = 0.35f;
		style_t title_container_style;
		title_container_style.display_dir = DISPLAY_DIR::VERTICAL;
		title_container_style.float_val = FLOAT::END;
		push_style(title_container_style);
		create_container(1.f, container_height, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED);

		style_t title_style;
		title_style.background_color = create_color(255, 60, 54);
		title_style.padding = glm::vec2(10);
		title_style.border_radius = 15.f;
		push_style(title_style);
		create_text("Platformer Game", TEXT_SIZE::TITLE);
		pop_style();

		end_container();
		pop_style();

		style_t options_style;
		options_style.display_dir = DISPLAY_DIR::VERTICAL;
		options_style.float_val = FLOAT::CENTER;
		options_style.content_spacing = 10.f;
		push_style(options_style);
		create_container(1.f, 0.55f, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED);

		style_t btn_style;
		btn_style.background_color = create_color(48, 128, 255);
		btn_style.padding = glm::vec2(10, 10);
		const float max_radius = 20.f;
		btn_style.border_radius = fmod(platformer::time_t::cur_time * 15.f, max_radius * 2);
		if (btn_style.border_radius > max_radius) {
			btn_style.border_radius = max_radius - (btn_style.border_radius - max_radius);
		}
		btn_style.border_radius = 15.f;
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
#endif
		end_panel();

		autolayout_hierarchy();

		render_ui();

	} else if (app.scene_manager.cur_level == GAME_OVER_SCREEN_LEVEL) {

		static bool first = true;
		start_of_frame(first);
		first = false;

		style_t panel_style;
		panel_style.display_dir = DISPLAY_DIR::VERTICAL;
		panel_style.float_val = FLOAT::CENTER;
		push_style(panel_style);
		create_panel("main menu panel");
		pop_style();	

		style_t container_style;
		container_style.float_val = FLOAT::CENTER;
		container_style.display_dir = DISPLAY_DIR::HORIZONTAL;
		push_style(container_style);
		create_container(1.f, 0.f, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::FIT_CONTENT);
		pop_style();

		style_t title_style;
		title_style.background_color = create_color(255, 60, 54);
		title_style.padding = glm::vec2(10);
		title_style.border_radius = 15.f;
		push_style(title_style);
		create_text("GAME OVER", TEXT_SIZE::TITLE);
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
