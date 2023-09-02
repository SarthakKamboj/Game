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
// #include <fstream>

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

void init(application_t& app) {
	app.window = init_sdl();
	// app.running = true;
    // initialize opengl data for a rectangle
	init_rectangle_data();
    // init_placed_world_items();
}