#define _USE_MATH_DEFINES
//std includes
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <utility>
#include <map>
#include <tuple>

//glew include
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//GLM include
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// OpenAL include
#include <AL/alut.h>
// OpenAL Defines
#define NUM_BUFFERS 3
#define NUM_SOURCES 3
#define NUM_ENVIRONMENTS 1

// Include Headers
#include "Headers/AnimationUtils.h"
#include "Headers/Box.h"
#include "Headers/Colisiones.h"
#include "Headers/Cylinder.h"
#include "Headers/FirstPersonCamera.h"
#include "Headers/FontTypeRendering.h"
#include "Headers/Model.h"
#include "Headers/Shader.h"
#include "Headers/ShadowBox.h"
#include "Headers/Sphere.h"
#include "Headers/ThirdPersonCamera.h"
#include "Headers/TimeManager.h"
#include "Headers/Terrain.h"
#include "Headers/Texture.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
/***************************************************************************************/
/********************Constantes y Varables usadas en Tiempo y Pantalla *****************/
/***************************************************************************************/
const size_t SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
const double GRAVITY = 1.81;

bool beginScene = false, pressOption = false, exitApp = false;

double deltaTime;
double currTime, lastTime;

int lastMousePosX, offsetX = 0;
int lastMousePosY, offsetY = 0;
size_t screenWidth, screenHeight;

float distanceFromTarget = 7.0;
std::shared_ptr<ThirdPersonCamera> camera(new ThirdPersonCamera());

GLFWwindow *window;
/*********************************************************************************************/
/********************Shaders/Vista de Colisión/Terreno del Mapa*******************************/
/*********************************************************************************************/
Shader shader, shaderSkybox, shaderMulLighting, shaderTerrain, shaderTexture;
Shader shaderDepth, shaderViewDepth;
Shader shaderParticles;

Sphere sphereSkybox(20, 20);
Box boxTerrain;

Box boxCollider;
Sphere sphereCollider(10, 10);
Cylinder rayCollider(10, 10, 1.0, 1.0, 1.0);


FontTypeRendering::FontTypeRendering *modelText;
Box boxIntro;

Terrain terrain(-1, -1, 200, 8, "../Textures/heightmap.png");
/********************************************************************************************/
/************************************Estructura de modelos***********************************/
/********************************************************************************************/
// Modelo Principal
Model modelTankChasis;
Model modelTankTurret;
Model modelTankTracks;
Model modelTankCannon;
glm::mat4 modelMatrixTankChasis = glm::mat4(1.0f);
glm::mat4 modelMatrixTankTurret = glm::mat4(1.0f);
glm::mat4 modelMatrixTankCannon = glm::mat4(1.0f);
size_t animDHTankTracksIndex = 1, animDHTankCannonIndex = 1;

//Proyectiles del jugador
Model modelProyectile;
const double VEL_PROY = 1.1, COOLDOWN = 0.12;
const size_t MAX_N_PROYECTILES = 10;
size_t currProy = 0;
double currCool = 0.0;

std::vector<glm::mat4> vecModelMatrixProy;
std::vector<bool> vecTriggerFire;
std::vector<double> vecStartTimeShoot;

//Salto del jugador
bool isJump = false;
double tmvJump = 0.0, startTimeJump = 0.0;

// Modelos de Edificios
Model modelBuildingA;
std::map<std::string, std::vector<std::pair<glm::vec3, float>>> dataBuilds = {
	{"EdificioA", {{glm::vec3(-36.52, 0, -23.24), 111.37}, {glm::vec3(-52.73, 0, -3.90), 25.0}}},
	{"EdificioB", {{glm::vec3(-36.52, 0, -23.24), 111.37}, {glm::vec3(-52.73, 0, -3.90), 25.0}}},
	{"EdificioC", {{glm::vec3(-36.52, 0, -23.24), 111.37}, {glm::vec3(-52.73, 0, -3.90), 25.0}}}
};

// Soldado Enemigo
Model modelSoldierEnemy;
Routine rSE1(1, 1 ,5 ,300);//routineSoldierEnemy
std::vector<size_t> vecAnimationSEIndex = {
	0
};

void init_rSE1(){
	rSE1.setKeyFrame(0, 0, 90.0f, glm::vec3(-7.03, 0, -19.14));
	rSE1.setKeyFrame(1, 0, 80.0f, glm::vec3(24.41, 0, -34.57));
	rSE1.setKeyFrame(2, 0, 70.0f, glm::vec3(-10.15, 0, -54.1));
	rSE1.setKeyFrame(3, 0, 60.0f, glm::vec3(-24.41, 0, 34.57));
	rSE1.setKeyFrame(4, 0, 50.0f, glm::vec3( 10.15, 0, 54.1));
	rSE1.setAtCero();
}
/********************************************************************************************/
/***********************************************Valores GL***********************************/
/********************************************************************************************/
// Framesbuffers
ShadowBox * shadowBox;
GLuint depthMap, depthMapFBO;
Box boxViewDepth;

GLuint textureTerrain[5]; // textureCespedID, textureTerrainRID, textureTerrainGID, textureTerrainBID, textureTerrainBlendMapID;
GLuint textureBeginMenuID[2];
GLuint textureCurrentMenuID, textureScreenID, textureParticleID, textureSkyboxID;

GLenum types[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

std::string fileNames[6] = { 
	"../Textures/mp_bloodvalley/blood-valley_ft.tga",
	"../Textures/mp_bloodvalley/blood-valley_bk.tga",
	"../Textures/mp_bloodvalley/blood-valley_up.tga",
	"../Textures/mp_bloodvalley/blood-valley_dn.tga",
	"../Textures/mp_bloodvalley/blood-valley_rt.tga",
	"../Textures/mp_bloodvalley/blood-valley_lf.tga" };
/********************************************************************************************/
/************************************Capa de Colision****************************************/
/********************************************************************************************/

std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>> buildingsCollOBB;
std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>> mainCharCollOBB;
std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>> enemyCollOBB;
std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>> proyectileCollSBB;
std::map<std::string, std::tuple<AbstractModel::RAY, glm::mat4, glm::mat4>> collidersRAY;

/********************************************************************************************/
/************************************Opciones de Al*****************************************/
/********************************************************************************************/
// Listener
ALfloat listenerPos[] = { 0.0, 0.0, 4.0 };
ALfloat listenerVel[] = { 0.0, 0.0, 0.0 };
ALfloat listenerOri[] = { 0.0, 0.0, 1.0, 0.0, 1.0, 0.0 };
// Source 0
ALfloat source0Pos[] = { -2.0, 0.0, 0.0 };
ALfloat source0Vel[] = { 0.0, 0.0, 0.0 };
// Source 1
ALfloat source1Pos[] = { 2.0, 0.0, 0.0 };
ALfloat source1Vel[] = { 0.0, 0.0, 0.0 };
// Source 2
ALfloat source2Pos[] = { 2.0, 0.0, 0.0 };
ALfloat source2Vel[] = { 0.0, 0.0, 0.0 };
// Buffers
ALuint buffer[NUM_BUFFERS];
ALuint source[NUM_SOURCES];
ALuint environment[NUM_ENVIRONMENTS];
// Configs
ALsizei size, freq;
ALenum format;
ALvoid *data;
size_t ch;
ALboolean loop;
std::vector<bool> sourcesPlay = {true, true, true};

/*********************************************************************************************/
/********************************Declaracion de Funciones*************************************/
/*********************************************************************************************/
// Se definen todos las funciones.
void init(int width, int height, std::string strTitle, bool bFullScreen);
void destroy();
void reshapeCallback(GLFWwindow *Window, int widthRes, int heightRes);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow *window, int button, int state, int mod);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void initParticleBuffers();
void prepareLightScene();
void prepareDepthScene();

