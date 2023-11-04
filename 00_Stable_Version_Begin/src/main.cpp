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
#include "Headers/TimeManager.h"
#include "Headers/Shader.h"
#include "Headers/Sphere.h"
#include "Headers/Cylinder.h"
#include "Headers/Box.h"
#include "Headers/FirstPersonCamera.h"
#include "Headers/Texture.h"
#include "Headers/Model.h"
#include "Headers/Terrain.h"
#include "Headers/AnimationUtils.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

int screenWidth;
int screenHeight;

GLFWwindow *window;

//Shader con skybox, multiples luces, terreno del escenario, pruebas
Shader shaderSkybox, shaderMulLighting, shaderTerrain, shaderTest;

std::shared_ptr<FirstPersonCamera> camera(new FirstPersonCamera());

Sphere skyboxSphere(20, 20);
Box boxCesped; 

//Objetos para pruebas de colisiones
Sphere sphereDrawable(10, 10);
Box boxDrawable;
Cylinder rayDrawable(10,10);

//Informacion de las alturas del terreno
Terrain terrain(-1, -1, 200, 8, "../Textures/heightmap.png");

//Elementos de un modelo
Model modelEjem;
glm::mat4 modelMatrix = glm::mat4(1.0f);
size_t animationModelIndex = 0;

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
int lastMousePosX, offsetX = 0;
int lastMousePosY, offsetY = 0;
//Variables de tiempo
double deltaTime;
double currTime, lastTime;

//Físicas Generales
const float GRAVITY = 1.81f;
double tmv = 0.0, startTimeJump = 0.0;
bool isNotJump = true;

//Colisiones
std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>> lay_Colition_SBB;
std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>> lay_Colition_OBB;

class animationMatrix{
	private: std::vector<std::vector<float>> keyFramesModelJoints;
	private: std::vector<std::vector<glm::mat4>> keyFramesModel;
	private: std::vector<float> currModelJs;
	private: std::vector<glm::mat4> matrixModel;

	private: glm::mat4 currMatrixModel;

	private: size_t idFrModelJs, idFrModelJsN, maxNStepModelJs, numStepModelJs, idFrModel, idFrModelN, maxNStepModel, numStepModel;
	private: float intpolModelJs = 0.0, intpolModel = 0.0;

	private: bool record;
	
	public: animationMatrix(const size_t nJoints = 0, const size_t maxNStepModelJs = 20, const size_t maxNStepModel = 100){
		this->idFrModelJs = 0;
		this->idFrModelJsN = 1;
		this->maxNStepModelJs = maxNStepModelJs;
		this->numStepModelJs = 0;
		this->idFrModel = 0;
		this->idFrModelN = 1;
		this->maxNStepModel = maxNStepModel;
		this->numStepModel = 0;
		this->record = true;
		this->currMatrixModel = glm::mat4(1);
		this->currModelJs.resize(nJoints);
	}

	// Variables to interpolation key frames
	//fileName = "../animaciones/animation_dart_joints.txt";
	public: void loadFrameData(const std::string fileNameMat, const std::string fileNameJn){
		keyFramesModelJoints = getKeyRotFrames(fileNameMat);
		keyFramesModel = getKeyFrames(fileNameJn);
	}
	public: void recordCurrFrame(const std::string fileJoints,const std::string fileMatrix){
		if(this->record){
			this->record = false;
			std::ofstream myfile;
			myfile.open(fileJoints);
			appendFrame(myfile, currModelJs);
			myfile.close();
			myfile.open(fileJoints);
			appendFrame(myfile, matrixModel);
			myfile.close();
		}
	}
	public: void animate(){
		if(this->keyFramesModelJoints.size() > 0){
			this->intpolModelJs = this->numStepModelJs / (float) this->maxNStepModelJs;
			this->numStepModelJs++;
			if(this->intpolModelJs > 1.0f){
				this->intpolModelJs = 0;
				this->numStepModelJs = 0;
				this->idFrModelJs = idFrModelJsN;
				this->idFrModelJsN++;
			}
			if(this->idFrModelJsN > (this->keyFramesModelJoints.size() -1))
				this->idFrModelJsN = 0;
			for(size_t i = 0; this->currModelJs.size(); i++)
				this->currModelJs[i] = interpolate(this->keyFramesModelJoints, this->idFrModelJs, this->idFrModelJsN, i, this->intpolModelJs);
		}
		
		if(this->keyFramesModel.size() > 0){
			this->intpolModel = this->numStepModel / (float) this->maxNStepModelJs;
			this->numStepModel++;
			if(this->intpolModel > 1.0f){
				this->numStepModel = 0;
				this->intpolModel = 0;
				this->idFrModel = idFrModelN;
				this->idFrModelN++;
			}
			if(this->idFrModelN > (this->keyFramesModel.size() - 1))
				this->idFrModelN = 0;
			this->currMatrixModel = interpolate(this->keyFramesModel, this->idFrModel, this->idFrModelN, 0, this->intpolModel);
		}
	}

