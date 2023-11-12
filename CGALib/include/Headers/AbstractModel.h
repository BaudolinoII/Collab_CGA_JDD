/*
Creado por Reynaldo Martell
Computacion Grafica e Interaccion Humano Computadora
Fecha: 08/02/2018
*/

#ifndef ABSTRACTMODEL_H
#define ABSTRACTMODEL_H

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
#include <vector>
#include <memory>
#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include "Shader.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

class DLL_PUBLIC AbstractModel{
	public: class SBB {
		public: glm::vec3 c;
		public: float ratio;

		public: SBB() {}
		public: SBB(glm::vec3 c, float ratio) {
			this->c = c;
			this->ratio = ratio;
		}
	};
	public: class AABB {
		public: glm::vec3 mins;
		public: glm::vec3 maxs;
		public: AABB() {}
		public: AABB(glm::vec3 mins, glm::vec3 maxs) {
			this->mins = mins;
			this->maxs = maxs;
		}	
		public: AABB(glm::vec3 c, float width, float height, float length) {
			mins.x = c.x - width / 2.0;
			mins.y = c.y - height / 2.0;
			mins.z = c.z - length / 2.0;
			maxs.x = c.x + width / 2.0;
			maxs.y = c.y + height / 2.0;
			maxs.z = c.z + length / 2.0;
		}
		public: AABB(float minx, float miny, float minz, float maxx, float maxy, float maxz) {
			mins.x = minx;
			mins.y = miny;
			mins.z = minz;
			maxs.x = maxx;
			maxs.y = maxy;
			maxs.z = maxz;
		}
	};
	public: class OBB {
		public: glm::vec3 c;
		public: glm::vec3 e;
		public: glm::quat u;
		public: OBB(){}
		public: OBB(glm::vec3 c, glm::vec3 e, glm::quat u) {
			this->c = c;
			this->e = e;
			this->u = u;
		}
		
	};
	public: class RAY{
		public: float mD;
		public: glm::vec3 rDir;
		public: glm::vec3 ori;
		public: glm::vec3 rmd;
		public: glm::vec3 tR;
		public: glm::mat4 mat;

		public: RAY(){}
		public: RAY(float mD, glm::mat4& mat){
			this->mD = mD;
			this->mat = mat;
			this->rDir = mat[2];
			this->ori = mat[3];
			this->rmd = this->ori + this->rDir * (this->mD / 2.0f);
			this->tR = this->ori + this->rDir * this->mD;
		}
	};

	public: class Vertex {
		public: glm::vec3 m_pos;
		public: glm::vec2 m_tex;
		public: glm::vec3 m_normal;

		public: Vertex() {}
		public:	Vertex(glm::vec3 m_pos, glm::vec2 m_tex, glm::vec3 m_normal) : m_pos(m_pos), m_tex(m_tex), m_normal(m_normal) {}
	};
	public: class VertexColor {
		public: glm::vec3 m_pos;
		public: glm::vec3 m_color;

		public: VertexColor() {}
		public: VertexColor(glm::vec3 m_pos, glm::vec3 m_color) : m_pos(m_pos), m_color(m_color) {}
	};
	
	protected: Shader * shader_ptr;
	protected: glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
	protected: glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);
	protected: glm::vec4 color;
	protected: glm::vec3 orientation;
	protected: GLuint VAO, VBO, EBO;
	protected: std::vector<GLuint> index;
	protected: std::vector<Vertex> vertexArray;
	protected: AABB aabb;
	protected: SBB sbb;
	protected: OBB obb;

	public: void init() {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertexArray.size() * sizeof(vertexArray[0]), vertexArray.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(index[0]), index.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertexArray[0]), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertexArray[0]), (GLvoid*)(sizeof(vertexArray[0].m_pos)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertexArray[0]), (GLvoid*)(sizeof(vertexArray[0].m_pos) + sizeof(vertexArray[0].m_tex)));
		glEnableVertexAttribArray(3);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	public: void update() {
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		GLvoid* p = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		memcpy(p, vertexArray.data(), vertexArray.size() * sizeof(vertexArray[0]));
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	public: void render(glm::mat4 parentTrans = glm::mat4(1.0f)) {
		render(0, index.size(), parentTrans);
	}
	public: void render(int indexInit, int indexSize, glm::mat4 parentTrans = glm::mat4(1.0f)) {
		shader_ptr->turnOn();
		glBindVertexArray(VAO);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), this->scale);
		glm::mat4 translate = glm::translate(glm::mat4(1.0f), this->position);
		glm::quat oX = glm::angleAxis<float>(glm::radians(orientation.x), glm::vec3(1.0, 0.0, 0.0));
		glm::quat oY = glm::angleAxis<float>(glm::radians(orientation.y), glm::vec3(0.0, 1.0, 0.0));
		glm::quat oZ = glm::angleAxis<float>(glm::radians(orientation.z), glm::vec3(0.0, 0.0, 1.0));
		glm::quat ori = oZ * oY * oX;
		glm::mat4 modelMatrix = parentTrans * translate * glm::mat4_cast(ori) * scale;
		this->shader_ptr->setMatrix4("model", 1, GL_FALSE, glm::value_ptr(modelMatrix));
		this->shader_ptr->setVectorFloat4("ourColor", glm::value_ptr(color));
		glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, (GLuint*)(indexInit * sizeof(GLuint)));
		glBindVertexArray(0);
		shader_ptr->turnOff();
		this->enableFillMode();
	}
	public: void destroy() {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &EBO);
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &VAO);
	}

	public: GLuint getVAO(){
		return this->VAO;
	}
	public: void setShader(Shader * shader_ptr) {
		this->shader_ptr = shader_ptr;
	}

	public: Shader * getShader() {
		return this->shader_ptr;
	}
	public: glm::vec3 getPosition() {
		return this->position;
	}
	public: glm::vec3 getScale() {
		return this->scale;
	}
	public: glm::vec3 getOrientation() {
		return this->orientation;
	}

	public: void setPosition(glm::vec3 position) {
		this->position = position;
	}
	public: void setScale(glm::vec3 scale) {
		this->scale = scale;
	}
	public: void setOrientation(glm::vec3 orientation) {
		this->orientation = orientation;
	}

	public: void enableWireMode() {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	public: void enableFillMode() {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	public: void setColor(const glm::vec4& color) {
		this->shader_ptr->turnOn();
		this->shader_ptr->setVectorFloat4("ourColor", glm::value_ptr(color));
		this->shader_ptr->turnOff();
		this->color = color;
	}
	
	public: const std::vector<Vertex>& getVertexArray() const {
		return vertexArray;
	}

	public: const AABB& getAAbb() const {
		return aabb;
	}
	public: const SBB& getSbb() const {
		return sbb;
	}
	public: const OBB& getObb() const {
		return obb;
	}

};

#endif // ABSTRACTMODEL_H
