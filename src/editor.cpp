#include "./editor.h"

bool isInSelectedItems(std::vector<EditorItem> items, objid id){
  for (auto item : items){
    if (item.id == id){
      return true;
    }
  }
  return false;
}

void setSelectedIndex(EditorContent& editor, objid id, std::string name, bool addMultiple){
  std::cout << "INFO: EDITOR: ADD SELECTED INDEX" << std::endl;
  editor.selectedObj = EditorItem {
    .id = id,
    .name = name,
  };
  if (addMultiple && !isInSelectedItems(editor.selectedObjs, id)){
    editor.selectedObjs.push_back(editor.selectedObj);
  }
  std::cout << "INFO: SELECTED ITEMS: ";
  for (auto item : editor.selectedObjs){
    std::cout << item.id << " ";
  }
  std::cout << std::endl;
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
void rmAllObjects(EditorContent& editor, std::function<void(objid)> rmObjectById){
  std::cout << "INFO: EDITOR: RM ALL OBJECTS" << std::endl;
  editor.selectedObj = EditorItem {
    .id = -1,
    .name = "",
  };

  for (auto item : editor.selectedObjs){
    rmObjectById(item.id);
  }
  editor.selectedObjs = {};
}

bool isSelected(EditorContent& editor, objid id){
  return editor.selectedObj.id == id;
}

objid selected(EditorContent& editor, bool multiselectmode){
  if (multiselectmode){
    return true;
  }
  return editor.selectedObj.id;
}

std::vector<objid> selectedIds(EditorContent& editor, bool multiselectmode){
  if (!multiselectmode){
    return { editor.selectedObj.id };
  }
  std::vector<objid> ids;
  for (auto item : editor.selectedObjs){
    ids.push_back(item.id);
  }
  return ids;
}