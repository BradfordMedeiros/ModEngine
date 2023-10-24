#include "./bulletdebug.h"

BulletDebugDrawer::BulletDebugDrawer(int32_t(*drawLine)(glm::vec3 fromPos, glm::vec3 toPos, bool permaline, int32_t owner)){
  this -> setDebugMode(btIDebugDraw::DBG_DrawWireframe );
  this -> debugDrawLine = drawLine; 
}
BulletDebugDrawer::~BulletDebugDrawer(){}


void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor){
  this -> debugDrawLine(btToGlm(from), btToGlm(to), false, 0);
}
void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color){
  this -> debugDrawLine(btToGlm(from), btToGlm(to), false, 0);
}
void BulletDebugDrawer::drawSphere(const btVector3& p, btScalar radius, const btVector3& color){
  std::cout << "CRITICAL DEBUG WARNING: DRAW SPHERE: not yet implemented" << std::endl;
  assert(false);
}
void BulletDebugDrawer::drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha){
  std::cout << "CRITICAL DEBUG WARNING: DRAW TRIANGLE: not yet implemented" << std::endl;
  assert(false);
}
void BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color){
  std::cout << "CRITICAL DEBUG WARNING: DRAW CONTACT POINT: not yet implemented" << std::endl;
  assert(false);
}
void BulletDebugDrawer::reportErrorWarning(const char* warningString){
  std::cout << "BULLET DEBUG WARNING: REPORT ERROR WARNING: " << warningString << std::endl;
  assert(false);
}
void BulletDebugDrawer::draw3dText(const btVector3& location, const char* textString){
  std::cout << "CRITICAL DEBUG WARNING: not yet implemented" << std::endl;
  assert(false);
}
