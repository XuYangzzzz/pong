


/**
* Author:Regan Zhu
* Assignment: Pong Clone
* Date due: 2023-06-29, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <cstdlib>
#include <iostream>

enum AppStatus { RUNNING, TERMINATED };
enum AppMode { PLAYING, LEFTEND, RIGHTEND};
enum AppPlayer {SINGLE, MULTI};
enum AppBalls {ONE, TWO, THREE};

constexpr int WINDOW_WIDTH  = 960,
              WINDOW_HEIGHT = 720;

constexpr float BG_RED     = 1.0f,
                BG_GREEN   = 0.7f,
                BG_BLUE    = 0.786f,
                BG_OPACITY = 1.0f;

constexpr float ROT_INCREMENT = 1.0f;


constexpr GLint NUMBER_OF_TEXTURES = 1, // to be generated, that is
                LEVEL_OF_DETAIL    = 0, // mipmap reduction image level
                TEXTURE_BORDER     = 0; // this value MUST be zero

constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";


constexpr float MILLISECONDS_IN_SECOND = 1000.0;
constexpr float DEGREES_PER_SECOND     = 1.0f;

constexpr char paddle1_FILEPATH[]   = "paddle.png",
               paddle2_FILEPATH[] = "paddle.png",
                ball_FILEPATH[] = "ball.png",
                start_FILEPATH[] = "start.png",
                left_FILEPATH[] = "left.png",
                right_FILEPATH[] = "right.png",
                winner_FILEPATH[] = "winner.png",
                ball2_FILEPATH[] = "ball.png",
                ball3_FILEPATH[] = "ball.png",
                end_FILEPATH[] = "end.png";

constexpr glm::vec3 INIT_SCALE      = glm::vec3(0.1f, 2.0f, 0.0f),
                    INIT_START_SCALE = glm::vec3(4.0f, 2.0f, 0.0f),
                    INIT_BALL_SCALE = glm::vec3(1.0f, 1.0f, 0.0f),
                    INIT_POS_paddle1   = glm::vec3(-4.8f, 0.0f, 0.0f),
                    INIT_POS_paddle2 = glm::vec3(4.8f, 0.0f, 0.0f),
                    INIT_POS_start = glm::vec3(0.0f),
                    INIT_POS_left   = glm::vec3(-1.0f, 1.0f, 0.0f),
                    INIT_POS_right   = glm::vec3(1.0f, 1.0f, 0.0f),
                    INIT_POS_winner   = glm::vec3(0.0f, 1.0f, 0.0f);

AppStatus g_app_status = RUNNING;
AppMode g_app_mode = PLAYING;
AppPlayer g_app_player = MULTI;
AppBalls g_app_numballs = ONE;

SDL_Window* g_display_window;

ShaderProgram g_shader_program = ShaderProgram();
glm::mat4 g_view_matrix,
          g_paddle1_matrix,
          g_paddle2_matrix,
          g_ball1_matrix,
        g_ball2_matrix,
        g_ball3_matrix,
        g_start_matrix,
          g_projection_matrix,
          g_trans_matrix,
        g_left_matrix,
        g_right_matrix,
        g_winner_matrix,
        g_end_matrix;

float g_triangle_rotate = 0.0f;
float g_previous_ticks  = 0.0f;

//glm::vec3 g_model_movement = glm::vec3(0.0f,0.0f,0.0f);
//glm::vec3 g_model_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_paddle1_pos = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_paddle1_movement = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_paddle2_pos = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_paddle2_movement = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_ball1_pos = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 g_ball1_movement = glm::vec3(0.4f,0.4f,0.0f);
glm::vec3 g_ball2_pos = glm::vec3(0.0f);
glm::vec3 g_ball2_movement = glm::vec3(-0.4f,0.4f,0.0f);
glm::vec3 g_ball3_pos = glm::vec3(0.0f);
glm::vec3 g_ball3_movement = glm::vec3(0.4f,-0.4f,0.0f);
glm::vec3 g_start_pos = glm::vec3(0.0f);
glm::vec3 g_start_movement = glm::vec3(0.0f);



float g_model_speed = 5.0f;  // move 1 unit per second

//glm::vec3 g_rotation_bw   = glm::vec3(0.0f, 0.0f, 0.0f),
//          g_rotation_neon = glm::vec3(0.0f, 0.0f, 0.0f);

GLuint g_paddle1_texture_id,
       g_paddle2_texture_id,
        g_ball_texture_id,
        g_ball2_texture_id,
        g_ball3_texture_id,
        g_start_texture_id,
        g_left_texture_id,
        g_right_texture_id,
        g_winner_texture_id,
        g_end_texture_id;

////for heartbeat scaling stuff
constexpr float GROWTH_FACTOR = 1.2f;  // growth rate of 1.0% per frame
constexpr float SHRINK_FACTOR = 0.8f;  // growth rate of -1.0% per frame
constexpr int MAX_FRAME = 100;           // this value is, of course, up to you

int g_frame_counter = 0;
bool g_is_growing = true;
 
GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}






void initialise()
{
   SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Pong Clone",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        SDL_Quit();
        exit(1);
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_paddle1_matrix       = glm::mat4(1.0f);
    g_paddle2_matrix     = glm::mat4(1.0f);
    g_start_matrix = glm::mat4(1.0f);
    g_ball1_matrix = glm::mat4(1.0f);
    g_ball2_matrix = glm::mat4(1.0f);
    g_ball3_matrix = glm::mat4(1.0f);
    g_end_matrix = glm::mat4(1.0f);
    g_left_matrix = glm::mat4(1.0f);
    g_right_matrix = glm::mat4(1.0f);
    g_winner_matrix = glm::mat4(1.0f);
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    g_paddle1_texture_id   = load_texture(paddle1_FILEPATH);
    g_paddle2_texture_id = load_texture(paddle2_FILEPATH);
    g_start_texture_id = load_texture(start_FILEPATH);
    g_ball_texture_id = load_texture(ball_FILEPATH);
    g_end_texture_id = load_texture(end_FILEPATH);
    g_left_texture_id = load_texture(left_FILEPATH);
    g_right_texture_id = load_texture(right_FILEPATH);
    g_winner_texture_id = load_texture(winner_FILEPATH);
    g_ball2_texture_id = load_texture(ball2_FILEPATH);
    g_ball3_texture_id = load_texture(ball3_FILEPATH);


    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
//    g_model_movement = glm::vec3(0.0f);
    g_paddle1_movement = glm::vec3(0.0f);
    g_paddle2_movement = glm::vec3(0.0f);
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch(event.type){
            case SDL_QUIT:
                g_app_status = TERMINATED;
                break;
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                    case SDLK_q:
                        g_app_status = TERMINATED;
                        break;
//                    case SDLK_RIGHT:
//                        g_model_movement.x = 1.0f;
                    case SDLK_SPACE:
                        g_app_mode = PLAYING;
                        break;
                    case SDLK_t:
                        if(g_app_player == MULTI){
                            g_app_player = SINGLE;
                        }else{
                            g_app_player = MULTI;
                        }
                        break;
                    case SDLK_1:
                        g_app_numballs = ONE;
                        break;
                    case SDLK_2:
                        g_app_numballs = TWO;
                        break;
                    case SDLK_3:
                        g_app_numballs = THREE;
                        break;
                }
                break;
        }
        break;
    }
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
//    if (key_state[SDL_SCANCODE_RIGHT]) g_paddle1_poszz.x = 1.0f;
//    else if(key_state[SDL_SCANCODE_LEFT]) g_model_movement.x = -1.0f;
    if (key_state[SDL_SCANCODE_UP])
    {
        g_paddle2_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        g_paddle2_movement.y = -1.0f;
    }
    if (key_state[SDL_SCANCODE_W])
    {
        g_paddle1_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_S])
    {
        g_paddle1_movement.y = -1.0f;
    }
    
    // This makes sure that the player can't "cheat" their way into moving
    // faster
    if (glm::length(g_paddle1_movement) > 1.0f)
    {
        g_paddle1_movement = glm::normalize(g_paddle1_movement);
    }
    if (glm::length(g_paddle2_movement) > 1.0f)
    {
        g_paddle2_movement = glm::normalize(g_paddle2_movement);
    }
}

void update()
{
    if (g_app_mode == PLAYING){
        if(g_app_numballs == ONE){
            /* Delta Time Calculations */
            float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND; // current # of ticks
            float delta_time = ticks - g_previous_ticks; // tick difference from the last frame
            g_previous_ticks = ticks;
            
    //        g_triangle_rotate += 1 * delta_time;
            g_paddle1_pos += g_paddle1_movement * delta_time *5.0f;
            g_paddle2_pos += g_paddle2_movement * delta_time *5.0f;
            g_ball1_pos += g_ball1_movement * delta_time *5.0f;

            //    g_paddle1_pos.y = r * sin(g_triangle_rotate);
            //    g_paddle2_pos.y = -r * sin(g_triangle_rotate);
            
            //for when paddle hits its boundaries
            //top and bottom
            if (g_paddle1_pos.y > 3.0f || g_paddle1_pos.y < -3.0f){
                g_paddle1_pos.y -= g_paddle1_movement.y * delta_time * 5.0f;
            }
            if (g_paddle2_pos.y > 3.0f || g_paddle2_pos.y < -3.0f){
                g_paddle2_pos.y -= g_paddle2_movement.y * delta_time * 5.0f;
            }
            if (g_ball1_pos.y > 3.4f || g_ball1_pos.y < -3.4f){
                g_ball1_pos.y -= g_ball1_pos.y * delta_time ;
                g_ball1_movement.y = g_ball1_movement.y * -1;
            }
            
            if (g_ball1_pos.x > 5.0f ){
                g_app_mode = LEFTEND;
            }
            if(g_ball1_pos.x < -5.0f){
                g_app_mode = RIGHTEND;
            }

            //Collision Detection
            float x_distance = fabs(g_ball1_pos.x + INIT_POS_start.x - g_paddle2_pos.x - INIT_POS_paddle2.x) -
                ((INIT_BALL_SCALE.x + INIT_SCALE.x) / 2.0f);

            float y_distance = fabs(g_ball1_pos.y + INIT_POS_start.y - g_paddle2_pos.y- INIT_POS_paddle2.y) -
                ((INIT_BALL_SCALE.y + INIT_SCALE.y) / 2.0f);
            
            if (x_distance < 0.0f && y_distance < 0.0f)
            {
                g_ball1_movement.x = g_ball1_movement.x * -1;
                g_ball1_pos.x -= g_ball1_pos.x * delta_time  ;
            }
            float x_distance_left = fabs(g_ball1_pos.x + INIT_POS_start.x - g_paddle1_pos.x- INIT_POS_paddle1.x) -
                ((INIT_BALL_SCALE.x + INIT_SCALE.x) / 2.0f);

            float y_distance_left = fabs(g_ball1_pos.y + INIT_POS_start.y - g_paddle1_pos.y - INIT_POS_paddle1.y) -
                ((INIT_BALL_SCALE.y + INIT_SCALE.y) / 2.0f);
            
            if (x_distance_left < 0.0f && y_distance_left < 0.0f)
            {
                g_ball1_movement.x = g_ball1_movement.x * -1;
                g_ball1_pos.x -= g_ball1_pos.x * delta_time ;
            }
            
            g_paddle1_matrix = glm::mat4(1.0f);
            g_paddle2_matrix = glm::mat4(1.0f);
            g_ball1_matrix = glm::mat4(1.0f);
            
            
            if(g_app_player == SINGLE){
                g_triangle_rotate += 1 * delta_time;
                g_paddle1_pos.y = 2.93f * sin(g_triangle_rotate);

            }
            g_paddle1_matrix = glm::translate(g_paddle1_matrix, INIT_POS_paddle1);
            g_paddle1_matrix = glm::translate(g_paddle1_matrix, g_paddle1_pos);
            g_paddle1_matrix = glm::scale(g_paddle1_matrix, INIT_SCALE);
            
            g_paddle2_matrix = glm::translate(g_paddle2_matrix, INIT_POS_paddle2);
            g_paddle2_matrix = glm::translate(g_paddle2_matrix, g_paddle2_pos);
            g_paddle2_matrix = glm::scale(g_paddle2_matrix, INIT_SCALE);
            
            g_ball1_matrix = glm::translate(g_ball1_matrix, INIT_POS_start);
            g_ball1_matrix = glm::translate(g_ball1_matrix, g_ball1_pos);
    //        g_ball1_matrix = glm::rotate(g_ball1_matrix, glm::radians(0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
            g_ball1_matrix = glm::scale(g_ball1_matrix, INIT_BALL_SCALE);
        }
        else if (g_app_numballs == TWO){
            /* Delta Time Calculations */
            float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND; // current # of ticks
            float delta_time = ticks - g_previous_ticks; // tick difference from the last frame
            g_previous_ticks = ticks;
            
    //        g_triangle_rotate += 1 * delta_time;
            g_paddle1_pos += g_paddle1_movement * delta_time *5.0f;
            g_paddle2_pos += g_paddle2_movement * delta_time *5.0f;
            g_ball1_pos += g_ball1_movement * delta_time *5.0f;
            g_ball2_pos += g_ball2_movement * delta_time *5.0f;

            //    g_paddle1_pos.y = r * sin(g_triangle_rotate);
            //    g_paddle2_pos.y = -r * sin(g_triangle_rotate);
            
            //for when paddle hits its boundaries
            //top and bottom
            if (g_paddle1_pos.y > 3.0f || g_paddle1_pos.y < -3.0f){
                g_paddle1_pos.y -= g_paddle1_movement.y * delta_time * 5.0f;
            }
            if (g_paddle2_pos.y > 3.0f || g_paddle2_pos.y < -3.0f){
                g_paddle2_pos.y -= g_paddle2_movement.y * delta_time * 5.0f;
            }
            if (g_ball1_pos.y > 3.4f || g_ball1_pos.y < -3.4f){
                g_ball1_pos.y -= g_ball1_pos.y * delta_time ;
                g_ball1_movement.y = g_ball1_movement.y * -1;
            }
            if (g_ball2_pos.y > 3.4f || g_ball2_pos.y < -3.4f){
                g_ball2_pos.y -= g_ball2_pos.y * delta_time ;
                g_ball2_movement.y = g_ball2_movement.y * -1;
            }
            
            if (g_ball1_pos.x > 5.0f ||g_ball2_pos.x > 5.0f){
                g_app_mode = LEFTEND;
            }
            if(g_ball1_pos.x < -5.0f||g_ball2_pos.x < -5.0f){
                g_app_mode = RIGHTEND;
            }

            //Collision Detection
            //ball 1
            float x_distance = fabs(g_ball1_pos.x + INIT_POS_start.x - g_paddle2_pos.x - INIT_POS_paddle2.x) -
                ((INIT_BALL_SCALE.x + INIT_SCALE.x) / 2.0f);

            float y_distance = fabs(g_ball1_pos.y + INIT_POS_start.y - g_paddle2_pos.y- INIT_POS_paddle2.y) -
                ((INIT_BALL_SCALE.y + INIT_SCALE.y) / 2.0f);
            
            if (x_distance < 0.0f && y_distance < 0.0f)
            {
                g_ball1_movement.x = g_ball1_movement.x * -1;
                g_ball1_pos.x -= g_ball1_pos.x * delta_time  ;
            }
            float x_distance_left = fabs(g_ball1_pos.x + INIT_POS_start.x - g_paddle1_pos.x- INIT_POS_paddle1.x) -
                ((INIT_BALL_SCALE.x + INIT_SCALE.x) / 2.0f);

            float y_distance_left = fabs(g_ball1_pos.y + INIT_POS_start.y - g_paddle1_pos.y - INIT_POS_paddle1.y) -
                ((INIT_BALL_SCALE.y + INIT_SCALE.y) / 2.0f);
            
            if (x_distance_left < 0.0f && y_distance_left < 0.0f)
            {
                g_ball1_movement.x = g_ball1_movement.x * -1;
                g_ball1_pos.x -= g_ball1_pos.x * delta_time ;
            }
            //ball2
            x_distance = fabs(g_ball2_pos.x + INIT_POS_start.x - g_paddle2_pos.x - INIT_POS_paddle2.x) -
                ((INIT_BALL_SCALE.x + INIT_SCALE.x) / 2.0f);

            y_distance = fabs(g_ball2_pos.y + INIT_POS_start.y - g_paddle2_pos.y- INIT_POS_paddle2.y) -
                ((INIT_BALL_SCALE.y + INIT_SCALE.y) / 2.0f);
            
            if (x_distance < 0.0f && y_distance < 0.0f)
            {
                g_ball2_movement.x = g_ball2_movement.x * -1;
                g_ball2_pos.x -= g_ball2_pos.x * delta_time  ;
            }
             x_distance_left = fabs(g_ball2_pos.x + INIT_POS_start.x - g_paddle1_pos.x- INIT_POS_paddle1.x) -
                ((INIT_BALL_SCALE.x + INIT_SCALE.x) / 2.0f);

             y_distance_left = fabs(g_ball2_pos.y + INIT_POS_start.y - g_paddle1_pos.y - INIT_POS_paddle1.y) -
                ((INIT_BALL_SCALE.y + INIT_SCALE.y) / 2.0f);
            
            if (x_distance_left < 0.0f && y_distance_left < 0.0f)
            {
                g_ball2_movement.x = g_ball2_movement.x * -1;
                g_ball2_pos.x -= g_ball2_pos.x * delta_time ;
            }
            
            
            
            g_paddle1_matrix = glm::mat4(1.0f);
            g_paddle2_matrix = glm::mat4(1.0f);
            g_ball1_matrix = glm::mat4(1.0f);
            g_ball2_matrix = glm::mat4(1.0f);
            
            
            if(g_app_player == SINGLE){
                g_triangle_rotate += 1 * delta_time;
                g_paddle1_pos.y = 2.93f * sin(g_triangle_rotate);

            }
            g_paddle1_matrix = glm::translate(g_paddle1_matrix, INIT_POS_paddle1);
            g_paddle1_matrix = glm::translate(g_paddle1_matrix, g_paddle1_pos);
            g_paddle1_matrix = glm::scale(g_paddle1_matrix, INIT_SCALE);
            
            g_paddle2_matrix = glm::translate(g_paddle2_matrix, INIT_POS_paddle2);
            g_paddle2_matrix = glm::translate(g_paddle2_matrix, g_paddle2_pos);
            g_paddle2_matrix = glm::scale(g_paddle2_matrix, INIT_SCALE);
            
            g_ball1_matrix = glm::translate(g_ball1_matrix, INIT_POS_start);
            g_ball1_matrix = glm::translate(g_ball1_matrix, g_ball1_pos);
    //        g_ball1_matrix = glm::rotate(g_ball1_matrix, glm::radians(0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
            g_ball1_matrix = glm::scale(g_ball1_matrix, INIT_BALL_SCALE);
            g_ball2_matrix = glm::translate(g_ball2_matrix, INIT_POS_start);
            g_ball2_matrix = glm::translate(g_ball2_matrix, g_ball2_pos);
    //        g_ball1_matrix = glm::rotate(g_ball1_matrix, glm::radians(0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
            g_ball2_matrix = glm::scale(g_ball2_matrix, INIT_BALL_SCALE);
        }
        else if (g_app_numballs == THREE){
            /* Delta Time Calculations */
            float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND; // current # of ticks
            float delta_time = ticks - g_previous_ticks; // tick difference from the last frame
            g_previous_ticks = ticks;
            
    //        g_triangle_rotate += 1 * delta_time;
            g_paddle1_pos += g_paddle1_movement * delta_time *5.0f;
            g_paddle2_pos += g_paddle2_movement * delta_time *5.0f;
            g_ball1_pos += g_ball1_movement * delta_time *5.0f;
            g_ball2_pos += g_ball2_movement * delta_time *5.0f;
            g_ball3_pos += g_ball3_movement * delta_time *5.0f;

            //    g_paddle1_pos.y = r * sin(g_triangle_rotate);
            //    g_paddle2_pos.y = -r * sin(g_triangle_rotate);
            
            //for when paddle hits its boundaries
            //top and bottom
            if (g_paddle1_pos.y > 3.0f || g_paddle1_pos.y < -3.0f){
                g_paddle1_pos.y -= g_paddle1_movement.y * delta_time * 5.0f;
            }
            if (g_paddle2_pos.y > 3.0f || g_paddle2_pos.y < -3.0f){
                g_paddle2_pos.y -= g_paddle2_movement.y * delta_time * 5.0f;
            }
            if (g_ball1_pos.y > 3.4f || g_ball1_pos.y < -3.4f){
                g_ball1_pos.y -= g_ball1_pos.y * delta_time ;
                g_ball1_movement.y = g_ball1_movement.y * -1;
            }
            if (g_ball2_pos.y > 3.4f || g_ball2_pos.y < -3.4f){
                g_ball2_pos.y -= g_ball2_pos.y * delta_time ;
                g_ball2_movement.y = g_ball2_movement.y * -1;
            }
            if (g_ball3_pos.y > 3.4f || g_ball3_pos.y < -3.4f){
                g_ball3_pos.y -= g_ball3_pos.y * delta_time ;
                g_ball3_movement.y = g_ball3_movement.y * -1;
            }
            
            if (g_ball1_pos.x > 5.0f ||g_ball2_pos.x > 5.0f ||g_ball3_pos.x >5.0f){
                g_app_mode = LEFTEND;
            }
            if(g_ball1_pos.x < -5.0f|| g_ball2_pos.x < -5.0f||g_ball3_pos.x<-5.0f ){
                g_app_mode = RIGHTEND;
            }

            //Collision Detection
            //ball 1
            float x_distance = fabs(g_ball1_pos.x + INIT_POS_start.x - g_paddle2_pos.x - INIT_POS_paddle2.x) -
                ((INIT_BALL_SCALE.x + INIT_SCALE.x) / 2.0f);

            float y_distance = fabs(g_ball1_pos.y + INIT_POS_start.y - g_paddle2_pos.y- INIT_POS_paddle2.y) -
                ((INIT_BALL_SCALE.y + INIT_SCALE.y) / 2.0f);
            
            if (x_distance < 0.0f && y_distance < 0.0f)
            {
                g_ball1_movement.x = g_ball1_movement.x * -1;
                g_ball1_pos.x -= g_ball1_pos.x * delta_time  ;
            }
            float x_distance_left = fabs(g_ball1_pos.x + INIT_POS_start.x - g_paddle1_pos.x- INIT_POS_paddle1.x) -
                ((INIT_BALL_SCALE.x + INIT_SCALE.x) / 2.0f);

            float y_distance_left = fabs(g_ball1_pos.y + INIT_POS_start.y - g_paddle1_pos.y - INIT_POS_paddle1.y) -
                ((INIT_BALL_SCALE.y + INIT_SCALE.y) / 2.0f);
            
            if (x_distance_left < 0.0f && y_distance_left < 0.0f)
            {
                g_ball1_movement.x = g_ball1_movement.x * -1;
                g_ball1_pos.x -= g_ball1_pos.x * delta_time ;
            }
            //ball2
            x_distance = fabs(g_ball2_pos.x + INIT_POS_start.x - g_paddle2_pos.x - INIT_POS_paddle2.x) -
                ((INIT_BALL_SCALE.x + INIT_SCALE.x) / 2.0f);

            y_distance = fabs(g_ball2_pos.y + INIT_POS_start.y - g_paddle2_pos.y- INIT_POS_paddle2.y) -
                ((INIT_BALL_SCALE.y + INIT_SCALE.y) / 2.0f);
            
            if (x_distance < 0.0f && y_distance < 0.0f)
            {
                g_ball2_movement.x = g_ball2_movement.x * -1;
                g_ball2_pos.x -= g_ball2_pos.x * delta_time  ;
            }
             x_distance_left = fabs(g_ball2_pos.x + INIT_POS_start.x - g_paddle1_pos.x- INIT_POS_paddle1.x) -
                ((INIT_BALL_SCALE.x + INIT_SCALE.x) / 2.0f);

             y_distance_left = fabs(g_ball2_pos.y + INIT_POS_start.y - g_paddle1_pos.y - INIT_POS_paddle1.y) -
                ((INIT_BALL_SCALE.y + INIT_SCALE.y) / 2.0f);
            
            if (x_distance_left < 0.0f && y_distance_left < 0.0f)
            {
                g_ball2_movement.x = g_ball2_movement.x * -1;
                g_ball2_pos.x -= g_ball2_pos.x * delta_time ;
            }
            
            //ball3
            x_distance = fabs(g_ball3_pos.x + INIT_POS_start.x - g_paddle2_pos.x - INIT_POS_paddle2.x) -
                ((INIT_BALL_SCALE.x + INIT_SCALE.x) / 2.0f);

            y_distance = fabs(g_ball3_pos.y + INIT_POS_start.y - g_paddle2_pos.y- INIT_POS_paddle2.y) -
                ((INIT_BALL_SCALE.y + INIT_SCALE.y) / 2.0f);
            
            if (x_distance < 0.0f && y_distance < 0.0f)
            {
                g_ball3_movement.x = g_ball3_movement.x * -1;
                g_ball3_pos.x -= g_ball3_pos.x * delta_time  ;
            }
             x_distance_left = fabs(g_ball3_pos.x + INIT_POS_start.x - g_paddle1_pos.x- INIT_POS_paddle1.x) -
                ((INIT_BALL_SCALE.x + INIT_SCALE.x) / 2.0f);

             y_distance_left = fabs(g_ball3_pos.y + INIT_POS_start.y - g_paddle1_pos.y - INIT_POS_paddle1.y) -
                ((INIT_BALL_SCALE.y + INIT_SCALE.y) / 2.0f);
            
            if (x_distance_left < 0.0f && y_distance_left < 0.0f)
            {
                g_ball3_movement.x = g_ball3_movement.x * -1;
                g_ball3_pos.x -= g_ball3_pos.x * delta_time ;
            }
            
            g_paddle1_matrix = glm::mat4(1.0f);
            g_paddle2_matrix = glm::mat4(1.0f);
            g_ball1_matrix = glm::mat4(1.0f);
            g_ball2_matrix = glm::mat4(1.0f);
            g_ball3_matrix = glm::mat4(1.0f);
            
            if(g_app_player == SINGLE){
                g_triangle_rotate += 1 * delta_time;
                g_paddle1_pos.y = 2.93f * sin(g_triangle_rotate);

            }
            g_paddle1_matrix = glm::translate(g_paddle1_matrix, INIT_POS_paddle1);
            g_paddle1_matrix = glm::translate(g_paddle1_matrix, g_paddle1_pos);
            g_paddle1_matrix = glm::scale(g_paddle1_matrix, INIT_SCALE);
            
            g_paddle2_matrix = glm::translate(g_paddle2_matrix, INIT_POS_paddle2);
            g_paddle2_matrix = glm::translate(g_paddle2_matrix, g_paddle2_pos);
            g_paddle2_matrix = glm::scale(g_paddle2_matrix, INIT_SCALE);
            
            g_ball1_matrix = glm::translate(g_ball1_matrix, INIT_POS_start);
            g_ball1_matrix = glm::translate(g_ball1_matrix, g_ball1_pos);
    //        g_ball1_matrix = glm::rotate(g_ball1_matrix, glm::radians(0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
            g_ball1_matrix = glm::scale(g_ball1_matrix, INIT_BALL_SCALE);
            g_ball2_matrix = glm::translate(g_ball2_matrix, INIT_POS_start);
            g_ball2_matrix = glm::translate(g_ball2_matrix, g_ball2_pos);
    //        g_ball1_matrix = glm::rotate(g_ball1_matrix, glm::radians(0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
            g_ball2_matrix = glm::scale(g_ball2_matrix, INIT_BALL_SCALE);
            g_ball3_matrix = glm::translate(g_ball3_matrix, INIT_POS_start);
            g_ball3_matrix = glm::translate(g_ball3_matrix, g_ball3_pos);
    //        g_ball1_matrix = glm::rotate(g_ball1_matrix, glm::radians(0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
            g_ball3_matrix = glm::scale(g_ball3_matrix, INIT_BALL_SCALE);
        }
    }
    else if(g_app_mode == LEFTEND){
        g_left_matrix = glm::mat4(1.0f);
        g_end_matrix = glm::mat4(1.0f);
        g_winner_matrix = glm::mat4(1.0f);
        g_end_matrix = glm::translate(g_end_matrix, INIT_POS_start);
        g_end_matrix = glm::scale(g_end_matrix, INIT_BALL_SCALE);
        g_winner_matrix = glm::translate(g_winner_matrix, INIT_POS_winner);
        g_winner_matrix = glm::scale(g_winner_matrix, INIT_BALL_SCALE);
        g_left_matrix = glm::translate(g_left_matrix, INIT_POS_left);
        g_left_matrix = glm::scale(g_left_matrix, INIT_BALL_SCALE);

    }
    else if(g_app_mode == RIGHTEND){
        g_right_matrix = glm::mat4(1.0f);
        g_end_matrix = glm::mat4(1.0f);
        g_winner_matrix = glm::mat4(1.0f);
        g_end_matrix = glm::translate(g_end_matrix, INIT_POS_start);
        g_end_matrix = glm::scale(g_end_matrix, INIT_BALL_SCALE);
        g_winner_matrix = glm::translate(g_winner_matrix, INIT_POS_winner);
        g_winner_matrix = glm::scale(g_winner_matrix, INIT_BALL_SCALE);
        g_right_matrix = glm::translate(g_right_matrix, INIT_POS_right);
        g_right_matrix = glm::scale(g_right_matrix, INIT_BALL_SCALE);

    }
}

