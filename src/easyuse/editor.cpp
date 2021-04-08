#include "./editor.h"

bool isInSelectedItems(std::vector<EditorItem> items, objid id){
  for (auto item : items){
    if (item.id == id){
      return true;
    }
  }
  return false;
}

void setSelectedIndex(EditorContent& editor, objid id, std::string name, bool reset){
  std::cout << "INFO: EDITOR: ADD SELECTED INDEX" << std::endl;
  bool inSelected = isInSelectedItems(editor.selectedObjs, id);
  if (reset){
    if (inSelected && editor.selectedObjs.size() == 1){
      editor.selectedObjs = {};
    }else{
      editor.selectedObjs = {};
      editor.selectedObjs.push_back(EditorItem {
        .id = id,
        .name = name,
      });
    }
  }else{
    if (inSelected){
      unsetSelectedIndex(editor, id, false);
    }else {
      editor.selectedObjs.push_back(EditorItem {
        .id = id,
        .name = name,
      });
    }
  }
}
void unsetSelectedIndex(EditorContent& editor, objid id, bool clearFromClipboard){
  std::vector<EditorItem> selectedObjs;
  for (auto item : editor.selectedObjs){
    if (item.id != id){
      selectedObjs.push_back(item);
    }
  }
  std::vector<EditorItem> clipboardObjs;
  for (auto item : editor.clipboardObjs){
    if (item.id != id){
      clipboardObjs.push_back(item);
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
  std::cout << "INFO: EDITOR: COPY ALL OBJECTS" << std::endl;
  for (auto item : editor.clipboardObjs){
    copyObject(item.id);
  }
}
void setClipboardFromSelected(EditorContent& editor){
  std::cout << "set clipboard size to: " << editor.selectedObjs.size() << std::endl;
  editor.clipboardObjs = editor.selectedObjs;
}
void rmAllObjects(EditorContent& editor, std::function<void(objid)> rmObjectById){
  std::cout << "INFO: EDITOR: RM ALL OBJECTS" << std::endl;
  for (auto item : editor.selectedObjs){
    rmObjectById(item.id);
  }
  editor.selectedObjs = {};
}

bool isSelected(EditorContent& editor, objid id){
  for (auto item : editor.selectedObjs){
    if (item.id == id){
      return true;
    }
  }
  return false;
}

objid selected(EditorContent& editor){
  if (editor.selectedObjs.size() > 0){
    return editor.selectedObjs.at(editor.selectedObjs.size() - 1).id;
  }
  return -1;
}

std::vector<objid> selectedIds(EditorContent& editor){
  std::vector<objid> ids;
  for (auto item : editor.selectedObjs){
    ids.push_back(item.id);
  }
  return ids;
}