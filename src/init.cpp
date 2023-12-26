#include <string>
#include <stdexcept>
#include <iostream>
#include "glad/glad.h"
#include "constants.h"
#include "app.h"
#include "SDL.h"
#include "renderer/opengl/buffers.h"
#include "renderer/opengl/resources.h"
#include "renderer/opengl/vertex.h"
#include "renderer/basic/shape_renders.h"
#include <vector>
#include "stb/stb_image.h"
#include "physics/physics.h"
#include "gameobjects/gos.h"
#include "renderer/basic/shape_renders.h"
#include "ui/ui.h"


void GLAPIENTRY MyOpenGLErrorCallbackFunc(GLenum source, GLenum debugErrorType, GLuint errorID, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    switch(debugErrorType)
    {
        case GL_DEBUG_TYPE_ERROR:
        {
            printf("GL Type error: %s\nGL error id: %i\n", message, errorID);
        }break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        {
            printf("GL deprecated gl function usage error: %s\nGL error id: %i\n", message, errorID);
        }break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        {
            printf("GL undefined behavior error: %s\nGL error id: %i\n", message, errorID);
        }break;
		default:
		{
			printf("opengl error");
		}
    };
};

/// <summary>
/// This function initalizes SDL to OpenGL Version 4.1, 24-bit depth buffer, height of WINDOW_HEIGHT, 
/// width of WINDOW_WIDTH, blending enabled, and mirrored texture wrapping.
/// </summary>
/// <returns>The SDL window that gets created.</returns>
/// <see>See constants.h for WINDOW_HEIGHT and WINDOW_WIDTH</see>
SDL_Window* init_sdl() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		const char* sdl_error = SDL_GetError();
		std::string error_msg = "SDL could not be initialized: " + std::string(sdl_error);
		throw std::runtime_error(error_msg);
	}

    // setting sdl attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	SDL_Window* window = SDL_CreateWindow("window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    // if window is null, then window creation that an issue
	if (window == NULL) {
		const char* sdl_error = SDL_GetError();
		std::string error_msg = "SDL could not be initialized: " + std::string(sdl_error);
		throw std::runtime_error(error_msg);
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);
    // load opengl functions
	gladLoadGLLoader(SDL_GL_GetProcAddress);

	SDL_GL_MakeCurrent(window, context);

    // setting game default texture parameters and blending settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);

	// glEnable(GL_DEBUG_OUTPUT);
	// glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	// glDebugMessageCallback( MyOpenGLErrorCallbackFunc, 0 );

	return window;
}

/// <summary>
/// Initialize OpenGL data to render a quad to the screen. It creates the 4 vertices and stores it
/// in a VAO, VBO, and EBO. The rectangle vertex and fragment shaders are loaded where defaults are loaded.
/// The texture unit is defaulted to 0, the projection matrix is orthographic, and the view matrix
/// is the identity matrix.
/// </summary>
void init_quad_data() {

	opengl_object_data& data = quad_render_t::obj_data;

    // create the vertices of the rectangle
	vertex_t vertices[4];
	vertices[0] = create_vertex(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0,1,1), glm::vec2(1,1)); // top right
	vertices[1] = create_vertex(glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0,0,1), glm::vec2(1,0)); // bottom right
	vertices[2] = create_vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0,1,0), glm::vec2(0,0)); // bottom left
	vertices[3] = create_vertex(glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(1,0,0), glm::vec2(0,1)); // top left

    // setting the vertices in the vbo
	data.vbo = create_vbo((float*)vertices, sizeof(vertices));

    // creating the indicies for the rectangle
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

    // set up ebo with indicies
	data.ebo = create_ebo(indices, sizeof(indices));

    // create vao and link the vbo/ebo to that vao
	data.vao = create_vao();
	bind_vao(data.vao);
	vao_enable_attribute(data.vao, data.vbo, 0, 3, GL_FLOAT, sizeof(vertex_t), offsetof(vertex_t, position));
	vao_enable_attribute(data.vao, data.vbo, 1, 3, GL_FLOAT, sizeof(vertex_t), offsetof(vertex_t, color));
	vao_enable_attribute(data.vao, data.vbo, 2, 2, GL_FLOAT, sizeof(vertex_t), offsetof(vertex_t, tex_coord));
	bind_ebo(data.ebo);
	unbind_vao();
	unbind_ebo();

    // load in shader for these rectangle quads because the game is 2D, so everything is basically a solid color or a texture
	data.shader = create_shader("C:/Sarthak/projects/game/resources/shaders/rectangle.vert", "C:/Sarthak/projects/game/resources/shaders/rectangle.frag");
    // set projection matrix in the shader
	glm::mat4 projection = glm::ortho(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT);
	shader_set_mat4(data.shader, "projection", projection);
	shader_set_mat4(data.shader, "view", glm::mat4(1.0f));
	shader_set_int(data.shader, "tex", 0);
}

