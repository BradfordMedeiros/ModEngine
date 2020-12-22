#include "./sql.h"

// @TODO handle escaping properly

std::string escapeCSVEntry(std::string data){
  // maybe just ban commas and newlines for now? 
  return data;
}

std::string tablePath(std::string tableName){
  return "./res/state/" + tableName + ".csv";  // TODO do paths better bro
}
void createTable(std::string tableName, std::vector<std::string> columns){
  saveFile(tablePath(tableName), join(columns, ',') + "\n");
}

void deleteTable(std::string tableName){
  rmFile(tablePath(tableName));
}

std::vector<int> getColumnIndexs(std::vector<std::string> header, std::vector<std::string> columns){
  std::vector<int> indexs;
  for (auto column : columns){
    bool foundCol = false;
    for (int i = 0; i < header.size(), !foundCol; i++){
      if (header.at(i) == column){
        indexs.push_back(i);
        foundCol = true;
      }
    }
    assert(foundCol);
  }
  return indexs;
}

std::vector<std::vector<std::string>> select(std::string tableName, std::vector<std::string> columns, SqlFilter filter){
  auto tableContent = loadFile(tablePath(tableName));
  auto rawRows = split(tableContent, '\n');
  auto header = split(rawRows.at(0), ',');

  std::vector<std::vector<std::string>> rows;

  auto filterIndex = -1;
  if (filter.hasFilter){
    filterIndex = getColumnIndexs(header, { filter.column }).at(0);
  }

  auto indexs = getColumnIndexs(header, columns);

  for (int i = 1; i < rawRows.size(); i++){
    std::vector<std::string> row;
    auto columnContent = split(rawRows.at(i), ',');
    for (auto index : indexs){
      row.push_back(columnContent.at(index));
    }
    if (filter.hasFilter != -1){
      auto columnValue = columnContent.at(filterIndex);
      if (!filter.invert && columnValue != filter.value){
        continue;
      }
      if (filter.invert && columnValue == filter.value){
        continue;
      }
    }
    rows.push_back(row);
  }
  return rows;
}

void executeSqlQuery(SqlQuery& query){

}

void testQuery(){
  //createTable("quests", { "name", "complete", "value" });
  auto results = select(
    "quests", 
    {"value", "name", "value"}, 
    SqlFilter{ 
      .hasFilter = true,
      .column = "complete",
      .value = "false",
      .invert = false,
    });
  for (auto result : results){
    std::cout << join(result, ' ') << std::endl;
  }
}
