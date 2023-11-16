#define _USE_MATH_DEFINES
//std includes
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <map>
#include <tuple>

//glfw include
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//GLM include
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Include loader Model class
#include "Headers/AbstractModel.h"
#include "Headers/AnimationUtils.h"
#include "Headers/Box.h"
#include "Headers/Colisiones.h"
#include "Headers/Cylinder.h"
#include "Headers/ThirdPersonCamera.h"
#include "Headers/Model.h"
#include "Headers/Shader.h"
#include "Headers/Sphere.h"
#include "Headers/Texture.h"
#include "Headers/Terrain.h"
#include "Headers/TimeManager.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

//Ventana de la aplicación
size_t screenWidth, screenHeight;
GLFWwindow *window;

//Camara 3 persona
float lastMousePosX, offsetX = 0;
float lastMousePosY, offsetY = 0;
std::shared_ptr<ThirdPersonCamera> camera(new ThirdPersonCamera());

//Shader con skybox, multiples luces, terreno del escenario, pruebas
Shader shaderSkybox, shaderMulLighting, shaderTerrain, shaderTest;

Sphere skyboxSphere(20, 20);
Sphere proyectileSphere(10, 10);
Box boxCesped; 

//Objetos para pruebas de colisiones
Sphere sphereDrawable(10, 10);
Box boxDrawable;
Cylinder rayDrawable(10,10);

//Informacion de las alturas del terreno
Terrain terrain(-1, -1, 200, 8, "../Textures/heightmap.png");

//Elementos de un modelo
Model modelEjem;

//Elementos del Tanque Duck-Hunter
Model modelTank_Chasis;
Model modelTank_Turret;
Model modelTank_Cannon;
Model modelTank_Track;
Model modelTank_Proyectile;
glm::mat4 modelTankMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrixTank_Canon = glm::mat4(1.0f);
size_t animationTankIndex = 0;



/////////////////////////////class Balistic{
bool DH_trigger = false;
const float DH_cooldown = 0.25f;//Enfriamiento
float DH_time_cooldown = 0.0f;//tiempo transcurrido para enfriamiento

const size_t MAX_PROYECTILES = 20;
const float BALISTIC = 0.8f;//Gravedad del proyectil
const float SPEED_PROY = 1.1f;//Velocidad del proyectil
int count_proyectiles = 0;
std::vector<float> vec_startTime = {};//Tiempo de inicio
std::vector<glm::vec3> vec_proyectile = {}; //Lista de vectores de la direccion del proyectil
std::vector<glm::mat4> vec_proy_pos = {};   //Lista de matrices para la posicion de cada vector
std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>> lay_SBB_Proyectile_Player;//Mapa de colisiones

AbstractModel::SBB proyectile_Cage(glm::vec3(0.0f), 50.0f);//Jaula esférica para contener los proyectiles dentro del mapa
Sphere drawableCage(10, 10, 50.0f);
//};
GLuint textureTerrain[5]; // textureCespedID, textureTerrainRID, textureTerrainGID, textureTerrainBID, textureTerrainBlendMapID;
GLuint skyboxTextureID;

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

bool exitApp = false;

//Variables de tiempo
double deltaTime;
double currTime, lastTime;

//Físicas Generales
const float GRAVITY = 3.62f;
double tmv = 0.0, startTimeJump = 0.0;
bool isNotJump = true;

//Colisiones
std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>> lay_Colition_SBB;
std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>> lay_Colition_OBB;
std::map<std::string, std::tuple<AbstractModel::RAY, glm::mat4, glm::mat4>> lay_Colition_RAY;
std::map<std::string, bool> collisionDetection;

// Se definen todos las funciones.
void init(int width, int height, std::string strTitle, bool bFullScreen);
void destroy();
void reshapeCallback(GLFWwindow *Window, int widthRes, int heightRes);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow *window, int button, int state, int mod);
bool processInput(bool continueApplication = true);

