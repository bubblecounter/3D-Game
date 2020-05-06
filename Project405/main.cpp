//Written by A.Furkan Okuyucu 19736
//It is a 3D video game written using OpenGL, glew, glfw, libraries.
//
//Goal: Collecting as many cubes as possible whoose emoji is same as player emoji.
//
//Points: +5 point for collecting cube with same emoji.
//		  -2 points for collecting cube with different emoji.
//
//Interactions: WASD for player movement
//				SPACE for moving up or down
//				P for pausing the game
//				R for resuming the game
//				C for opening Cheat Mode (In cheat mode you can go everywhere using mouse and WASD
//				V for closing the Cheat Mode
//
//Functionalities:
//1.3D viewing and objects
//2.User Input (with keyboard for player movement, with mouse for camera movement in cheat mode)
//3.Lighting and Smooth Shading: 1 light source at top front right corner of the scene
//							   : normal vectors for surfaces defined in vertices array
//							   : Ambient, Diffuse and Specualr lighting applied on objects
//4.Texture Mapping			   : Used while drawing scene and emojis on cubes
//							   : Texture coordinates for surfaces defined in vertices array
//
//Advanced Computer Graphics Techniques
//1.Collision Detection: Implemented using Axis Aligned Bounding Boxes(AABB) algorithm
//					   : Used to detect collision between player and other cubes
//2. Shadow Mapping	   : First rendered the scene from light perspective
//				       : The depth values stored in a texture
//					   : Then rendered the scene as usual with shadow map
//					   : In fragment shader  for each fragment check whether the depth value in shadow map is
//					smaller than the closest depth value. Do shadowing with these info
//					   : used shadow bias to prevent shadow acne
//					   : Set the resolution of shadow as follows WIDTH = 4096, HEIGHT = 4096 to make the shadows smoother
//
//References:
//For AABB implementation :https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-detection
//For Shadow Mapping	  :https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
//For Text Rendering	  :https://learnopengl.com/In-Practice/Text-Rendering
//For most of the other stuff implemented: Tutorials from "https://learnopengl.com"

#include <sstream>
#include <fstream>
#include <iostream>
#include <map>

#include <GL/glew.h>				// GLEW, binds OpenGL functions
#include <GLFW/glfw3.h>				//handles user input, creates opengl context

#define STB_IMAGE_IMPLEMENTATION	//to load images
#include "stb_image.h" 
#include <glm/glm.hpp>				//for matrix translation, tranformation
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>				// to render fonts
#include FT_FREETYPE_H  

#include "Cube.h"

//Defined Functions
void glfwInitialization();
void glewInitialization();
GLFWwindow * windowCreation();
int createShader(GLenum type, const char* shaderpath);
int createShaderProgram(GLenum type, const char* shaderpath, GLenum type2, const char* shaderpath2);
void loadTexture(unsigned int &texture1, GLenum type, const char* imagesource);
void RenderText(int shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color); //For text rendering

//Callback Functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
using namespace std;

//Camera
glm::vec3 cameraPos = glm::vec3(0.0f, -2.5f, 20.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float lastX = 400, lastY = 300;		//center
float yaw = -89.0f, pitch = 0.0f;
bool firstMouse = true;
float fov = 60.0f;					//field of view angle 

//Timing
float deltaTime = 0.0f;				// Time between current frame and last frame
float lastFrame = 0.0f;				// Time of last frame

// Lighting
glm::vec3 lightPos(3.0f, 0.0f, 23.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);//White

//Player
glm::vec3 playerPos = glm::vec3(0.0f, -4.0f, 10.0f);
Cube player = Cube(playerPos, 1.0f, (rand() % (3 - 1 + 1)) + 1, 2.5f);
AABB endbox = AABB(glm::vec3(-15.0f, -9.0f, 20.0f), glm::vec3(15.0f, 9.0f, 20.0f));
AABB leftboundingbox = AABB(glm::vec3(-4.75f, -4.5f, -6.0f), glm::vec3(-4.75f, 4.5f, 20.0f));
AABB rightboundingbox = AABB(glm::vec3(4.75f, -4.5f, -6.0f), glm::vec3( 4.75f, 4.5f, 20.0f));
AABB backbox = AABB(glm::vec3(-4.75f, -4.5f, -6.0f), glm::vec3(4.75f, 4.5f,-6.0f));
//Game Logic
bool goUp = false;
bool goDown = false;
bool cheatMode = false;
bool pause = false;
bool sameExist = false;
int destroyedCubes = 0;
string score = "Score:";
string gameStatus = "Playing";
int points = 0;

//Text Rendering
struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};
map<GLchar, Character> Characters;