bool processInput(bool continueApplication = true);

// Implementacion de todas las funciones.
void init(int width, int height, std::string strTitle, bool bFullScreen){
	/*******************************************
	 * Inicio de fundamentales (No modificar)
	 *******************************************/
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		exit(-1);
	}

	screenWidth = width;
	screenHeight = height;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (bFullScreen)
		window = glfwCreateWindow(width, height, strTitle.c_str(), glfwGetPrimaryMonitor(), nullptr);
	else
		window = glfwCreateWindow(width, height, strTitle.c_str(), nullptr, nullptr);

	if (window == nullptr) {
		std::cerr << "Error to create GLFW window, you can try download the last version of your video card that support OpenGL 3.3+" << std::endl;
		destroy();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glfwSetWindowSizeCallback(window, reshapeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Init glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Failed to initialize glew" << std::endl;
		exit(-1);
	}

	glViewport(0, 0, screenWidth, screenHeight);
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	/*******************************************
	 * Carga de Elementos Globales
	 *******************************************/

	// Inicialización de los shaders
	shader.initialize("../Shaders/colorShader.vs", "../Shaders/colorShader.fs");
	shaderSkybox.initialize("../Shaders/skyBox.vs", "../Shaders/skyBox_fog.fs");
	shaderMulLighting.initialize("../Shaders/iluminacion_textura_animation_shadow.vs", "../Shaders/multipleLights_shadow.fs");
	shaderTerrain.initialize("../Shaders/terrain_shadow.vs", "../Shaders/terrain_shadow.fs");
	shaderTexture.initialize("../Shaders/texturizado.vs", "../Shaders/texturizado.fs");
	shaderViewDepth.initialize("../Shaders/texturizado.vs", "../Shaders/texturizado_depth_view.fs");
	shaderDepth.initialize("../Shaders/shadow_mapping_depth.vs", "../Shaders/shadow_mapping_depth.fs");
	shaderParticles.initialize("../Shaders/particlesFountain.vs", "../Shaders/particlesFountain.fs");

	// Inicializacion de los objetos.
	sphereSkybox.init();
	sphereSkybox.setShader(&shaderSkybox);
	sphereSkybox.setScale(glm::vec3(20.0f));

	boxCollider.init();
	boxCollider.setShader(&shader);
	boxCollider.setColor(glm::vec4(1.0));

	sphereCollider.init();
	sphereCollider.setShader(&shader);
	sphereCollider.setColor(glm::vec4(1.0));

	rayCollider.init();
	rayCollider.setShader(&shader);
	rayCollider.setColor(glm::vec4(1.0));

	boxTerrain.init();
	boxTerrain.setShader(&shaderMulLighting);

	boxIntro.init();
	boxIntro.setShader(&shaderTexture);
	boxIntro.setScale(glm::vec3(2.0, 2.0, 1.0));

	boxViewDepth.init();
	boxViewDepth.setShader(&shaderViewDepth);

	modelBuildingA.loadModel("../models/rock/rock.obj");

	// MainCharacter Tanque Duck-Hunter
	modelTankChasis.loadModel("../models/DuckHunter/chasis.obj");
	modelTankTurret.loadModel("../models/DuckHunter/turret.obj");
	modelTankCannon.loadModel("../models/DuckHunter/cannon.fbx");
	modelTankTracks.loadModel("../models/DuckHunter/track.fbx");
	
	/*******************************************
	* Relleno de Vectores Proyectil
	*******************************************/
	modelProyectile.loadModel("../models/DuckHunter/Proyectile.obj");
	for(size_t i = 0; i < MAX_N_PROYECTILES; i++){
		vecModelMatrixProy.push_back(glm::mat4(1.0f));
		vecTriggerFire.push_back(false);
		vecStartTimeShoot.push_back(0.0);
	}
	
	// Guardian
	modelSoldierEnemy.loadModel("../models/Soldier/Soldier1.fbx");
	init_rSE1();

	// Terreno
	terrain.init();

	prepareLightScene();

	// Se inicializa el model de render text
	modelText = new FontTypeRendering::FontTypeRendering(screenWidth, screenHeight);
	modelText->Initialize();

	camera->setPosition(glm::vec3(0.0, 3.0, 4.0));
	camera->setDistanceFromTarget(distanceFromTarget);
	camera->setSensitivity(1.0);

	/*******************************************
	 * Carga y asignacion de Texturas
	 *******************************************/
	
	// Carga de texturas para el skybox
	Texture skyboxTexture = Texture("");
	glGenTextures(1, &textureSkyboxID);
	// Tipo de textura CUBE MAP
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureSkyboxID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(types); i++) {
		skyboxTexture = Texture(fileNames[i]);
		skyboxTexture.loadImage(true);
		if (skyboxTexture.getData()) {
			glTexImage2D(types[i], 0, skyboxTexture.getChannels() == 3 ? GL_RGB : GL_RGBA, skyboxTexture.getWidth(), skyboxTexture.getHeight(), 0,
			skyboxTexture.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, skyboxTexture.getData());
		} else
			std::cout << "Failed to load texture" << std::endl;
		skyboxTexture.freeImage();
	}
	// Definiendo las texturas del terreno
	std::string textNameTerrain[5] = {"../Textures/grassy2.png", "../Textures/mud.png", "../Textures/grassFlowers.png", "../Textures/path.png", "../Textures/blendMap2.png"};
	for(size_t i = 0; i < 5; i++){
		Texture text(textNameTerrain[i]);
		text.loadImage();
		glGenTextures(1, &textureTerrain[i]);
		glBindTexture(GL_TEXTURE_2D, textureTerrain[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (text.getData()) {
			glTexImage2D(GL_TEXTURE_2D, 0, text.getChannels() == 3 ? GL_RGB : GL_RGBA, text.getWidth(), text.getHeight(), 0,
			text.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, text.getData());
			glGenerateMipmap(GL_TEXTURE_2D);
		} else
			std::cout << "Failed to load texture" << std::endl;
		text.freeImage();
	}

	// Definiendo las texturas del Menu introductorio
	std::string textNameBegMenu[2] = {"../Textures/Intro1.png", "../Textures/Intro2.png"};
	for(size_t i = 0; i < 2; i++){
		Texture text(textNameBegMenu[i]);
		text.loadImage();
		glGenTextures(1, &textureBeginMenuID[i]);
		glBindTexture(GL_TEXTURE_2D, textureBeginMenuID[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (text.getData()) {
			glTexImage2D(GL_TEXTURE_2D, 0, text.getChannels() == 3 ? GL_RGB : GL_RGBA, text.getWidth(), text.getHeight(), 0,
			text.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, text.getData());
			glGenerateMipmap(GL_TEXTURE_2D);
		} else
			std::cout << "Failed to load texture" << std::endl;
		text.freeImage();
	}

	// Definiendo la textura
	Texture textureScreen("../Textures/Screen.png");
	textureScreen.loadImage(); // Cargar la textura
	glGenTextures(1, &textureScreenID); // Creando el id de la textura del landingpad
	glBindTexture(GL_TEXTURE_2D, textureScreenID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textureScreen.getData()){
		glTexImage2D(GL_TEXTURE_2D, 0, textureScreen.getChannels() == 3 ? GL_RGB : GL_RGBA, textureScreen.getWidth(), textureScreen.getHeight(), 0,
		textureScreen.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textureScreen.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textureScreen.freeImage(); // Liberamos memoria

	// Definiendo la textura
	Texture textParticles("../Textures/bluewater.png");
	textParticles.loadImage(); // Cargar la textura
	glGenTextures(1, &textureParticleID); // Creando el id de la textura del landingpad
	glBindTexture(GL_TEXTURE_2D, textureParticleID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textParticles.getData()){
		glTexImage2D(GL_TEXTURE_2D, 0, textParticles.getChannels() == 3 ? GL_RGB : GL_RGBA, textParticles.getWidth(), textParticles.getHeight(), 0,
		textParticles.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textParticles.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textParticles.freeImage(); // Liberamos memoria

	/*******************************************
	 * OpenAL init
	 *******************************************/
	alutInit(0, nullptr);
	alListenerfv(AL_POSITION, listenerPos);
	alListenerfv(AL_VELOCITY, listenerVel);
	alListenerfv(AL_ORIENTATION, listenerOri);
	alGetError(); // clear any error messages
	if (alGetError() != AL_NO_ERROR) {
		std::cout << "- Error creating buffers !!\n";
		exit(1);
	}
	else {
		std::cout << "init() - No errors yet.\n";
	}
	// Generate buffers, or else no sound will happen!
	alGenBuffers(NUM_BUFFERS, buffer);
	buffer[0] = alutCreateBufferFromFile("../sounds/fountain.wav");
	buffer[1] = alutCreateBufferFromFile("../sounds/fire.wav");
	buffer[2] = alutCreateBufferFromFile("../sounds/darth_vader.wav");
	int errorAlut = alutGetError();
	if (errorAlut != ALUT_ERROR_NO_ERROR){
		std::cout << "- Error open files with alut " << errorAlut << " !!\n";
		exit(2);
	}

	alGetError(); /* clear error */
	alGenSources(NUM_SOURCES, source);

	if (alGetError() != AL_NO_ERROR) {
		std::cout << "- Error creating sources !!\n";
		exit(2);
	}
	else {
		std::cout << "init - no errors after alGenSources\n";
	}
	alSourcef(source[0], AL_PITCH, 1.0f);
	alSourcef(source[0], AL_GAIN, 3.0f);
	alSourcefv(source[0], AL_POSITION, source0Pos);
	alSourcefv(source[0], AL_VELOCITY, source0Vel);
	alSourcei(source[0], AL_BUFFER, buffer[0]);
	alSourcei(source[0], AL_LOOPING, AL_TRUE);
	alSourcef(source[0], AL_MAX_DISTANCE, 2000);

	alSourcef(source[1], AL_PITCH, 1.0f);
	alSourcef(source[1], AL_GAIN, 0.5f);
	alSourcefv(source[1], AL_POSITION, source1Pos);
	alSourcefv(source[1], AL_VELOCITY, source1Vel);
	alSourcei(source[1], AL_BUFFER, buffer[1]);
	alSourcei(source[1], AL_LOOPING, AL_TRUE);
	alSourcef(source[1], AL_MAX_DISTANCE, 1000);

	alSourcef(source[2], AL_PITCH, 1.0f);
	alSourcef(source[2], AL_GAIN, 0.3f);
	alSourcefv(source[2], AL_POSITION, source2Pos);
	alSourcefv(source[2], AL_VELOCITY, source2Vel);
	alSourcei(source[2], AL_BUFFER, buffer[2]);
	alSourcei(source[2], AL_LOOPING, AL_TRUE);
	alSourcef(source[2], AL_MAX_DISTANCE, 2000);

	/*******************************************
	 * Inicializacion del framebuffer para
	 * almacenar el buffer de profunidadad
	 *******************************************/
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void destroy() {
	glfwDestroyWindow(window);
	glfwTerminate();

	// Shaders Delete
	shader.destroy();
	shaderMulLighting.destroy();
	shaderSkybox.destroy();
	shaderTerrain.destroy();
	/*shaderParticles.destroy();*/

	// Basic objects Delete
	sphereSkybox.destroy();
	boxTerrain.destroy();

	boxCollider.destroy();
	sphereCollider.destroy();
	rayCollider.destroy();

	boxIntro.destroy();
	boxViewDepth.destroy();

	// Custom objects Delete
	modelBuildingA.destroy();

	modelTankChasis.destroy();
	modelTankTurret.destroy();
	modelTankCannon.destroy();
	modelTankTracks.destroy();

	modelProyectile.destroy();

	modelSoldierEnemy.destroy();

	// Terrains objects Delete
	terrain.destroy();

	// Textures Delete
	glBindTexture(GL_TEXTURE_2D, 0);
	for(size_t i = 0; i < 5; i++){
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &textureTerrain[i]);
	}
	
	glDeleteTextures(1, &textureBeginMenuID[0]);
	glDeleteTextures(1, &textureBeginMenuID[1]);
	glDeleteTextures(1, &textureScreenID);
	glDeleteTextures(1, &textureParticleID);

	// Cube Maps Delete
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDeleteTextures(1, &textureSkyboxID);
}
void reshapeCallback(GLFWwindow *Window, int widthRes, int heightRes) {
	screenWidth = widthRes;
	screenHeight = heightRes;
	glViewport(0, 0, widthRes, heightRes);
}
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			exitApp = true;
			break;
		}
	}
}
void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
	offsetX = xpos - lastMousePosX;
	offsetY = ypos - lastMousePosY;
	lastMousePosX = xpos;
	lastMousePosY = ypos;
}
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset){
	distanceFromTarget -= yoffset;
	camera->setDistanceFromTarget(distanceFromTarget);
}
void mouseButtonCallback(GLFWwindow *window, int button, int state, int mod) {
	if (state == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_RIGHT:
			break;
		case GLFW_MOUSE_BUTTON_LEFT:
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			break;
		}
	}
}
bool processInput(bool continueApplication) {
	if (exitApp || glfwWindowShouldClose(window) != 0) {
		return false;
	}
	/*******************************************
	 * Lógica del Menu de inicio
	 *******************************************/
	if(!beginScene){
		bool presionarEnter = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
		if(textureCurrentMenuID == textureBeginMenuID[0] && presionarEnter){
			beginScene = true;
			textureCurrentMenuID = textureScreenID;
		}
		else if(!pressOption && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
			pressOption = true;
			if(textureCurrentMenuID == textureBeginMenuID[0])
				textureCurrentMenuID = textureBeginMenuID[1];
			else if(textureCurrentMenuID == textureBeginMenuID[1])
				textureCurrentMenuID = textureBeginMenuID[0];
		}
		else if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
			pressOption = false;
	}
	/*******************************************
	 * Controladores por Mando
	 *******************************************/
	if (glfwJoystickPresent(GLFW_JOYSTICK_1) == GL_TRUE) {
		std::cout << "Esta presente el joystick" << std::endl;
		int axesCount, buttonCount;
		const float * axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
		std::cout << "Número de ejes disponibles :=>" << axesCount << std::endl;
		std::cout << "Left Stick X axis: " << axes[0] << std::endl;
		std::cout << "Left Stick Y axis: " << axes[1] << std::endl;
		std::cout << "Left Trigger/L2: " << axes[2] << std::endl;
		std::cout << "Right Stick X axis: " << axes[3] << std::endl;
		std::cout << "Right Stick Y axis: " << axes[4] << std::endl;
		std::cout << "Right Trigger/R2: " << axes[5] << std::endl;

		if(fabs(axes[1]) > 0.2){
			modelMatrixTankChasis = glm::translate(modelMatrixTankChasis, glm::vec3(0, 0, -axes[1] * 0.1));
			animDHTankTracksIndex = 0;
		}if(fabs(axes[0]) > 0.2){
			modelMatrixTankChasis = glm::rotate(modelMatrixTankChasis, glm::radians(-axes[0] * 0.5f), glm::vec3(0, 1, 0));
			animDHTankTracksIndex = 0;
		}

		if(fabs(axes[3]) > 0.2){
			camera->mouseMoveCamera(axes[3], 0.0, deltaTime);
		}if(fabs(axes[4]) > 0.2){
			camera->mouseMoveCamera(0.0, axes[4], deltaTime);
		}

		const unsigned char * buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
		std::cout << "Número de botones disponibles :=>" << buttonCount << std::endl;
		if(buttons[0] == GLFW_PRESS)
			std::cout << "Se presiona x" << std::endl;

		if(!isJump && buttons[0] == GLFW_PRESS){
			isJump = true;
			startTimeJump = currTime;
			tmvJump = 0;
		}
	}
	/*******************************************
	* Funcionalidad del Mouse
	*******************************************/
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
		if(COOLDOWN <= currCool){
			if(!vecTriggerFire[currProy]){//Solo se reinicia Si está activo
				vecStartTimeShoot[currProy] = currTime;
				vecModelMatrixProy[currProy] = glm::scale(modelMatrixTankCannon, glm::vec3(75.0f));
				vecTriggerFire[currProy] = true;
			}
			currProy = (currProy + 1) % MAX_N_PROYECTILES;
			currCool -= COOLDOWN;
		}
		animDHTankCannonIndex = 5;//Disparando
	}else
		animDHTankCannonIndex = 0;//No disparo
	

	camera->mouseMoveCamera(offsetX, offsetY, deltaTime);
	offsetX = 0.0f;
	offsetY = 0.0f;

	/*******************************************
	 * Controles por Teclado
	 *******************************************/
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		modelMatrixTankChasis = glm::rotate(modelMatrixTankChasis, 0.03f, glm::vec3(0, 1, 0));
		animDHTankTracksIndex = 0;
	} else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
		modelMatrixTankChasis = glm::rotate(modelMatrixTankChasis, -0.03f, glm::vec3(0, 1, 0));
		animDHTankTracksIndex = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
		modelMatrixTankChasis = glm::translate(modelMatrixTankChasis, glm::vec3(0.0, 0.0, 0.05));
		animDHTankTracksIndex = 0;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		modelMatrixTankChasis = glm::translate(modelMatrixTankChasis, glm::vec3(0.0, 0.0, -0.05));
		animDHTankTracksIndex = 0;
	}
	if(!isJump && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
		isJump = true;
		startTimeJump = currTime;
		tmvJump = 0;
	}

	glfwPollEvents();
	return continueApplication;
}

