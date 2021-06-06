#ifndef MOD_EDITOR
#define MOD_EDITOR

#include <iostream>
#include <vector>
#include <functional>
#include "../common/util.h"

struct EditorItem {
  objid id;
  std::string name;
};

struct EditorContent {
  std::vector<EditorItem> selectedObjs;
  std::vector<EditorItem> clipboardObjs;
};

void setSelectedIndex(EditorContent& editor, objid id, std::string name, bool reset);
void unsetSelectedIndex(EditorContent& editor, objid id, bool clearFromClipboard);
void clearSelectedIndexs(EditorContent& editor);
void copyAllObjects(EditorContent& editor, std::function<void(objid)> copyObject);
void setClipboardFromSelected(EditorContent& editor);
bool isSelected(EditorContent& editor, objid id);
objid selected(EditorContent& editor);
std::vector<objid> selectedIds(EditorContent& editor);

#endif