#include "./translations_test.h"

void moveRelativeIdentityTest(){
  auto newPos = moveRelative(
    glm::vec3(0.f, 0.f, 0.f), 
    quatFromDirection(glm::vec3(0.f, 0.f, -1.f)), 
    glm::vec3(0.f, 0.f, -1.f), 
    false
  );
  glm::vec3 expectedVec(0.f, 0.f, -1.f);
  if (!aboutEqual(newPos, expectedVec)){
    throw std::logic_error("expected vector: " + print(expectedVec));
  }
}

void moveRelativeRotateRight(){
  auto newPos = moveRelative(
    glm::vec3(0.f, 0.f, 0.f), 
    quatFromDirection(glm::vec3(1.f, 0.f, 0.f)), 
    glm::vec3(0.f, 0.f, -1.f), 
    false
  );
  glm::vec3 expectedVec(1.f, 0.f, 0.f);
  if (!aboutEqual(newPos, expectedVec)){
    throw std::logic_error("expected vector: " + print(expectedVec));
  }
}