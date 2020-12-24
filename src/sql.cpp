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
  std::cout << "creating: " << tableName << "-- " << join(columns, ',') << std::endl;
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

struct TableData {
  std::vector<std::string> header;
  std::vector<std::string> rawRows;
};
TableData readTableData(std::string tableName){
  auto tableContent = loadFile(tablePath(tableName));
  auto rawRows = split(tableContent, '\n');
  auto header = split(rawRows.at(0), ',');
  return TableData{
    .header = header,
    .rawRows = rawRows,
  };
}

std::vector<std::vector<std::string>> select(std::string tableName, std::vector<std::string> columns, SqlFilter filter){
  auto tableData = readTableData(tableName);
  std::vector<std::vector<std::string>> rows;

  auto filterIndex = -1;
  if (filter.hasFilter){
    filterIndex = getColumnIndexs(tableData.header, { filter.column }).at(0);
  }

  auto indexs = getColumnIndexs(tableData.header, columns);

  for (int i = 1; i < tableData.rawRows.size(); i++){
    std::vector<std::string> row;
    auto columnContent = split(tableData.rawRows.at(i), ',');
    for (auto index : indexs){
      row.push_back(columnContent.at(index));
    }
    if (filter.hasFilter){
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

std::string findValue(std::string columnToFind, std::vector<std::string>& columns, std::vector<std::string>& values){
  for (int i = 0; i < columns.size(); i++){
    if (columnToFind == columns.at(i)){
      return values.at(i);
    }
  }
  return "";
}

void insert(std::string tableName, std::vector<std::string> columns, std::vector<std::string> values){
  auto header = readTableData(tableName).header;
  auto indexs = getColumnIndexs(header, columns);
  
  std::vector<std::string> valuesToInsert;
  for (int i = 0; i < header.size(); i++){
    valuesToInsert.push_back(findValue(header.at(i), columns, values));
  }
  appendFile(tablePath(tableName), join(valuesToInsert, ',') + "\n");
}

std::vector<std::vector<std::string>> executeSqlQuery(SqlQuery& query){
//SQL_SELECT, SQL_INSERT, SQL_UPDATE, SQL_DELETE, SQL_CREATE_TABLE, SQL_DELETE_TABLE
  if (query.type == SQL_SELECT){
    auto selectData = std::get_if<SqlSelect>(&query.queryData);
    return select(query.table, selectData -> columns, selectData -> filter);
  }else if (query.type == SQL_INSERT){
    auto insertData = std::get_if<SqlInsert>(&query.queryData);
    insert(query.table, insertData -> columns, insertData -> values);
    return {};
  }else if (query.type == SQL_UPDATE){
    assert(false);
  }else if (query.type == SQL_DELETE){
    assert(false);
  }else if (query.type == SQL_CREATE_TABLE){
    auto createData = std::get_if<SqlCreate>(&query.queryData);
    createTable(query.table, createData -> columns);
    return {};
  }else if (query.type == SQL_DELETE_TABLE){
    deleteTable(query.table);
    return {};
  }
  assert(false);
  return {};
}