	public: void addCurrJoint(const size_t index, const float value){
		this->currModelJs[index] += value;
	}
	public: void setCurrJoint(const size_t index, const float value){
		this->currModelJs[index] = value;
	}
	public: float getCurrJoint(const size_t index){
		return this->currModelJs[index];
	}
	public: void setCurrMatrix(glm::mat4 matrix){
		this->currMatrixModel = matrix;
	}
	public: glm::mat4 getCurrMatrix(){
		return this->currMatrixModel;
	}
	public: void enableRecord(){
		this->record = true;
	}
	public: bool isRecord(){
		return this->record;
	}
	/*
	//Declaración de la rutina
	animationMatrix modelAnimationMatrix(1);

	//Guardar key frames
	if(modelAnimationMatrix.isRecord() && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		modelAnimationMatrix.recordCurrFrame("../animaciones/animation.txt", "../animaciones/animationJn.txt");
	else if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
		modelAnimationMatrix.enableRecord();

	//Modificación en controladores
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		modelAnimationMatrix.addCurrJoint(0, 0.02f);

	//Cargar las animaciones
	modelAnimationMatrix.loadFrameData("../animaciones/animation.txt", "../animaciones/animationJn.txt");

	//Modificación previa a la ejecucion
	modelAnimationMatrix.setCurrMatrix(glm::translate(glm::mat4, glm::vec3(0.0f, 0.0f, 0.0f)));

	//Implementación en el loop principal
	modelAnimationMatrix.animate();
	glm::mat4 modelMatrixBody = modelAnimationMatrix.getCurrMatrix();
	modelMatrixBody = glm::translate(modelMatrixBody, glm::vec3(0.0f, 0.0f, 0.0f));
	modelMatrixBody = glm::rotate(modelMatrixBody, modelAnimationMatrix.getCurrJoint(0), glm::vec3(1, 0, 0));
	modelM.render(modelMatrixBody);
	*/
};

// Se definen todos las funciones.
void init(int width, int height, std::string strTitle, bool bFullScreen);
void destroy();
void reshapeCallback(GLFWwindow *Window, int widthRes, int heightRes);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow *window, int button, int state, int mod);
bool processInput(bool continueApplication = true);

