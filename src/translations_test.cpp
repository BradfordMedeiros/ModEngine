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

struct calcLineIntersectionTestValues {
  glm::vec3 fromPos;
  glm::vec3 fromDir;
  glm::vec3 toPos;
  glm::vec3 toDir;
  glm::vec3 intersectionPoint;
  bool intersects;
};

// visualization: https://www.geogebra.org/3d
// eg (1 + t, 3 + 2t, 2) for a line
void calcLineIntersectionTest(){
  std::vector<calcLineIntersectionTestValues> lineTests = {
    calcLineIntersectionTestValues { // point at origin
      .fromPos = glm::vec3(0.f, 0.f, 0.f),
      .fromDir = glm::vec3(0.f, 0.f, 0.f),
      .toPos = glm::vec3(0.f, 0.f, 0.f),
      .toDir = glm::vec3(0.f, 0.f, 0.f),
      .intersectionPoint = glm::vec3(0.f, 0.f, 0.f),
      .intersects = true,
    },
    calcLineIntersectionTestValues { // up dir offset by x = 1 aka parellel vectors
      .fromPos = glm::vec3(1.f, 0.f, 0.f),
      .fromDir = glm::vec3(0.f, 1.f, 0.f),
      .toPos = glm::vec3(2.f, 0.f, 0.f),
      .toDir = glm::vec3(0.f, 1.f, 0.f),
      .intersects = false,
    },
    calcLineIntersectionTestValues { // same origin point, different dirs
      .fromPos = glm::vec3(0.f, 0.f, 0.f),
      .fromDir = glm::vec3(0.f, 1.f, 0.f),
      .toPos = glm::vec3(0.f, 0.f, 0.f),
      .toDir = glm::vec3(1.f, 0.f, 0.f),
      .intersectionPoint = glm::vec3(0.f, 0.f, 0.f),
      .intersects = true,
    },
    calcLineIntersectionTestValues {  // two vectors that intersect from different points
      .fromPos = glm::vec3(0.f, 0.f, 0.f),
      .fromDir = glm::vec3(1.f, 1.f, 0.f),
      .toPos = glm::vec3(1.f, 2.f, 1.f),
      .toDir = glm::vec3(0.f, 1.f, 1.f),
      .intersectionPoint = glm::vec3(1.f, 1.f, 0.f),
      .intersects = true,
    },
    calcLineIntersectionTestValues {    // one points above the other, dirs are up and down
      .fromPos = glm::vec3(0.f, 0.f, 0.f),
      .fromDir = glm::vec3(0.f, 1.f, 0.f),
      .toPos = glm::vec3(0.f, 2.f, 0.f),
      .toDir = glm::vec3(0.f, -1.f, 0.f),
      .intersects = true,
    },
    calcLineIntersectionTestValues {    // diagnol and parellel
      .fromPos = glm::vec3(0.f, 0.f, 0.f),
      .fromDir = glm::vec3(-2.f, -2.f, 0.f),
      .toPos = glm::vec3(2.f, 2.f, 0.f),
      .toDir = glm::vec3(1.f, 1.f, 0.f),
      .intersects = true,
    },
    calcLineIntersectionTestValues {    //  parellel but not on same line, kind of like diagnol case
      .fromPos = glm::vec3(0.f, 0.f, 0.f),
      .fromDir = glm::vec3(-2.f, -2.f, 0.f),
      .toPos = glm::vec3(2.f, 2.f, -1.f),
      .toDir = glm::vec3(1.f, 1.f, 0.f),
      .intersects = false,
    },
  };

  for (int i = 0; i < lineTests.size(); i++){
    glm::vec3 intersectPoint(0.f, 0.f, 0.f);
    auto lineTest = lineTests.at(i);
    bool intersects = calcLineIntersection(lineTest.fromPos, lineTest.fromDir, lineTest.toPos, lineTest.toDir, &intersectPoint);
    if (intersects != lineTest.intersects){
      throw std::logic_error("incorrect line intersection value for line index: " + std::to_string(i) + " actual: " + (intersects ? "true" : "false"));
    }else if(intersects){
      auto intersectionCorrect = aboutEqual(intersectPoint, lineTest.intersectionPoint);
      if (!intersectionCorrect){
        throw std::logic_error("incorrect line intersection value for line index: " + std::to_string(i) + " actual: " + (intersects ? "true" : "false"));
      }
    }
  }
}