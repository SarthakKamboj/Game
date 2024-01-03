#include "renderer.h"
#include "glad/glad.h"
#include <vector>
#include "renderer/basic/shape_renders.h"
#include "constants.h"
#include "ui/ui.h"
#include <iostream>
#include "utils/io.h"
#include "input/input.h"
#include "audio/audio.h"

extern input::user_input_t input_state;

static bool ui_updated = true;

void render_infos(info_pair_t* pairs, int num_pairs) {
	style_t pair_style;
	pair_style.content_spacing = 20;
	pair_style.display_dir = DISPLAY_DIR::HORIZONTAL;
	pair_style.horizontal_align_val = ALIGN::CENTER;
	pair_style.vertical_align_val = ALIGN::CENTER;

	style_t dark_text_style;
	glm::vec3 dark_blue = DARK_BLUE;
	dark_text_style.color = dark_blue;
	// dark_text_style.background_color = RED;

	style_t right_text_style;
	right_text_style.color = create_color(111, 111, 111);

	style_t info_pairs_container_style;
	info_pairs_container_style.display_dir = DISPLAY_DIR::VERTICAL;
	info_pairs_container_style.horizontal_align_val = ALIGN::CENTER;
	info_pairs_container_style.vertical_align_val = ALIGN::CENTER;
	info_pairs_container_style.content_spacing = 20;
	// info_pairs_container_style.background_color = GREEN;

	style_t col_style;
	col_style.display_dir = DISPLAY_DIR::HORIZONTAL;
	col_style.horizontal_align_val = ALIGN::CENTER;
	col_style.vertical_align_val = ALIGN::CENTER;
	col_style.content_spacing = 20;
	// col_style.background_color = GREEN;

	push_style(info_pairs_container_style);
	create_container(1.f, 200, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::FIT_CONTENT, "col info");
	pop_style();

 #if 1	

	for (int i = 0; i < num_pairs; i++) {
		info_pair_t& credit_pair = pairs[i];

		push_style(col_style);
		create_container(1.f, 0.f, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::FIT_CONTENT, "");
		pop_style();

		style_t left_col_style;
		left_col_style.display_dir = DISPLAY_DIR::HORIZONTAL;
		left_col_style.horizontal_align_val = ALIGN::END;
		left_col_style.vertical_align_val = ALIGN::CENTER;
		// left_col_style.background_color = RED;
		push_style(left_col_style);
		create_container(.5f, 0.f, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::FIT_CONTENT, "");
		pop_style();

		push_style(dark_text_style);
		create_text(credit_pair.left);
		pop_style();

		end_container();

		style_t right_col_style;
		right_col_style.display_dir = DISPLAY_DIR::HORIZONTAL;
		right_col_style.horizontal_align_val = ALIGN::START;
		right_col_style.vertical_align_val = ALIGN::CENTER;
		// right_col_style.background_color = BLUE;
		push_style(right_col_style);
		create_container(.5f, 0.f, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::FIT_CONTENT, "");
		pop_style();

		push_style(right_text_style);
		create_text(credit_pair.right);
		pop_style();

		end_container();

		end_container();	
	}	

 #else
	push_style(left_col_style);
	create_container(.4f, 1.f, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "left col roles");
	pop_style();

	for (int i = 0; i < num_pairs; i++) {
		info_pair_t& credit_pair = pairs[i];
		push_style(dark_text_style);
		create_text(credit_pair.left);
		pop_style();
	}	

	end_container();

	push_style(right_col_style);
	create_container(.4f, 1.f, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "right col roles");
	pop_style();

	for (int i = 0; i < num_pairs; i++) {
		info_pair_t& credit_pair = pairs[i];
		push_style(right_text_style);
		create_text(credit_pair.right);
		pop_style();
	}	

	end_container();
#endif

	end_container();
}

