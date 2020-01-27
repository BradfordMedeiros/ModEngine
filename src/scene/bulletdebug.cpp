#include "./bulletdebug.h"

BulletDebugDrawer::BulletDebugDrawer(){
  this -> setDebugMode(btIDebugDraw::DBG_DrawWireframe);
}

BulletDebugDrawer::~BulletDebugDrawer(){ }


void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor){
  std::cout << "bullet debug drawer -- draw line" << std::endl;
}
void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color){
  std::cout << "bullet debug drawer -- draw line" << std::endl;
}
void BulletDebugDrawer::drawSphere(const btVector3& p, btScalar radius, const btVector3& color){}
void BulletDebugDrawer::drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha){}
void BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color){}
void BulletDebugDrawer::reportErrorWarning(const char* warningString){}
void BulletDebugDrawer::draw3dText(const btVector3& location, const char* textString){}
