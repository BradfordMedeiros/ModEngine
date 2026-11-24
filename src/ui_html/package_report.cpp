#include "./package_report.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

struct PackageSizeNode {
   std::string name;
   std::string path;
   uintmax_t size = 0;
   std::vector<PackageSizeNode> children;
};

std::string htmlEscape(std::string value){
   std::string escaped;
   for (char character : value){
      if (character == '&'){
         escaped += "&amp;";
      }else if (character == '<'){
         escaped += "&lt;";
      }else if (character == '>'){
         escaped += "&gt;";
      }else if (character == '"'){
         escaped += "&quot;";
      }else{
         escaped += character;
      }
   }
   return escaped;
}

std::string jsonEscape(std::string value){
   std::string escaped;
   const char* hex = "0123456789abcdef";
   for (unsigned char character : value){
      if (character == '\\'){
         escaped += "\\\\";
      }else if (character == '"'){
         escaped += "\\\"";
      }else if (character == '\n'){
         escaped += "\\n";
      }else if (character == '\r'){
         escaped += "\\r";
      }else if (character == '\t'){
         escaped += "\\t";
      }else if (character < 0x20 || character == '<' || character == '>' || character == '&'){
         escaped += "\\u00";
         escaped += hex[(character >> 4) & 0x0f];
         escaped += hex[character & 0x0f];
      }else{
         escaped += character;
      }
   }
   return escaped;
}

std::string formatBytes(uintmax_t bytes){
   const char* units[] = { "B", "KiB", "MiB", "GiB" };
   double value = static_cast<double>(bytes);
   size_t unit = 0;
   while (value >= 1024.0 && unit < 3){
      value /= 1024.0;
      unit++;
   }
   std::ostringstream result;
   result << std::fixed << std::setprecision(unit == 0 ? 0 : 2) << value << " " << units[unit];
   return result.str();
}

bool comparePackageFileSize(const PackageFileSize& left, const PackageFileSize& right){
   if (left.size != right.size){
      return left.size > right.size;
   }
   return left.path < right.path;
}

PackageSizeNode* findPackageSizeChild(PackageSizeNode& node, std::string name){
   for (auto& child : node.children){
      if (child.name == name){
         return &child;
      }
   }
   return NULL;
}

void addPackageSizeFile(PackageSizeNode& root, const PackageFileSize& file){
   root.size += file.size;
   PackageSizeNode* current = &root;
   std::stringstream pathStream(file.path);
   std::string part;
   std::string currentPath;
   while (std::getline(pathStream, part, '/')){
      if (part.empty()){
         continue;
      }
      if (!currentPath.empty()){
         currentPath += "/";
      }
      currentPath += part;

      auto child = findPackageSizeChild(*current, part);
      if (child == NULL){
         current->children.push_back(PackageSizeNode {
            .name = part,
            .path = currentPath,
         });
         child = &current->children.back();
      }
      child->size += file.size;
      current = child;
   }
}

bool comparePackageSizeNode(const PackageSizeNode& left, const PackageSizeNode& right){
   if (left.size != right.size){
      return left.size > right.size;
   }
   return left.name < right.name;
}

void sortPackageSizeTree(PackageSizeNode& node){
   std::sort(node.children.begin(), node.children.end(), comparePackageSizeNode);
   for (auto& child : node.children){
      sortPackageSizeTree(child);
   }
}

void writePackageSizeRows(std::ostream& report, const PackageSizeNode& node, size_t parentId, size_t depth, size_t& nextId){
   for (auto& child : node.children){
      auto id = nextId++;
      auto isFolder = !child.children.empty();
      report << "<tr data-id=\"" << id << "\" data-parent=\"" << parentId
             << "\" data-folder=\"" << (isFolder ? "true" : "false") << "\">\n";
      report << "<td style=\"padding-left:" << (depth * 1.25 + 0.5) << "rem\">";
      if (isFolder){
         report << "<button class=\"toggle\" onclick=\"toggleRow(" << id << ", this)\">+</button>";
      }else{
         report << "<span class=\"file-marker\">-</span>";
      }
      report << htmlEscape(child.name) << "</td><td class=\"size\">" << formatBytes(child.size)
             << "</td><td>";
      if (isFolder){
         report << htmlEscape(child.path);
      }else{
         report << "<a class=\"source-link\" data-package-path=\"" << htmlEscape(child.path)
                << "\" onclick=\"openSourceFile(this); return false;\">" << htmlEscape(child.path) << "</a>";
      }
      report << "</td></tr>\n";
      if (isFolder){
         writePackageSizeRows(report, child, id, depth + 1, nextId);
      }
   }
}

}

