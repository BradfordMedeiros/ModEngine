#include "./camera.h"

glm::vec3 calculateFront(float yaw, float pitch){
   glm::vec3 front;
   front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
   front.y = sin(glm::radians(pitch));
   front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
   glm::vec3 cameraFront = glm::normalize(front);
   return cameraFront;
}
Camera::Camera(glm::vec3 position, glm::vec3 up, float speed, float sensitivity, float pitch, float yaw): position(position), up(up), speed(speed), sensitivity(sensitivity), pitch(pitch), yaw(yaw) {
   this->front = calculateFront(yaw, pitch);  
}

void Camera::moveFront(float deltaTime){
  this->position += this->speed * this->front * deltaTime;
}
void Camera::moveBack(float deltaTime){
  this->position -= this->speed * this->front * deltaTime;
}
void Camera::moveLeft(float deltaTime){
  this->position -= this->speed * glm::normalize(glm::cross(this->front, this->up)) * deltaTime;
}
void Camera::moveRight(float deltaTime){
  this->position += this->speed * glm::normalize(glm::cross(this->front, this->up)) * deltaTime;
}
void Camera::setFront(float yaw, float pitch){
  if(pitch > 89.0f){
    pitch = 89.0f;
  }
  if(pitch < -89.0f){
    pitch = -89.0f;
  }
  this->yaw = yaw;
  this->pitch = pitch;
  this->front = calculateFront(this->yaw, this->pitch);
}
// @todo fix this, makes looking around feel jerky
void Camera::setFrontDelta(float deltaYaw, float deltaPitch, float deltaTime){
  this->setFront(this->yaw + (deltaYaw * this->sensitivity * deltaTime), this->pitch + (deltaPitch * this->sensitivity * deltaTime));
}
glm::mat4 Camera::renderView(){
   return glm::lookAt(this->position, this->position + this->front, this->up);
}
