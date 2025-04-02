#include "./editor.h"

bool isInSelectedItems(std::vector<objid> items, objid id){
  for (auto itemId : items){
    if (itemId == id){
      return true;
    }
  }
  return false;
}

void setSelectedIndex(EditorContent& editor, objid id, bool reset){
  std::cout << "INFO: EDITOR: ADD SELECTED INDEX" << std::endl;
  bool inSelected = isInSelectedItems(editor.selectedObjs, id);
  if (reset){
    if (inSelected && editor.selectedObjs.size() == 1){
      editor.selectedObjs = {};
    }else{
      editor.selectedObjs = {};
      editor.selectedObjs.push_back(id);
    }
  }else{
    if (inSelected){
      unsetSelectedIndex(editor, id, false);
    }else {
      editor.selectedObjs.push_back(id);
    }
  }

}
void setActiveObj(EditorContent& editor, objid id){
  std::cout << "setting active obj to: " << id << std::endl;
  editor.activeObj = id;
}
void setNoActiveObj(EditorContent& editor){
  editor.activeObj = 0;
}
void unsetSelectedIndex(EditorContent& editor, objid id, bool clearFromClipboard){
  std::vector<objid> selectedObjs;
  for (auto itemId : editor.selectedObjs){
    if (itemId != id){
      selectedObjs.push_back(itemId);
    }
  }
  std::vector<objid> clipboardObjs;
  for (auto itemId : editor.clipboardObjs){
    if (itemId != id){
      clipboardObjs.push_back(itemId);
    }
  }
  editor.selectedObjs = selectedObjs;
  editor.clipboardObjs = clipboardObjs;
}

void clearSelectedIndexs(EditorContent& editor){
  std::cout << "INFO: EDITOR: CLEAR SELECTED INDEXES" << std::endl;
  editor.selectedObjs = {};
}
void copyAllObjects(EditorContent& editor, std::function<void(objid)> copyObject){
  modlog("editor", std::string("copying objects from clipboard, size = ") + std::to_string(editor.clipboardObjs.size()));
  for (auto itemId : editor.clipboardObjs){
    copyObject(itemId);
  }
}
void setClipboardFromSelected(EditorContent& editor){
  std::cout << "set clipboard size to: " << editor.selectedObjs.size() << std::endl;
  editor.clipboardObjs = editor.selectedObjs;
}

bool isSelected(EditorContent& editor, objid id){
  for (auto itemId : editor.selectedObjs){
    if (itemId == id){
      return true;
    }
  }
  return false;
}

std::optional<objid> latestSelected(EditorContent& editor){
  return editor.selectedObjs.size() == 0 ? std::nullopt : std::optional<objid>(editor.selectedObjs.at(editor.selectedObjs.size() - 1));
}

objid activeSelected(EditorContent& editor){
  return editor.activeObj;
}

std::vector<objid> selectedIds(EditorContent& editor){
  std::vector<objid> ids;
  for (auto item : editor.selectedObjs){
    ids.push_back(item);
  }
  return ids;
}