void addOrUpdateColliders(std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>> &lay_Colition, std::string name, AbstractModel::OBB collider, glm::mat4 mat);
void addOrUpdateColliders(std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>> &lay_Colition, std::string name, AbstractModel::SBB collider, glm::mat4 mat);

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
	skyboxSphere.setScale(glm::vec3(20.0f, 20.0f, 20.0f));

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

	//Modelos
	modelEjem.loadModel("../models/mayow/personaje2.fbx");
	modelEjem.setShader(&shaderMulLighting);
	
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
	sphereDrawable.destroy();
	boxCesped.destroy();
	boxDrawable.destroy();
	terrain.destroy();

	modelEjem.destroy();

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
			std::cout << "lastMousePos.y:" << lastMousePosY << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_LEFT:
			std::cout << "lastMousePos.x:" << lastMousePosX << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			std::cout << "lastMousePos.x:" << lastMousePosX << std::endl;
			std::cout << "lastMousePos.y:" << lastMousePosY << std::endl;
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
	offsetX = 0;
	offsetY = 0;

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
		modelMatrix = glm::rotate(modelMatrix, 0.02f, glm::vec3(0, 1, 0));
		animationModelIndex = 0;
	} else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
		modelMatrix = glm::rotate(modelMatrix, -0.02f, glm::vec3(0, 1, 0));
		animationModelIndex = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, 0.02));
		animationModelIndex = 0;
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, -0.02));
		animationModelIndex = 0;
	}
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && isNotJump){
		animationModelIndex = 0;
		startTimeJump = currTime;
		tmv = 0;
		isNotJump = false;
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

void addOrUpdateColliders(std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>> &lay_Colition, std::string name, AbstractModel::OBB collider, glm::mat4 mat){
	std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator it = lay_Colition.find(name);
	if(it != lay_Colition.end()){
		std::get<0>(it->second) = collider;
		std::get<2>(it->second) = mat;
	}else
		lay_Colition[name] = std::make_tuple(collider, glm::mat4(1.0f), mat);
}
void addOrUpdateColliders(std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>> &lay_Colition, std::string name, AbstractModel::SBB collider, glm::mat4 mat){
	std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator it = lay_Colition.find(name);
	if(it != lay_Colition.end()){
		std::get<0>(it->second) = collider;
		std::get<2>(it->second) = mat;
	}else
		lay_Colition[name] = std::make_tuple(collider, glm::mat4(1.0f), mat);
}

void applicationLoop() {
	bool psi = true;
	lastTime = TimeManager::Instance().GetTime();

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

		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) screenWidth / (float) screenHeight, 0.01f, 100.0f);
		glm::mat4 view = camera->getViewMatrix();

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
		float currHeight = terrain.getHeightTerrain(modelMatrix[3][0], modelMatrix[3][2]);
		glm::vec3 ejey = glm::normalize(terrain.getNormalTerrain(modelMatrix[3][0], modelMatrix[3][2]));
		glm::vec3 ejex = glm::vec3(modelMatrix[0]);
		glm::vec3 ejez = glm::normalize(glm::cross(ejex, ejey));
		ejex = glm::normalize(glm::cross(ejey, ejez));
		modelMatrix[0] = glm::vec4(ejex, 0.0);
		modelMatrix[1] = glm::vec4(ejey, 0.0);
		modelMatrix[2] = glm::vec4(ejez, 0.0);
		//Aplicando el desplazamiento por gravedad
		modelMatrix[3][1] = -GRAVITY * tmv * tmv + 3.1 * tmv + currHeight;
		tmv = currTime - startTimeJump;

		if(modelMatrix[3][1] < currHeight){
			isNotJump = true;
			modelMatrix[3][1] = currHeight; 
		}

		glm::mat4 modelMatrixBody = glm::mat4(modelMatrix);
		modelMatrixBody = glm::scale(modelMatrixBody, glm::vec3(0.021f));
		modelEjem.setAnimationIndex(animationModelIndex);
		modelEjem.render(modelMatrixBody);
		animationModelIndex = 1;

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
		glm::mat4 modelMatrixRay = glm::mat4(modelMatrix);//Rayo se sujeta al frente del modelo
		modelMatrixRay = glm::translate(modelMatrixRay, glm::vec3(0.0f,1.0f,0.0f));
		const float maxDistance = 10.0f;
		glm::vec3 rayDirection = modelMatrixRay[2];
		glm::vec3 ori = modelMatrixRay[3];
		glm::vec3 rmd = ori + rayDirection*(maxDistance/2.0f);
		glm::vec3 targetRay = ori + rayDirection * maxDistance;
		modelMatrixRay[3] = glm::vec4(rmd, 1.0f);
		//Forma de Rayo
		modelMatrixRay = glm::rotate(modelMatrixRay, glm::radians(90.0f), glm::vec3(1,0,0));
		modelMatrixRay = glm::scale(modelMatrixRay, glm::vec3(0.05f, maxDistance, 0.5f));
		rayDrawable.render(modelMatrixRay);

		/*******************************************
		 * Collider Spheres
		 *******************************************/
		glm::mat4 modelMatrixCollider = glm::mat4(modelMatrix);
		AbstractModel::SBB bodyColliderSBB;
		modelMatrixCollider = glm::scale(modelMatrixCollider, glm::vec3(1.0f));//Trasladar al centro del modelo
		modelMatrixCollider = glm::translate(modelMatrixCollider,modelEjem.getSbb().c);
		bodyColliderSBB.c = modelMatrixCollider[3];
		bodyColliderSBB.ratio = modelEjem.getSbb().ratio * 1.0f;//Escala del radio
		addOrUpdateColliders(lay_Colition_SBB, "ModelSphere", bodyColliderSBB, modelMatrix);//Se carga en la capa correspondiente
		/*******************************************
		* Visualizacion de Collider Spheres
		*******************************************/
		std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator itSBB;
		for(itSBB = lay_Colition_SBB.begin(); itSBB != lay_Colition_SBB.end(); itSBB++){
			glm::mat4 matrixCollider = glm::mat4(1.0f);
			matrixCollider = glm::translate(matrixCollider, std::get<0>(itSBB->second).c);
			matrixCollider = glm::scale(matrixCollider, glm::vec3(std::get<0>(itSBB->second).ratio * 2.0f));
			sphereDrawable.enableWireMode();
			boxDrawable.render(matrixCollider);
		}
		/*******************************************
		 * Collider Boxes
		 *******************************************/
		modelMatrixCollider = glm::mat4(modelMatrix);
		AbstractModel::OBB bodyColliderOBB;
		bodyColliderOBB.u = glm::quat_cast(modelMatrixCollider);//Orientación de la caja antes que la escala
		modelMatrixCollider = glm::scale(modelMatrixCollider, glm::vec3(1.0f));
		modelMatrixCollider = glm::translate(modelMatrixCollider,modelEjem.getObb().c);//Trasladar al centro del modelo
		bodyColliderOBB.c = modelMatrixCollider[3];
		bodyColliderOBB.e = modelEjem.getObb().e * glm::vec3(1.0f) * glm::vec3(0.5f);//Escala del modelo, el segundo vector es un ajuste manual
		addOrUpdateColliders(lay_Colition_OBB, "ModelBox", bodyColliderOBB, modelMatrix);//Se carga en la capa correspondiente
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

		glfwSwapBuffers(window);
	}
}

int main(int argc, char **argv) {
	init(1920, 980, "Guild of Engines V1.0.1", false);
	applicationLoop();
	destroy();
	return 1;
}