void initParticleBuffers() {
    // Generate the buffers
	GLuint initVel, startTime, VAOParticles;
	size_t nParticles = 10;

    glGenBuffers(1, &initVel);   // Initial velocity buffer
    glGenBuffers(1, &startTime); // Start time buffer

    // Allocate space for all buffers
    int size = nParticles * 3 * sizeof(float);
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), NULL, GL_STATIC_DRAW);

    // Fill the first velocity buffer with random velocities
    glm::vec3 v(0.0f);
    float velocity, theta, phi;
    GLfloat *data = new GLfloat[nParticles * 3];
    for (unsigned int i = 0; i < nParticles; i++) {

        theta = glm::mix(0.0f, glm::pi<float>() / 6.0f, ((float)rand() / RAND_MAX));
        phi = glm::mix(0.0f, glm::two_pi<float>(), ((float)rand() / RAND_MAX));

        v.x = sinf(theta) * cosf(phi);
        v.y = cosf(theta);
        v.z = sinf(theta) * sinf(phi);

        velocity = glm::mix(0.6f, 0.8f, ((float)rand() / RAND_MAX));
        v = glm::normalize(v) * velocity;

        data[3 * i] = v.x;
        data[3 * i + 1] = v.y;
        data[3 * i + 2] = v.z;
    }
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);

    // Fill the start time buffer
    delete[] data;
    data = new GLfloat[nParticles];
    float time = 0.0f;
    float rate = 0.00075f;
    for (unsigned int i = 0; i < nParticles; i++) {
        data[i] = time;
        time += rate;
    }
    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), data);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    delete[] data;

    glGenVertexArrays(1, &VAOParticles);
    glBindVertexArray(VAOParticles);
    glBindBuffer(GL_ARRAY_BUFFER, initVel);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, startTime);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}
