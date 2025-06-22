#include "./octree_shapes.h"

extern std::vector<FaceTexture> defaultTextureCoords;

Octree unsubdividedOctree {
  .rootNode = OctreeDivision {
    .fill = FILL_FULL,
    .material = OCTREE_MATERIAL_DEFAULT,
    .shape = ShapeBlock{},
    .faces = defaultTextureCoords,
    .divisions = {},
  },
};

Octree subdividedOne {
  .rootNode = OctreeDivision {
    .fill = FILL_MIXED,
    .material = OCTREE_MATERIAL_DEFAULT,
    .shape = ShapeBlock{},
    .faces = defaultTextureCoords,
    .divisions = {
      OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
      OctreeDivision { .fill = FILL_EMPTY, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
      OctreeDivision { 
        .fill = FILL_MIXED,
        .faces = defaultTextureCoords,
        .divisions = {
          OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_EMPTY, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_EMPTY, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
        },
      },
      OctreeDivision { .fill = FILL_EMPTY, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
      OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
      OctreeDivision { .fill = FILL_EMPTY, .shape = ShapeBlock{}, .faces = defaultTextureCoords},
      OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
      OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
    },
  },
};