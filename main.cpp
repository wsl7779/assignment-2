/**
* Author: Will Lee
* Assignment: Pong Clone
* Date due: 2023-10-21, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

#include <ctime>
#include "cmath"

enum Coordinate
{
    x_coordinate,
    y_coordinate
};

const int WINDOW_WIDTH = 1080,
WINDOW_HEIGHT = 720;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const char PLAYER_SPRITE_FILEPATH[] = "assets/crusadie.png";
const char PLAYER_SPRITE_FILEPATH2[] = "assets/redadie.png";
const char PLAYER_SPRITE_FILEPATH_BALL[] = "assets/missile.png";
const char PLAYER_SPRITE_FILEPATH_WINLEFT[] = "assets/youwinyel.jpg";
const char PLAYER_SPRITE_FILEPATH_WINRIGHT[] = "assets/youwinred.jpg";
const char PLAYER_SPRITE_FILEPATH_BACKGROUND[] = "assets/dusty.jpg";

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 view_matrix, m_model_matrix, m_projection_matrix, m_trans_matrix;
glm::mat4 m_init_matrix;

glm::mat4 view_matrix_ball, m_model_matrix_ball, m_projection_matrix_ball, m_trans_matrix_ball;

glm::mat4 view_matrix2, m_model_matrix2, m_projection_matrix2, m_trans_matrix2;
glm::mat4 m_init_matrix2;

glm::mat4 view_matrix_win, m_model_matrix_win, m_projection_matrix_win, m_trans_matrix_win;
glm::mat4 m_init_matrix_win;

glm::mat4 view_matrix_win2, m_model_matrix_win2, m_projection_matrix_win2, m_trans_matrix_win2;
glm::mat4 m_init_matrix_win2;

glm::mat4 view_matrix_bg, m_model_matrix_bg, m_projection_matrix_bg, m_trans_matrix_bg;

float m_previous_ticks = 0.0f;
float g_timer = 0.0f;
float g_collision_cooldown = 100.0f;
bool g_on_cd = false;

float m_previous_ticks2 = 0.0f;
float g_timer2 = 0.0f;
float g_collision_cooldown2 = 100.0f;
bool g_on_cd2 = false;

bool going_up = true;
bool multiplayer = true;
bool left_win = false;
bool right_win = false;

GLuint g_player_texture_id;
GLuint g_ball_texture_id;
GLuint g_player_texture_id2;
GLuint g_win_texture_id;
GLuint g_win_texture_id2;
GLuint g_background_id;

glm::vec3 g_player_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);
float g_player_speed = 2.0f;

glm::vec3 g_player2_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_player2_movement = glm::vec3(0.0f, 0.0f, 0.0f);
float g_player2_speed = 2.0f;

const glm::vec3 PLAYER_INIT_POS = glm::vec3(-3.5f, 0.0f, 0.0f),
PLAYER_INIT_SCA = glm::vec3(2.0f, 2.0f, 0.0f);

const glm::vec3 PLAYER2_INIT_POS = glm::vec3(3.5f, 0.0f, 0.0f),
PLAYER2_INIT_SCA = glm::vec3(2.0f, 2.0f, 0.0f);

const glm::vec3 BALL_INIT_POS = glm::vec3(0.0f, 0.0f, 0.0f),
BALL_INIT_SCA = glm::vec3(1.0f, 1.0f, 0.0f);

glm::vec3 g_ball_position = glm::vec3(-2.0f, 0.0f, 0.0f);
glm::vec3 g_ball_movement = glm::vec3(1.0f, 1.0f, 0.0f);
float g_ball_speed = 1.5f;


GLuint load_texture(const char* filepath)
{

    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        LOG(filepath);
        assert(false);
    }


    GLuint texture_id;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    stbi_image_free(image);

    return texture_id;
}

void initialise()
{
    // Initialise video and joystick subsystems
    SDL_Init(SDL_INIT_VIDEO);


    g_display_window = SDL_CreateWindow("No you take it",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);


    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    view_matrix = glm::mat4(1.0f);
    m_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(m_projection_matrix);
    g_shader_program.set_view_matrix(view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, PLAYER_INIT_POS);
    m_model_matrix = glm::scale(m_model_matrix, PLAYER_INIT_SCA);
    m_init_matrix = m_model_matrix;

    g_player_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);

    m_model_matrix2 = glm::mat4(1.0f);
    m_model_matrix2 = glm::translate(m_model_matrix2, PLAYER2_INIT_POS);
    m_model_matrix2 = glm::scale(m_model_matrix2, PLAYER2_INIT_SCA);
    m_init_matrix2 = m_model_matrix2;

    g_player_texture_id2 = load_texture(PLAYER_SPRITE_FILEPATH2);

    m_model_matrix_ball = glm::mat4(1.0f);

    g_ball_texture_id = load_texture(PLAYER_SPRITE_FILEPATH_BALL);

    m_model_matrix_win = glm::mat4(1.0f);
    m_model_matrix_win = glm::scale(m_model_matrix_win, glm::vec3(0.0f, 0.0f, 0.0f));

    g_win_texture_id = load_texture(PLAYER_SPRITE_FILEPATH_WINLEFT);

    m_model_matrix_win2 = glm::mat4(1.0f);
    m_model_matrix_win2 = glm::scale(m_model_matrix_win2, glm::vec3(0.0f, 0.0f, 0.0f));

    g_win_texture_id2 = load_texture(PLAYER_SPRITE_FILEPATH_WINRIGHT);

    m_model_matrix_bg = glm::mat4(1.0f);
    m_model_matrix_bg = glm::scale(m_model_matrix_bg, glm::vec3(10.0f, 10.0f, 0.0f));

    g_background_id = load_texture(PLAYER_SPRITE_FILEPATH_BACKGROUND);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void process_input()
{
    g_player_movement = glm::vec3(0.0f);
    g_player2_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                g_game_is_running = false;
            case SDLK_t:
                multiplayer = !multiplayer;
                if (multiplayer) {
                    g_player2_speed = 2.0f;
                }
                else {
                    g_player2_speed = 1.0f;
                }
                break;
            default:
                break;
            }
        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_W]) {
        if (g_player_position.y < (3.75f / 2.0f)) {
            g_player_movement.y = 1.0f; 
        }
    }
    else if (key_state[SDL_SCANCODE_S]) {
        if (g_player_position.y > (-3.75f / 2.0f)) {
            g_player_movement.y = -1.0f; 
        }
    }

    if (key_state[SDL_SCANCODE_UP]) {
        if (multiplayer && g_player2_position.y < (3.75f / 2.0f)) {
            g_player2_movement.y = 1.0f; 
        }
    }
    else if (key_state[SDL_SCANCODE_DOWN]) {
        if (multiplayer && g_player2_position.y > (-3.75f / 2.0f)) {
            g_player2_movement.y = -1.0f;
        }
    }
}


void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - m_previous_ticks; // the delta time is the difference from the last frame
    m_previous_ticks = ticks;

    if ((!left_win) && (!right_win)) {
        float collision_factor = 1.0f;
        float x_distance = fabs((g_player_position.x - 3.5f) - g_ball_position.x) - ((PLAYER_INIT_SCA.x * collision_factor + BALL_INIT_SCA.x * collision_factor) / 2.0f);
        float y_distance = fabs(g_player_position.y - g_ball_position.y) - ((PLAYER_INIT_SCA.y * collision_factor + BALL_INIT_SCA.y * collision_factor) / 2.0f);

        float x_distance2 = fabs((g_player2_position.x + 3.5f) - g_ball_position.x) - ((PLAYER2_INIT_SCA.x * collision_factor + BALL_INIT_SCA.x * collision_factor) / 2.0f);
        float y_distance2 = fabs(g_player2_position.y - g_ball_position.y) - ((PLAYER2_INIT_SCA.y * collision_factor + BALL_INIT_SCA.y * collision_factor) / 2.0f);


        if (g_timer >= g_collision_cooldown * delta_time) {
            g_on_cd = false;
        }

        if (g_timer2 >= g_collision_cooldown2 * delta_time) {
            g_on_cd2 = false;
        }

        if (x_distance < 0.0f && y_distance < 0.0f && (!g_on_cd))
        {
            g_ball_movement.x *= -1;
            g_on_cd = true;
            g_timer = 0.0f;
        }

        if (x_distance2 < 0.0f && y_distance2 < 0.0f && (!g_on_cd2))
        {
            g_ball_movement.x *= -1;
            g_on_cd2 = true;
            g_timer2 = 0.0f;
        }

        g_timer += delta_time;
        g_timer2 += delta_time;


        if ((g_player2_position.y < (-3.75f / 2.0f)) || (g_player2_position.y > (3.75f / 2.0f))) {
            going_up = !going_up;
        }

        if (!multiplayer) {
            if (going_up) {
                g_player2_movement.y = 1.0f;
            }
            else {
                g_player2_movement.y = -1.0f;
            }
        }

        if (g_ball_position.x > 5.0f) {
            left_win = true;
        }

        if (g_ball_position.x < -5.0f) {
            right_win = true;
        }

        if (g_ball_position.y > 3.75f || g_ball_position.y < -3.75f) {
            g_ball_movement.y *= -1;
        }

        g_player_position += g_player_movement * g_player_speed * delta_time;
        m_model_matrix = m_init_matrix;
        m_model_matrix = glm::translate(m_model_matrix, g_player_position);

        g_player2_position += g_player2_movement * g_player2_speed * delta_time;
        m_model_matrix2 = m_init_matrix2;
        m_model_matrix2 = glm::translate(m_model_matrix2, g_player2_position);

        g_ball_position += g_ball_movement * g_ball_speed * delta_time;
        m_model_matrix_ball = glm::mat4(1.0f);
        m_model_matrix_ball = glm::translate(m_model_matrix_ball, g_ball_position);
    }
    else if (left_win) {
        m_model_matrix_win = glm::mat4(1.0f);
        m_model_matrix_win = glm::scale(m_model_matrix_win, glm::vec3(5.0f, 5.0f, 5.0f));
    }
    else if (right_win) {
        m_model_matrix_win2 = glm::mat4(1.0f);
        m_model_matrix_win2 = glm::scale(m_model_matrix_win2, glm::vec3(5.0f, 5.0f, 5.0f));
    }
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);


    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };


    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    draw_object(m_model_matrix_bg, g_background_id);
    draw_object(m_model_matrix, g_player_texture_id);
    draw_object(m_model_matrix_ball, g_ball_texture_id);
    draw_object(m_model_matrix2, g_player_texture_id2);
    draw_object(m_model_matrix_win, g_win_texture_id);
    draw_object(m_model_matrix_win2, g_win_texture_id2);


    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
}


int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}