void prepareLightScene(){
	terrain.setShader(&shaderTerrain);
	
	modelBuildingA.setShader(&shaderMulLighting);

	modelTankChasis.setShader(&shaderMulLighting);
	modelTankTurret.setShader(&shaderMulLighting);
	modelTankCannon.setShader(&shaderMulLighting);
	modelTankTracks.setShader(&shaderMulLighting);

	modelProyectile.setShader(&shaderMulLighting);

	modelSoldierEnemy.setShader(&shaderMulLighting);
}
void prepareDepthScene(){
	terrain.setShader(&shaderDepth);

	modelBuildingA.setShader(&shaderDepth);

	modelTankChasis.setShader(&shaderDepth);
	modelTankTurret.setShader(&shaderDepth);
	modelTankCannon.setShader(&shaderDepth);
	modelTankTracks.setShader(&shaderDepth);

	modelProyectile.setShader(&shaderDepth);

	modelSoldierEnemy.setShader(&shaderDepth);
}
void renderSolidScene(){
	/*******************************************
	* Terrain Cesped
	*******************************************/
	// Se activa la textura del agua
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureTerrain[0]);
	shaderTerrain.setInt("backgroundTexture", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureTerrain[1]);
	shaderTerrain.setInt("textureR", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textureTerrain[2]);
	shaderTerrain.setInt("textureG", 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, textureTerrain[3]);
	shaderTerrain.setInt("textureB", 3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, textureTerrain[4]);
	shaderTerrain.setInt("textureBlendMap", 4);
	shaderTerrain.setVectorFloat2("scaleUV", glm::value_ptr(glm::vec2(80, 80)));
	terrain.setPosition(glm::vec3(100, 0, 100));
	terrain.render();
	shaderTerrain.setVectorFloat2("scaleUV", glm::value_ptr(glm::vec2(0, 0)));
	glBindTexture(GL_TEXTURE_2D, 0);

	/*******************************************
	 * Renderización de objetos fijos
	 *******************************************/
	//Building Render
	std::map<std::string, std::vector<std::pair<glm::vec3, float>>>::iterator itBuild;
	std::vector<std::pair<glm::vec3, float>>::iterator jtBuild;
	for(itBuild = dataBuilds.begin(); itBuild != dataBuilds.end(); itBuild++){
		if(!itBuild->first.compare("EdificioA"))
			for(jtBuild = itBuild->second.begin(); jtBuild != itBuild->second.end(); jtBuild++){
				jtBuild->first.y = terrain.getHeightTerrain(jtBuild->first.x, jtBuild->first.z);
				modelBuildingA.setPosition(jtBuild->first);
				modelBuildingA.setOrientation(glm::vec3(0, jtBuild->second, 0));
				modelBuildingA.render();
			}
		if(!itBuild->first.compare("EdificioB"))
			for(jtBuild = itBuild->second.begin(); jtBuild != itBuild->second.end(); jtBuild++){
				jtBuild->first.y = terrain.getHeightTerrain(jtBuild->first.x, jtBuild->first.z);
				modelBuildingA.setPosition(jtBuild->first);
				modelBuildingA.setOrientation(glm::vec3(0, jtBuild->second, 0));
				modelBuildingA.render();
			}
		if(!itBuild->first.compare("EdificioC"))
			for(jtBuild = itBuild->second.begin(); jtBuild != itBuild->second.end(); jtBuild++){
				jtBuild->first.y = terrain.getHeightTerrain(jtBuild->first.x, jtBuild->first.z);
				modelBuildingA.setPosition(jtBuild->first);
				modelBuildingA.setOrientation(glm::vec3(0, jtBuild->second, 0));
				modelBuildingA.render();
			}
	}
	
	/*****************************************
	 * Objetos animados por huesos
	 * **************************************/
	float currHeight = terrain.getHeightTerrain(modelMatrixTankChasis[3][0], modelMatrixTankChasis[3][2]);
	glm::vec3 ejey = glm::normalize(terrain.getNormalTerrain(modelMatrixTankChasis[3][0], modelMatrixTankChasis[3][2]));
	glm::vec3 ejex = glm::vec3(modelMatrixTankChasis[0]);
	glm::vec3 ejez = glm::normalize(glm::cross(ejex, ejey));
	ejex = glm::normalize(glm::cross(ejey, ejez));
	modelMatrixTankChasis[0] = glm::vec4(ejex, 0.0);
	modelMatrixTankChasis[1] = glm::vec4(ejey, 0.0);
	modelMatrixTankChasis[2] = glm::vec4(ejez, 0.0);
	modelMatrixTankChasis[3][1] = -(GRAVITY * tmvJump * tmvJump) + (2.8 * tmvJump) + currHeight;
	tmvJump = currTime - startTimeJump;
	if(modelMatrixTankChasis[3][1] < currHeight){
		isJump = false;
		modelMatrixTankChasis[3][1] = currHeight;
	}
	glm::mat4 modelMatrixTankChasisBody = glm::mat4(modelMatrixTankChasis);
	modelTankChasis.render(modelMatrixTankChasisBody);

	glm::mat4 modelMatrixTankAux = glm::translate(modelMatrixTankChasis, glm::vec3(-1.43085f, 0.64637f, 0.107476f));
	modelMatrixTankAux = glm::scale(modelMatrixTankAux, glm::vec3(0.01f));
	modelTankTracks.setAnimationIndex(animDHTankTracksIndex);
	modelTankTracks.render(modelMatrixTankAux);

	modelMatrixTankAux = glm::translate(modelMatrixTankChasis, glm::vec3(1.43085f, 0.64637f, 0.107476f));
	modelMatrixTankAux = glm::scale(modelMatrixTankAux, glm::vec3(-0.01f));
	modelTankTracks.render(modelMatrixTankAux);

	modelMatrixTankTurret = glm::translate(modelMatrixTankChasis, glm::vec3(0.0f, 2.09877f, -0.211106f));
	modelMatrixTankTurret = glm::rotate(modelMatrixTankTurret, camera->getAngleAroundTarget() + 0.75f, glm::vec3(0, 1, 0)); //Movimiento de 360° para el eje Y
	modelTankTurret.render(modelMatrixTankTurret);

	modelMatrixTankCannon = glm::translate( modelMatrixTankTurret, glm::vec3(0.0f, -0.08286f, 1.600726f)); 
	modelMatrixTankCannon = glm::rotate(modelMatrixTankCannon, glm::radians(45.0f * glm::sin(camera->getPitch()) - 15.0f), glm::vec3(1, 0, 0)); //Movimiento limitado en X > 0°
	modelMatrixTankCannon = glm::scale(modelMatrixTankCannon, glm::vec3(0.01f));
	modelTankCannon.setAnimationIndex(animDHTankCannonIndex);
	modelTankCannon.render(modelMatrixTankCannon);

	
	animDHTankTracksIndex = 1;//IDLE Index

	
	/*******************************************
	* Renderizado de proyectiles
	*******************************************/
	
	//Distancia Limite
	for(size_t i = 0; i < MAX_N_PROYECTILES; i++){
		if(vecTriggerFire[i]){
			vecModelMatrixProy[i] = glm::translate(vecModelMatrixProy[i], glm::vec3(0.0f, 0.0f, 1.0f + VEL_PROY * (currTime - vecStartTimeShoot[i])));
			vecTriggerFire[i] = glm::distance(glm::vec3(vecModelMatrixProy[i][3]), glm::vec3(modelMatrixTankCannon[3])) <= 65.0f;
		}else{
			//vecStartTimeShoot[currProy] = currTime;
			vecModelMatrixProy[i] = glm::scale(modelMatrixTankCannon, glm::vec3(75.0f));
		}

		modelProyectile.render(vecModelMatrixProy[i]);
	}
	/*******************************************
	 * Objetos Con Rutina de Animación
	 *******************************************/
	rSE1.animacion();
	for(int i = 0; i < vecAnimationSEIndex.size(); i++){
		glm::mat4 modelMatrixSoldierEnemy = translate(glm::mat4(1.0f), rSE1.getVector(i));
		modelMatrixSoldierEnemy = glm::rotate(modelMatrixSoldierEnemy, glm::radians(rSE1.getScale(i)), glm::vec3(0,1,0));
		glm::vec3 axisY = glm::normalize(terrain.getNormalTerrain(modelMatrixSoldierEnemy[3][0], modelMatrixSoldierEnemy[3][2]));
		glm::vec3 axisX = glm::vec3(modelMatrixSoldierEnemy[0]);
		glm::vec3 axisZ = glm::normalize(glm::cross(axisX, axisY));
		axisX = glm::normalize(glm::cross(axisY, axisZ));
		modelMatrixSoldierEnemy[0] = glm::vec4(axisX, 0.0);
		modelMatrixSoldierEnemy[1] = glm::vec4(axisY, 0.0);
		modelMatrixSoldierEnemy[2] = glm::vec4(axisZ, 0.0);
		//Aplicando el desplazamiento por gravedad
		modelMatrixSoldierEnemy[3][1] = terrain.getHeightTerrain(modelMatrixSoldierEnemy[3][0], modelMatrixSoldierEnemy[3][2]) + 0.85f;
		modelMatrixSoldierEnemy = glm::scale(modelMatrixSoldierEnemy, glm::vec3(0.0008f));
		modelSoldierEnemy.render(modelMatrixSoldierEnemy);
		modelSoldierEnemy.setAnimationIndex(vecAnimationSEIndex[i]);
	}

	/*******************************************
	 * Skybox
	 *******************************************/
	GLint oldCullFaceMode;
	GLint oldDepthFuncMode;
	// deshabilita el modo del recorte de caras ocultas para ver las esfera desde adentro
	glGetIntegerv(GL_CULL_FACE_MODE, &oldCullFaceMode);
	glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFuncMode);
	shaderSkybox.setFloat("skybox", 0);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);
	glActiveTexture(GL_TEXTURE0);
	sphereSkybox.render();
	glCullFace(oldCullFaceMode);
	glDepthFunc(oldDepthFuncMode);
}
void renderAlphaScene(bool render = true){
	/**********
	 * Update the position with alpha objects
	 */

	/**********
	 * Sorter with alpha objects
	 */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	/**********
	 * Render de las transparencias
	 */
	
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	if(render){
		/************Render de imagen de frente**************/
		shaderTexture.setMatrix4("projection", 1, false, glm::value_ptr(glm::mat4(1.0)));
		shaderTexture.setMatrix4("view", 1, false, glm::value_ptr(glm::mat4(1.0)));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureCurrentMenuID);
		shaderTexture.setInt("outTexture", 0);
		glEnable(GL_BLEND);
		boxIntro.render();
		glDisable(GL_BLEND);

		modelText->render("Texto en OpenGL", -1, 0);
	}
}
void renderScene(){
	renderSolidScene();
	renderAlphaScene(false);
}

