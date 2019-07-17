#include "./camera.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>

glm::vec3 calculateFront(float yaw, float pitch){
   glm::vec3 front;
   front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
   front.y = sin(glm::radians(pitch));
   front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
   glm::vec3 cameraFront = glm::normalize(front);
   return cameraFront;
}

Camera::Camera(glm::vec3 position, glm::vec3 up, float speed, float pitch, float yaw): position(position), up(up), speed(speed), pitch(pitch), yaw(yaw) {
   this->front = calculateFront(yaw, pitch);  
}

void Camera::moveFront(){
  this->position += this->speed * this->front;
}
void Camera::moveBack(){
  this->position -= this->speed * this->front;
}
void Camera::moveLeft(){
  this->position -= this->speed * glm::normalize(glm::cross(this->front, this->up));
}
void Camera::moveRight(){
  this->position += this->speed * glm::normalize(glm::cross(this->front, this->up));
}
void Camera::setFront(float yaw, float pitch){
  if(pitch > 89.0f){
    pitch = 89.0f;
  }
  if(pitch < -89.0f){
    pitch = -89.0f;
  }

  std::cout << "yaw: " << yaw << std::endl;
  std::cout << "pitch " << pitch << std::endl;
  this->yaw = yaw;
  this->pitch = pitch;
  this->front = calculateFront(this->yaw, this->pitch);
}

void Camera::setFrontDelta(float deltaYaw, float deltaPitch){
  this->setFront(this->yaw + deltaYaw, this->pitch + deltaPitch);
}
glm::mat4 Camera::renderView(){
   return glm::lookAt(this->position, this->position + this->front, this->up);
}
  