bool writePackageSizeReport(const char* output, std::vector<PackageFileSize> files){
   std::sort(files.begin(), files.end(), comparePackageFileSize);

   uintmax_t totalBytes = 0;
   for (auto& file : files){
      totalBytes += file.size;
   }
   PackageSizeNode root { .name = "package", .path = "package" };
   for (auto& file : files){
      addPackageSizeFile(root, file);
   }
   sortPackageSizeTree(root);

   std::ofstream report(output);
   if (!report){
      std::cerr << "unable to write package report: " << output << std::endl;
      return false;
   }

   const uintmax_t packageHeaderBytes = 12;
   const uintmax_t fileMetadataBytes = 268;
   const uintmax_t packageOverhead = packageHeaderBytes + fileMetadataBytes * files.size();

   report << "<!doctype html>\n<html><head><meta charset=\"utf-8\">\n";
   report << "<title>Package size report</title>\n";
   report << "<style>body{font-family:system-ui,sans-serif;margin:2rem;color:#222}"
          << "table{border-collapse:collapse;width:100%;margin-top:1rem}"
          << "th,td{padding:.35rem .5rem;border-bottom:1px solid #ddd;text-align:left}"
          << "th{background:#f3f3f3}.size{text-align:right;white-space:nowrap}"
          << "button.toggle{width:1.5rem;margin-right:.35rem}"
          << ".file-marker{display:inline-block;width:1.85rem;text-align:center;color:#999}"
          << "tr[data-folder=true]{font-weight:600}"
          << ".package-contents tr.stripe{background:#f7f7f7}"
          << ".package-contents tr[data-id]:hover{background:#e8f0f8}"
          << ".source-link{color:inherit;text-decoration:none;cursor:default}"
          << ".source-link.available{color:#06c;text-decoration:underline;cursor:pointer}"
          << ".muted{color:#666}</style></head><body>\n";
   report << "<h1>Package size report</h1>\n";
   report << "<p><strong>" << files.size() << "</strong> files, "
          << "<strong>" << formatBytes(totalBytes) << "</strong> data, "
          << "<strong>" << formatBytes(packageOverhead) << "</strong> estimated metadata overhead.</p>\n";
   report << "<button onclick=\"downloadRawReport()\">Download raw JSON</button>\n";
   report << "<button onclick=\"chooseSourceRoot()\">Choose source root</button> "
          << "<span id=\"source-status\" class=\"muted\">No source directory selected.</span>\n";
   report << "<h2>Package contents</h2>\n";
   report << "<table class=\"package-contents\"><tr><th>Path</th><th class=\"size\">Size</th><th>Total path</th></tr>\n";
   size_t nextId = 1;
   writePackageSizeRows(report, root, 0, 0, nextId);
   report << "</table>\n";
   report << "<script>\n"
          << "const rawReport = {\n"
          << "  fileCount: " << files.size() << ",\n"
          << "  dataBytes: " << totalBytes << ",\n"
          << "  metadataBytes: " << packageOverhead << ",\n"
          << "  files: [\n";
   for (size_t i = 0; i < files.size(); i++){
      auto& file = files.at(i);
      report << "    {path: \"" << jsonEscape(file.path) << "\", sizeBytes: " << file.size << "}";
      if (i + 1 < files.size()){
         report << ",";
      }
      report << "\n";
   }
   report << "  ]\n"
          << "};\n"
          << "function downloadRawReport(){\n"
          << "  const blob = new Blob([JSON.stringify(rawReport, null, 2)], {type: 'application/json'});\n"
          << "  const url = URL.createObjectURL(blob);\n"
          << "  const link = document.createElement('a');\n"
          << "  link.href = url;\n"
          << "  link.download = 'package-size-report.json';\n"
          << "  link.click();\n"
          << "  URL.revokeObjectURL(url);\n"
          << "}\n"
          << "let sourceHandles = new Map();\n"
          << "function normalizeSourcePath(path){\n"
          << "  return path.replace(/\\\\/g, '/').replace(/^((\\.\\/)|(\\.\\.\\/))+/g, '');\n"
          << "}\n"
          << "async function indexSourceDirectory(directory, prefix){\n"
          << "  for await (const entry of directory.values()){\n"
          << "    const path = prefix ? prefix + '/' + entry.name : entry.name;\n"
          << "    if (entry.kind === 'directory') await indexSourceDirectory(entry, path);\n"
          << "    else sourceHandles.set(normalizeSourcePath(path), entry);\n"
          << "  }\n"
          << "}\n"
          << "function findSourceHandle(packagePath){\n"
          << "  const normalized = normalizeSourcePath(packagePath);\n"
          << "  for (const [sourcePath, handle] of sourceHandles){\n"
          << "    if (sourcePath === normalized || sourcePath.endsWith('/' + normalized) || normalized.endsWith('/' + sourcePath)) return handle;\n"
          << "  }\n"
          << "  return null;\n"
          << "}\n"
          << "function updateSourceLinks(){\n"
          << "  document.querySelectorAll('.source-link').forEach(link => {\n"
          << "    link.classList.toggle('available', findSourceHandle(link.dataset.packagePath) !== null);\n"
          << "  });\n"
          << "}\n"
          << "async function chooseSourceRoot(){\n"
          << "  const status = document.getElementById('source-status');\n"
          << "  if (!window.showDirectoryPicker){ status.textContent = 'This browser does not support directory access.'; return; }\n"
          << "  try{\n"
          << "    const directory = await window.showDirectoryPicker();\n"
          << "    sourceHandles = new Map();\n"
          << "    await indexSourceDirectory(directory, '');\n"
          << "    status.textContent = sourceHandles.size + ' source files indexed.';\n"
          << "    updateSourceLinks();\n"
          << "  }catch(error){\n"
          << "    if (error.name !== 'AbortError') status.textContent = 'Unable to access the selected directory.';\n"
          << "  }\n"
          << "}\n"
          << "async function openSourceFile(link){\n"
          << "  const handle = findSourceHandle(link.dataset.packagePath);\n"
          << "  if (!handle){ document.getElementById('source-status').textContent = 'Select the matching source root first.'; return; }\n"
          << "  const file = await handle.getFile();\n"
          << "  const url = URL.createObjectURL(file);\n"
          << "  window.open(url, '_blank', 'noopener');\n"
          << "  setTimeout(() => URL.revokeObjectURL(url), 60000);\n"
          << "}\n"
          << "const rows = Array.from(document.querySelectorAll('tr[data-id]'));\n"
          << "const expanded = new Set();\n"
          << "function updateRows(){\n"
          << "  rows.forEach(row => row.style.display = 'none');\n"
          << "  function showChildren(parent){\n"
          << "    rows.filter(row => row.dataset.parent === String(parent)).forEach(row => {\n"
          << "      row.style.display = '';\n"
          << "      if (expanded.has(Number(row.dataset.id))) showChildren(row.dataset.id);\n"
          << "    });\n"
          << "  }\n"
          << "  showChildren(0);\n"
          << "  rows.filter(row => row.style.display !== 'none').forEach((row, index) => {\n"
          << "    row.classList.toggle('stripe', index % 2 === 1);\n"
          << "  });\n"
          << "}\n"
          << "function toggleRow(id, button){\n"
          << "  if (expanded.has(id)){ expanded.delete(id); button.textContent = '+'; }\n"
          << "  else { expanded.add(id); button.textContent = '-'; }\n"
          << "  updateRows();\n"
          << "}\n"
          << "updateRows();\n"
          << "</script>\n";
   report << "<p class=\"muted\">Folders show aggregate size; expand them to inspect their contents.</p>\n";
   report << "</body></html>\n";
   report.close();

   std::cout << "package size report: " << output << std::endl;
   return true;
}

bool packageSizeReport(const char* output){
   return writePackageSizeReport(output, mountedPackageFiles());
}