/// <summary>
/// Cleans the line by removing all tabs, spaces, and commas unless they are inside keys and value. 
/// It removes the characters between key and value, except ":" if 
/// this is a key value line. File line is considered done at the \n if it exists or
/// whatever the last character is. Everything after is zero-ed out.
/// </summary>
/// <param name="line">A full line from a file</param>
void clean_line(char* line) {
	int orig_line_len = strlen(line);
	int cleaned_idx = 0;
	bool opened_quote = false;
	for (int i = 0; i < orig_line_len; i++) {
		char c = line[i];
		if (c == '\t' && !opened_quote) continue;
		if (c == ' ' && !opened_quote) continue;
		if (c == ',' && !opened_quote) continue;
		if (c == '\n') break;
		if (c == '\"') {
			opened_quote = !opened_quote;
		}
		line[cleaned_idx] = c;
		cleaned_idx++;
	}
	memset(line + cleaned_idx, 0, orig_line_len-cleaned_idx+1);
}

/// <summary>
/// Stores a key and value both as char buffers.
/// </summary>
struct key_val_t {
	char* key = NULL;
	char* val = NULL;
};

/// <summary>
/// Get the key and value from a buffer from the LDTK JSON file. Some key assumptions since it is a LDTK file
/// is that keys are always words with quotation marks. Also, since only customFields is being
/// parsed, values are also words with quotation marks. But this could change in the future when
/// ints or floats become values as well. Note for the input buffer, the : between key and value will
/// be replaced with NULL so the key and value char* can be extrapolated from the buffer itself. So buffer
/// will get modified.
/// </summary>
/// <param name="buffer">The key-value line that has to be parsed. The key and value must both be in quotation marks.</param>
/// <returns>A char* to the key and value (both extracted from input buffer)</returns>
key_val_t separate_key_val(char* buffer) {
	int len = strlen(buffer);
	key_val_t key_val;
	bool key_section = true;
	bool opened_quote = false;
	for (int i = 0; i < len; i++) {
		const char p = buffer[i];
		if (p == '\"') {
			buffer[i] = 0;
			opened_quote = !opened_quote;
			if (opened_quote) {
				if (key_section) {
					key_val.key = buffer + i + 1;
				} else {
					key_val.val = buffer + i + 1;
				}
			}
		}
		if (p == ':') {
			buffer[i] = 0;
			key_section = false;
			key_val.val = buffer + i + 1;
		}
	}
	return key_val;
}

/// <summary>
/// Stores a color as rgb.
/// </summary>
struct color_t {
	unsigned char r = 0, g = 0, b = 0;

	color_t() {}

	color_t(unsigned char r, unsigned char g, unsigned char b) {
		this->r = r;
		this->g = g;
		this->b = b;
	}

	bool operator==(const color_t& other_color) {
		return r == other_color.r && g == other_color.g && b == other_color.b;
	}
};

/// <summary>
/// Maps some gameobject char buffer identifier to a color
/// </summary>
struct color_conversion_t {
	char m_item_name[128]{};
	color_t color;

