#ifndef COLISIONES_H_
#define COLISIONES_H_

#include <map>
#include "AbstractModel.h"
#include "Box.h"
#include "Sphere.h"
#include "Cylinder.h"

void addOrUpdateColliders(std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> > &colliders, std::string name, AbstractModel::OBB collider, glm::mat4 transform) {
	std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> >::iterator it = colliders.find(name);
	if (it != colliders.end()){
		std::get<0>(it->second) = collider;
		std::get<2>(it->second) = transform;
	}else
		colliders[name] = std::make_tuple(collider, glm::mat4(1.0), transform);
}
void addOrUpdateColliders(std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> > &colliders,std::string name) {
	std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> >::iterator it =colliders.find(name);
	if (it != colliders.end())
		std::get<1>(it->second) = std::get<2>(it->second);
}
void addOrUpdateColliders(std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> > &colliders, std::string name, AbstractModel::SBB collider, glm::mat4 transform) {
	std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> >::iterator it = colliders.find(name);
	if (it != colliders.end()){
		std::get<0>(it->second) = collider;
		std::get<2>(it->second) = transform;
	}else
		colliders[name] = std::make_tuple(collider, glm::mat4(1.0), transform);
}
void addOrUpdateColliders(std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> > &colliders, std::string name) {
	std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> >::iterator it = colliders.find(name);
	if (it != colliders.end())
		std::get<1>(it->second) = std::get<2>(it->second);
}
void addOrUpdateColliders(std::map<std::string, std::tuple<AbstractModel::RAY, glm::mat4, glm::mat4> > &colliders, std::string name, AbstractModel::RAY collider, glm::mat4 transform) {
	std::map<std::string, std::tuple<AbstractModel::RAY, glm::mat4, glm::mat4> >::iterator it = colliders.find(name);
	if (it != colliders.end()){
		std::get<0>(it->second) = collider;
		std::get<2>(it->second) = transform;
	}else
		colliders[name] = std::make_tuple(collider, glm::mat4(1.0), transform);
}
void addOrUpdateColliders(std::map<std::string, std::tuple<AbstractModel::RAY, glm::mat4, glm::mat4> > &colliders, std::string name) {
	std::map<std::string, std::tuple<AbstractModel::RAY, glm::mat4, glm::mat4> >::iterator it = colliders.find(name);
	if (it != colliders.end())
		std::get<1>(it->second) = std::get<2>(it->second);
}

void addOrUpdateCollisionDetection(std::map<std::string, bool> &collisionDetector, std::string name, bool isCollision) {
	std::map<std::string, bool>::iterator colIt = collisionDetector.find(name);
	if(colIt != collisionDetector.end()){
		if(!colIt->second)
			colIt->second = isCollision;
	}else
		collisionDetector[name] = isCollision;
}

void setColliderOBB(std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>> &collider_lay, std::string tag, glm::mat4 modelMatrix, AbstractModel::OBB obb, glm::vec3 scale){
	AbstractModel::OBB colliderOBB;
	glm::mat4 matrixCollider = glm::mat4(modelMatrix);
	//matrixCollider = glm::rotate(matrixCollider, glm::radians(-90.0f), glm::vec3(1, 0, 0));

	colliderOBB.u = glm::quat_cast(matrixCollider);
	matrixCollider = glm::scale(matrixCollider, scale);
	matrixCollider = glm::translate(matrixCollider, obb.c);
	colliderOBB.e = obb.e * scale * glm::vec3(1.1f);
	colliderOBB.c = glm::vec3(matrixCollider[3]);
	addOrUpdateColliders(collider_lay, tag, colliderOBB, modelMatrix);
}
void setColliderSBB(std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>> &collider_lay, std::string tag, glm::mat4 modelMatrix, AbstractModel::SBB sbb, float scale){
	AbstractModel::SBB colliderSBB;
	glm::mat4 matrixCollider = glm::scale(modelMatrix, glm::vec3(scale));
	matrixCollider = glm::translate(matrixCollider, sbb.c);
	colliderSBB.c = glm::vec3(matrixCollider[3]);
	colliderSBB.ratio = sbb.ratio * scale;
	addOrUpdateColliders(collider_lay, tag, colliderSBB, modelMatrix);
}
void setColliderRAY(std::map<std::string, std::tuple<AbstractModel::RAY, glm::mat4, glm::mat4>> &collider_lay, std::string tag, glm::mat4 modelMatrix, float range){
	glm::mat4 modelMatrixRay = glm::mat4(modelMatrix);
	AbstractModel::RAY ray(range, modelMatrixRay);
	addOrUpdateColliders(collider_lay, "RayMainCharacter", ray, modelMatrix);
}

