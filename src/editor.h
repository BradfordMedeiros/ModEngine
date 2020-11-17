#ifndef MOD_EDITOR
#define MOD_EDITOR

#include <iostream>
#include "./common/util.h"

struct EditorContent {

};

void addSelectedIndex(EditorContent& editor, objid id, std::string name);
void clearSelectedIndexs(EditorContent& editor);
void copyAllObjects(EditorContent& editor, glm::vec3 offset);
void mirrorAllObjects(EditorContent& editor);
void rmAllObjects(EditorContent& editor);

#endif