void applicationLoop() {
	bool psi = true;

	glm::vec3 axis;
	glm::vec3 target;
	float angleTarget;

	glm::vec3 lightPos = glm::vec3(10.0, 10.0, -10.0);
	shadowBox = new ShadowBox(-lightPos, camera.get(), 15.0f, 0.1f, 45.0f);

	modelMatrixTankChasis = glm::translate(modelMatrixTankChasis, glm::vec3(13.0f, 0.05f, -5.0f));
	modelMatrixTankChasis = glm::rotate(modelMatrixTankChasis, glm::radians(-90.0f), glm::vec3(0, 1, 0));

	lastTime = TimeManager::Instance().GetTime();
	textureCurrentMenuID = textureBeginMenuID[0];

	/*******************************************
	* Propiedades de neblina
	*******************************************/
	shaderMulLighting.setVectorFloat3("fogColor", glm::value_ptr(glm::vec3(0.5, 0.5, 0.4)));
	shaderTerrain.setVectorFloat3("fogColor", glm::value_ptr(glm::vec3(0.5, 0.5, 0.4)));
	shaderSkybox.setVectorFloat3("fogColor", glm::value_ptr(glm::vec3(0.5, 0.5, 0.4)));

	/*******************************************
	* Propiedades Luz direccional
	*******************************************/
	shaderMulLighting.setVectorFloat3("directionalLight.light.ambient", glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
	shaderMulLighting.setVectorFloat3("directionalLight.light.diffuse", glm::value_ptr(glm::vec3(0.5, 0.5, 0.5)));
	shaderMulLighting.setVectorFloat3("directionalLight.light.specular", glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
	shaderMulLighting.setVectorFloat3("directionalLight.direction", glm::value_ptr(glm::vec3(-0.707106781, -0.707106781, 0.0)));

	shaderTerrain.setVectorFloat3("directionalLight.light.ambient", glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
	shaderTerrain.setVectorFloat3("directionalLight.light.diffuse", glm::value_ptr(glm::vec3(0.5, 0.5, 0.5)));
	shaderTerrain.setVectorFloat3("directionalLight.light.specular", glm::value_ptr(glm::vec3(0.2, 0.2, 0.2)));
	shaderTerrain.setVectorFloat3("directionalLight.direction", glm::value_ptr(glm::vec3(-0.707106781, -0.707106781, 0.0)));

	/*******************************************
	* Propiedades SpotLights
	*******************************************/
	// Spot position
	std::vector<glm::vec3> SpotPosition = {glm::vec3(0.0f, 0.0f, 0.0f)};
	std::vector<glm::vec3> SpotOrientation = {glm::vec3(0.0f, 0.0f, 0.0f)};
	shaderMulLighting.setInt("spotLightCount", SpotPosition.size());
	shaderTerrain.setInt("spotLightCount", SpotPosition.size());
	for(size_t i = 0; i < SpotPosition.size(); i++){
		shaderMulLighting.setVectorFloat3("spotLights[" + std::to_string(i) + "].light.ambient", glm::value_ptr(glm::vec3(0.45, 0.3, 0.01)));
		shaderMulLighting.setVectorFloat3("spotLights[" + std::to_string(i) + "].light.diffuse", glm::value_ptr(glm::vec3(0.6, 0.4, 0.2)));
		shaderMulLighting.setVectorFloat3("spotLights[" + std::to_string(i) + "].light.specular", glm::value_ptr(glm::vec3(0.7, 0.5, 0.3)));
		shaderMulLighting.setFloat("spotLights[" + std::to_string(i) + "].constant", 1.0);
		shaderMulLighting.setFloat("spotLights[" + std::to_string(i) + "].linear", 0.09);
		shaderMulLighting.setFloat("spotLights[" + std::to_string(i) + "].quadratic", 0.02);
		shaderMulLighting.setFloat("spotLights[" + std::to_string(i) + "].cutOff", cos(glm::radians(12.5f)));
		shaderMulLighting.setFloat("spotLights[" + std::to_string(i) + "].outerCutOff", cos(glm::radians(15.0f)));
		shaderTerrain.setVectorFloat3("spotLights[" + std::to_string(i) + "].light.ambient", glm::value_ptr(glm::vec3(0.45, 0.3, 0.01)));
		shaderTerrain.setVectorFloat3("spotLights[" + std::to_string(i) + "].light.diffuse", glm::value_ptr(glm::vec3(0.6, 0.4, 0.2)));
		shaderTerrain.setVectorFloat3("spotLights[" + std::to_string(i) + "].light.specular", glm::value_ptr(glm::vec3(0.7, 0.5, 0.3)));
		shaderTerrain.setFloat("spotLights[" + std::to_string(i) + "].constant", 1.0);
		shaderTerrain.setFloat("spotLights[" + std::to_string(i) + "].linear", 0.09);
		shaderTerrain.setFloat("spotLights[" + std::to_string(i) + "].quadratic", 0.02);
		shaderTerrain.setFloat("spotLights[" + std::to_string(i) + "].cutOff", cos(glm::radians(2.5f)));
		shaderTerrain.setFloat("spotLights[" + std::to_string(i) + "].outerCutOff", cos(glm::radians(15.0f)));
	}

	/*******************************************
	* Propiedades PointLights
	*******************************************/
	// Point position
	std::vector<glm::vec3> lampPosition = {glm::vec3(0.0f, 0.0f, 0.0f)};
	shaderMulLighting.setInt("pointLightCount", lampPosition.size());
	shaderTerrain.setInt("pointLightCount", lampPosition.size());
	for(size_t i = 0; i < lampPosition.size(); i++){
		shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.ambient", glm::value_ptr(glm::vec3(0.45, 0.3, 0.01)));
		shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.diffuse", glm::value_ptr(glm::vec3(0.6, 0.4, 0.01)));
		shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.specular", glm::value_ptr(glm::vec3(0.7, 0.5, 0.01)));
		shaderMulLighting.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0);
		shaderMulLighting.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.09);
		shaderMulLighting.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.02);
		shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.ambient", glm::value_ptr(glm::vec3(0.45, 0.3, 0.01)));
		shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.diffuse", glm::value_ptr(glm::vec3(0.6, 0.4, 0.01)));
		shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.specular", glm::value_ptr(glm::vec3(0.7, 0.5, 0.01)));
		shaderTerrain.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0);
		shaderTerrain.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.09);
		shaderTerrain.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.02);
	}

	rSE1.setPlay(true);

	while (psi) {
		currTime = TimeManager::Instance().GetTime();
		//Actualizacion del enfriamiento del disparo
		currCool += (currTime - lastTime);
		if(currCool > COOLDOWN)
			currCool = COOLDOWN;

		if(currTime - lastTime < 0.016666667){
			glfwPollEvents();
			continue;
		}
		lastTime = currTime;
		TimeManager::Instance().CalculateFrameRate(true);
		deltaTime = TimeManager::Instance().DeltaTime;
		psi = processInput(true);

		std::map<std::string, bool> collisionDetection;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 modelCameraMatrix = glm::translate(modelMatrixTankChasis, glm::vec3(-1.1f, 2.25f, 0.0f));
		axis = glm::axis(glm::quat_cast(modelCameraMatrix));
		angleTarget = glm::angle(glm::quat_cast(modelCameraMatrix));
		target = modelCameraMatrix[3];

		if(std::isnan(angleTarget))
			angleTarget = 0.0;
		if(axis.y < 0)
			angleTarget = -angleTarget;

		camera->setCameraTarget(target);
		camera->setAngleTarget(angleTarget + 0.85f);
		camera->setDistanceFromTarget(7.5f);
		camera->updateCamera();

		glm::mat4 view = camera->getViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) screenWidth / (float) screenHeight, 0.01f, 100.0f);

		shadowBox->update(screenWidth, screenHeight);
		glm::vec3 centerBox = shadowBox->getCenter();

		// Projection light shadow mapping
		glm::mat4 lightProjection = glm::mat4(1.0f), lightView = glm::mat4(1.0f);
		glm::mat4 lightSpaceMatrix;
		lightProjection[0][0] = 2.0f / shadowBox->getWidth();
		lightProjection[1][1] = 2.0f / shadowBox->getHeight();
		lightProjection[2][2] = -2.0f / shadowBox->getLength();
		lightProjection[3][3] = 1.0f;
		lightView = glm::lookAt(centerBox, centerBox + glm::normalize(-lightPos), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		shaderDepth.setMatrix4("lightSpaceMatrix", 1, false, glm::value_ptr(lightSpaceMatrix));
		// Settea la matriz de vista y projection al shader con solo color
		shader.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shader.setMatrix4("view", 1, false, glm::value_ptr(view));

		// Settea la matriz de vista y projection al shader con skybox
		shaderSkybox.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shaderSkybox.setMatrix4("view", 1, false, glm::value_ptr(glm::mat4(glm::mat3(view))));
		// Settea la matriz de vista y projection al shader con multiples luces
		shaderMulLighting.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shaderMulLighting.setMatrix4("view", 1, false, glm::value_ptr(view));
		shaderMulLighting.setMatrix4("lightSpaceMatrix", 1, false, glm::value_ptr(lightSpaceMatrix));
		// Settea la matriz de vista y projection al shader con multiples luces
		shaderTerrain.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shaderTerrain.setMatrix4("view", 1, false, glm::value_ptr(view));
		shaderTerrain.setMatrix4("lightSpaceMatrix", 1, false, glm::value_ptr(lightSpaceMatrix));
		// Settea la matriz de vista y projection al shader para el fountain
		shaderParticles.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shaderParticles.setMatrix4("view", 1, false, glm::value_ptr(view));

		/*******************************************
		 * Propiedades Luz direccional
		 *******************************************/
		shaderMulLighting.setVectorFloat3("viewPos", glm::value_ptr(camera->getPosition()));
		shaderTerrain.setVectorFloat3("viewPos", glm::value_ptr(camera->getPosition()));

		/************Render de imagen de frente**************/
		if(!beginScene){
			shaderTexture.setMatrix4("projection", 1, false, glm::value_ptr(glm::mat4(1.0)));
			shaderTexture.setMatrix4("view", 1, false, glm::value_ptr(glm::mat4(1.0)));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureCurrentMenuID);
			shaderTexture.setInt("outTexture", 0);
			boxIntro.render();
			glfwSwapBuffers(window);
			continue;
		}

		/*******************************************
		 * 1.- We render the depth buffer
		 *******************************************/
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// render scene from light's point of view
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		prepareDepthScene();
		renderScene();
		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/*******************************************
		 * Debug to view the texture view map
		 *******************************************/
		// reset viewport
		/*glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// render Depth map to quad for visual debugging
		shaderViewDepth.setMatrix4("projection", 1, false, glm::value_ptr(glm::mat4(1.0)));
		shaderViewDepth.setMatrix4("view", 1, false, glm::value_ptr(glm::mat4(1.0)));
		shaderViewDepth.setFloat("near_plane", near_plane);
		shaderViewDepth.setFloat("far_plane", far_plane);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		boxViewDepth.setScale(glm::vec3(2.0, 2.0, 1.0));
		boxViewDepth.render();*/

		/*******************************************
		 * 2.- We render the normal objects
		 *******************************************/
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		prepareLightScene();
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		shaderMulLighting.setInt("shadowMap", 10);
		shaderTerrain.setInt("shadowMap", 10);
		renderSolidScene();

		/*******************************************
		 * Creacion de colliders
		 * IMPORTANT do this before interpolations
		 *******************************************/
		std::vector<size_t> counterBuildCollider(dataBuilds.size());
		std::map<std::string, std::vector<std::pair<glm::vec3, float>>>::iterator itBuild;
		std::vector<std::pair<glm::vec3, float>>::iterator jtBuild;
		for(itBuild = dataBuilds.begin(); itBuild != dataBuilds.end(); itBuild++){
			if(!itBuild->first.compare("EdificioA"))
				for(jtBuild = itBuild->second.begin(); jtBuild != itBuild->second.end(); jtBuild++){
					glm::mat4 modelMatrixBuilding= glm::translate(glm::mat4(1.0f), jtBuild->first);
					setColliderOBB(buildingsCollOBB, "EdificioA " + std::to_string(counterBuildCollider[0]), modelMatrixBuilding ,modelBuildingA.getObb(), glm::vec3(1.0f));
					counterBuildCollider[0]++;
				}
			if(!itBuild->first.compare("EdificioB"))
				for(jtBuild = itBuild->second.begin(); jtBuild != itBuild->second.end(); jtBuild++){
					glm::mat4 modelMatrixBuilding= glm::translate(glm::mat4(1.0f), jtBuild->first);
					setColliderOBB(buildingsCollOBB, "EdificioB " + std::to_string(counterBuildCollider[1]), modelMatrixBuilding ,modelBuildingA.getObb(), glm::vec3(1.0f));
					counterBuildCollider[1]++;
				}
			if(!itBuild->first.compare("EdificioC"))
				for(jtBuild = itBuild->second.begin(); jtBuild != itBuild->second.end(); jtBuild++){
					glm::mat4 modelMatrixBuilding= glm::translate(glm::mat4(1.0f), jtBuild->first);
					setColliderOBB(buildingsCollOBB, "EdificioC " + std::to_string(counterBuildCollider[2]), modelMatrixBuilding ,modelBuildingA.getObb(), glm::vec3(1.0f));
					counterBuildCollider[2]++;
				}
		}

		//Adjunto y actualizacion de colisiones
		for(size_t i = 0; i < MAX_N_PROYECTILES; i++)
			setColliderSBB(proyectileCollSBB, "ProyectilePlayer" + std::to_string(i), vecModelMatrixProy[i], modelProyectile.getSbb(), 0.5f);

		setColliderOBB(mainCharCollOBB, "MainCharacterChasis", modelMatrixTankChasis, modelTankChasis.getObb(), glm::vec3(1.0f));
		setColliderOBB(mainCharCollOBB, "MainCharacterTurret", modelMatrixTankTurret, modelTankTurret.getObb(), glm::vec3(1.0f));
		setColliderRAY(collidersRAY, "RayMainCharacter", modelMatrixTankCannon, 10.0f);
		
		for(size_t i = 0; i < vecAnimationSEIndex.size(); i++){
			glm::mat4 modelMatrixColliderSE = glm::translate(glm::mat4(1.0f), rSE1.getVector(i));
			modelMatrixColliderSE = glm::rotate(modelMatrixColliderSE, glm::radians(rSE1.getScale(i)), glm::vec3(0, 1, 0));
			modelMatrixColliderSE[3][1] = terrain.getHeightTerrain(modelMatrixColliderSE[3][0], modelMatrixColliderSE[3][2]) + 1.35f;
			setColliderOBB(enemyCollOBB, "SoldadoEnemigoL1_" + std::to_string(i), modelMatrixColliderSE, modelSoldierEnemy.getObb(), glm::vec3(0.05f, 0.075f, 0.05f));
		}
		/*******************************************
		 * Render de colliders
		 *******************************************/
		renderLayerRAY(collidersRAY, rayCollider, glm::vec4(1.0f));
		renderLayerOBB(mainCharCollOBB, boxCollider, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		renderLayerOBB(buildingsCollOBB, boxCollider, glm::vec4(1.0f));
		renderLayerOBB(enemyCollOBB, boxCollider, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		renderLayerSBB(proyectileCollSBB, sphereCollider, glm::vec4(0.0f, 0.5f, 0.5f, 1.0f));

		/**********Render de transparencias***************/
		renderAlphaScene();
		/*******************************************
		 * Render de colliders
		 *******************************************/
		/*********************Pruebas de colisiones****************************/
		for (std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator it = proyectileCollSBB.begin(); it != proyectileCollSBB.end(); it++) {
			bool isCollision = false; size_t i = 0;
			for (std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator jt = enemyCollOBB.begin(); jt != enemyCollOBB.end(); jt++) {
				if (testSBBOBB(std::get<0>(it->second), std::get<0>(jt->second))) {
					std::cout << "Enemigo impactado " << jt->first << " por el proyectil " << it->first << std::endl;
					vecAnimationSEIndex[i] = 4;//Animacion de muerte
					isCollision = true;
				}
			i = (i + 1) % vecAnimationSEIndex.size();
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isCollision);
		}
		/*********************Accion en Colisiones****************************/
		std::map<std::string, bool>::iterator itCollision;
		for (itCollision = collisionDetection.begin(); itCollision != collisionDetection.end(); itCollision++) {
			std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator sbbBuscado = proyectileCollSBB.find(itCollision->first);
			std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator obbBuscado = enemyCollOBB.find(itCollision->first);
			if (sbbBuscado != proyectileCollSBB.end()) {
				if (!itCollision->second) 
					addOrUpdateColliders(proyectileCollSBB, itCollision->first);
			}
			if (obbBuscado != enemyCollOBB.end()) {
				if (!itCollision->second) 
					addOrUpdateColliders(enemyCollOBB, itCollision->first);
				else {
					for(size_t i = 0; i < vecAnimationSEIndex.size(); i++)
						if (itCollision->first.compare("SoldadoEnemigoL1_" + std::to_string(i)) == 0){
							//vecAnimationSEIndex[i] = 3;//Animacion de morir
							//std::cout << "Soldado " << i << " Abatido!!!";
						}
							//modelMatrixTankChasis = std::get<1>(obbBuscado->second); //Retornar a la posicion anterior
				}
			}
		}
	
		glfwSwapBuffers(window);

		/****************************
		 * Open AL sound data
		 ****************************/
		source0Pos[0] = modelMatrixTankChasis[3].x;
		source0Pos[1] = modelMatrixTankChasis[3].y;
		source0Pos[2] = modelMatrixTankChasis[3].z;
		alSourcefv(source[0], AL_POSITION, source0Pos);

		source1Pos[0] = modelMatrixTankChasis[3].x;
		source1Pos[1] = modelMatrixTankChasis[3].y;
		source1Pos[2] = modelMatrixTankChasis[3].z;
		alSourcefv(source[1], AL_POSITION, source1Pos);
		
		source2Pos[0] = modelMatrixTankChasis[3].x;
		source2Pos[1] = modelMatrixTankChasis[3].y;
		source2Pos[2] = modelMatrixTankChasis[3].z;
		alSourcefv(source[2], AL_POSITION, source2Pos);

		// Listener for the Thris person camera
		listenerPos[0] = modelMatrixTankChasis[3].x;
		listenerPos[1] = modelMatrixTankChasis[3].y;
		listenerPos[2] = modelMatrixTankChasis[3].z;
		alListenerfv(AL_POSITION, listenerPos);

		glm::vec3 upModel = glm::normalize(modelMatrixTankChasis[1]);
		glm::vec3 frontModel = glm::normalize(modelMatrixTankChasis[2]);

		listenerOri[0] = frontModel.x;
		listenerOri[1] = frontModel.y;
		listenerOri[2] = frontModel.z;
		listenerOri[3] = upModel.x;
		listenerOri[4] = upModel.y;
		listenerOri[5] = upModel.z;

		// Listener for the First person camera
		// listenerPos[0] = camera->getPosition().x;
		// listenerPos[1] = camera->getPosition().y;
		// listenerPos[2] = camera->getPosition().z;
		// alListenerfv(AL_POSITION, listenerPos);
		// listenerOri[0] = camera->getFront().x;
		// listenerOri[1] = camera->getFront().y;
		// listenerOri[2] = camera->getFront().z;
		// listenerOri[3] = camera->getUp().x;
		// listenerOri[4] = camera->getUp().y;
		// listenerOri[5] = camera->getUp().z;
		alListenerfv(AL_ORIENTATION, listenerOri);

		for(unsigned int i = 0; i < sourcesPlay.size(); i++)
			if(sourcesPlay[i]){
				sourcesPlay[i] = false;
				alSourcePlay(source[i]);
			}
		
	}
}

int main(int argc, char **argv) {
	init(1920, 1020, "Guild of Engines V1.1.0", false);
	applicationLoop();
	destroy();
	return 1;
}