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
		panel_style.vertical_align_val = ALIGN::CENTER;
		panel_style.horizontal_align_val = ALIGN::CENTER;
		push_style(panel_style);
		create_panel("main menu panel");
		pop_style();

		float main_section_height_percent = 0.85f;

		style_t main_section_style;
		main_section_style.background_color = create_color(255, 255, 255);
		main_section_style.content_spacing = 0;
		main_section_style.display_dir = DISPLAY_DIR::HORIZONTAL;
		main_section_style.horizontal_align_val = ALIGN::CENTER;
		main_section_style.vertical_align_val = ALIGN::CENTER;
		push_style(main_section_style);
		create_container(1.f, main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "main section");

		create_image_container(tex_handle, app.window_height * 0.5f, app.window_height * 0.5f, WIDGET_SIZE::PIXEL_BASED, WIDGET_SIZE::PIXEL_BASED, "character image");

		style_t text_style;
		text_style.color = dark_blue;
		text_style.background_color = create_color(255, 0, 0);
		push_style(text_style);
		create_button("Night Run", TEXT_SIZE::TITLE);
		pop_style();

		end_container();
		pop_style();

		style_t bottom_bar;
		bottom_bar.background_color = dark_blue;
		bottom_bar.content_spacing = 0;
		bottom_bar.display_dir = DISPLAY_DIR::HORIZONTAL;
		bottom_bar.horizontal_align_val = ALIGN::SPACE_BETWEEN;
		bottom_bar.vertical_align_val = ALIGN::CENTER;
		push_style(bottom_bar);
		create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "bottom bar");

		style_t options_btn_style;
		options_btn_style.background_color = create_color(255, 255, 255);
		options_btn_style.border_radius = 10.f;
		options_btn_style.color = dark_blue;
		options_btn_style.padding = glm::vec2(10);
		push_style(options_btn_style);
		create_button("Settings");
		create_button("Quit");
		pop_style();

		end_container();
		pop_style();

		end_panel();

		autolayout_hierarchy();

		render_ui();

	} else if (app.scene_manager.cur_level == GAME_OVER_SCREEN_LEVEL) {

		static bool first = true;
		start_of_frame(first);
		first = false;

		style_t panel_style;
		panel_style.display_dir = DISPLAY_DIR::VERTICAL;
		panel_style.horizontal_align_val = ALIGN::CENTER;
		panel_style.vertical_align_val = ALIGN::CENTER;
		push_style(panel_style);
		create_panel("main menu panel");
		pop_style();	

		style_t title_style;
		title_style.background_color = create_color(255, 60, 54);
		title_style.padding = glm::vec2(10);
		title_style.border_radius = 15.f;
		push_style(title_style);
		create_text("GAME OVER", TEXT_SIZE::TITLE);
		pop_style();

		end_panel();

		autolayout_hierarchy();
		render_ui();

	} else {
		// regular stuff
		draw_quad_renders(app);
	}


	SDL_GL_SwapWindow(app.window);
}
