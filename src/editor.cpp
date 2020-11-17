#include "./editor.h"

void addSelectedIndex(EditorContent& editor, objid id, std::string name){
  std::cout << "INFO: EDITOR: ADD SELECTED INDEX" << std::endl;
  editor.selectedObj = EditorItem {
    .id = id,
    .name = name,
  };
}
void clearSelectedIndexs(EditorContent& editor){
  std::cout << "INFO: EDITOR: CLEAR SELECTED INDEXES" << std::endl;
  editor.selectedObj = EditorItem {
    .id = -1,
    .name = "",
  };
  editor.selectedObjs = {};
}
void copyAllObjects(EditorContent& editor, glm::vec3 offset){
  std::cout << "INFO: EDITOR: COPY ALL OBJECTS" << std::endl;
}
void mirrorAllObjects(EditorContent& editor){
  std::cout << "INFO: EDITOR: MIRROR ALL OBJECTS" << std::endl;
}
void rmAllObjects(EditorContent& editor){
  std::cout << "INFO: EDITOR: RM ALL OBJECTS" << std::endl;
}

bool isSelected(EditorContent& editor, objid id){
  return editor.selectedObj.id == id;
}

objid selected(EditorContent& editor){
  return editor.selectedObj.id;
}