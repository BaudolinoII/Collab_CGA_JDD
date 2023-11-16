/*
 * AnimationUtils.h
 *
 *  Created on: Oct 14, 2019
 *      Author: rey
 */

#ifndef HEADERS_ANIMATIONUTILS_H_
#define HEADERS_ANIMATIONUTILS_H_

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllexport))
    #else
      #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllimport))
    #else
      #define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define DLL_PUBLIC
    #define DLL_LOCAL
  #endif
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

std::string DLL_PUBLIC matToString(glm::mat4 matrix);
void DLL_PUBLIC appendFrame(std::ofstream &outputFile,
	std::vector<glm::mat4> matrixList);
void DLL_PUBLIC appendFrame(std::ofstream &outputFile,
	std::vector<float> jointsList);
std::vector<std::vector<glm::mat4>> DLL_PUBLIC getKeyFrames(std::string fileName);
std::vector<std::vector<float>> DLL_PUBLIC getKeyRotFrames(std::string fileName);
glm::mat4 DLL_PUBLIC interpolate(std::vector<std::vector<glm::mat4>> keyFrames, int index,
	int indexNext, int jointID, float interpolation);
float DLL_PUBLIC interpolate(std::vector<std::vector<float>> keyFrames, int index,
	int indexNext, int jointID, float interpolation);

std::string matToString(glm::mat4 matrix) {
	std::stringstream ss;
	const float *pSource = (const float*)glm::value_ptr(matrix);
	for (int i = 0; i < 16; ++i) {
		ss << pSource[i];
		if(i < 15)
			ss << ",";
	}
	return ss.str();
}

void appendFrame(std::ofstream &outputFile,
		std::vector<glm::mat4> matrixList){
	std::stringstream ss;
	for(unsigned int i = 0; i < matrixList.size(); i++)
		ss << matToString(matrixList[i]) << "|";
	//outputFile << ss.str() << "|" << std::endl;
	outputFile << ss.str() << std::endl;
}

void appendFrame(std::ofstream &outputFile,
		std::vector<float> jointsList){
	std::stringstream ss;
	for(unsigned int i = 0; i < jointsList.size(); i++)
		ss << jointsList[i] << "|";
	outputFile << ss.str() << std::endl;
	//outputFile << ss.str() << "|" << std::endl;
}

std::vector<std::vector<glm::mat4>> getKeyFrames(std::string fileName) {
	std::vector<std::vector<glm::mat4>> keyFrames;
	std::string line;
	std::ifstream infile(fileName);
	std::string s = line;
	while (std::getline(infile, line))
	{
		std::vector<glm::mat4> transforms;
		s = line;
		size_t pos1 = 0;
		std::string token1;
		std::string delimiter1 = "|";
		while ((pos1 = s.find(delimiter1)) != std::string::npos) {
			token1 = s.substr(0, pos1);
			//std::cout << token1 << std::endl;

			size_t pos2 = 0;
			std::string token2;
			std::string delimiter2 = ",";

			int i = 0;
			int j = 0;
			glm::mat4 transform;
			float aaa[16];
			while ((pos2 = token1.find(delimiter2)) != std::string::npos) {
				token2 = token1.substr(0, pos2);
				aaa[i++] = atof(token2.c_str());
				/*if (j == 0)
					transform[i].x = atof(token2.c_str());
				if (j == 1)
					transform[i].y = atof(token2.c_str());
				if (j == 2)
					transform[i].z = atof(token2.c_str());
				if (j == 3)
					transform[i].w = atof(token2.c_str());*/
				/*j++;
				if (j > 3) {
					i++;
					j = 0;
				}*/
				token1.erase(0, pos2 + delimiter2.length());
			}
			aaa[i++] = atof(token1.c_str());
			transform = glm::make_mat4(aaa);
			transforms.push_back(transform);
			s.erase(0, pos1 + delimiter1.length());
		}
		keyFrames.push_back(transforms);
	}
	return keyFrames;
}

std::vector<std::vector<float>> getKeyRotFrames(std::string fileName) {
	std::vector<std::vector<float>> keyFrames;
	std::string line;
	std::ifstream infile(fileName);
	std::string s = line;
	while (std::getline(infile, line))
	{
		std::vector<float> rotations;
		s = line;
		size_t pos1 = 0;
		std::string token1;
		std::string delimiter1 = "|";
		while ((pos1 = s.find(delimiter1)) != std::string::npos) {
			token1 = s.substr(0, pos1);
			//std::cout << token1 << std::endl;
			float rotation = atof(token1.c_str());
			rotations.push_back(rotation);
			s.erase(0, pos1 + delimiter1.length());
		}
		keyFrames.push_back(rotations);
	}
	return keyFrames;
}

glm::mat4 interpolate(std::vector<std::vector<glm::mat4>> keyFrames, int index,
		int indexNext, int jointID, float interpolation){
	glm::quat firstQuat;
	glm::quat secondQuat;
	glm::quat finalQuat;
	glm::mat4 interpoltaedMatrix;
	glm::vec4 transformComp1;
	glm::vec4 transformComp2;
	glm::vec4 finalTrans;

	firstQuat = glm::quat_cast(keyFrames[index][jointID]);
	secondQuat = glm::quat_cast(keyFrames[indexNext][jointID]);
	finalQuat = glm::slerp(firstQuat, secondQuat, interpolation);
	interpoltaedMatrix = glm::mat4_cast(finalQuat);
	transformComp1 = keyFrames[index][jointID] * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	transformComp2 = keyFrames[indexNext][jointID] * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	finalTrans = (float)(1.0 - interpolation) * transformComp1 + transformComp2 * interpolation;
	interpoltaedMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(finalTrans)) * interpoltaedMatrix;
	return interpoltaedMatrix;
}

float interpolate(std::vector<std::vector<float>> keyFrames, int index,
		int indexNext, int jointID, float interpolation){
	return (float)(1.0 - interpolation) * keyFrames[index][jointID] + keyFrames[indexNext][jointID] * interpolation;
}

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
	public: void loadFrameData(std::string fileNameMat, std::string fileNameJn){
		keyFramesModelJoints = getKeyRotFrames(fileNameMat);
		keyFramesModel = getKeyFrames(fileNameJn);
	}
	public: void recordCurrFrame(std::string fileJoints, std::string fileMatrix){
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

#endif /* SRC_HEADERS_ANIMATIONUTILS_H_ */