void renderLayerOBB(std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>> &collider_lay, Box &boxCollider, glm::vec4 color){
	std::map<std::string, std::tuple<AbstractModel::OBB,glm::mat4,glm::mat4>>::iterator it;
	for (it = collider_lay.begin(); it != collider_lay.end(); it++) {
		glm::mat4 matrixCollider = glm::translate(glm::mat4(1.0), std::get<0>(it->second).c);
		matrixCollider = matrixCollider * glm::mat4(std::get<0>(it->second).u);
		matrixCollider = glm::scale(matrixCollider, std::get<0>(it->second).e * 2.0f);
		boxCollider.setColor(color);
		boxCollider.enableWireMode();
		boxCollider.render(matrixCollider);
	}
}
void renderLayerSBB(std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>> &collider_lay, Sphere &sphereCollider, glm::vec4 color){
	std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> >::iterator it;
	for (it = collider_lay.begin(); it != collider_lay.end(); it++) {
		glm::mat4 matrixCollider =  glm::translate(glm::mat4(1.0), std::get<0>(it->second).c);
		matrixCollider = glm::scale(matrixCollider, glm::vec3(std::get<0>(it->second).ratio * 2.0f));
		sphereCollider.setColor(color);
		sphereCollider.enableWireMode();
		sphereCollider.render(matrixCollider);
	}
}
void renderLayerRAY(std::map<std::string, std::tuple<AbstractModel::RAY, glm::mat4, glm::mat4>> &collider_lay, Cylinder &rayCollider, glm::vec4 color){
	std::map<std::string, std::tuple<AbstractModel::RAY, glm::mat4, glm::mat4>>::iterator it;
	for(it = collider_lay.begin(); it != collider_lay.end(); it++){
		glm::mat4 matrixCollider = glm::mat4(std::get<0>(it->second).mat);
		matrixCollider[3] = glm::vec4(std::get<0>(it->second).rmd, 1.0f);
		//matrixCollider = glm::scale(matrixCollider, glm::vec3(0.05f, 0.05f, std::get<0>(it->second).mD));
		rayCollider.setColor(color);
		rayCollider.render(matrixCollider);
	}
}

