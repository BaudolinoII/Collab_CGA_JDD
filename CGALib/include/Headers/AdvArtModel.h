#pragma once

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

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"
class Routine{
	private: std::vector<std::vector<glm::mat4>> KeyFrames;
    private: std::vector<glm::mat4> currMatrix;
    private: size_t begin, end, cFrm, nFrm;
    private: float detLV, cDet, interpolation;
    private: bool loop, active;
	public: Routine(const bool loop = false, const float detLV = 300.0f) {
        this->loop = loop;
        this->detLV = detLV;
        this->active = false;
        this->interpolation = 1.0f / detLV; 
    }
    
    public: void setBegin(const size_t begin = 0){
        this->begin = begin;
        this->cDet = 0.0f;
        this->cFrm = begin;
        this->nFrm = begin + 1;
        currMatrix = KeyFrames[0];
    }
    public: void setKeyFrame(std::vector<glm::mat4> mat_Value){
        this->KeyFrames.push_back(mat_Value);
    }
	
    public: void animation(){
        if(this->active)
            if(this->cDet < this->detLV){
                currMatrix.clear();
                for(int i = 0; i < this->KeyFrames.size(); i++){//Interpolacion
                    glm::quat finalQuat = glm::slerp(glm::quat(KeyFrames[cFrm][i]), glm::quat(KeyFrames[nFrm][i]), interpolation);
                    glm::vec4 transformComp1 = KeyFrames[cFrm][i] * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                    glm::vec4 transformComp2 = KeyFrames[nFrm][i] * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                    glm::vec4 finalTrans = (float)(1.0f - interpolation) * transformComp1 + transformComp2 * interpolation;
                    currMatrix.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(finalTrans)) * glm::mat4_cast(finalQuat));
                }
                cDet++;
            } else {
                this->cDet = 0.0f;
                this->cFrm = nFrm;
                this->nFrm++;
                if(this->nFrm > this->end)
                    if(this->loop)
                        this->nFrm = this->begin;
                    else
                        this->active = false;
            }
        
    }

	public: glm::mat4 getMatrix(size_t index){
        return currMatrix[index];
    }
};
class AAModel {
    private: std::vector<std::pair<Model&, std::string>> vec_Model;
    private: std::vector<std::pair<glm::mat4, char>> vec_Matrix;
    private: std::vector<std::pair<Routine&, std::string>> vec_Routine;
    private: std::unordered_map<std::string, size_t> map_Model, map_Routine;
    private: std::vector<std::string> stack_model;
    private: std::string direction;
    private: char status;

	public: AAModel(const std::string manifesto){
        this->direction = manifesto.substr(0, manifesto.find_last_of('/') - manifesto.size());//Extrae la direccion del manifiesto
        std::string corpus = readManifest(manifesto);
        this->status = 's';
        std::vector<std::string> vec;
        size_t line = corpus.find_first_of('\n');
        size_t begin = 0;
        while (line != std::string::npos) {
            std::string subcorp = corpus.substr(begin, line - begin + 1); subcorp.append(1, '\n');
            vec = makeChain(subcorp);
            if (!vec.empty())
                for (std::string s : vec)
                    loadObjects(s);
            begin = line + 1;
            line = corpus.find_first_of('\n', begin);
        }
    }
	public: void update(){}
	public: void init(){}

