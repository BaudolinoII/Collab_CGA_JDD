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
#include "Headers/Box.h"
#include "Headers/Colisiones.h"
#include "Headers/Cylinder.h"
#include "Headers/Model.h"
#include "Headers/Shader.h"
#include "Headers/Sphere.h"
#include "Headers/Texture.h"
#include "Headers/Terrain.h"
#include "Headers/FirstPersonCamera.h"
#include "Headers/TimeManager.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

//Ventana de la aplicación
size_t screenWidth, screenHeight;
GLFWwindow *window;

//Camara 3 persona
float lastMousePosX, offsetX = 0;
float lastMousePosY, offsetY = 0;
std::shared_ptr<FirstPersonCamera> camera(new FirstPersonCamera());

//Shader con skybox, multiples luces, terreno del escenario, pruebas
Shader shaderSkybox, shaderMulLighting, shaderTerrain, shaderTest;

const GLsizei SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

Sphere skyboxSphere(20, 20);
Box boxCesped;

//Informacion de las alturas del terreno
Terrain terrain(-1, -1, 200, 8, "../Textures/heightmap.png");

GLuint textureTerrain[5]; // textureCespedID, textureTerrainRID, textureTerrainGID, textureTerrainBID, textureTerrainBlendMapID;
GLuint skyboxTextureID;
GLuint depthMap, depthMapFBO;

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
	shaderMulLighting.initialize("../Shaders/iluminacion_textura_animation.vs","../Shaders/multipleLights.fs");
	shaderTerrain.initialize("../Shaders/terrain.vs", "../Shaders/terrain.fs");

	// Inicializacion de los objetos.
	skyboxSphere.init();
	skyboxSphere.setShader(&shaderSkybox);
	skyboxSphere.setScale(glm::vec3(20.0f));

	boxCesped.init();
	boxCesped.setShader(&shaderMulLighting);

	// Terreno
	terrain.init();
	terrain.setShader(&shaderTerrain);

	camera->setPosition(glm::vec3(0.0, 3.0, 4.0));

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

	terrain.destroy();

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

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera->moveFrontCamera(true, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera->moveFrontCamera(false, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera->moveRightCamera(false, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera->moveRightCamera(true, deltaTime);
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		camera->mouseMoveCamera(offsetX, offsetY, deltaTime);
	offsetX = 0.0f;
	offsetY = 0.0f;

	glfwPollEvents();
	return continueApplication;
}

void applicationLoop() {
	bool psi = true;
	lastTime = TimeManager::Instance().GetTime();

	glm::vec3 axis;
	glm::vec3 target;
	float angleTarget;

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

		glfwSwapBuffers(window);
	}
}

int main(int argc, char **argv) {
	init(1920, 980, "Plantilla Vacía", false);
	applicationLoop();
	destroy();
	return 1;
}