// Implementacion de todas las funciones.
void init(int width, int height, std::string strTitle, bool bFullScreen) {
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
		std::cerr << "Error to create GLFW window, you can try download the last version of your video card that support OpenGL 3.3+"<< std::endl;
		destroy();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glfwSetWindowSizeCallback(window, reshapeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
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

	// Inicialización de los shaders
	shaderSkybox.initialize("../Shaders/skyBox.vs", "../Shaders/skyBox.fs");
	shaderMulLighting.initialize("../Shaders/iluminacion_textura_animation.vs", "../Shaders/multipleLights.fs");
	shaderTerrain.initialize("../Shaders/terrain.vs", "../Shaders/terrain.fs");
	shaderTest.initialize("../Shaders/colorShader.vs", "../Shaders/colorShader.fs");

	// Inicializacion de los objetos.
	skyboxSphere.init();
	skyboxSphere.setShader(&shaderSkybox);
	skyboxSphere.setScale(glm::vec3(20.0f));

	proyectileSphere.init();
	proyectileSphere.setShader(&shaderSkybox);
	proyectileSphere.setScale(glm::vec3(1.0f));
	proyectileSphere.setColor(glm::vec4(0.0f, 0.5f, 0.5f, 1.0f));

	drawableCage.init();
	drawableCage.setShader(&shaderTest);
	drawableCage.setScale(glm::vec3(1.0f));
	drawableCage.setColor(glm::vec4(1.0f));

	boxCesped.init();
	boxCesped.setShader(&shaderMulLighting);

	sphereDrawable.init();
	sphereDrawable.setShader(&shaderTest);
	sphereDrawable.setColor(glm::vec4(1.0f));

	boxDrawable.init();
	boxDrawable.setShader(&shaderTest);
	boxDrawable.setColor(glm::vec4(1.0f));

	rayDrawable.init();
	rayDrawable.setShader(&shaderTest);
	rayDrawable.setColor(glm::vec4(1.0f));

	// Terreno
	terrain.init();
	terrain.setShader(&shaderTerrain);

	camera->setPosition(glm::vec3(0.0, 3.0, 4.0));

	//Tanque Duck-Hunter
	modelTank_Chasis.loadModel("../models/DuckHunter/chasis.obj");
	modelTank_Chasis.setShader(&shaderMulLighting);
	modelTank_Turret.loadModel("../models/DuckHunter/turret.obj");
	modelTank_Turret.setShader(&shaderMulLighting);
	modelTank_Cannon.loadModel("../models/DuckHunter/cannon.obj");
	modelTank_Cannon.setShader(&shaderMulLighting);
	modelTank_Track.loadModel("../models/DuckHunter/track.obj");
	modelTank_Track.setShader(&shaderMulLighting);
	modelTank_Proyectile.loadModel("../models/DuckHunter/Proyectile.obj");
	modelTank_Proyectile.setShader(&shaderMulLighting);

	// Carga de texturas para el skybox
	Texture skyboxTexture = Texture("");
	glGenTextures(1, &skyboxTextureID);
	// Tipo de textura CUBE MAP
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Texturas del skybox
	for (size_t i = 0; i < ARRAY_SIZE_IN_ELEMENTS(types); i++) {
		skyboxTexture = Texture(fileNames[i]);
		skyboxTexture.loadImage(true);
		if (skyboxTexture.getData()) {
			glTexImage2D(types[i], 0, skyboxTexture.getChannels() == 3 ? GL_RGB : GL_RGBA, skyboxTexture.getWidth(), skyboxTexture.getHeight(), 0,
			skyboxTexture.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, skyboxTexture.getData());
		} else
			std::cout << "Failed to load texture" << std::endl;
		skyboxTexture.freeImage();
	}

	// Definiendo las texturas
	std::string textName[5] = {"../Textures/grassy2.png", "../Textures/mud.png", "../Textures/grassFlowers.png", "../Textures/path.png", "../Textures/blendMap.png"};
	for(size_t i = 0; i < 5; i++){
		Texture text(textName[i]);
		text.loadImage();
		glGenTextures(1, &textureTerrain[i]);
		glBindTexture(GL_TEXTURE_2D, textureTerrain[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (text.getData()) {
			std::cout << "Numero de canales :=> " << text.getChannels() << std::endl;
			glTexImage2D(GL_TEXTURE_2D, 0, text.getChannels() == 3 ? GL_RGB : GL_RGBA, text.getWidth(), text.getHeight(), 0,
			text.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, text.getData());
			glGenerateMipmap(GL_TEXTURE_2D);
		} else
			std::cout << "Failed to load texture" << std::endl;
		text.freeImage();
	}	
}
void destroy() {
	glfwDestroyWindow(window);
	glfwTerminate();

	shaderMulLighting.destroy();
	shaderSkybox.destroy();
	shaderTerrain.destroy();

	skyboxSphere.destroy();

	proyectileSphere.destroy();
	drawableCage.destroy();

	sphereDrawable.destroy();
	boxCesped.destroy();
	boxDrawable.destroy();
	terrain.destroy();

	modelTank_Chasis.destroy();
	modelTank_Turret.destroy();
	modelTank_Cannon.destroy();
	modelTank_Track.destroy();
	modelTank_Proyectile.destroy();

	for(size_t i = 0; i < 5; i++){
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &textureTerrain[i]);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDeleteTextures(1, &skyboxTextureID);
}
void reshapeCallback(GLFWwindow *Window, int widthRes, int heightRes) {
	screenWidth = widthRes;
	screenHeight = heightRes;
	glViewport(0, 0, widthRes, heightRes);
}
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode) {
	if (action == GLFW_PRESS) 
		switch (key) {
			case GLFW_KEY_ESCAPE:
				exitApp = true;
				break;
			}
}
void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
	offsetX = xpos - lastMousePosX;
	offsetY = ypos - lastMousePosY;
	lastMousePosX = xpos;
	lastMousePosY = ypos;
}
void mouseButtonCallback(GLFWwindow *window, int button, int state, int mod) {
	if (state == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_RIGHT://Escudo
			break;
		case GLFW_MOUSE_BUTTON_LEFT://Disparar
		DH_trigger = true;
		DH_time_cooldown = DH_cooldown;
		std::cout << "Disparando " << count_proyectiles << " proyectiles" << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			break;
		}
	}
	if (state == GLFW_RELEASE) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_RIGHT://Escudo
			break;
		case GLFW_MOUSE_BUTTON_LEFT://Disparar
			DH_trigger = false;
			DH_time_cooldown = 0;
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

	//if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		//camera->moveFrontCamera(true, deltaTime);
	//if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		//camera->moveFrontCamera(false, deltaTime);
	//if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		//camera->moveRightCamera(false, deltaTime);
	//if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		//camera->moveRightCamera(true, deltaTime);
	//if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	camera->mouseMoveCamera(offsetX, offsetY, deltaTime);
	offsetX = 0.0f;
	offsetY = 0.0f;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
		modelTankMatrix = glm::rotate(modelTankMatrix, 0.03f, glm::vec3(0, 1, 0));
		animationTankIndex = 0;
	} else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
		modelTankMatrix = glm::rotate(modelTankMatrix, -0.03f, glm::vec3(0, 1, 0));
		animationTankIndex = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
		modelTankMatrix = glm::translate(modelTankMatrix, glm::vec3(0.0, 0.0, 0.05));
		animationTankIndex = 0;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
		modelTankMatrix = glm::translate(modelTankMatrix, glm::vec3(0.0, 0.0, -0.05));
		animationTankIndex = 0;
	}
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && isNotJump){
		animationTankIndex = 0;
		startTimeJump = currTime;
		tmv = 0;
		isNotJump = false;
	}
	
	
	//Hacer parte de un método pidiendo current y deltaTime
	if(DH_trigger){
			if(DH_time_cooldown >= DH_cooldown){
			DH_time_cooldown -= DH_cooldown;
			if(vec_proyectile.size() < MAX_PROYECTILES){//Añadir proyectiles
				float timeNow = currTime;
				vec_startTime.push_back(timeNow);
				vec_proy_pos.push_back(glm::mat4(modelMatrixTank_Canon));
				vec_proyectile.push_back(glm::normalize(modelMatrixTank_Canon[3] - glm::vec4(modelTank_Turret.getSbb().c, 1.0f)));
				count_proyectiles++;
				std::cout << "Se ha generado correctamente un proyectil, existen: " << count_proyectiles << std::endl;
			}
		}
		DH_time_cooldown += deltaTime;
	}
	
	/*
	// Seleccionar modelo
	if (enableCountSelected && glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS){
		enableCountSelected = false;
		modelSelected++;
		if(modelSelected > 2)
			modelSelected = 0;
		if(modelSelected == 1)
			fileName = "../animaciones/animation_dart_joints.txt";
		if (modelSelected == 2)
			fileName = "../animaciones/animation_dart.txt";	
		std::cout << "modelSelected:" << modelSelected << std::endl;
	}
	else if(glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)
		enableCountSelected = true;*/


	glfwPollEvents();
	return continueApplication;
}

