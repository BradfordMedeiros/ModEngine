#ifndef MOD_EDITOR
#define MOD_EDITOR

#include <iostream>
#include <vector>
#include <functional>
#include "../common/util.h"

struct EditorContent {
  std::vector<objid> selectedObjs;
  std::vector<objid> clipboardObjs;
};

void setSelectedIndex(EditorContent& editor, objid id, bool reset);
void unsetSelectedIndex(EditorContent& editor, objid id, bool clearFromClipboard);
std::optional<objid> latestSelected(EditorContent& content);
std::vector<objid> selectedIds(EditorContent& editor);

#endif