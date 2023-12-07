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

class Frame {//Variables para GUARDAR Key Frames tanto vectoriales como escalares
	public: std::vector<float> scl_var;
	public: std::vector<glm::vec3> vec_var;
	public: Frame(const size_t scl_com, const size_t vec_com) {
		this->setComplexity(scl_com, vec_com);
	}
	private: void setComplexity(const size_t scl_com, const size_t vec_com){
		for(size_t i = 0; i < scl_com; i++)
			this->scl_var.push_back(0.0f);
		for(size_t i = 0; i < vec_com; i++)
			this->vec_var.push_back(glm::vec3(0.0f));
	}
};


class Routine {
	private: std::vector<Frame> KeyFrames;
	private: size_t complexity_scl, complexity_vec, currDetail, nFrames, detail;
	private: int currFrame;
	private: std::vector<float> delta_scl;
	private: std::vector<float> curr_value_scl;
	private: std::vector<glm::vec3> delta_vec;
	private: std::vector<glm::vec3> curr_value_vec;
	private: bool play, cicle;

	public: Routine(const size_t complexity_scl, const size_t complexity_vec, const size_t nFrames, const size_t detail, const bool cicle = false) {
		this->nFrames = nFrames;
		this->complexity_scl = complexity_scl;
		this->complexity_vec = complexity_vec;
		for (size_t i = 0; i < this->nFrames; i++)
			this->KeyFrames.push_back(Frame(this->complexity_scl, this->complexity_vec));
		for (unsigned int i = 0; i < this->complexity_scl; i++) { 
			this->curr_value_scl.push_back(0.0f); 
			this->delta_scl.push_back(0.0f); 
		}
		for (unsigned int i = 0; i < this->complexity_vec; i++) {
			this->curr_value_vec.push_back(glm::vec3(0.0f));
			this->delta_vec.push_back(glm::vec3(0.0f));
		}
		this->currFrame = 0;
		this->currDetail = 0;
		this->detail = detail;
		this->play = false; this->cicle = cicle;
	}
	
	public: void interpolation() {
		for (size_t i = 0; i < complexity_scl; i++)
			delta_scl[i] = (KeyFrames[currFrame + 1].scl_var[i] - KeyFrames[currFrame].scl_var[i]) / (float)detail;
		for (size_t i = 0; i < complexity_vec; i++){
			delta_vec[i][0] = (KeyFrames[currFrame + 1].vec_var[i][0] - KeyFrames[currFrame].vec_var[i][0]) / (float)detail;
			delta_vec[i][1] = (KeyFrames[currFrame + 1].vec_var[i][1] - KeyFrames[currFrame].vec_var[i][1]) / (float)detail;
			delta_vec[i][2] = (KeyFrames[currFrame + 1].vec_var[i][2] - KeyFrames[currFrame].vec_var[i][2]) / (float)detail;
		}
	}
	public: void setAtCero() {
		for (size_t i = 0; i < complexity_scl; i++) curr_value_scl[i] = KeyFrames[0].scl_var[i];
		for (size_t i = 0; i < complexity_vec; i++) curr_value_vec[i] = KeyFrames[0].vec_var[i];
		currFrame = 0;
		currDetail = 0;
		interpolation();
	}
	public: void animacion() {//Movimiento del personaje
		if (play)
			if (currDetail >= detail) {//end of animation between frames?
				if (currFrame < nFrames - 2) {//Next frame interpolations
					currDetail = 0; //Reset counter
					currFrame++;
					interpolation();
				}else if (cicle) //end of total animation?
						  setAtCero();
					  else
						  play = false;
			}else{
				for (unsigned int i = 0; i < complexity_scl; i++)//Cambio de las variables
					curr_value_scl[i] += delta_scl[i];
				for (unsigned int i = 0; i < complexity_vec; i++){//Cambio de las variables
					curr_value_vec[i][0] += delta_vec[i][0];
					curr_value_vec[i][1] += delta_vec[i][1];
					curr_value_vec[i][2] += delta_vec[i][2];
				}
				currDetail++;
			}
	}
	public: float getScale(size_t index){
		if(index < this->complexity_scl)
			return this->curr_value_scl[index];
		return 0.0f;
	}
	public: glm::vec3 getVector(size_t index){
		if(index < this->complexity_vec)
			return this->curr_value_vec[index];
		return glm::vec3(0.0f);
	}
	public: void setKeyFrame(size_t KFindex, size_t index, float scl, glm::vec3 vec){
		if(KFindex >= this->KeyFrames.size())
			return;
		if(index < this->complexity_scl)
			this->KeyFrames[KFindex].scl_var[index] = scl;
		if(index < this->complexity_vec)
			this->KeyFrames[KFindex].vec_var[index] = vec;
	}
	public: void setKeyFrame(size_t KFindex, size_t index, glm::vec3 vec){
		if(KFindex >= this->KeyFrames.size())
			return;
		if(index < this->complexity_scl)
			if(index > 0)
				this->KeyFrames[KFindex].scl_var[index] = this->KeyFrames[KFindex].scl_var[index - 1];
			else
				this->KeyFrames[KFindex].scl_var[index] = 0.0f;
		if(index < this->complexity_vec)	
			this->KeyFrames[KFindex].vec_var[index] = vec;
	}
	public: void setKeyFrame(size_t KFindex, size_t index, float scl){
		if(KFindex >= this->KeyFrames.size())
			return;
		if(index < this->complexity_scl)
			this->KeyFrames[KFindex].scl_var[index] = scl;
		if(index < this->complexity_vec)
			if(index > 0)
				this->KeyFrames[KFindex].vec_var[index] = this->KeyFrames[KFindex].vec_var[index - 1];
			else
				this->KeyFrames[KFindex].vec_var[index] = glm::vec3(0.0f);
	}
	public: void setPlay(const bool play){
		this->play = play;
	}
};

#endif /* SRC_HEADERS_ANIMATIONUTILS_H_ */
