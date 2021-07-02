#include "./sql.h"

// @TODO handle escaping properly

std::string escapeCSVEntry(std::string data){
  // maybe just ban commas and newlines for now? 
  return data;
}

std::string tablePath(std::string tableName){
  return "./res/state/" + tableName + ".csv";  // TODO do paths better bro
}

std::string createHeader(std::vector<std::string> columns){
  return join(columns, ',') + "\n";
}

void createTable(std::string tableName, std::vector<std::string> columns){
  std::cout << "creating: " << tableName << "-- " << join(columns, ',') << std::endl;
  saveFile(tablePath(tableName), createHeader(columns));
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

std::string createRow(std::vector<std::string> values){
  return join(values, ',') + "\n";
}

void insert(std::string tableName, std::vector<std::string> columns, std::vector<std::string> values){
  auto header = readTableData(tableName).header;
  auto indexs = getColumnIndexs(header, columns);
  
  std::vector<std::string> valuesToInsert;
  for (int i = 0; i < header.size(); i++){
    valuesToInsert.push_back(findValue(header.at(i), columns, values));
  }
  appendFile(tablePath(tableName), createRow(valuesToInsert));
}

void update(std::string tableName, std::vector<std::string>& columns, std::vector<std::string>& values, SqlFilter& filter){
  assert(filter.hasFilter);
  auto tableData = readTableData(tableName);
  auto allRows = select(tableName, tableData.header, SqlFilter{ .hasFilter = false });

  std::string content = createHeader(tableData.header);
  for (auto row : allRows){
    if (filter.column == row.at(0) && filter.value == row.at(1)){ // this is wrong
      content = content + "this one should be updated\n";
    }else{
      content = content + createRow(row);
    }
  }
  saveFile(tablePath(tableName), content);
}

void deleteRows(std::string tableName, SqlFilter& filter){
  assert(filter.hasFilter);

  auto tableData = readTableData(tableName);
  auto copyFilter = filter;
  copyFilter.invert = !filter.invert;

  auto rowsToKeep = select(tableName, tableData.header, copyFilter);

  std::string content = createHeader(tableData.header);
  for (auto row : rowsToKeep){
    content = content + createRow(row);
  }
  saveFile(tablePath(tableName), content);
}

std::vector<std::vector<std::string>> executeSqlQuery(SqlQuery& query){
//SQL_SELECT, SQL_INSERT, SQL_UPDATE, SQL_DELETE, SQL_CREATE_TABLE, SQL_DELETE_TABLE
  /*if (query.type == SQL_SELECT){
    auto selectData = std::get_if<SqlSelect>(&query.queryData);
    return select(query.table, selectData -> columns, selectData -> filter);
  }else if (query.type == SQL_INSERT){
    auto insertData = std::get_if<SqlInsert>(&query.queryData);
    insert(query.table, insertData -> columns, insertData -> values);
    return {};
  }else if (query.type == SQL_UPDATE){
    auto updateData = std::get_if<SqlUpdate>(&query.queryData);
    update(query.table, updateData -> columns, updateData -> values, updateData -> filter);
    return {};
  }else if (query.type == SQL_DELETE){
    auto deleteData = std::get_if<SqlDelete>(&query.queryData);
    deleteRows(query.table, deleteData -> filter);
    return {};
  }else if (query.type == SQL_CREATE_TABLE){
    auto createData = std::get_if<SqlCreate>(&query.queryData);
    createTable(query.table, createData -> columns);
    return {};
  }else if (query.type == SQL_DELETE_TABLE){
    deleteTable(query.table);
    return {};
  }
  assert(false);*/
  return {{"one", "two", "three"}, {"hello", "wow", "go"}};
}

SqlQuery compileSqlQuery(std::string queryString){
  SqlQuery query {
    .type = SQL_SELECT,
    .table = "testtable",
    .queryData = SqlSelect{
      .columns = { "somecolumn1", "somecolumn2" },
      .filter = SqlFilter {
        .hasFilter = false,
        .column = "",
        .value = "",
        .invert = false,
      }
    }
  }; 
  return query;
}


/* std::string mainCommand = scm_to_locale_string(scm_list_ref(sqlQuery, scm_from_int64(0)));
  std::string table = scm_to_locale_string(scm_list_ref(sqlQuery, scm_from_int64(1)));

  SqlQuery query {
    .type = SQL_SELECT,
    .table = table,
  };

  if (mainCommand == "select"){
    std::cout << "setting query to select" << std::endl;
    query.type = SQL_SELECT;
    query.queryData = SqlSelect{
      .columns = listToVecString(scm_list_ref(sqlQuery, scm_from_int64(2))),
      .filter = SqlFilter {
        .hasFilter = false,
        .column = "",
        .value = "",
        .invert = false,
      }
    };
  }else if (mainCommand == "insert"){
    query.type = SQL_INSERT;
    query.queryData = SqlInsert {
      .columns = listToVecString(scm_list_ref(sqlQuery, scm_from_int64(2))),
      .values = listToVecString(scm_list_ref(sqlQuery, scm_from_int64(3))),
    };
  }else if (mainCommand == "update"){
    query.type = SQL_UPDATE;
    std::cout << "WARNING: GENERIC UPDATE, NOT USING ACTUAL VALUES" << std::endl;
    query.queryData = SqlUpdate {
      .columns = { "name" },
      .values = { "no-one" },
      .filter = SqlFilter {
        .hasFilter = true,
        .column = "name",
        .value = "unknown",
        .invert = false,
      }
    };
  }else if (mainCommand == "delete"){
    query.type = SQL_DELETE;
    std::cout << "WARNING: GENERIC DELETE, NOT USING ACTUAL VALUES" << std::endl;
    query.queryData = SqlDelete {
      .filter = SqlFilter {
        .hasFilter = true,
        .column = "description",
        .value = "hello",
        .invert = false,
      }
    };
  }else if (mainCommand == "create-table"){
    std::cout << "setting query to create" << std::endl;
    query.type = SQL_CREATE_TABLE;
    query.queryData = SqlCreate{
      .columns = listToVecString(scm_list_ref(sqlQuery, scm_from_int64(2))),
    };
  }else if (mainCommand == "delete-table"){
    query.type = SQL_DELETE_TABLE;
  }

  std::cout << "INFO: executing sql query" << std::endl;
  auto sqlResponse = executeSqlQuery(query);
  std::cout << "INFO: finished executing query" << std::endl;
  return listToSCM(sqlResponse);*/