void applicationLoop() {
	bool psi = true;
	lastTime = TimeManager::Instance().GetTime();

	glm::vec3 axis;
	glm::vec3 target;
	float angleTarget;

	camera->setSensitivity(1.375f);

	/*******************************************
	* Propiedades Luz direccional
	*******************************************/
	shaderMulLighting.setVectorFloat3("directionalLight.light.ambient", glm::value_ptr(glm::vec3(0.3, 0.3, 0.3)));
	shaderMulLighting.setVectorFloat3("directionalLight.light.diffuse", glm::value_ptr(glm::vec3(0.7, 0.7, 0.7)));
	shaderMulLighting.setVectorFloat3("directionalLight.light.specular", glm::value_ptr(glm::vec3(0.9, 0.9, 0.9)));
	shaderMulLighting.setVectorFloat3("directionalLight.direction", glm::value_ptr(glm::vec3(-1.0, 0.0, 0.0)));

	shaderTerrain.setVectorFloat3("directionalLight.light.ambient", glm::value_ptr(glm::vec3(0.3, 0.3, 0.3)));
	shaderTerrain.setVectorFloat3("directionalLight.light.diffuse", glm::value_ptr(glm::vec3(0.7, 0.7, 0.7)));
	shaderTerrain.setVectorFloat3("directionalLight.light.specular", glm::value_ptr(glm::vec3(0.9, 0.9, 0.9)));
	shaderTerrain.setVectorFloat3("directionalLight.direction", glm::value_ptr(glm::vec3(-1.0, 0.0, 0.0)));

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


	while (psi) {
		currTime = TimeManager::Instance().GetTime();
		if(currTime - lastTime < 0.016666667){
			glfwPollEvents();
			continue;
		}
		lastTime = currTime;
		TimeManager::Instance().CalculateFrameRate(true);
		deltaTime = TimeManager::Instance().DeltaTime;
		psi = processInput(true);

		// Variables donde se guardan las matrices de cada articulacion por 1 frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 modelCameraMatrix = glm::translate(modelTankMatrix, glm::vec3(-1.1f, 2.25f, 0.0f));
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
		// Settea la matriz de pruebas
		shaderTest.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shaderTest.setMatrix4("view", 1, false, glm::value_ptr(view));
		// Settea la matriz de vista y projection al shader con skybox
		shaderSkybox.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shaderSkybox.setMatrix4("view", 1, false, glm::value_ptr(glm::mat4(glm::mat3(view))));
		// Settea la matriz de vista y projection al shader con multiples luces
		shaderMulLighting.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shaderMulLighting.setMatrix4("view", 1, false, glm::value_ptr(view));
		shaderTerrain.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shaderTerrain.setMatrix4("view", 1, false, glm::value_ptr(view));

		/*******************************************
		 * Propiedades Luz direccional
		 *******************************************/
		shaderMulLighting.setVectorFloat3("viewPos", glm::value_ptr(camera->getPosition()));
		shaderTerrain.setVectorFloat3("viewPos", glm::value_ptr(camera->getPosition()));

		/*******************************************
		 * Propiedades SpotLights
		 *******************************************/
		for(size_t i = 0; i < SpotPosition.size(); i++){
			shaderMulLighting.setVectorFloat3("spotLights[" + std::to_string(i) + "].position", glm::value_ptr(SpotPosition[i]));
			shaderMulLighting.setVectorFloat3("spotLights[" + std::to_string(i) + "].direction", glm::value_ptr(SpotOrientation[i]));
			shaderTerrain.setVectorFloat3("spotLights[" + std::to_string(i) + "].position", glm::value_ptr(SpotPosition[i]));
			shaderTerrain.setVectorFloat3("spotLights[" + std::to_string(i) + "].direction", glm::value_ptr(SpotOrientation[i]));
		}

		/*******************************************
		 * Propiedades PointLights
		 *******************************************/
		for(int i = 0; i < lampPosition.size(); i++){
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(i) + "].position", glm::value_ptr(lampPosition[i]));
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(i) + "].position", glm::value_ptr(lampPosition[i]));
		}

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

		/*****************************************
		 * Objetos Animados en FBX
		 * **************************************/
		
		//Ajuste de personajes al suelo
		float currHeight = terrain.getHeightTerrain(modelTankMatrix[3][0], modelTankMatrix[3][2]);
		glm::vec3 ejey = glm::normalize(terrain.getNormalTerrain(modelTankMatrix[3][0], modelTankMatrix[3][2]));
		glm::vec3 ejex = glm::vec3(modelTankMatrix[0]);
		glm::vec3 ejez = glm::normalize(glm::cross(ejex, ejey));
		ejex = glm::normalize(glm::cross(ejey, ejez));
		modelTankMatrix[0] = glm::vec4(ejex, 0.0);
		modelTankMatrix[1] = glm::vec4(ejey, 0.0);
		modelTankMatrix[2] = glm::vec4(ejez, 0.0);
		//Aplicando el desplazamiento por gravedad
		modelTankMatrix[3][1] = -(GRAVITY * tmv * tmv) + (2.8 * tmv) + currHeight;
		tmv = currTime - startTimeJump;

		if(modelTankMatrix[3][1] < currHeight){
			isNotJump = true;
			modelTankMatrix[3][1] = currHeight; 
		}
		//Chasis
		glm::mat4 modelTankMatrix_Chasis = glm::mat4(modelTankMatrix);
		modelTank_Chasis.render(modelTankMatrix_Chasis);
		//modelTank_Track.setAnimationIndex(animationTankIndex);

		//Tracks
		glm::mat4 modelMatrixTank_aux = glm::translate(modelTankMatrix_Chasis, glm::vec3(-1.43085f, 0.64637f, 1.73221f));
		modelTank_Track.render(modelMatrixTank_aux);

		//Torreta
		modelMatrixTank_aux = glm::translate(modelTankMatrix_Chasis, glm::vec3(0.0f, 2.09877f, -0.211106f));
		modelMatrixTank_aux = glm::rotate(modelMatrixTank_aux, camera->getAngleAroundTarget() + 0.75f, glm::vec3(0, 1, 0)); //Movimiento de 360° para el eje Y
		modelTank_Turret.render(modelMatrixTank_aux);

		//Cañon
		modelMatrixTank_Canon = glm::translate( modelMatrixTank_aux, glm::vec3(0.0f, -0.08286f, 1.600726f));
		if(camera->getPitch() >= 0.0f)
			modelMatrixTank_Canon = glm::rotate(modelMatrixTank_Canon, 0.0f, glm::vec3(1, 0, 0)); //Movimiento limitado en X > 0°
		else
			modelMatrixTank_Canon = glm::rotate(modelMatrixTank_Canon, camera->getPitch(), glm::vec3(1, 0, 0)); //Altura aplicada al cañón
		modelTank_Cannon.render(modelMatrixTank_Canon);
		modelMatrixTank_Canon = glm::translate(modelMatrixTank_Canon, glm::vec3(0.0f,0.0f,4.0f));//Traslacion al final del cañón
		animationTankIndex = 1;

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
		skyboxSphere.render();
		glCullFace(oldCullFaceMode);
		glDepthFunc(oldDepthFuncMode);

		/*******************************************
		 * Raycast
		 *******************************************/
		glm::mat4 modelMatrixRay = glm::mat4(modelMatrixTank_Canon);//Rayo se sujeta al frente del modelo
		AbstractModel::RAY ray(10.0f, modelMatrixRay);
		addOrUpdateColliders(lay_Colition_RAY, "ModelRay", ray, modelMatrixTank_Canon);
		/*******************************************
		 * Collider Proyectiles Player
		 *******************************************/
		for(size_t i = 0; i < count_proyectiles; i++){
			AbstractModel::SBB bodyColliderSBB;
			glm::mat4 modelMatrixCollider = glm::translate(vec_proy_pos[i], modelTank_Proyectile.getSbb().c);//Localización al la salida del cañon al centro del proyectil
			float dTime = currTime - vec_startTime[i];
			//Velocidad aplicada en XZ, Gravedad colocada a Y
			modelMatrixCollider[3][0] = vec_proyectile[i][0] * SPEED_PROY * dTime + vec_proy_pos[i][3][0];
			modelMatrixCollider[3][1] = -(BALISTIC * dTime * dTime) + vec_proy_pos[i][3][1];
			modelMatrixCollider[3][2] = vec_proyectile[i][2] * SPEED_PROY * dTime + vec_proy_pos[i][3][2];
			
			bodyColliderSBB.c = modelMatrixCollider[3];
			bodyColliderSBB.ratio = modelTank_Proyectile.getSbb().ratio;

			addOrUpdateColliders(lay_SBB_Proyectile_Player, "Proyectile["+ std::to_string(i) + "]", bodyColliderSBB, vec_proy_pos[i]);//Se carga en la capa correspondiente
			modelTank_Proyectile.render(modelMatrixCollider);
			std::cout << "Proyectil " << i << " actualizado correctamente de " << count_proyectiles << std::endl;	
		}
		
		/*******************************************
		 * Collider Boxes
		 *******************************************/
		glm::mat4 modelMatrixCollider = glm::mat4(modelTankMatrix_Chasis);
		AbstractModel::OBB bodyColliderOBB;
		bodyColliderOBB.u = glm::quat_cast(modelMatrixCollider);//Orientación de la caja antes que la escala
		modelMatrixCollider = glm::scale(modelMatrixCollider, glm::vec3(1.0f));
		modelMatrixCollider = glm::translate(modelMatrixCollider,modelTank_Chasis.getObb().c);//Trasladar al centro del modelo
		bodyColliderOBB.c = modelMatrixCollider[3];
		bodyColliderOBB.e = modelTank_Chasis.getObb().e * glm::vec3(1.0f) * glm::vec3(1.0f);//Escala del modelo, el segundo vector es un ajuste manual
		addOrUpdateColliders(lay_Colition_OBB, "ModelBox", bodyColliderOBB, modelTankMatrix_Chasis);//Se carga en la capa correspondiente
		/*******************************************
		 * Visualizacion de Collider Boxes
		 *******************************************/
		std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator itOBB;
		for(itOBB = lay_Colition_OBB.begin(); itOBB != lay_Colition_OBB.end(); itOBB++){
			glm::mat4 matrixCollider = glm::mat4(1.0f);
			matrixCollider = glm::translate(matrixCollider, std::get<0>(itOBB->second).c);
			matrixCollider = matrixCollider * glm::mat4(std::get<0>(itOBB->second).u);
			matrixCollider = glm::scale(matrixCollider, std::get<0>(itOBB->second).e * 2.0f);
			boxDrawable.enableWireMode();
			boxDrawable.render(matrixCollider);
		}
		/*******************************************
		* Visualizacion de Jaula de proyectiles
		*******************************************/
		glGetIntegerv(GL_CULL_FACE_MODE, &oldCullFaceMode);
		glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFuncMode);
		glCullFace(GL_FRONT);
		glDepthFunc(GL_LEQUAL);
		glActiveTexture(GL_TEXTURE0);
		drawableCage.enableWireMode();
		drawableCage.render();
		glCullFace(oldCullFaceMode);
		glDepthFunc(oldDepthFuncMode);
		/*******************************************
		* Visualizacion de Proyectiles
		*******************************************/
		std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator itSBB;
		for(itSBB = lay_SBB_Proyectile_Player.begin(); itSBB != lay_SBB_Proyectile_Player.end(); itSBB++){
			glm::mat4 matrixCollider = glm::mat4(1.0f);
			matrixCollider = glm::translate(matrixCollider, std::get<0>(itSBB->second).c);
			matrixCollider = glm::scale(matrixCollider, glm::vec3(std::get<0>(itSBB->second).ratio * 2.0f));
			proyectileSphere.enableWireMode();
			proyectileSphere.render(matrixCollider);
		}
		/*******************************************
		* Visualizacion de Collider Ray
		*******************************************/
		std::map<std::string, std::tuple<AbstractModel::RAY, glm::mat4, glm::mat4>>::iterator itRAY;
		for(itRAY = lay_Colition_RAY.begin(); itRAY != lay_Colition_RAY.end(); itRAY++){
			glm::mat4 matrixCollider = glm::mat4(std::get<0>(itRAY->second).mat);
			matrixCollider[3] = glm::vec4(std::get<0>(itRAY->second).rmd, 1.0f);
			matrixCollider = glm::scale(matrixCollider, glm::vec3(0.05f, 0.05f, std::get<0>(itRAY->second).mD));
			rayDrawable.render(matrixCollider);
		}
		
		/*******************************************
		* Actualizacion de Colisiones
		*******************************************/
		size_t index_proyectile = 0;//Se descartará todo proyectil que salga del terreno
		for (std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator it = lay_SBB_Proyectile_Player.begin(); it != lay_SBB_Proyectile_Player.end(); it++){
			if(!testSBBSBB(proyectile_Cage, std::get<0>(it->second))){//Si el proyectil sale de la jaula
				vec_startTime.erase(vec_startTime.begin() + index_proyectile);
				vec_proy_pos.erase(vec_proy_pos.begin() + index_proyectile);
				vec_proyectile.erase(vec_proyectile.begin() + index_proyectile);
				std::cout << "Proyectil " << index_proyectile << " descartado, salio del alcance" << std::endl;	
				count_proyectiles--;
				if(count_proyectiles < 0)
					count_proyectiles = 0;
			}else if(std::get<0>(it->second).c[1] <= terrain.getHeightTerrain(std::get<0>(it->second).c[0], std::get<0>(it->second).c[2])){//Si el proyectil está por debajo de la altura del terreno
				vec_startTime.erase(vec_startTime.begin() + index_proyectile);
				vec_proy_pos.erase(vec_proy_pos.begin() + index_proyectile);
				vec_proyectile.erase(vec_proyectile.begin() + index_proyectile);
				std::cout << "Proyectil " << index_proyectile << " descartado, choco en el suelo" << std::endl;	
				count_proyectiles--;
				if(count_proyectiles < 0)
					count_proyectiles = 0;
			}else
				index_proyectile++;
		}

		//Rayo con Esfera
		for (std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator it = lay_Colition_SBB.begin(); it != lay_Colition_SBB.end(); it++) {
			if(testRaySBB(ray, std::get<0>(it->second))){
				//std::cout << "Hay colision entre el rayo y el modelo " << it->first << std::endl;
			}
		}
		//Rayo con Caja
		for (std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator it = lay_Colition_OBB.begin(); it != lay_Colition_OBB.end(); it++) {
			if(testRayOBB(ray, std::get<0>(it->second))){
				//std::cout << "Hay colision entre el rayo y el modelo " << it->first << std::endl;
			}
		}
		//Esfera con Esfera
		for (std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator it = lay_Colition_SBB.begin(); it != lay_Colition_SBB.end(); it++) {
			bool isCollision = false;
			for (std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator jt = lay_Colition_SBB.begin(); jt != lay_Colition_SBB.end(); jt++) {
				if (it != jt && testSBBSBB(std::get<0>(it->second), std::get<0>(jt->second))) {
					//std::cout << "Hay collision entre " << it->first << " y el modelo " << jt->first << std::endl;
					isCollision = true;
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isCollision);
		}
		//Caja con Caja
		for (std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator it = lay_Colition_OBB.begin(); it != lay_Colition_OBB.end(); it++) {
			bool isColision = false;
			for (std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator jt = lay_Colition_OBB.begin(); jt != lay_Colition_OBB.end(); jt++) {
				if (it != jt && testOBBOBB(std::get<0>(it->second), std::get<0>(jt->second))) {
					//std::cout << "Hay colision entre " << it->first << " y el modelo" << jt->first << std::endl;
					isColision = true;
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isColision);
		}
		//Esfera con Caja
		for (std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator it = lay_Colition_SBB.begin(); it != lay_Colition_SBB.end(); it++) {
			bool isCollision = false;
			for (std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator jt = lay_Colition_OBB.begin(); jt != lay_Colition_OBB.end(); jt++) {
				if (testSBBOBB(std::get<0>(it->second), std::get<0>(jt->second))) {
					//std::cout << "Hay colision del " << it->first << " y el modelo" << jt->first << std::endl;
					isCollision = true;
					addOrUpdateCollisionDetection(collisionDetection, jt->first, true);
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isCollision);
		}
		
		//Busqueda de Colisiones
		for (std::map<std::string, bool>::iterator itCollision = collisionDetection.begin(); itCollision != collisionDetection.end(); itCollision++) {
			std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator sbbBuscado = lay_Colition_SBB.find(itCollision->first);
			std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator obbBuscado = lay_Colition_OBB.find(itCollision->first);
			if (sbbBuscado != lay_Colition_SBB.end())
				if (!itCollision->second) 
					addOrUpdateColliders(lay_Colition_SBB, itCollision->first);
			if (obbBuscado != lay_Colition_OBB.end())
				if (!itCollision->second) 
					addOrUpdateColliders(lay_Colition_OBB, itCollision->first);
				else {
					//Si algo está haciendo colisión, entonces debería regresar a la posicion anterior válida
					/*if (itCollision->first.compare("mayow") == 0)
						modelMatrixMayow = std::get<1>(obbBuscado->second);
					if (itCollision->first.compare("dart") == 0)
						modelMatrixDart = std::get<1>(obbBuscado->second);*/
				}
			
		}
		glfwSwapBuffers(window);
	}
}

int main(int argc, char **argv) {
	init(1920, 980, "Guild of Engines V1.0.1", false);
	applicationLoop();
	destroy();
	return 1;
}