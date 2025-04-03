#include "./editor.h"

void setSelectedIndex(EditorContent& editor, objid id, bool reset){
  std::cout << "INFO: EDITOR: ADD SELECTED INDEX" << std::endl;
  bool inSelected = false;
  for (auto itemId : editor.selectedObjs){
    if (itemId == id){
      inSelected = true;
      break;
    }
  }

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

void unsetSelectedIndex(EditorContent& editor, objid id, bool clearFromClipboard){
  std::vector<objid> selectedObjs;
  for (auto itemId : editor.selectedObjs){
    if (itemId != id){
      selectedObjs.push_back(itemId);
    }
  }
  editor.selectedObjs = selectedObjs;
  editor.clipboardObjs.erase(id);
}

std::optional<objid> latestSelected(EditorContent& editor){
  return editor.selectedObjs.size() == 0 ? std::nullopt : std::optional<objid>(editor.selectedObjs.at(editor.selectedObjs.size() - 1));
}
