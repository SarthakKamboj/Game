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
// #include <fstream>
#include "stb/stb_image.h"
#include "physics/physics.h"

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

	return window;
}

void init_rectangle_data() {

	opengl_object_data& data = rectangle_render_t::obj_data;

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
	vao_enable_attribute(data.vao, data.vbo, 0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offsetof(vertex_t, position));
	vao_enable_attribute(data.vao, data.vbo, 1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offsetof(vertex_t, color));
	vao_enable_attribute(data.vao, data.vbo, 2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offsetof(vertex_t, tex_coord));
	bind_ebo(data.ebo);
	unbind_vao();
	unbind_ebo();

    // load in shader for these rectangle quads because the game is 2D, so everything is basically a solid color or a texture
	data.shader = create_shader((SHADERS_PATH + "\\rectangle.vert").c_str(), (SHADERS_PATH + "\\rectangle.frag").c_str());
    // set projection matrix in the shader
	glm::mat4 projection = glm::ortho(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT);
	shader_set_mat4(data.shader, "projection", projection);
	shader_set_mat4(data.shader, "view", glm::mat4(1.0f));
	// shader_set_mat4(data.shader, "projection", glm::mat4(1.0f));
	shader_set_int(data.shader, "tex", 0);
}

// void init_placed_world_items() {
//     // TODO: generalize level data paths
//     // const char* file_path = "C:\\Sarthak\\projects\\Platformer\\Editor\\level1.txt";
//     // const char* file_path = "C:\\Sarthak\\projects\\editor\\build\\level10.gme";
//     const char* file_path = "C:\\Sarthak\\projects\\game\\levels\\level01.gme";

//     /*
//         FILE FORMAT
//         ------------
//         WORLD_ITEMS
//         list of world items as format: world item name, texture path, grid squares wide, grid squares high

//         PLACED_ITEMS
//         list of placed items in reference to the world items described in the beginning of the file, 
//         referenced by index of the world item as format: index into world items section, grid x pos, grid y pos
//     */

//     FILE* file;
//     file = fopen(file_path, "r");
// 	size_t delim_len = std::string(WORLD_ITEM_TEXT_FILE_DELIM).size();
//     std::map<int, std::string> idx_to_type;
//     int i = 0;
//     if (file) {
//         // whether in the placed items section or not
//         bool placed_items_section = false;
//         char line[1024];
//         while (!feof(file)) {
//             memset(line, 0, 1024);
//             // get the current line in the file
//             fgets(line, 1024, file);
//             // if just WORLD_ITEMS title, then skip
//             if (strcmp(line, "WORLD_ITEMS\n") == 0) continue;
//             // in the world items section and looking at possible world item
//             if (!placed_items_section && (strcmp(line, "PLACED_ITEMS\n") != 0)) {
//                 std::string delim(WORLD_ITEM_TEXT_FILE_DELIM);
//                 size_t delim_len = delim.size();
//                 // world_item_format is mentioned above in file format comment
//                 static std::string world_item_format = "%1023s" + delim + "%1023s" + delim + "%i" + delim + "%i\n";
//                 static const char* world_item_format_char = world_item_format.c_str();

//                 char name[1024]{};
//                 char path[1024]{};
//                 int width = 0;
//                 int height = 0;
//                 sscanf(line, world_item_format_char, name, path, &width, &height);
//                 std::string name_str(name);

//                 // for right now, texture path, width, and height are not being used but will be in the future
//                 if (strcmp(name, "ground") == 0) {
//                     ground_block_t::tex_handle = create_texture(path, 0);
//                 }

//                 // correlate the item name to a particular index to be used as reference by the placed world items
//                 idx_to_type[i] = name_str;
// 				i++;
//                 continue;
//             } 
//             // if the line is the PLACED_ITEMS title, indicating we are now in the placed items section
//             if (strcmp(line, "PLACED_ITEMS\n") == 0) {
//                 placed_items_section = true;
//                 continue;
//             }
//             // line break after the placed items section
// 			if ((strcmp(line, "\n") == 0) && placed_items_section) {
// 				break;
// 			}

//             std::string delim(WORLD_ITEM_TEXT_FILE_DELIM);
//             size_t delim_len = delim.size();
//             // format for placed item described above in file format comment
//             static std::string placed_item_format = "%i " + delim + " %i " + delim + " %i\n";
//             static const char* placed_item_format_char = placed_item_format.c_str();

//             int idx = -1;
//             int x = -1;
//             int y = -1;
//             sscanf(line, placed_item_format_char, &idx, &x, &y);

