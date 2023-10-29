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
#include "Shader.h"

class AModel{
    private: std::unordered_map<std::string,size_t> map_model;
    private: std::vector<Model> vec_model;
    private: std::string direction;

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
                    msg.append(buffer + "\n");
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
    private: void loadModels(std::string args, std::string shader){
      std::vector<std::string> cmd = makeChain(args);
      std::string order = cmd[0];
      Shader shad; shad.initialize(shader+ ".vs", shader + ".fs");
      cmd.erase(cmd.begin());
      if (!order.compare("LOAD_MODELS")) //Carga todos los modelos con las direcciones posteriores y almacena en el mapa
        for (std::string s : cmd) {
          Model mod; mod.loadModel(this->direction + s + ".obj"); mod.setShader(&shad);
          map_model[s] = vec_model.size();
          this->vec_model.push_back(mod);
        }
    }
    
    public: AModel(std::string manifesto, std::string shader){
        this->direction = manifesto.substr(0, manifesto.find_last_of('/') - manifesto.size());//Extrae la direccion del manifiesto
        std::string corpus = readManifest(manifesto);
        std::vector<std::string> vec;
        size_t line = corpus.find_first_of('\n');
        size_t begin = 0;
        while (line != std::string::npos) {
            std::string subcorp = corpus.substr(begin, line - begin + 1); subcorp.append(1, '\n');
            vec = makeChain(subcorp);
            if (!vec.empty())
                for (std::string s : vec)
                  this->loadModels(s, shader);
            begin = line + 1;
            line = corpus.find_first_of('\n', begin);
        }
    } 
    

};