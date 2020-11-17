#ifndef MOD_EDITOR
#define MOD_EDITOR

#include <iostream>
#include <vector>
#include "./common/util.h"

struct EditorItem {
  objid id;
  std::string name;
};

struct EditorContent {
  EditorItem selectedObj;
  std::vector<EditorItem> selectedObjs;
};

void setSelectedIndex(EditorContent& editor, objid id, std::string name, bool addMultiple = false);
void clearSelectedIndexs(EditorContent& editor);
void copyAllObjects(EditorContent& editor, glm::vec3 offset);
void mirrorAllObjects(EditorContent& editor);
void rmAllObjects(EditorContent& editor);
bool isSelected(EditorContent& editor, objid id);
objid selected(EditorContent& editor, bool multiselectmode = false);
std::vector<objid> selectedIds(EditorContent& editor, bool multiselectmode);

#endif