bool testSLABPLane(float p, float v, float min, float max, float &tmin, float &tmax) {
    if (fabs(v) <= 0.01) {
        return p >= min && p <= max;
    }
    float odd = 1 / v;
    float t1 = (min - p) * odd;
    float t2 = (max - p) * odd;
    if (t1 > t2) {
        float aux = t1;
        t1 = t2;
        t2 = aux;
    }
    if (t1 > tmin) {
        tmin = t1;
    }
    if (t2 < tmax) {
        tmax = t2;
    }
    if (tmin > tmax)
        return false;
    return true;
}
bool intersectSegmentAABB(glm::vec3 o, glm::vec3 t, AbstractModel::AABB collider) {
    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;
    glm::vec3 d = glm::normalize(t - o);
    if (!testSLABPLane(o.x, d.x, collider.mins.x, collider.maxs.x, tmin, tmax))
        return false;
    if (!testSLABPLane(o.y, d.y, collider.mins.y, collider.maxs.y, tmin, tmax))
        return false;
    if (!testSLABPLane(o.z, d.z, collider.mins.z, collider.maxs.z, tmin, tmax))
        return false;
    if (tmin >= 0 && tmin <= glm::length(t - o))
        return true;
    return false;
}
bool testRayOBB(AbstractModel::RAY ray, AbstractModel::OBB obb){
	glm::quat qinv = glm::inverse(obb.u);
	glm::vec3 cAABB = qinv * obb.c;
	AbstractModel::AABB aabb(cAABB - obb.c, cAABB + obb.c);
	return intersectSegmentAABB(qinv * ray.ori, qinv * ray.tR, aabb);
}
bool testRaySBB(AbstractModel::RAY ray, AbstractModel::SBB sbb) {
	glm::vec3 vDirToSphere = sbb.c - ray.ori;// Vector del Origen del rayo al centro de la esfera.
	float fLineLength = glm::distance(ray.ori, ray.tR);// Distancia del origen al destino del rayo.
	float t = glm::dot(vDirToSphere, ray.rDir);// Proyección escalar de vDirToSphere sobre la direccion del rayo.

	glm::vec3 vClosestPoint;
	if (t <= 0.0f)// Si la distancia proyectada del origen es menor o igual que cero
		vClosestPoint = ray.ori;// Significa que el punto mas cercano al centro es el origen.
	else if (t >= fLineLength)// Si la proyección escalar del origen es mayor a distancia del origen
		vClosestPoint = ray.tR;// al destino, el punto mas cercano es el destino.
	else // En caso contrario de calcula el punto sobre la linea usando t.
		vClosestPoint = ray.ori + ray.rDir * (t);
	
	return glm::distance(sbb.c, vClosestPoint) <= sbb.ratio; // Se prueba si el punto mas cercao esta contenido en el radio de la esfera.
}
bool testSBBSBB(AbstractModel::SBB a, AbstractModel::SBB b) {
	return glm::distance(a.c, b.c) <= (a.ratio + b.ratio); // Si la distancia entre los centros en menor o igual a la suma de los radios, significa que están en contacto
}
bool testSBBOBB(AbstractModel::SBB sbb, AbstractModel::OBB obb){
	float d = 0;
	glm::quat qinv = glm::inverse(obb.u);
	sbb.c = qinv * glm::vec4(sbb.c, 1.0);
	obb.c = qinv * glm::vec4(obb.c, 1.0);
	AbstractModel::AABB aabb;
	aabb.mins = obb.c - obb.e;
	aabb.maxs = obb.c + obb.e;
	if (sbb.c[0] >= aabb.mins[0] && sbb.c[0] <= aabb.maxs[0]
			&& sbb.c[1] >= aabb.mins[1] && sbb.c[1] <= aabb.maxs[1]
			&& sbb.c[2] >= aabb.mins[2] && sbb.c[2] <= aabb.maxs[2])
		return true;
	for (int i = 0; i < 3; i++){
		if(sbb.c[i] < aabb.mins[i])
			d += (sbb.c[i] - aabb.mins[i]) * (sbb.c[i] - aabb.mins[i]);
		else if(sbb.c[i] > aabb.maxs[i])
			d += (sbb.c[i] - aabb.maxs[i]) * (sbb.c[i] - aabb.maxs[i]);
	}
	if(d <= sbb.ratio * sbb.ratio)
		return true;
	return false;
}
bool testOBBOBB(AbstractModel::OBB a, AbstractModel::OBB b){
	float EPSILON = 0.0001;
	float ra, rb;
	glm::mat3 R = glm::mat4(0.0), AbsR = glm::mat4(0.0);
	glm::mat3 matA = glm::mat3(a.u);
	glm::mat3 matB = glm::mat3(b.u);

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			R[i][j] = glm::dot(matA[i], matB[j]);

	glm::vec3 t = b.c - a.c;
	t = glm::vec3(glm::dot(t, matA[0]), glm::dot(t, matA[1]), glm::dot(t, matA[2]));
	
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			AbsR[i][j] = fabs(R[i][j]) + EPSILON;

	for (int i = 0; i < 3; i++) {
		ra = a.e[i];
		rb = b.e[0] * AbsR[i][0] + b.e[1] * AbsR[i][1] + b.e[2] * AbsR[i][2];
		if (fabs(t[i]) > ra + rb) return false;
	}

	for (int i = 0; i < 3; i++) {
		ra = a.e[0] * AbsR[0][i] + a.e[1] * AbsR[1][i] + a.e[2] * AbsR[2][i];
		rb = b.e[i];
		if (fabs(t[0] * R[0][i] + t[1] * R[1][i] + t[2] * R[2][i]) > ra + rb) return false;
	}

	ra = a.e[1] * AbsR[2][0] + a.e[2] * AbsR[1][0];
	rb = b.e[1] * AbsR[0][2] + b.e[2] * AbsR[0][1];
	if(fabs(t[2] * R[1][0] - t[1] * R[2][0]) > ra + rb) return false;

	ra = a.e[1] * AbsR[2][1] + a.e[2] * AbsR[1][1];
	rb = b.e[0] * AbsR[0][2] + b.e[2] * AbsR[0][0];
	if(fabs(t[2] * R[1][1] - t[1] * R[2][1]) > ra + rb) return false;
	
	ra = a.e[1] * AbsR[2][2] + a.e[2] * AbsR[1][2];
	rb = b.e[0] * AbsR[0][1] + b.e[1] * AbsR[0][0];
	if(fabs(t[2] * R[1][2] - t[1] * R[2][2]) > ra + rb) return false;
	
	ra = a.e[0] * AbsR[2][0] + a.e[2] * AbsR[0][0];
	rb = b.e[1] * AbsR[1][2] + b.e[2] * AbsR[1][1];
	if(fabs(t[0] * R[2][0] - t[2] * R[0][0]) > ra + rb) return false;
	
	ra = a.e[0] * AbsR[2][1] + a.e[2] * AbsR[0][1];
	rb = b.e[0] * AbsR[1][2] + b.e[2] * AbsR[1][0];
	if(fabs(t[0] * R[2][1] - t[2] * R[0][1]) > ra + rb) return false;
	
	ra = a.e[0] * AbsR[2][2] + a.e[2] * AbsR[0][2];
	rb = b.e[0] * AbsR[1][1] + b.e[1] * AbsR[1][0];
	if(fabs(t[0] * R[2][2] - t[2] * R[0][2]) > ra + rb) return false;

	ra = a.e[0] * AbsR[1][0] + a.e[1] * AbsR[0][0];
	rb = b.e[1] * AbsR[2][2] + b.e[2] * AbsR[2][1];
	if(fabs(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb) return false;
	
	ra = a.e[0] * AbsR[1][1] + a.e[1] * AbsR[0][1];
	rb = b.e[0] * AbsR[2][2] + b.e[2] * AbsR[2][0];
	if(fabs(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb) return false;
	
	ra = a.e[0] * AbsR[1][2] + a.e[1] * AbsR[0][2];
	rb = b.e[0] * AbsR[2][1] + b.e[1] * AbsR[2][0];
	if(fabs(t[1] * R[0][2] - t[0] * R[1][2]) > ra + rb) return false;

	return true;
}

#endif /* COLISIONES_H_ */