void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so use 6, not 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
        
        // Vertices
        float vertices[] =
        {
            -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
            -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
        };

        // Textures
        float texture_coordinates[] =
        {
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
            0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
        };
        
        glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false,
                              0, vertices);
        glEnableVertexAttribArray(g_shader_program.get_position_attribute());
        
        glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
                              false, 0, texture_coordinates);
        glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
        
        // Bind texture
        if(g_app_mode== PLAYING){
            draw_object(g_paddle1_matrix, g_paddle1_texture_id);
            draw_object(g_paddle2_matrix, g_paddle2_texture_id);
            if(g_app_numballs == ONE){
                draw_object(g_ball1_matrix, g_ball_texture_id);
            }
            if(g_app_numballs == TWO){
                draw_object(g_ball1_matrix, g_ball_texture_id);
                draw_object(g_ball2_matrix, g_ball_texture_id);
            }
            if(g_app_numballs == THREE){
                draw_object(g_ball1_matrix, g_ball_texture_id);
                draw_object(g_ball2_matrix, g_ball_texture_id);
                draw_object(g_ball3_matrix, g_ball_texture_id);
            }
        }
    if(g_app_mode== LEFTEND){
        draw_object(g_end_matrix, g_end_texture_id);
        draw_object(g_winner_matrix, g_winner_texture_id);
        draw_object(g_left_matrix, g_left_texture_id);
    }
    if(g_app_mode== RIGHTEND){
        draw_object(g_end_matrix, g_end_texture_id);
        draw_object(g_winner_matrix, g_winner_texture_id);
        draw_object(g_right_matrix, g_right_texture_id);
    }
        // We disable two attribute arrays now
        glDisableVertexAttribArray(g_shader_program.get_position_attribute());
        glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
        
        SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


int main()
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