	/// <summary>
	/// Constructor for a color conversion between an item in the game and its corresponding color in
	/// the level map. Note the color string must be in RGB hex format with ascii letters being uppercase
	/// with format #______, not 0x______. So "#3C4F9A" is fine but not "#3c4f9a" since it is not uppercase.
	/// "0x3C4F9A" is not okay either because it starts with 0x instead of #.
	/// </summary>
	/// <param name="item_name">Name of gameobject associated with color</param>
	/// <param name="color_str">Color represented as string in uppercase hex format with # in the front</param>
	color_conversion_t(const char* item_name, const char* color_str) {
		memcpy(m_item_name, item_name, strlen(item_name));
		assert(color_str[0] == '#');
		color_str++;
		for (int i = 0; i < 6; i++) {
			unsigned char* color_val = NULL;
			if (i / 2 == 0) {
				color_val = &color.r;
			}
			else if (i / 2 == 1) {
				color_val = &color.g;
			}
			else {
				color_val = &color.b;
			}
			unsigned char hex_char = color_str[i];
			unsigned char hex_val = 0;
			switch (hex_char) {
				case '0': {
					hex_val = 0;
				}
					 break;
				case '1': {
					hex_val = 1;
				}
					 break;
				case '2': {
					hex_val = 2;
				}
					 break;				 
				case '3': {
					hex_val = 3;
				}
					 break;
				case '4': {
					hex_val = 4;
				}
					 break;
				case '5': {
					hex_val = 5;
				}
					 break;
				case '6': {
					hex_val = 6;
				}
					 break;
				case '7': {
					hex_val = 7;
				}
					 break;
				case '8': {
					hex_val = 8;
				}
					 break;
				case '9': {
					hex_val = 9;
				}
					 break;
				case 'A': {
					hex_val = 10;
				}
					 break;
				case 'B': {
					hex_val = 11;
				}
					 break;
				case 'C': {
					hex_val = 12;
				}
					 break;
				case 'D': {
					hex_val = 13;
				}
					 break;
				case 'E': {
					hex_val = 14;
				}
					 break;
				case 'F': {
					hex_val = 15;
				}
					 break;
				default: {
					hex_val = 0;
				}
			}
			*color_val = *color_val * 16;
			*color_val += hex_val;
		}
	}
};

static std::vector<color_conversion_t> color_conversions;
/// <summary>
/// Reads the customFields part of the LDTK JSON since this is where the color conversions exist and extracts these conversions.
/// Must ensure the file position indicator for the file is AFTER the customFields section is declared 
/// (the "customFields" and the "{") in the JSON file. The function returns upon the } and does not handle the opening {. The
/// color conversions are stored in color_conversions.
/// </summary>
/// <param name="file">C file pointer to the LDTK JSON file</param>
void read_color_map_info(FILE* file) {
	while (!feof(file)) {
		char line[128]{};
		fgets(line, sizeof(line), file);
		clean_line(line);

		if (strcmp(line, "}") == 0) {
			break;
		}

		key_val_t key_val = separate_key_val(line);
		color_conversion_t conversion(key_val.key, key_val.val);
		color_conversions.push_back(conversion);
	}
}

// int object_transform_handle = -1;
glm::vec2 read_entity_data(FILE* file) {
	glm::vec2 pos(0);
	while (!feof(file)) {
		char line[128]{};
		fgets(line, sizeof(line), file);
		clean_line(line);

		if (strcmp(line, "{") == 0) continue;
		if (strcmp(line, "}") == 0) continue;
		if (strcmp(line, "]") == 0) {
			break;
		}

		key_val_t key_val = separate_key_val(line);
		if (strcmp(key_val.key, "x") == 0) {
			pos.x = static_cast<int>(strtol(key_val.val, NULL, 10)) / LEVEL_MAP_GRID_SIZE;
		}
		if (strcmp(key_val.key, "y") == 0) {
			pos.y = static_cast<int>(strtol(key_val.val, NULL, 10)) / LEVEL_MAP_GRID_SIZE;
		}
	}	
	return pos;
}

