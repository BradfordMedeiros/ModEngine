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
  objid forceSelectIndex;
  objid activeObj;
  std::vector<EditorItem> selectedObjs;
  std::vector<EditorItem> clipboardObjs;
};

void setSelectedIndex(EditorContent& editor, objid id, std::string name, bool reset);
void setNoActiveObj(EditorContent& editor);
void setActiveObj(EditorContent& editor, objid id);
void unsetSelectedIndex(EditorContent& editor, objid id, bool clearFromClipboard);
void clearSelectedIndexs(EditorContent& editor);
void copyAllObjects(EditorContent& editor, std::function<void(objid)> copyObject);
void setClipboardFromSelected(EditorContent& editor);
bool isSelected(EditorContent& editor, objid id);
std::optional<objid> latestSelected(EditorContent& content);
objid activeSelected(EditorContent& editor);
std::vector<objid> selectedIds(EditorContent& editor);

#endif