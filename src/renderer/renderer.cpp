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

bool ui_updated = true;

static aspect_ratio_t aspect_ratios[5] = {
	aspect_ratio_t {
		ASPECT_RATIO::A_1920x1080,
		"1920 x 1080",
		1920,
		1080
	},
	aspect_ratio_t {
		ASPECT_RATIO::A_1600x900,
		"1600 x 900",
		1600,
		900
	},
	aspect_ratio_t {
		ASPECT_RATIO::A_1440x990,
		"1440 x 990",
		1440,
		990
	},
	aspect_ratio_t {
		ASPECT_RATIO::A_1366x768,
		"1366 x 768",
		1366,
		768
	},
	aspect_ratio_t {
		ASPECT_RATIO::A_1280x1024,
		"1280 x 1024",
		1280,
		1024
	}
};

bool something_changed(settings_changed_t settings_changed) {
	return settings_changed.aspect_ratio || settings_changed.bck_music || settings_changed.sound_fx || settings_changed.windowed;
}

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

	ui_updated = ui_updated || app.controller_state_changed;

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
		main_section_style.background_color = WHITE;
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
		options_btn_style.background_color = WHITE;
		options_btn_style.border_radius = 10.f;
		options_btn_style.color = dark_blue;
		options_btn_style.padding = glm::vec2(10);
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		bool btn_clicked = false;
		bool change_to_settings = false;
		if (app.game_controller) {
			create_text("Settings (X)");
			change_to_settings = input_state.controller_x_pressed;
		} else if (create_button("Settings")) {
			change_to_settings = true;
		}
		
		if (change_to_settings) {
			btn_clicked = true;
			ui_updated = true;
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = SETTINGS_LEVEL;
		}

		bool change_to_level1 = false;
		if (app.game_controller) {
			create_text("Play (A)");
			if (input_state.controller_a_pressed) {
				change_to_level1 = true;
			}
		} else {
			create_text("Play (Enter)");
			change_to_level1 = input_state.enter_pressed;
		}

		if (change_to_level1) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = 1;
		}

		bool change_to_quit = false;
		if (app.game_controller) {
			create_text("Quit (B)");
			change_to_quit = input_state.controller_b_pressed;
		} else if (create_button("Quit")) {
			change_to_quit = true;
		}
	
		if (change_to_quit) {
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

		// if (!btn_clicked && input_state.some_key_pressed) {
		// 	app.scene_manager.queue_level_load = true;
		// 	app.scene_manager.level_to_load = 1;
		// }

	} else if (app.scene_manager.cur_level == SETTINGS_LEVEL) {
		static settings_changed_t settings_changed;
		static bool_settings_state_t settings_state;

		if (app.new_level_just_loaded) {
			settings_changed = settings_changed_t();

			settings_state.bck_muted = app.bck_muted;
			settings_state.is_full_screen = app.is_full_screen;
			settings_state.sound_fx_muted = app.sound_fx_muted;
		}

		start_of_frame(ui_updated);
		ui_updated = false;


		style_t panel_style;
		panel_style.display_dir = DISPLAY_DIR::VERTICAL;
		panel_style.vertical_align_val = ALIGN::CENTER;
		panel_style.horizontal_align_val = ALIGN::CENTER;
		panel_style.background_color = WHITE;
		push_style(panel_style);
		create_panel("main menu panel");
		pop_style();

		float main_section_height_percent = 0.85f;
		// static bool something_changed = false;

		style_t main_section_style;	
		main_section_style.background_color = WHITE;
		main_section_style.display_dir = DISPLAY_DIR::VERTICAL;
		main_section_style.horizontal_align_val = ALIGN::CENTER;
		main_section_style.vertical_align_val = ALIGN::CENTER;
		main_section_style.content_spacing = 50;
		// main_section_style.background_color = RED;
		push_style(main_section_style);
		create_container(0.9f, main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "main section");
		pop_style();

		style_t options_sections_style;
		options_sections_style.content_spacing = 50;
		options_sections_style.display_dir = DISPLAY_DIR::HORIZONTAL;
		options_sections_style.horizontal_align_val = ALIGN::CENTER;
		options_sections_style.vertical_align_val = ALIGN::CENTER;
		push_style(options_sections_style);
		create_container(1.f, main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::FIT_CONTENT, "options section");
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
		disabled_text_style.color = GREY;
		disabled_text_style.padding = glm::vec2(25, 10);

		push_style(enabled_text_style);
		static int selected_option = 0;
		const char* options[5] = {
			aspect_ratios[0].str,
			aspect_ratios[1].str,
			aspect_ratios[2].str,
			aspect_ratios[3].str,
			aspect_ratios[4].str
		};

		if (create_selector(selected_option, options, 5, 200.f, 20.f, selected_option)) {
			ui_updated = true;
			aspect_ratio_t& ratio = aspect_ratios[selected_option];
			settings_changed.aspect_ratio = (ratio.width != app.window_width || ratio.height != app.window_height);
		}

		pop_style();

		push_style(settings_state.sound_fx_muted ? disabled_text_style : enabled_text_style);
		if (create_button("SOUND")) {
			settings_changed.sound_fx = !settings_changed.sound_fx;
			settings_state.sound_fx_muted = !settings_state.sound_fx_muted;
			ui_updated = true;			
		}
		pop_style();

		push_style(settings_state.bck_muted ? disabled_text_style : enabled_text_style);
		if (create_button("MUSIC")) {
			settings_changed.bck_music = !settings_changed.bck_music;
			settings_state.bck_muted = !settings_state.bck_muted;
			ui_updated = true;
		}
		pop_style();

		end_container();

		push_style(col_style);
		create_container(0.f, 0.f, WIDGET_SIZE::FIT_CONTENT, WIDGET_SIZE::FIT_CONTENT, "right column text");
		pop_style();

		push_style(settings_state.is_full_screen ? disabled_text_style : enabled_text_style);
		if (create_button("WINDOWED")) {
			settings_changed.windowed = !settings_changed.windowed;
			settings_state.is_full_screen = !settings_state.is_full_screen;
			ui_updated = true;
		}
		pop_style();

		push_style(enabled_text_style);
		if (create_button("CREDITS")) {
			ui_updated = true;
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = CREDITS_LEVEL;
		}
		create_text("RESET GAME");
		pop_style();

		end_container();
		end_container();

		bool changed = something_changed(settings_changed);

		style_t btn_style;
		btn_style.color = WHITE;
		btn_style.background_color = changed ? DARK_BLUE : GREY;
		btn_style.border_radius = 10.f;
		btn_style.padding = glm::vec2(10);
		push_style(btn_style);
		if (create_button("Save") && changed) {
			
			if (settings_changed.windowed) {
				if (app.is_full_screen) {
					SDL_SetWindowFullscreen(app.window, 0);
				} else {
					SDL_SetWindowFullscreen(app.window, SDL_WINDOW_FULLSCREEN);
				}
				app.is_full_screen = !app.is_full_screen;
			}

			if (settings_changed.bck_music) {
				if (app.bck_muted) {
					unmute_bck_sound();
				} else {
					mute_bck_sound();
				}
				app.bck_muted = !app.bck_muted;
			}

			if (settings_changed.sound_fx) {
				if (app.sound_fx_muted) {
					unmute_sounds();
				} else {
					mute_sounds();
				}
				app.sound_fx_muted = !app.sound_fx_muted;
			}

			if (settings_changed.aspect_ratio) {
				aspect_ratio_t& aspect_ratio = aspect_ratios[selected_option];
				SDL_SetWindowSize(app.window, aspect_ratio.width, aspect_ratio.height);
			}

			settings_changed = settings_changed_t();
		}
		pop_style();

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
		options_btn_style.background_color = WHITE;
		options_btn_style.border_radius = 10.f;
		options_btn_style.color = dark_blue;
		options_btn_style.padding = glm::vec2(10);
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		bool main_menu_load = false;
		if (app.game_controller) {
			create_text("Back (B)");
			if (input_state.controller_b_pressed) {
				main_menu_load = true;
			}
		} else if (create_button("Back")) {
			main_menu_load = true;
		}

		if (main_menu_load) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = MAIN_MENU_LEVEL;
			ui_updated = true;
			settings_changed = settings_changed_t();
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
		main_section_style.background_color = WHITE;
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
		options_btn_style.background_color = WHITE;
		options_btn_style.border_radius = 10.f;
		options_btn_style.color = dark_blue;
		options_btn_style.padding = glm::vec2(10);
		push_style(options_btn_style);
		
		if (app.game_controller) {
			create_text("Yes (X)");
			if (input_state.controller_x_pressed) {
				app.running = false;
			}
		} else if (create_button("Yes")) {
			app.running = false;	
		}
		pop_style();

		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);

		bool main_menu_load = false;
		if (app.game_controller) {
			create_text("No (B)");
			if (input_state.controller_b_pressed) {
				main_menu_load = true;
			}
		} else if (create_button("No")) {
			main_menu_load = true;
		}

		if (main_menu_load) {
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
		options_btn_style.background_color = WHITE;
		options_btn_style.border_radius = 10.f;
		options_btn_style.color = dark_blue;
		options_btn_style.padding = glm::vec2(10);
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		bool back_pressed = false;
		if (app.game_controller) {
			create_text("Back (B)");
			if (input_state.controller_b_pressed) {
				back_pressed = true;
			}
		} else if (create_button("Back")) {
			back_pressed = true;
		}

		if (back_pressed) {
			app.scene_manager.queue_level_load = true;	
			app.scene_manager.level_to_load = SETTINGS_LEVEL;
			ui_updated = true;
		}
		pop_style();

		end_container();
		pop_style();

		style_t main_section_style;
		main_section_style.background_color = WHITE;
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
		panel_style.vertical_align_val = ALIGN::CENTER;
		panel_style.horizontal_align_val = ALIGN::CENTER;
		push_style(panel_style);
		create_panel("main panel");
		pop_style();

		float main_section_height_percent = 0.85f;

		style_t main_section_style;
		main_section_style.background_color = WHITE;
		main_section_style.content_spacing = 30;
		main_section_style.display_dir = DISPLAY_DIR::VERTICAL;
		main_section_style.horizontal_align_val = ALIGN::CENTER;
		main_section_style.vertical_align_val = ALIGN::CENTER;
		push_style(main_section_style);
		create_container(1.f, main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "main section");

		style_t text_style;
		text_style.color = dark_blue;
		push_style(text_style);
		create_text("GAME OVER", TEXT_SIZE::TITLE);
		create_text("THANK YOU FOR PLAYING!");
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
		options_btn_style.background_color = WHITE;
		options_btn_style.border_radius = 10.f;
		options_btn_style.color = dark_blue;
		options_btn_style.padding = glm::vec2(10);
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		bool go_to_main_menu = false;
		if (app.game_controller) {
			create_text("Continue (X)");
			if (input_state.controller_x_pressed) {
				go_to_main_menu = true;
			}
		} else if (create_button("Continue")) {
			go_to_main_menu = true;
		}
		pop_style();

		if (go_to_main_menu) {
			ui_updated = true;
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = MAIN_MENU_LEVEL;
		}

		end_container();
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
