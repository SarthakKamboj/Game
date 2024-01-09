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

static aspect_ratio_t aspect_ratios[ASPECT_RATIO::NUM_RATIOS] = {
	aspect_ratio_t {
		ASPECT_RATIO::A_1600x900,
		"1600 x 900",
		1600,
		900
	},
	aspect_ratio_t {
		ASPECT_RATIO::A_1600x1000,
		"1600 x 1000",
		1600,
		1000
	},
	aspect_ratio_t {
		ASPECT_RATIO::A_1600x1200,
		"1600 x 1200",
		1600,
		1200
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
	dark_text_style.color = DARK_BLUE;

	style_t right_text_style;
	right_text_style.color = create_color(111, 111, 111);

	style_t info_pairs_container_style;
	info_pairs_container_style.display_dir = DISPLAY_DIR::VERTICAL;
	info_pairs_container_style.horizontal_align_val = ALIGN::CENTER;
	info_pairs_container_style.vertical_align_val = ALIGN::CENTER;
	info_pairs_container_style.content_spacing = 20;

	style_t col_style;
	col_style.display_dir = DISPLAY_DIR::HORIZONTAL;
	col_style.horizontal_align_val = ALIGN::CENTER;
	col_style.vertical_align_val = ALIGN::CENTER;
	col_style.content_spacing = 20;

	push_style(info_pairs_container_style);
	create_container(1.f, 200, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::FIT_CONTENT, "col info");
	pop_style();

	for (int i = 0; i < num_pairs; i++) {
		info_pair_t& credit_pair = pairs[i];

		push_style(col_style);
		create_container(1.f, 0.f, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::FIT_CONTENT, "");
		pop_style();

		style_t left_col_style;
		left_col_style.display_dir = DISPLAY_DIR::HORIZONTAL;
		left_col_style.horizontal_align_val = ALIGN::END;
		left_col_style.vertical_align_val = ALIGN::CENTER;
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
		push_style(right_col_style);
		create_container(.5f, 0.f, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::FIT_CONTENT, "");
		pop_style();

		push_style(right_text_style);
		create_text(credit_pair.right);
		pop_style();

		end_container();

		end_container();	
	}	

	end_container();
}

void init_renderer(application_t& app) {
	for (int i = 0; i < ASPECT_RATIO::NUM_RATIOS; i++)	 {
		aspect_ratio_t& aspect_ratio = aspect_ratios[i];
		aspect_ratio.mode_index = get_mode_index(aspect_ratio.width, aspect_ratio.height);

		if (aspect_ratio.ratio == ASPECT_RATIO::A_1600x900) {
			SDL_DisplayMode mode{};
			SDL_GetDisplayMode(0, aspect_ratio.mode_index, &mode);
			int success = SDL_SetWindowDisplayMode(app.window, &mode);
			if (success != 0) {
				printf("ran into an issue with setting display mode index %i\n", aspect_ratio.mode_index);
				const char* sdlError = SDL_GetError();
				if (sdlError && sdlError[0] != '\0') {
					std::cout << "SDL Error: " << sdlError << std::endl;
					SDL_ClearError();
				}	
			}
			SDL_SetWindowSize(app.window, app.window_width, app.window_width * (aspect_ratio.height / aspect_ratio.width));
			const char* sdlError = SDL_GetError();
			if (sdlError && sdlError[0] != '\0') {
				printf("ran into an issue with setting window size after display mode\n");
				std::cout << "SDL Error: " << sdlError << std::endl;
				SDL_ClearError();
			}
		}
	}
	
}

int get_mode_index(float w, float h) {
	int num_display_modes = SDL_GetNumDisplayModes(0);
	float ratio = w / h;
	for (int i = 0; i < num_display_modes; i++) {
		SDL_DisplayMode display_mode{};
		if (SDL_GetDisplayMode(0, i, &display_mode) != 0) {
			SDL_Log("SDL_GetDisplayMode failed: %s", SDL_GetError());
		}
		// SDL_Log("Mode %i\tname: %s\t%i x %i and ratio is %f",
		// 		i,
		// 		SDL_GetPixelFormatName(display_mode.format),
		// 		display_mode.w, display_mode.h, static_cast<float>(display_mode.w) / display_mode.h);
		
		float display_ratio = static_cast<float>(display_mode.w) / display_mode.h;
		if (floor(display_ratio * 1000) == floor(ratio * 1000)) {
			return i;
		}
	}
	return -1;
}

void hs_score_to_char(float in, char* out) {
	if (in == -1.f) {
		sprintf(out, "N/A", in);
	} else {
		sprintf(out, "%.3fs", in);
	}
}

void render(application_t& app) {
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

	style_t btn_style;
	btn_style.background_color = WHITE;
	btn_style.hover_background_color = DARK_BLUE + glm::vec3(0.1f);
	btn_style.color = DARK_BLUE;
	btn_style.hover_color = WHITE;
	btn_style.border_radius = 10.f;
	btn_style.padding = glm::vec2(10);
	btn_style.margin = glm::vec2(40, 0);

	// main menu
	if (app.scene_manager.cur_level == MAIN_MENU_LEVEL) {
		
		start_of_frame();
		
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
		text_style.color = DARK_BLUE;
		push_style(text_style);
		create_text("Night Run", TEXT_SIZE::TITLE);
		pop_style();

		end_container();
		pop_style();

		style_t bottom_bar;
		bottom_bar.background_color = DARK_BLUE;
		bottom_bar.content_spacing = 0;
		bottom_bar.display_dir = DISPLAY_DIR::HORIZONTAL;
		bottom_bar.horizontal_align_val = ALIGN::SPACE_BETWEEN;
		bottom_bar.vertical_align_val = ALIGN::CENTER;
		push_style(bottom_bar);
		create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "bottom bar");

		style_t options_btn_style;
		options_btn_style.background_color = WHITE;
		options_btn_style.hover_background_color = DARK_BLUE + glm::vec3(0.1f);
		options_btn_style.border_radius = 10.f;
		options_btn_style.color = DARK_BLUE;
		options_btn_style.hover_color = WHITE;
		options_btn_style.padding = glm::vec2(10);
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		bool change_to_settings = false;
		if (create_button("Settings")) {
			change_to_settings = true;
		}
		
		if (change_to_settings) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = SETTINGS_LEVEL;
		}

		bool change_to_level1 = false;
		if (create_button("Play")) {
			change_to_level1 = true;
		}

		if (change_to_level1) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = 1;
		}

		bool change_to_quit = false;
		if (create_button("Quit")) {
			change_to_quit = true;
		}
	
		if (change_to_quit) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = QUIT_LEVEL;
		}
		pop_style();

		end_container();
		pop_style();

		end_panel();

		autolayout_hierarchy();
		render_ui();

	} else if (app.scene_manager.cur_level == SETTINGS_LEVEL) {
		static int selected_option = 0;

		static settings_changed_t settings_changed;
		static bool_settings_state_t settings_state;

		if (app.new_level_just_loaded) {
			settings_changed = settings_changed_t();

			aspect_ratio_t& ratio = aspect_ratios[selected_option];
			float cur_ratio = app.window_width / app.window_height;
			float selected_ratio = ratio.width / ratio.height;
			printf("aspect ratio int, cur: %f, selected: %f\n", cur_ratio, selected_ratio);

			settings_changed.aspect_ratio = floor(cur_ratio * 100) != floor(selected_ratio * 100);

			printf("aspect ratio int * 100, cur: %f, selected: %f\n", floor(cur_ratio * 100), floor(selected_ratio * 100));

			settings_state.bck_muted = app.bck_muted;
			settings_state.is_full_screen = app.is_full_screen;
			settings_state.sound_fx_muted = app.sound_fx_muted;
		}

		start_of_frame();

		style_t panel_style;
		panel_style.display_dir = DISPLAY_DIR::VERTICAL;
		panel_style.vertical_align_val = ALIGN::CENTER;
		panel_style.horizontal_align_val = ALIGN::CENTER;
		panel_style.background_color = WHITE;
		push_style(panel_style);
		create_panel("main menu panel");
		pop_style();

		float main_section_height_percent = 0.85f;

		style_t main_section_style;	
		main_section_style.background_color = WHITE;
		main_section_style.display_dir = DISPLAY_DIR::VERTICAL;
		main_section_style.horizontal_align_val = ALIGN::CENTER;
		main_section_style.vertical_align_val = ALIGN::CENTER;
		main_section_style.content_spacing = 50;
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
		enabled_text_style.hover_background_color = DARK_BLUE;
		enabled_text_style.hover_color = WHITE;

		enabled_text_style.color = DARK_BLUE;
		enabled_text_style.padding = glm::vec2(25, 10);

		style_t disabled_text_style;
		disabled_text_style.hover_background_color = DARK_BLUE;
		disabled_text_style.color = GREY;
		disabled_text_style.padding = glm::vec2(25, 10);

		push_style(enabled_text_style);
		const char* options[ASPECT_RATIO::NUM_RATIOS]{};

		for (int i = 0; i < ASPECT_RATIO::NUM_RATIOS; i++) {
			options[i] = aspect_ratios[i].str;
		}

		if (create_selector(selected_option, options, ASPECT_RATIO::NUM_RATIOS, 200.f, 20.f, selected_option, "aspect_ratio_selector")) {
			aspect_ratio_t& ratio = aspect_ratios[selected_option];
			float cur_ratio = app.window_width / app.window_height;
			float selected_ratio = ratio.width / ratio.height;
			printf("aspect ratio changed, cur: %f, selected: %f\n", cur_ratio, selected_ratio);

			if (floor(cur_ratio * 100) == floor(selected_ratio * 100)) {
				settings_changed.aspect_ratio = false;
			} else {
				settings_changed.aspect_ratio = (ratio.mode_index != -1);
				if (ratio.mode_index == -1) {
					printf("could not find a display mode for %s\n", ratio.str);
				}
			}
		}

		pop_style();

		push_style(settings_state.sound_fx_muted ? disabled_text_style : enabled_text_style);
		if (create_button("SOUND")) {
			settings_changed.sound_fx = !settings_changed.sound_fx;
			settings_state.sound_fx_muted = !settings_state.sound_fx_muted;
		}
		pop_style();

		push_style(settings_state.bck_muted ? disabled_text_style : enabled_text_style);
		if (create_button("MUSIC")) {
			settings_changed.bck_music = !settings_changed.bck_music;
			settings_state.bck_muted = !settings_state.bck_muted;
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
		}
		pop_style();

		push_style(enabled_text_style);
		if (create_button("CREDITS")) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = CREDITS_LEVEL;
		}
		if (create_button("HIGH SCORES")) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = HIGH_SCORES_LEVEL;
		}
		pop_style();

		end_container();
		end_container();

		bool changed = something_changed(settings_changed);

		style_t btn_style;
		btn_style.color = WHITE;
		btn_style.background_color = changed ? DARK_BLUE : GREY;
		btn_style.hover_background_color = btn_style.background_color + glm::vec3(0.1f);
		btn_style.border_radius = 10.f;
		btn_style.padding = glm::vec2(10);
		push_style(btn_style);
		if (create_button("Save") && changed) {
			
			if (settings_changed.windowed) {
				printf("full screen mode changed");
				if (app.is_full_screen) {
					SDL_SetWindowFullscreen(app.window, 0);
				} else {
					SDL_SetWindowFullscreen(app.window, SDL_WINDOW_FULLSCREEN);
				}
				app.is_full_screen = !app.is_full_screen;
			}

			if (settings_changed.bck_music) {
				printf("bck music changed");
				if (app.bck_muted) {
					unmute_bck_sound();
				} else {
					mute_bck_sound();
				}
				app.bck_muted = !app.bck_muted;
			}

			if (settings_changed.sound_fx) {
				printf("sound fx changed");
				if (app.sound_fx_muted) {
					unmute_sounds();
				} else {
					mute_sounds();
				}
				app.sound_fx_muted = !app.sound_fx_muted;
			}

			if (settings_changed.aspect_ratio) {
				printf("aspect ratio changed");
				aspect_ratio_t& aspect_ratio = aspect_ratios[selected_option];
				if (app.is_full_screen) {
					if (aspect_ratio.mode_index != -1) {
						SDL_DisplayMode mode;
						SDL_GetDisplayMode(0, aspect_ratio.mode_index, &mode);
						int success = SDL_SetWindowDisplayMode(app.window, &mode);
						if (success != 0) {
							printf("ran into an issue with setting display mode index %i\n", aspect_ratio.mode_index);
							const char* sdlError = SDL_GetError();
							if (sdlError && sdlError[0] != '\0') {
								std::cout << "SDL Error: " << sdlError << std::endl;
								SDL_ClearError();
							}	
						}
						SDL_SetWindowSize(app.window, app.window_width, app.window_width * (aspect_ratio.height / aspect_ratio.width));
						const char* sdlError = SDL_GetError();
						if (sdlError && sdlError[0] != '\0') {
							printf("ran into an issue with setting window size after display mode\n");
							std::cout << "SDL Error: " << sdlError << std::endl;
							SDL_ClearError();
						}	
					} else {
						printf("could not update because display mode index is -1\n");
					}
				} else {
					SDL_SetWindowSize(app.window, aspect_ratio.width, aspect_ratio.height);
				}
			}

			settings_changed = settings_changed_t();
		}
		pop_style();

		end_container();

		style_t bottom_bar;
		bottom_bar.background_color = DARK_BLUE;
		bottom_bar.content_spacing = 0;
		bottom_bar.display_dir = DISPLAY_DIR::HORIZONTAL;
		bottom_bar.horizontal_align_val = ALIGN::START;
		bottom_bar.vertical_align_val = ALIGN::CENTER;
		push_style(bottom_bar);
		create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "bottom bar");
		pop_style();

		style_t options_btn_style;
		options_btn_style.hover_background_color = DARK_BLUE + glm::vec3(0.2f);
		options_btn_style.hover_color = WHITE;

		options_btn_style.background_color = WHITE;
		options_btn_style.color = DARK_BLUE;

		options_btn_style.border_radius = 10.f;
		options_btn_style.padding = glm::vec2(10);
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		bool main_menu_load = false;
		if (create_button("Back")) {
			main_menu_load = true;
		}

		if (main_menu_load) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = MAIN_MENU_LEVEL;
			settings_changed = settings_changed_t();
		}
		pop_style();

		end_container();

		end_panel();

		autolayout_hierarchy();
		render_ui();

	} else if (app.scene_manager.cur_level == QUIT_LEVEL) {
		
		start_of_frame();
		
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
		text_style.color = DARK_BLUE;
		push_style(text_style);
		create_text("ARE YOU SURE YOU WANT TO QUIT?");
		pop_style();

		end_container();
		pop_style();

		style_t bottom_bar;
		bottom_bar.background_color = DARK_BLUE;
		bottom_bar.content_spacing = 0;
		bottom_bar.display_dir = DISPLAY_DIR::HORIZONTAL;
		bottom_bar.horizontal_align_val = ALIGN::END;
		bottom_bar.vertical_align_val = ALIGN::CENTER;
		push_style(bottom_bar);
		create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "bottom bar");

		style_t options_btn_style;
		options_btn_style.background_color = WHITE;
		options_btn_style.color = DARK_BLUE;
		options_btn_style.hover_background_color = DARK_BLUE + glm::vec3(0.1f);
		options_btn_style.hover_color = WHITE;
		options_btn_style.border_radius = 10.f;
		options_btn_style.padding = glm::vec2(10);
		push_style(options_btn_style);
		
		if (create_button("Yes")) {
			app.running = false;	
		}
		
		pop_style();

		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);

		bool main_menu_load = false;
		if (create_button("No")) {
			main_menu_load = true;
		}

		if (main_menu_load) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = MAIN_MENU_LEVEL;
		}
		pop_style();

		end_container();
		pop_style();

		end_panel();

		autolayout_hierarchy();
		render_ui();
	}  else if (app.scene_manager.cur_level == HIGH_SCORES_LEVEL) {
		start_of_frame();
		
		style_t panel_style;
		panel_style.display_dir = DISPLAY_DIR::VERTICAL;
		panel_style.vertical_align_val = ALIGN::CENTER;
		panel_style.horizontal_align_val = ALIGN::CENTER;
		push_style(panel_style);
		create_panel("main panel");
		pop_style();

		float main_section_height_percent = 0.85f;

		style_t top_bar;
		top_bar.background_color = DARK_BLUE;
		top_bar.content_spacing = 0;
		top_bar.display_dir = DISPLAY_DIR::HORIZONTAL;
		top_bar.horizontal_align_val = ALIGN::START;
		top_bar.vertical_align_val = ALIGN::CENTER;
		push_style(top_bar);
		create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "bottom bar");

		style_t options_btn_style;
		options_btn_style.hover_background_color = DARK_BLUE + glm::vec3(0.1f);
		options_btn_style.hover_color = WHITE;
		options_btn_style.background_color = WHITE;
		options_btn_style.color = DARK_BLUE;
		options_btn_style.border_radius = 10.f;
		options_btn_style.padding = glm::vec2(10);
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		bool back_pressed = false;
		if (create_button("Back")) {
			back_pressed = true;
		}

		if (back_pressed) {
			app.scene_manager.queue_level_load = true;	
			app.scene_manager.level_to_load = SETTINGS_LEVEL;
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
		dark_text_style.color = DARK_BLUE;
  		dark_text_style.margin.y = 10;

		push_style(dark_text_style);
		create_text("High Scores", TEXT_SIZE::TITLE);
		pop_style();

		char first[64]{};
		char second[64]{};
		char third[64]{};
		char fourth[64]{};

		hs_score_to_char(app.high_scores.times[0], first);
		hs_score_to_char(app.high_scores.times[1], second);
		hs_score_to_char(app.high_scores.times[2], third);	
		hs_score_to_char(app.high_scores.times[3], fourth);

		info_pair_t high_scores[4] = {
			{"1", first},
			{"2", second},
			{"3", third},
			{"4", fourth}
		};
		render_infos(high_scores, 4);

		end_container();
		pop_style();

		end_panel();

		autolayout_hierarchy();
		render_ui();	
	} else if (app.scene_manager.cur_level == GAME_OVER_SCREEN_LEVEL) {
			
		start_of_frame();
		
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
		text_style.color = DARK_BLUE;
		push_style(text_style);
		create_text("GAME OVER", TEXT_SIZE::TITLE);
		create_text("THANK YOU FOR PLAYING!");
		pop_style();

		end_container();
		pop_style();

		style_t bottom_bar;
		bottom_bar.background_color = DARK_BLUE;
		bottom_bar.content_spacing = 0;
		bottom_bar.display_dir = DISPLAY_DIR::HORIZONTAL;
		bottom_bar.horizontal_align_val = ALIGN::END;
		bottom_bar.vertical_align_val = ALIGN::CENTER;
		push_style(bottom_bar);
		create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "bottom bar");

		style_t options_btn_style;
		options_btn_style.background_color = WHITE;
		options_btn_style.hover_background_color = DARK_BLUE + glm::vec3(0.1f);
		options_btn_style.color = DARK_BLUE;
		options_btn_style.hover_color = WHITE;
		options_btn_style.border_radius = 10.f;
		options_btn_style.padding = glm::vec2(10);
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		bool go_to_main_menu = false;
		if (create_button("Continue")) {
			go_to_main_menu = true;
		}
		pop_style();

		if (go_to_main_menu) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = MAIN_MENU_LEVEL;
		}

		end_container();
		pop_style();

		end_panel();

		autolayout_hierarchy();
		render_ui();
	} else if (app.scene_manager.cur_level == CREDITS_LEVEL) {
				
		start_of_frame();
		
		style_t panel_style;
		panel_style.display_dir = DISPLAY_DIR::VERTICAL;
		panel_style.vertical_align_val = ALIGN::CENTER;
		panel_style.horizontal_align_val = ALIGN::CENTER;
		push_style(panel_style);
		create_panel("main panel");
		pop_style();

		float main_section_height_percent = 0.85f;

		style_t top_bar;
		top_bar.background_color = DARK_BLUE;
		top_bar.content_spacing = 0;
		top_bar.display_dir = DISPLAY_DIR::HORIZONTAL;
		top_bar.horizontal_align_val = ALIGN::START;
		top_bar.vertical_align_val = ALIGN::CENTER;
		push_style(top_bar);
		create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "bottom bar");

		style_t options_btn_style;
		options_btn_style.hover_background_color = DARK_BLUE + glm::vec3(0.1f);
		options_btn_style.hover_color = WHITE;
		options_btn_style.background_color = WHITE;
		options_btn_style.color = DARK_BLUE;
		options_btn_style.border_radius = 10.f;
		options_btn_style.padding = glm::vec2(10);
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		bool back_pressed = false;
		if (create_button("Back")) {
			back_pressed = true;
		}

		if (back_pressed) {
			app.scene_manager.queue_level_load = true;	
			app.scene_manager.level_to_load = SETTINGS_LEVEL;
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
		dark_text_style.color = DARK_BLUE;
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
			
		start_of_frame();
		
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
		text_style.color = DARK_BLUE;
		push_style(text_style);
		create_text("GAME OVER", TEXT_SIZE::TITLE);
		create_text("THANK YOU FOR PLAYING!");
		pop_style();

		end_container();
		pop_style();

		style_t bottom_bar;
		bottom_bar.background_color = DARK_BLUE;
		bottom_bar.content_spacing = 0;
		bottom_bar.display_dir = DISPLAY_DIR::HORIZONTAL;
		bottom_bar.horizontal_align_val = ALIGN::END;
		bottom_bar.vertical_align_val = ALIGN::CENTER;
		push_style(bottom_bar);
		create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "bottom bar");

		style_t options_btn_style;
		options_btn_style.background_color = WHITE;
		options_btn_style.hover_background_color = DARK_BLUE + glm::vec3(0.1f);
		options_btn_style.color = DARK_BLUE;
		options_btn_style.hover_color = WHITE;
		options_btn_style.border_radius = 10.f;
		options_btn_style.padding = glm::vec2(10);
		options_btn_style.margin = glm::vec2(40, 0);
		push_style(options_btn_style);
		
		bool go_to_main_menu = false;
		if (create_button("Continue")) {
			go_to_main_menu = true;
		}
		pop_style();

		if (go_to_main_menu) {
			app.scene_manager.queue_level_load = true;
			app.scene_manager.level_to_load = MAIN_MENU_LEVEL;
		}

		end_container();
		pop_style();

		end_panel();

		autolayout_hierarchy();
		render_ui();

	} else {
		static bool pause = false;
		if (input_state.p_pressed || input_state.controller_start_pressed) {
			app.paused = !app.paused;
		}

		if (app.paused) {
			start_of_frame();
			
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
			text_style.color = DARK_BLUE;
			text_style.margin.y = 20;
			push_style(text_style);
			create_text("PAUSED", TEXT_SIZE::TITLE);
			pop_style();
			push_style(btn_style);
			if (create_button("Continue")) {
				app.paused = false;	
			}
			if (create_button("Main Menu")) {
				app.scene_manager.level_to_load = MAIN_MENU_LEVEL;	
				app.scene_manager.queue_level_load = true;
				app.paused = false;
				resume_bck_sound();
				clear_sounds();
			}
			pop_style();

			end_container();
			pop_style();

			style_t bottom_bar;
			bottom_bar.background_color = DARK_BLUE;
			push_style(bottom_bar);
			create_container(1.f, 1.f - main_section_height_percent, WIDGET_SIZE::PARENT_PERCENT_BASED, WIDGET_SIZE::PARENT_PERCENT_BASED, "bottom bar");
			end_container();
			pop_style();

			end_panel();

			autolayout_hierarchy();
			render_ui();	
		} else {	
			// regular stuff
			draw_quad_renders(app);

			start_of_frame();

			style_t panel_style;
			panel_style.display_dir = DISPLAY_DIR::VERTICAL;
			panel_style.vertical_align_val = ALIGN::START;
			panel_style.horizontal_align_val = ALIGN::START;
			push_style(panel_style);
			create_panel("main panel");
			pop_style();

			char timer_buf[128]{};
			sprintf(timer_buf, "time: %.3fs", app.time_spent_in_levels);
			style_t text_style;
			text_style.margin = glm::vec2(20.f);
			push_style(text_style);
			create_text(timer_buf);
			pop_style();

			end_panel();

			autolayout_hierarchy();
			render_ui();	
		}
	}


	SDL_GL_SwapWindow(app.window);
}