	private: std::string readManifest(const std::string manifest) {
        std::ifstream readfile(manifest, std::ios::in);
        if (readfile.fail()) {
            std::cout << "Archivo no encontrado\n";
            return "ERROR";
        }
        std::string buffer, msg;
        size_t i = 1;
        do {
            std::getline(readfile, buffer);
            buffer.erase(0, buffer.find_first_not_of(" \t\n\r\f\v"));//Eliminia espacios al inicio
            buffer.erase(buffer.find_last_not_of(" \t\n\r\f\v") + 1);//Elimina espacios al final
            if (!buffer.empty())
                if (buffer[0] != '#') {//Si es solo comentario, Descarta
                    if (buffer.find_first_of('#') != std::string::npos)//Si existe un comentario
                        buffer.erase(buffer.find_first_of('#'), buffer.size() - buffer.find_first_of('#')); //Elimina todo lo continuo al #
                    msg.append(buffer);
                    msg.append(1, '\n');
                }
            i++;
        } while (!readfile.eof());
        readfile.close();
        return msg;
    }
    private: std::vector<std::string> makeChain(const std::string args) {
        std::vector<std::string> vec;
        size_t split = args.find_first_of(" ={()\n");
        size_t before = 0;
        while (split != std::string::npos) {
            if (before != split)
                vec.push_back(args.substr(before, split - before));
            before = split + 1;
            split = args.find_first_of(" ={()\n", before);
        }
        return vec;
    }
    private: void loadObjects(std::string args) {
        std::string currObj; bool isStruct = true; glm::mat4 mat(1);
        std::function<void(std::vector<std::string>)> cmdline = [&](std::vector<std::string> cmd) {
            if(this->status == 'e')
                return;
            std::string order = cmd[0];
            cmd.erase(cmd.begin());
            if (!order.compare("LOAD_MODELS")) {
                //Carga todos los modelos con las direcciones posteriores y almacena en el mapa
                for (std::string s : cmd) {
                    std::string location(this->direction);
                    this->map_Model[s] = this->vec_Model.size();
                    location.append(s); location.append(".obj");
                    Model mod; mod.loadModel(s);
                    this->vec_Model.push_back({mod, s});
                    this->vec_Matrix.push_back({ glm::mat4(1), 's' });
                }
            } else if (!order.compare("MOD_STRUCT")) {
                //Fija el nombre del objeto actual para aplicar las transformaciones a la matriz indicada
                //y coloca la pila para colocar la jerarquia
                currObj = cmd[0];
                std::string family;
                for(std::string s : this->stack_model){family.append(s);family.append(1,'.');}
                this->vec_Model.at(this->map_Model[currObj]).second = family + this->vec_Model.at(this->map_Model[currObj]).second;
                this->stack_model.push_back(cmd[0]);
                cmd.erase(cmd.begin());
                cmdline(cmd);
            } else if (!order.compare("SINGLE_MODEL")) {
                //Fija el nombre del objeto actual para aplicar las transformaciones a la matriz indicada
                //Reinicia la pila, se considera que estos objetos son declarados fuera de las estructuras
                currObj = cmd[0];
                this->stack_model.clear();
                cmd.erase(cmd.begin());
                cmdline(cmd);
            } else if (!order.compare("ROUTINE")) {
                //Construye una rutina y guarda en el mapa de rutinas
                currObj = cmd[0];
                cmd.erase(cmd.begin());
                this->map_Routine[cmd[0]] = this->map_Routine.size();
                Routine rout; isStruct = false;
                this->vec_Routine.push_back({rout,cmd[0]});
                cmdline(cmd);
            } else if (!order.compare("X_SIM")) {
                //Se le indica a la Matriz correspondiente que esta debe ser dublicada en el eje X
                currObj = cmd[0];
                std::string family;
                for(std::string s : this->stack_model){family.append(s);family.append(1,'.');}
                this->vec_Model.at(this->map_Model[currObj]).second = family + this->vec_Model.at(this->map_Model[currObj]).second;
                this->vec_Matrix.at(this->map_Model[currObj]).second = 'x';
            } else if (!order.compare("Y_SIM")) {
                //Se le indica a la Matriz correspondiente que esta debe ser dublicada en el eje y
                currObj = cmd[0];
                std::string family;
                for(std::string s : this->stack_model){family.append(s);family.append(1,'.');}
                this->vec_Model.at(this->map_Model[currObj]).second = family + this->vec_Model.at(this->map_Model[currObj]).second;
                this->vec_Matrix.at(this->map_Model[currObj]).second = 'y';
            } else if (!order.compare("Z_SIM")) {
                //Se le indica a la Matriz correspondiente que esta debe ser dublicada en el eje z
                currObj = cmd[0];
                std::string family;
                for(std::string s : this->stack_model){family.append(s);family.append(1,'.');}
                this->vec_Model.at(this->map_Model[currObj]).second = family + this->vec_Model.at(this->map_Model[currObj]).second;
                this->vec_Matrix.at(this->map_Model[currObj]).second = 'z';
            } else if (!order.compare("FRAME")) {
                if(isStruct){
                    std::cout << "Error de sintaxis. FRAME no pertenece a MOD_STRUCT";
                    this->status = 'e';
                    return;
                }else{
                    currObj = cmd[0];
                    cmd.erase(cmd.begin());
                    
                    //this->vec_Routine.at(this->map_Routine[currObj]).first.setKeyFrame();

                }
            } else if (!order.compare("THIS")) {

            } else if (!order.compare("BOTH")) {

            } else if (!order.compare("CLONE")) {

            } else if (!order.compare("ORG_STAT")) {

            } else if (!order.compare("}")) {

            } else if (!order.compare("POS")) {

            } else if (!order.compare("ROT")) {

            } else if (!order.compare("SCL")) {

            } else if (!order.compare("MODEL")) {

            } else if (!order.compare("loop")) {

            } else if (!order.compare("fps")) {

            } else if (!order.compare("ppf")) {

            }
        };
    }
};