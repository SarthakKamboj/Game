#include "renderer.h"
#include "glad/glad.h"
#include <vector>
#include "renderer/basic/shape_renders.h"
#include "constants.h"
#include "ui/ui.h"
#include <iostream>
#include "utils/io.h"
#include "input/input.h"

extern input::user_input_t input_state;

static bool ui_updated = true;

void render(application_t& app) {
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

	glm::vec3 dark_blue = create_color(1, 29, 45);

	// main menu
	if (app.scene_manager.cur_level == MAIN_MENU_LEVEL) {
		
		start_of_frame(ui_updated);
		ui_updated = false;
		
		static int tex_handle = -1;
		static bool first = true;
		if (first) {
			char resources_path[256]{};
			io::get_resources_folder_path(resources_path);
			char title_img_path[256]{};
			sprintf(title_img_path, "%s\\%s\\character\\idle\\0001.png", resources_path, ART_FOLDER);
			tex_handle = create_texture(title_img_path , 1);
			first = false;
		}
			
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
		main_section_style.content_spacing = 30;
		main_section_style.display_dir = DISPLAY_DIR::HORIZONTAL;
		main_section_style.horizontal_align_val = ALIGN::CENTER;
		main_section_style.vertical_align_val = ALIGN::CENTER;
		push_style(main_section_style);
		create_container(1.f, main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "main section");

		create_image_container(tex_handle, app.window_height * 0.5f, app.window_height * 0.5f, WIDGET_SIZE::PIXEL_BASED, WIDGET_SIZE::PIXEL_BASED, "character image");

		style_t text_style;
		text_style.color = dark_blue;
		push_style(text_style);
		create_text("Night Run", TEXT_SIZE::TITLE);
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
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		bool btn_clicked = false;
		if (create_button("Settings")) {
			btn_clicked = true;
			ui_updated = true;
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = SETTINGS_LEVEL;
		}
		if (create_button("Quit")) {
			// app.running = false;
			btn_clicked = true;
			ui_updated = true;
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = QUIT_LEVEL;
		}
		pop_style();

		end_container();
		pop_style();

		end_panel();

		autolayout_hierarchy();
		render_ui();

		if (!btn_clicked && input_state.some_key_pressed) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = 1;
		}

	} else if (app.scene_manager.cur_level == SETTINGS_LEVEL) {
		start_of_frame(ui_updated);
		ui_updated = false;

		style_t panel_style;
		panel_style.display_dir = DISPLAY_DIR::VERTICAL;
		panel_style.vertical_align_val = ALIGN::CENTER;
		panel_style.horizontal_align_val = ALIGN::CENTER;
		panel_style.background_color = create_color(255, 255, 255);
		push_style(panel_style);
		create_panel("main menu panel");
		pop_style();

		float main_section_height_percent = 0.85f;

		style_t main_section_style;
		main_section_style.background_color = create_color(255, 255, 255);
		main_section_style.content_spacing = 50;
		main_section_style.display_dir = DISPLAY_DIR::HORIZONTAL;
		main_section_style.horizontal_align_val = ALIGN::CENTER;
		main_section_style.vertical_align_val = ALIGN::CENTER;
		push_style(main_section_style);
		create_container(0.9f, main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "main section");
		pop_style();

		style_t col_style;
		col_style.display_dir = DISPLAY_DIR::VERTICAL;
		col_style.horizontal_align_val = ALIGN::START;
		col_style.vertical_align_val = ALIGN::SPACE_BETWEEN;
		col_style.content_spacing = 30;
		push_style(col_style);
		create_container(0.f, 0.f, WIDGET_SIZE::FIT_CONTENT, WIDGET_SIZE::FIT_CONTENT, "left column text");
		pop_style();

		style_t text_style;
		text_style.color = dark_blue;
		push_style(text_style);
		create_text("RESOLUTION");
		create_text("RESOLUTION OPTIONS");
		create_text("SOUND");
		create_text("MUSIC");
		pop_style();

		end_container();

		push_style(col_style);
		create_container(0.f, 0.f, WIDGET_SIZE::FIT_CONTENT, WIDGET_SIZE::FIT_CONTENT, "right column text");
		pop_style();

		push_style(text_style);
		create_text("WINDOWED");
		create_text("V-SYNC");
		create_text("CREDITS");
		create_text("RESET GAME");
		pop_style();

		end_container();

		end_container();

		style_t bottom_bar;
		bottom_bar.background_color = dark_blue;
		bottom_bar.content_spacing = 0;
		bottom_bar.display_dir = DISPLAY_DIR::HORIZONTAL;
		bottom_bar.horizontal_align_val = ALIGN::START;
		bottom_bar.vertical_align_val = ALIGN::CENTER;
		push_style(bottom_bar);
		create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "bottom bar");
		pop_style();

		style_t options_btn_style;
		options_btn_style.background_color = create_color(255, 255, 255);
		options_btn_style.border_radius = 10.f;
		options_btn_style.color = dark_blue;
		options_btn_style.padding = glm::vec2(10);
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		if (create_button("Back")) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = MAIN_MENU_LEVEL;
			ui_updated = true;
		}
		pop_style();

		end_container();

		end_panel();

		autolayout_hierarchy();
		render_ui();

	} else if (app.scene_manager.cur_level == QUIT_LEVEL) {
		
		start_of_frame(ui_updated);
		ui_updated = false;
		
		style_t panel_style;
		panel_style.display_dir = DISPLAY_DIR::VERTICAL;
		panel_style.vertical_align_val = ALIGN::CENTER;
		panel_style.horizontal_align_val = ALIGN::CENTER;
		push_style(panel_style);
		create_panel("main panel");
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

		style_t text_style;
		text_style.color = dark_blue;
		push_style(text_style);
		create_text("ARE YOU SURE YOU WANT TO QUIT?");
		pop_style();

		end_container();
		pop_style();

		style_t bottom_bar;
		bottom_bar.background_color = dark_blue;
		bottom_bar.content_spacing = 0;
		bottom_bar.display_dir = DISPLAY_DIR::HORIZONTAL;
		bottom_bar.horizontal_align_val = ALIGN::END;
		bottom_bar.vertical_align_val = ALIGN::CENTER;
		push_style(bottom_bar);
		create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "bottom bar");

		style_t options_btn_style;
		options_btn_style.background_color = create_color(255, 255, 255);
		options_btn_style.border_radius = 10.f;
		options_btn_style.color = dark_blue;
		options_btn_style.padding = glm::vec2(10);
		push_style(options_btn_style);
		
		bool btn_clicked = false;
		if (create_button("Yes")) {
			app.running = false;	
		}
		pop_style();

		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		if (create_button("No")) {
			ui_updated = true;
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = MAIN_MENU_LEVEL;
		}
		pop_style();

		end_container();
		pop_style();

		end_panel();

		autolayout_hierarchy();
		render_ui();
	} else if (app.scene_manager.cur_level == GAME_OVER_SCREEN_LEVEL) {
		start_of_frame(ui_updated);
		ui_updated = false;

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