//             // get what item name corresponds to the world item index used by this placed world item
//             std::string& type = idx_to_type[idx];
//             // for now, just handle the ground block
//             if (type == "ground") {
//                 glm::vec2 grid_pos(x, y);
//                 glm::vec3 pixel_pos((ground_block_t::WIDTH / 2) + ground_block_t::WIDTH * x, (ground_block_t::HEIGHT / 2) + ground_block_t::HEIGHT * y, 0.f);
//                 create_ground_block(pixel_pos, glm::vec3(1.f), 0.f);
//             }
//         }
//         fclose(file);
//     } else {
//         std::cout << "could not open world items file" << std::endl;
//     }
// }

enum class e_ldtk_value_type {
	STRING,
	ARRAY
};

void clean_line(const char* orig_line, char* cleaned_line) {
	int orig_line_len = strlen(orig_line);
	int cleaned_idx = 0;
	bool opened_quote = false;
	for (int i = 0; i < orig_line_len; i++) {
		char c = orig_line[i];
		if (c == '\t') continue;
		if (c == ' ' && !opened_quote) continue;
		if (c == ',' && !opened_quote) continue;
		if (c == '\n') break;
		if (c == '\"') {
			opened_quote = !opened_quote;
		}
		cleaned_line[cleaned_idx] = c;
		cleaned_idx++;
	}
}

void clean_line(char* line) {
	int orig_line_len = strlen(line);
	int cleaned_idx = 0;
	bool opened_quote = false;
	for (int i = 0; i < orig_line_len; i++) {
		char c = line[i];
		if (c == '\t') continue;
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

/* 
function sets : to 0, so buffer looks like key0value, so key can be used as separate string buffer
but return value gives start of val index-1, so val can be indexed as well as its own string
*/
struct key_val_t {
	char* key = NULL;
	char* val = NULL;
};
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

struct color_conversion_t {
	char m_item_name[128]{};
	// char m_color[128]{};
	color_t color;

	color_conversion_t(const char* item_name, const char* color_str) {
		memcpy(m_item_name, item_name, strlen(item_name));
		// memcpy(m_color, color, strlen(color));
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
void read_color_map_info(FILE* file) {
	while (!feof(file)) {
		char line[128]{};
		fgets(line, sizeof(line), file);
		clean_line(line);

		if (strcmp(line, "}") == 0) {
			std::cout << "closed section";
			break;
		}

		key_val_t key_val = separate_key_val(line);
		color_conversion_t conversion(key_val.key, key_val.val);
		color_conversions.push_back(conversion);
	}
}

void recursive_section_traverse(FILE* file) {
	while (!feof(file)) {
		char line[128]{};
		fgets(line, sizeof(line), file);
		clean_line(line);

		// reached end of section
		if (strcmp(line, "}") == 0) {
			break;
		}

		if (strcmp(line, "]") == 0) continue;

		key_val_t key_val = separate_key_val(line);
		if (key_val.key == NULL) {
			key_val_t key_val = separate_key_val(line);
		}
		if (key_val.key && strcmp(key_val.key, "identifier") == 0) {
			std::cout << key_val.key << " " << key_val.val << std::endl;
		}
		if (key_val.key && strcmp(key_val.key, "customFields") == 0) {
			assert(*key_val.val == '{');
			read_color_map_info(file);
		}
	}
}

void init_placed_world_items(const char* json_file_path, const char* level_img) {
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

	int level_file_width, level_file_height, num_channels;
	// data organized left to right row by row
	unsigned char* level_img_data = stbi_load(level_img, &level_file_width, &level_file_height, &num_channels, 0);
	if (level_img_data == NULL) return;

	assert(num_channels == 3 || num_channels == 4);

	for (int top_y = 0; top_y < level_file_height; top_y++) {
		for (int left_x = 0; left_x < level_file_width; left_x++) {
			unsigned char r, g, b;
			unsigned char* pixel_ptr = level_img_data + ((top_y * level_file_width) + left_x) * num_channels;
			r = *pixel_ptr;
			g = *(pixel_ptr+1);
			b = *(pixel_ptr+2);

			color_t level_pixel_color(r, g, b);
			for (int i = 0; i < color_conversions.size(); i++) {
				if (level_pixel_color == color_conversions[i].color) {
					int level_row = level_file_height - 1 - top_y;
					int level_col = left_x;

					int obj = create_transform(glm::vec3(level_col*40, level_row*40, 0), glm::vec3(1), 0);
					create_rectangle_render(obj, glm::vec3(0,1,1), 40, 40, false, 0, -1);
					create_rigidbody(obj, false, 40, 40, true);
				}
			}
		}
	}
}

void init(application_t& app) {
	app.window = init_sdl();
	// app.running = true;
    // initialize opengl data for a rectangle
	init_rectangle_data();
	const char* json_file = "C:/Sarthak/projects/game/resources/levels/level1/simplified/Level_0/data.json";
	// const char* img_file = "C:/Sarthak/projects/game/resources/levels/level1/simplified/Level_0/red.png";
	const char* img_file = "C:/Sarthak/projects/game/resources/levels/level1/simplified/Level_0/_composite.png";
    init_placed_world_items(json_file, img_file);
}