// int x_pos = 0, y_pos = 0;
glm::vec2 player_pos, final_flag_pos;
void read_entities(FILE* file) {
	while (!feof(file)) {
		char line[128]{};
		fgets(line, sizeof(line), file);
		clean_line(line);

		if (strcmp(line, "}") == 0) {
			break;
		}

		key_val_t key_val = separate_key_val(line);
		if (strcmp(key_val.key, "PlayerStart") == 0) {
			assert(*key_val.val == '[');
			player_pos = read_entity_data(file);
		}
		if (strcmp(key_val.key, "FinalFlag") == 0) {
			assert(*key_val.val == '[');
			final_flag_pos = read_entity_data(file);
		}
	}	
}

void skip_over_section(FILE* file) {
	int opened_braces = 1;
	while (!feof(file)) {
		char line[128]{};
		fgets(line, sizeof(line), file);
		clean_line(line);		

		if (strstr(line, "{") != NULL) {
			opened_braces++;
		}
		if (strstr(line, "}") != NULL) {
			opened_braces--;
		}

		if (strstr(line, "[") != NULL) {
			opened_braces++;
		}
		if (strstr(line, "]") != NULL) {
			opened_braces--;
		}

		if (opened_braces == 0) {
			return;
		}
	}
}

/// <summary>
/// Traverse a { } section of the LDTK JSON file. Store the gameobject to level composite image color mapping
/// in the customFields section of the JSON file. Also create the player based on the PlayerEntity in the entities section.
/// Note the { must have been traversed by the file at this point. This function handles 
/// exiting on } for the JSON file in general, not the "{".
/// </summary>
/// <param name="file">The C File pointer to the LDTK JSON file</param>
void recursive_section_traverse(FILE* file) {
	while (!feof(file)) {
		char line[128]{};
		fgets(line, sizeof(line), file);
		clean_line(line);

		// reached end of JSON file
		if (strcmp(line, "}") == 0) {
			break;
		}

		if (strcmp(line, "]") == 0) continue;

		key_val_t key_val = separate_key_val(line);
		// if (key_val.key == NULL) {
		// 	key_val_t key_val = separate_key_val(line);
		// }
		if (strcmp(key_val.val, "{}") == 0 ||  strcmp(key_val.val, "[]") == 0) continue;
		if (key_val.key && strcmp(key_val.key, "customFields") == 0 && strcmp(key_val.val, "{}") != 0) {
			assert(*key_val.val == '{');
			read_color_map_info(file);
		} else if (key_val.key && strcmp(key_val.key, "entities") == 0) {
			assert(*key_val.val == '{');
			read_entities(file);
		} else if (strcmp(key_val.val, "{") == 0 || strcmp(key_val.val, "[") == 0) {
			skip_over_section(file);
		}
	}
}