void render(application_t& app) {
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

	glm::vec3 dark_blue = DARK_BLUE;

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

		style_t enabled_text_style;
		enabled_text_style.color = dark_blue;
		enabled_text_style.padding = glm::vec2(25, 10);

		style_t disabled_text_style;
		disabled_text_style.color = create_color(211,211,211);
		disabled_text_style.padding = glm::vec2(25, 10);

		push_style(enabled_text_style);
		create_text("RESOLUTION");
		create_text("RESOLUTION OPTIONS");
		pop_style();

		push_style(app.sound_fx_muted ? disabled_text_style : enabled_text_style);
		if (create_button("SOUND")) {
			if (app.sound_fx_muted) {
				unmute_sounds();
			} else {
				mute_sounds();
			}
			app.sound_fx_muted = !app.sound_fx_muted;
			ui_updated = true;			
		}
		pop_style();

		push_style(app.bck_muted ? disabled_text_style : enabled_text_style);
		if (create_button("MUSIC")) {
			if (app.bck_muted) {
				unmute_bck_sound();
			} else {
				mute_bck_sound();
			}
			app.bck_muted = !app.bck_muted;
			ui_updated = true;
		}
		pop_style();

		end_container();

		push_style(col_style);
		create_container(0.f, 0.f, WIDGET_SIZE::FIT_CONTENT, WIDGET_SIZE::FIT_CONTENT, "right column text");
		pop_style();

		if (!app.is_full_screen) {
			push_style(disabled_text_style);
		} else {
			push_style(enabled_text_style);
		}
		if (create_button("WINDOWED")) {
			if (app.is_full_screen) {
				SDL_SetWindowFullscreen(app.window, 0);
			} else {
				SDL_SetWindowFullscreen(app.window, SDL_WINDOW_FULLSCREEN);
			}
			app.is_full_screen = !app.is_full_screen;
			ui_updated = true;
		}
		pop_style();

		push_style(enabled_text_style);
		create_text("V-SYNC");
		if (create_button("CREDITS")) {
			ui_updated = true;
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = CREDITS_LEVEL;
		}
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
	} else if (app.scene_manager.cur_level == CREDITS_LEVEL) {
				
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

		style_t top_bar;
		top_bar.background_color = dark_blue;
		top_bar.content_spacing = 0;
		top_bar.display_dir = DISPLAY_DIR::HORIZONTAL;
		top_bar.horizontal_align_val = ALIGN::START;
		top_bar.vertical_align_val = ALIGN::CENTER;
		push_style(top_bar);
		create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "bottom bar");

		style_t options_btn_style;
		options_btn_style.background_color = create_color(255, 255, 255);
		options_btn_style.border_radius = 10.f;
		options_btn_style.color = dark_blue;
		options_btn_style.padding = glm::vec2(10);
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		if (create_button("Back")) {
			app.scene_manager.queue_level_load = true;	
			app.scene_manager.level_to_load = SETTINGS_LEVEL;
			ui_updated = true;
		}
		pop_style();

		end_container();
		pop_style();

		style_t main_section_style;
		main_section_style.background_color = create_color(255, 255, 255);
		main_section_style.display_dir = DISPLAY_DIR::VERTICAL;
		main_section_style.horizontal_align_val = ALIGN::CENTER;
		main_section_style.vertical_align_val = ALIGN::CENTER;
		push_style(main_section_style);
		create_container(1.f, main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "main section");

		style_t dark_text_style;
		dark_text_style.color = dark_blue;
  		dark_text_style.margin.y = 10;
		// dark_text_style.background_color = RED;

		push_style(dark_text_style);
		create_text("ROLES", TEXT_SIZE::TITLE);
		pop_style();

		info_pair_t roles_pairs[3] = {
			{"Sarthak Kamboj", "Programmer"},
			{"Sarthak Kamboj", "Designer"},
			{"Sarthak Kamboj", "Art"}
		};
		render_infos(roles_pairs, 3);

		push_style(dark_text_style);
		create_text("GAME FEATURES", TEXT_SIZE::TITLE);
		pop_style();

		info_pair_t feature_pairs[4] = {
			{"Game Engine", "Custom"},
			{"Programming Language", "C++"},
			{"Windowing", "SDL"},
			{"Audio", "OpenAL"}
		};
		render_infos(feature_pairs, 4);

		// for (int i = 0; i < 3; i++) {
		// 	info_pair_t& feature_pair = feature_pairs[i];

		// 	// credit_pair_style.background_color = create_color(255 * (i/3.f), 0, 0);
		// 	push_style(pair_style);
		// 	create_container(0.f, 0.f, WIDGET_SIZE::FIT_CONTENT, WIDGET_SIZE::FIT_CONTENT, "credit pair");

		// 	push_style(dark_text_style);
		// 	create_text(feature_pair.left);
		// 	pop_style();

		// 	push_style(right_text_style);
		// 	create_text(feature_pair.right);
		// 	pop_style();

		// 	end_container();
		// 	pop_style();

		// }	

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
