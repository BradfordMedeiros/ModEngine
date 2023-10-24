#ifndef MOD_BULLETDEBUG
#define MOD_BULLETDEBUG

#include "LinearMath/btIDebugDraw.h"
#include <iostream>
#include <glm/glm.hpp>
#include "./physics_common.h"

class BulletDebugDrawer : public btIDebugDraw {
   int m_debugMode;
   int32_t (*debugDrawLine)(glm::vec3 fromPos, glm::vec3 toPos, bool permaline, int32_t owner);
public:
  BulletDebugDrawer(int32_t (*drawLine)(glm::vec3 fromPos, glm::vec3 toPos, bool permaline, int32_t owner));
  virtual ~BulletDebugDrawer();
  virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor) override;
  virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
  virtual void drawSphere(const btVector3& p, btScalar radius, const btVector3& color) override;
  virtual void drawTriangle(const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha) override;
  virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
  virtual void reportErrorWarning(const char* warningString) override;
  virtual void draw3dText(const btVector3& location, const char* textString) override;
  virtual void setDebugMode(int debugMode) override { this -> m_debugMode = debugMode; }
  virtual int getDebugMode() const override { return m_debugMode; }
};

#endif