/// <summary>
/// Responsible for loading the LDTK data from its corresponding JSON file and composite image. It just returns
/// if neither file could be opened. The apropriate gameobjects in the game are loaded from this file based on the
/// color of the pixels in the composition image.
/// </summary>
/// <param name="json_file_path">The absolute path to the JSON file for the level</param>
/// <param name="level_img">The absolute path to the composite image for the level</param>
void load_level(application_t& app, const char* json_file_path, const char* level_img) {
	FILE* file = fopen(json_file_path, "r");
	if (!file) return;
	while (!feof(file)) {
		char line[128]{};
		fgets(line, sizeof(line), file);
		clean_line(line);
		if (strcmp(line, "{") == 0) {
			recursive_section_traverse(file);
		}
	}
	fclose(file);

	int img_file_width, img_file_height, num_channels;
	// data organized left to right row by row
	stbi_set_flip_vertically_on_load(false);
	unsigned char* level_img_data = stbi_load(level_img, &img_file_width, &img_file_height, &num_channels, 0);
	if (level_img_data == NULL) return;

	assert(num_channels == 3 || num_channels == 4);

	const int LEVEL_MAP_ROWS = floor(img_file_height / LEVEL_MAP_GRID_SIZE);
	const int LEVEL_MAP_COLS = floor(img_file_width / LEVEL_MAP_GRID_SIZE);
	for (int top_y = 0; top_y < LEVEL_MAP_ROWS; top_y++) {
		for (int left_x = 0; left_x < LEVEL_MAP_COLS; left_x++) {
			unsigned char r, g, b;
			int img_file_row = top_y * LEVEL_MAP_GRID_SIZE;
			int img_file_col = left_x * LEVEL_MAP_GRID_SIZE;
			unsigned char* pixel_ptr = level_img_data + ((img_file_row * img_file_width) + img_file_col) * num_channels;
			r = *pixel_ptr;
			g = *(pixel_ptr+1);
			b = *(pixel_ptr+2);

			color_t level_pixel_color(r, g, b);
			for (int i = 0; i < color_conversions.size(); i++) {
				if (level_pixel_color == color_conversions[i].color) {
					int level_row = LEVEL_MAP_ROWS - 1 - top_y;
					int level_col = left_x;

					glm::vec3 world_pos(level_col * GAME_GRID_SIZE, level_row * GAME_GRID_SIZE, 0);
					if (strcmp(color_conversions[i].m_item_name, "Ground") == 0) {
						create_ground_block(world_pos, glm::vec3(1), 0);
					} 
					else if (strcmp(color_conversions[i].m_item_name, "Goomba") == 0) {
						create_goomba(world_pos);
					}
					else if (strcmp(color_conversions[i].m_item_name, "GoombaTurnAround") == 0) {
						add_goomba_turn_point(world_pos);
					}
					else if (strcmp(color_conversions[i].m_item_name, "Pipe") == 0) {
						create_pipe(world_pos);
					} 
					else if (strcmp(color_conversions[i].m_item_name, "Brick") == 0) {
						create_brick(world_pos);
					}
				}
			}
		}
	}
	stbi_image_free(level_img_data);

	color_conversions.clear();

	int level_row = LEVEL_MAP_ROWS - 1 - player_pos.y;
	int level_col = player_pos.x;
	app.main_character = create_main_character(glm::vec3(level_col*GAME_GRID_SIZE, level_row*GAME_GRID_SIZE, 0), glm::vec3(1), 0, glm::vec3(1,1,0));

	int flag_level_row = LEVEL_MAP_ROWS - 1 - final_flag_pos.y;
	int flag_level_col = final_flag_pos.x;
	create_final_flag(glm::vec3(flag_level_col*GAME_GRID_SIZE, flag_level_row*GAME_GRID_SIZE, 0));
}

void load_level(application_t& app, int level_num) {
	app.scene_manager.queue_level_load = false; 
	app.scene_manager.level_to_load = -1;

	if (level_num == 0) {
		app.scene_manager.cur_level = 0;		
		return;
	}

	level_num = std::min(std::max(1, level_num), 4);
	char json_file[512]{};
	char img_file[512]{};
	sprintf(json_file, "C:/Sarthak/projects/game/resources/levels/platformer_game/simplified/Level_%i/data.json", level_num);
	sprintf(img_file, "C:/Sarthak/projects/game/resources/levels/platformer_game/simplified/Level_%i/_composite.png", level_num);
    load_level(app, json_file, img_file);
	app.scene_manager.cur_level = level_num;
}


void init(application_t& app) {
	app.window = init_sdl();
    // initialize opengl data for a rectangle
	init_quad_data();	
	init_fonts();
	// make sure this is first so that backgrond quads get created first and are always rendered in the back
	init_parallax_bck_data();
	init_brick_data();
	init_mc_data();
	init_final_flag_data();
	init_goomba_data();
	init_ground_block_data();
	load_level(app, 0);
}

void scene_manager_load_level(scene_manager_t& sm, int level_num) {
	sm.queue_level_load = true;
	sm.level_to_load = level_num;
}