//Vertex Buffer and Vertex Array Objects
unsigned int VBO, cubeVAO, lightVAO, textVAO, textVBO;

int main() {
	glfwInitialization();

	//window creation
	GLFWwindow* window = windowCreation();

	//initialize glew
	glewInitialization();

	//Callback Functions
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);	//set callback function for window resizing
	glfwSetCursorPosCallback(window, mouse_callback);					//set callback function for mouse input

	//Set GL Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);		//hide cursor and follow it
	glEnable(GL_DEPTH_TEST);											//eneable dept
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Shader Programs Creation
	int shaderProgram = createShaderProgram(GL_VERTEX_SHADER, "shader.vs", GL_FRAGMENT_SHADER, "shader.fs");			//main
	int lampshaderProgram = createShaderProgram(GL_VERTEX_SHADER, "lampshader.vs", GL_FRAGMENT_SHADER, "lampshader.fs");//lamp
	int textshaderProgram = createShaderProgram(GL_VERTEX_SHADER, "text.vs", GL_FRAGMENT_SHADER, "text.fs");			//text rendering
	int depthShaderProgram = createShaderProgram(GL_VERTEX_SHADER, "depth.vs", GL_FRAGMENT_SHADER, "depth.fs");			//shadow mapping


	float vertices[] = {
		//For Cube objects
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		
		//bottom of scene
		-5.0f, -4.5f, -6.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 5.0f, -4.5f, -6.0f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 5.0f, -4.5f,  20.0f, 0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 5.0f, -4.5f,  20.0f, 0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-5.0f, -4.5f,  20.0f, 0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-5.0f, -4.5f, -6.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		//top of scene
		-5.0f,  4.5f, -6.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 5.0f,  4.5f, -6.0f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 5.0f,  4.5f,  20.0f, 0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 5.0f,  4.5f,  20.0f, 0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-5.0f,  4.5f,  20.0f, 0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-5.0f,  4.5f, -6.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		//back of scene
		-5.0f, -4.5f, -6.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 5.0f, -4.5f, -6.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 5.0f,  4.5f, -6.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 5.0f,  4.5f, -6.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-5.0f,  4.5f, -6.0f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-5.0f, -4.5f, -6.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		//right of scene
		 5.0f,  4.5f,  20.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 5.0f,  4.5f,  -6.0f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 5.0f, -4.5f,  -6.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 5.0f, -4.5f,  -6.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 5.0f, -4.5f,  20.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 5.0f,  4.5f,  20.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		//left of scene
		-5.0f,  4.5f,  20.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-5.0f,  4.5f,  -6.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-5.0f, -4.5f,  -6.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-5.0f, -4.5f,  -6.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-5.0f, -4.5f,  20.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-5.0f,  4.5f,  20.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
	};
	//initialize cube objects
	glm::vec3 cubePositions[] = {
	glm::vec3(-4.0f,-4.0f,-5.0f),
	glm::vec3(-2.0f,-4.0f,-5.0f),
	glm::vec3(0.0f,-4.0f,-5.0f),
	glm::vec3(2.0f,-4.0f,-5.0f),
	glm::vec3(4.0f,-4.0f,-5.0f),
	glm::vec3(-4.0f, 4.0f,-5.0f),
	glm::vec3(-2.0f, 4.0f,-5.0f),
	glm::vec3(0.0f, 4.0f,-5.0f),
	glm::vec3(2.0f, 4.0f,-5.0f),
	glm::vec3(4.0f, 4.0f,-5.0f),
	};
	Cube cubes[10];
	for (int i = 0; i < 11; i++) {
		cubes[i] = Cube(cubePositions[i], 1.0f, (rand() % (3 - 1 + 1)) + 1, ((rand() % (20 - 1 + 1)) + 1) / 10.0);
	}

	//Create Vertex Buffer and Vertex Array Objects for cubes
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindVertexArray(cubeVAO);
	//position of vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//normal vectors
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture coordinate
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//Create Vertex Array object for lamp
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//position of vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Load Textures
	unsigned int texture1, texture2, texture3, texture4, texture5, texture6;
	loadTexture(texture1, GL_RGB, "container.jpg");
	loadTexture(texture2, GL_RGBA, "face1.png");
	loadTexture(texture3, GL_RGBA, "face2.png");
	loadTexture(texture4, GL_RGBA, "face3.png");

    //******TEXT RENDERING*****
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(800), 0.0f, static_cast<GLfloat>(600));
	glUseProgram(textshaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(textshaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	// FreeType
	FT_Library ft;
	FT_Init_FreeType(&ft);
	// Load font as face
	FT_Face face;
	FT_New_Face(ft, "arial.ttf", 0, &face);
	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 48);
	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// Load first 128 characters of ASCII set
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		FT_Load_Char(face, c, FT_LOAD_RENDER);
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<GLchar, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 4);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	// Configure VAO/VBO for texture quads
	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	//*****SHADOW MAPPING*****
	const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096; // Define shadow resolution
	//Create Frame Buffer Object
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//shader config
	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 4);

	//*****Rendering loop*****
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//*****Render Object from Light's Perspective****		
		//Render depth of scene to texture (from light's perspective)
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 1.0f, far_plane = 30.0f;
		//lightProjection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, near_plane, far_plane);
		lightProjection = glm::perspective(glm::radians(120.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, 0.1f, 30.0f);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		glUseProgram(depthShaderProgram);
		glUniformMatrix4fv(glGetUniformLocation(depthShaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		// retrieve the matrix uniform locations
		unsigned int modelLoc = glGetUniformLocation(depthShaderProgram, "model");
		glm::mat4 model = glm::mat4(1.0f);
		//assign value to model matrix
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(cubeVAO);
		//Render player
		model = glm::translate(model, player.position);
		model = glm::scale(model, glm::vec3(player.scale));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//Render other cubes
		for (int i = 0; i < 10; i++) {
			if (cubes[i].destroyed == false) {
				model = glm::mat4(1.0f);
				model = glm::translate(model, cubes[i].position);
				model = glm::scale(model, glm::vec3(cubes[i].scale));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}
		//render scene 
		model = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 36, 66);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// reset viewport
		glViewport(0, 0, 800, 600);

		//*****Render Scene from Camera's Perspective as usual*****
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Activate textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texture3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, texture4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		//activate Shader
		glUseProgram(shaderProgram);

		glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
		glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

		// create transformation matrices
		model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);

		//*****Collision Detection using AABB*****
		for (int i = 0; i < 10; i++) {
			if (cubes[i].destroyed == false) {
				if (player.collidesWith(cubes[i])) {
					if (player.emojiNum == cubes[i].emojiNum) {
						points += 5;
					}
					else {
						points -= 2;
					}
					cubes[i].destroyed = true;
					destroyedCubes++;
				}
				if (cubes[i].collidesWith(endbox)) {
					cubes[i].destroyed = true;
					destroyedCubes++;
				}
			}
		}
		//*****Check Whether All Cubes destroyed*****
		if (destroyedCubes == 10) {
			gameStatus = "finished";
		}
		//*****Check Whether Cube with Player's emoji exist
		//if not exists change the emoji of Player
		if (gameStatus != "finished") {
			sameExist = false;
			for (int i = 0; i < 10; i++) {
				if (cubes[i].destroyed == false && player.emojiNum == cubes[i].emojiNum) {
					sameExist = true;
				}
			}
			if (!sameExist) {
				player.emojiNum = player.emojiNum % 3;
				player.emojiNum++;
			}
		}

		//Move player Up and Down
		if (goUp == true) {
			cameraPos.y += 1.2f * deltaTime;
			if (playerPos.y < 4.0f) {
				playerPos.y += 2.0f * deltaTime;
				player.updateCubePosition(playerPos);
			}
			else if (playerPos.y >= 4.0f) {
				playerPos.y = 4.0f;
				player.updateCubePosition(playerPos);
				if (cheatMode) {
					goUp = false;
				}
				else if (cameraPos.y >= 2.5) {
					cameraPos.y = 2.5;
					goUp = false;
				}
			}
		}
		else if (goDown == true) {
			cameraPos.y -= 1.2f * deltaTime;
			if (playerPos.y > -4.0f) {
				playerPos.y -= 2.0f * deltaTime;
				player.updateCubePosition(playerPos);
			}
			else if (playerPos.y <= -4.0f) {
				playerPos.y = -4.0f;
				player.updateCubePosition(playerPos);
				if (cheatMode) {
					goDown = false;
				}
				else if (cameraPos.y <= -2.5) {
					cameraPos.y = -2.5;
					goDown = false;
				}
			}
		}

		//Assign view and projection matrix w.r.t. Camera's perspective and position
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		projection = glm::perspective(glm::radians(fov), (float)800 / (float)600, 0.1f, 200.0f);

		// retrieve the matrices location
		modelLoc = glGetUniformLocation(shaderProgram, "model");
		unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
		unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

		//assign values to model view and projection matrix
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(cubeVAO);

		//Render player object
		model = glm::translate(model, player.position);
		model = glm::scale(model, glm::vec3(player.scale));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), player.emojiNum);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Render other cubes
		for (int i = 0; i < 10; i++) {
			if (cubes[i].destroyed == false) {
				if (pause == false) {
					cubes[i].updateCubePosition(cubes[i].position + (cubes[i].speed * deltaTime)*glm::vec3(0.0f, 0.0f, 1.0f));
				}
				model = glm::mat4(1.0f);
				model = glm::translate(model, cubes[i].position);
				model = glm::scale(model, glm::vec3(cubes[i].scale));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), cubes[i].emojiNum);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}

		//render scene 
		model = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 0);
		glDrawArrays(GL_TRIANGLES, 36, 66);

		//render lamp object
		glUseProgram(lampshaderProgram);
		glUniformMatrix4fv(glGetUniformLocation(lampshaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lampshaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(lightVAO);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		modelLoc = glGetUniformLocation(lampshaderProgram, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(glGetUniformLocation(lampshaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
		glUniform3fv(glGetUniformLocation(lampshaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Display Score and game Status
		RenderText(textshaderProgram, score + to_string(points), 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
		RenderText(textshaderProgram, gameStatus, 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));

		glfwSwapBuffers(window); // swap front and back buffers
		glfwPollEvents(); //check if inputs triggered

	}
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();//deletes all resources acquired by glfw
	return 0;
}

//A function to generate texture from given path of the image
//It is using stb_image library to get the data from images
void loadTexture(unsigned int &texture, GLenum type, const char* imagesource)
{
	unsigned char *data;
	int width, height, nrChannels;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set texture1 filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load(imagesource, &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
}
//Function to create shader program with defined vertex and fragment shaders
//It takes path of the 2 shader and their types as input
//It returns the id of the created shader program
int createShaderProgram(GLenum type, const char* shaderpath, GLenum type2, const char* shaderpath2)
{
	int vertexshader = createShader(type, shaderpath);
	int fragmentshader = createShader(type2, shaderpath2);
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexshader);
	glAttachShader(shaderProgram, fragmentshader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);
	return shaderProgram;
}
// Function to create shaders with given type and source path
//returns the shader id
int createShader(GLenum type, const char * shaderpath) {
	//read shader code from file
	string shaderCode;
	ifstream shaderFile;
	try {
		shaderFile.open(shaderpath);
		stringstream shaderStream;
		shaderStream << shaderFile.rdbuf();
		shaderFile.close();
		shaderCode = shaderStream.str();
	}
	catch (ifstream::failure e) {
		cout << "Error in reading shader with source path " << shaderpath << endl;
	}
	const char* shaderSource = shaderCode.c_str();
	int shader = glCreateShader(type);
	glShaderSource(shader, 1, &shaderSource, NULL);
	glCompileShader(shader);
	return shader;
}

//Function to display the text on screen
void RenderText(int shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	// Activate corresponding render state	
	glUseProgram(shader);
	glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBindVertexArray(textVAO);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices2[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices2), vertices2); // Be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(textVAO);
	glBindTexture(GL_TEXTURE_2D, 4);
}
//Function to create window
GLFWwindow* windowCreation()
{
	GLFWwindow* window = glfwCreateWindow(800, 600, "CS 405 Game Project", NULL, NULL);
	if (window == NULL) {
		cout << "Failed to Create GLFW window" << endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);
	return window;
}

void glewInitialization()
{
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		throw runtime_error((const char*)glewGetErrorString(err));
	}
}

void glfwInitialization()
{
	//glfw initialization
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

//a callback function which adjust viewport according to changes in window size
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

//a callback function which process keyboard inputs
void processInput(GLFWwindow *window) {
	float cameraSpeed = 2.5f * deltaTime;
	if (cheatMode) {
		cameraSpeed = 7.0f * deltaTime;
	}
	//if esc pressed close window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		
		if (!cheatMode && !player.collidesWith(backbox)) {
			playerPos += cameraSpeed * glm::vec3(0.0f, 0.0f, -1.0f);
			player.updateCubePosition(playerPos);
			cameraPos += cameraSpeed * cameraFront;
		}
		else if (cheatMode) {
			cameraPos += cameraSpeed * cameraFront;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		
		if (!cheatMode && !player.collidesWith(endbox)) {
			cameraPos -= cameraSpeed * cameraFront;
			playerPos -= cameraSpeed * glm::vec3(0.0f, 0.0f, -1.0f);
			player.updateCubePosition(playerPos);
		}
		else if(cheatMode){
			cameraPos -= cameraSpeed * cameraFront;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		
		if (!cheatMode && !player.collidesWith(rightboundingbox)) {
			playerPos += cameraSpeed * glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, -1.0f), cameraUp));
			player.updateCubePosition(playerPos);
			cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
		}
		else if (cheatMode) {
			cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
		}
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		if (!cheatMode && !player.collidesWith(leftboundingbox)) {
			playerPos -= cameraSpeed * glm::normalize(glm::cross(glm::vec3(0.0f, 0.0f, -1.0f), cameraUp)); // we normalize for consistent movement speed
			player.updateCubePosition(playerPos);
			cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp)); // we normalize for consistent movement speed
		}
		else if (cheatMode) {
			cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp)); // we normalize for consistent movement speed
		}
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (!cheatMode) {
			if (cameraPos.y < 0.0f) {
				goUp = true;
			}
			else {
				goDown = true;
			}
		}
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		cheatMode = true;
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
		if (cheatMode && goUp == false && goDown == false) {
			cheatMode = false;
			yaw = -90.0f;
			pitch = 0.0f;
			cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
			if (player.position.y < 0) {
				cameraPos.y = -2.5f;
			}
			else {
				cameraPos.y = 2.5f;
			}
			cameraPos.z = player.position.z + 10.0f;
			cameraPos.x = player.position.x;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		pause = true;
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		pause = false;
	}
}

void mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	if (cheatMode) {
		if (firstMouse) // this bool variable is initially set to true
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}
		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.05f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;
		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f) //if it passes 90 degree it reverses
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		front.y = sin(glm::radians(pitch));
		front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		cameraFront = glm::normalize(front);
	}
}

