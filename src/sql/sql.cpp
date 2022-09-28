#include "./sql.h"

namespace sql {

std::string escapeCSVEntry(std::string data){
  // maybe just ban commas and newlines for now? 
  return data;
}

std::string qualifyColumnName(std::string tablename, std::string columnname){
  auto parts = split(columnname, '.');
  auto isQualified = parts.size() == 2;
  assert(parts.size() == 1 || parts.size() == 2);
  if (isQualified){
    return columnname;
  }
  return tablename + "." + columnname;
}
std::string dequalifyColumnName(std::string columnname){
  auto parts = split(columnname, '.');
  auto isQualified = parts.size() == 2;
  assert(parts.size() == 1 || parts.size() == 2);
  if (isQualified){
    return parts.at(1);
  }
  return parts.at(0);
}
std::string tableFromQualifiedName(std::string columnname){
  auto parts = split(columnname, '.');
  auto isQualified = parts.size() == 2;
  assert(parts.size() == 1 || parts.size() == 2);
  if (!isQualified){
    return "";
  }  
  return parts.at(0);
}

std::string tablePath(std::string tableName, std::string basePath){
  return basePath + tableName + ".csv";  // TODO do paths better bro
}

struct HeaderData {
  std::vector<std::string> columns;
  std::vector<TypeTokenType> types;
};
struct TableData {
  HeaderData header;
  std::vector<std::vector<std::string>> rows;
};

std::string headerTypeStr(TypeTokenType type){
  if (type == TYPE_STRING){
    return "STRING";
  }
  if (type ==  TYPE_INT){
    return "INT";
  }
  assert(false);
  return "";
}
TypeTokenType strToHeaderType(std::string type){
  if (type == "STRING"){
    return TYPE_STRING;
  }
  if (type == "INT"){
    return TYPE_INT;
  }
  assert(false);
  return TYPE_STRING;
}

std::string createHeader(HeaderData data){
  assert(data.columns.size() == data.types.size());

  std::vector<std::string> nameWithType;
  for (int i = 0; i < data.columns.size(); i++){
    std::cout << "pushing back: " << data.columns.at(i) << std::endl;
    nameWithType.push_back(data.columns.at(i) + ":" + headerTypeStr(data.types.at(i)));
  }
  return join(nameWithType, ',') + "\n";
}

bool isValidColumnName(std::string columnname){
  return (columnname.find('.') == std::string::npos) && (columnname.find(',') == std::string::npos) && (columnname.find('\n') == std::string::npos) && (columnname.find(':') == std::string::npos);
}
void createTable(std::string tableName, std::vector<std::string> columns, std::vector<TypeTokenType> types, std::string basePath){
  auto filepath = tablePath(tableName, basePath);
  for (auto column : columns){
    assert(isValidColumnName(column));
  }
  auto headerStr = createHeader(HeaderData { .columns = columns, .types = types });
  std::cout << "header str: " << headerStr << ", size = " << headerStr.size() << std::endl;
  saveFile(filepath, headerStr);
}

void deleteTable(std::string tableName, std::string basePath){
  auto filepath = tablePath(tableName, basePath);
  std::remove(filepath.c_str());
}


TableData readTableData(std::string tableName, std::string basePath){
  auto tableContent = loadFile(tablePath(tableName, basePath));
  auto rawRows = filterWhitespace(split(tableContent, '\n'), 1);
  auto header = split(rawRows.at(0), ',');
  std::vector<std::string> qualifiedHeader;
  std::vector<TypeTokenType> types;
  for (auto col : header){
    //std::cout << "col " << col << std::endl;
    auto values = split(col, ':');
    assert(values.size() == 2);
    auto colname = values.at(0);
    auto columnType = strToHeaderType(values.at(1));
    qualifiedHeader.push_back(qualifyColumnName(tableName, colname ));
    types.push_back(columnType);
  }

  std::vector<std::vector<std::string>> rows;
  for (int i = 1; i < rawRows.size(); i++){
    auto columnContent = split(rawRows.at(i), ',');
    rows.push_back(columnContent);
    assert(columnContent.size() == header.size());
  }

  return TableData{
    .header = HeaderData {
      .columns = qualifiedHeader,
      .types = types,
    },
    .rows = rows,
  };
}

HeaderData readHeader(std::string tableName, std::string basePath){
  return readTableData(tableName, basePath).header;
}

std::vector<std::vector<std::string>> describeTable(std::string tableName, std::string basePath){
  std::vector<std::vector<std::string>> rows;
  auto headerInfo = readHeader(tableName, basePath);
  for (int i = 0; i < headerInfo.columns.size(); i++){
    auto headerColumn = headerInfo.columns.at(i);
    rows.push_back({ dequalifyColumnName(headerColumn), headerTypeStr(headerInfo.types.at(i)) });
  }
  return rows;
}
std::vector<std::vector<std::string>> showTables(std::string basePath){
  auto allFiles = listAllCsvFilesStems(basePath);
  std::vector<std::vector<std::string>> files;
  for (auto file : allFiles){
    files.push_back({ file });
  }
  return files;
}

void printColumns(std::vector<std::string> header){
  for (auto col : header){
    std::cout << col << " ";
  }
  std::cout << std::endl;
}

std::vector<int> getColumnIndexs(std::vector<std::string> header, std::vector<std::string> columns){
  std::vector<int> indexs;
  for (auto column : columns){
    bool foundCol = false;
    for (int i = 0; i < header.size(); i++){
      if (header.at(i) == column){
        indexs.push_back(i);
        foundCol = true;
      }
    }
    if (!foundCol){
      std::cout << "could not find col: " << column << std::endl;
      std::cout << "Header: ";
      printColumns(header);
      assert(foundCol);
    }
  }
  return indexs;
}

std::vector<int> getColumnsStarSelection(std::vector<std::string> header, std::vector<std::string> columns){
  std::vector<std::string> modifiedCols;
  for (auto column : columns){
    if (column == "*"){
      for (auto theColumn : header){
        bool columnAlreadyAdded = std::count(modifiedCols.begin(), modifiedCols.end(), theColumn) > 0;
        if (!columnAlreadyAdded){
          modifiedCols.push_back(theColumn);
        }
      }
      break;
    }else{
      modifiedCols.push_back(column);
    }
  }
  return getColumnIndexs(header, modifiedCols);
}

bool passesStringFilter(std::string& columnValue, int filterIndex, SqlFilter& filter){
  if (filter.type == EQUAL){
    return columnValue == filter.value;
  }
  if (filter.type == NOT_EQUAL){
    return columnValue != filter.value;
  }
  if (filter.type == GREATER_THAN){
    return columnValue > filter.value;
  }
  if (filter.type == GREATER_THAN_OR_EQUAL){
    return columnValue >= filter.value;
  }
  if (filter.type == LESS_THAN){
    return columnValue < filter.value;
  }
  if (filter.type == LESS_THAN_OR_EQUAL){
    return columnValue <= filter.value;
  }
  assert(false);
  return false;
}

bool maybeParseInt(std::string& value, int* parsedValue){
  char* notValid;
  int converted = static_cast<int>(strtol(value.c_str(), &notValid, 10));
  if (*notValid) {
    *parsedValue = 0;
    return false;
  }
  *parsedValue = converted;
  return true;
}

bool passesIntFilter(std::string& columnValue, int filterIndex, SqlFilter& filter){
  int value = 0;
  bool isInt = maybeParseInt(columnValue, &value);
  int filterValue = 0;
  bool filterIsInt = maybeParseInt(filter.value, &filterValue);
  if (!isInt || !filterIsInt){
     throw std::logic_error("invalid type for filter should be int");
  }
  if (filter.type == EQUAL){
    return value == filterValue;
  }
  if (filter.type == NOT_EQUAL){
    return value != filterValue;
  }
  if (filter.type == GREATER_THAN){
    return value > filterValue;
  }
  if (filter.type == GREATER_THAN_OR_EQUAL){
    return value >= filterValue;
  }
  if (filter.type == LESS_THAN){
    return value < filterValue;
  }
  if (filter.type == LESS_THAN_OR_EQUAL){
    return value <= filterValue;
  }
  assert(false); 
  return false;
}
bool passesFilter(HeaderData& header, std::string& columnValue, int filterIndex, SqlFilter& filter){
  auto columnType = header.types.at(filterIndex);
  if (columnType == TYPE_STRING){
    return passesStringFilter(columnValue, filterIndex, filter);
  }
  if (columnType == TYPE_INT){
    return passesIntFilter(columnValue, filterIndex, filter);
  }
  std::cout << "operator not supported" << std::endl;
  assert(false);
  return false;
}

struct GroupingKey {
  std::vector<std::string> values;
};

bool groupingKeysEqual(GroupingKey& key1, GroupingKey& key2){
  assert(key1.values.size() == key2.values.size());
  for (int i = 0; i < key1.values.size(); i++){
    if (key1.values.at(i) != key2.values.at(i)){
      return false;
    }
  }
  return true;
}
GroupingKey createGroupingKey(std::vector<std::string>& row, std::vector<int>& indexs){
  std::vector<std::string> values;
  for (auto index : indexs){
    values.push_back(row.at(index));
  }
  GroupingKey key { .values = values };
  return key;
}
std::string hashGroupingKey(GroupingKey& key){
  std::string hash = "";
  for (auto value : key.values){
    hash = hash + value + ",";
  }
  return hash;
}

std::vector<std::string> fullQualifiedNames(std::string tablename, std::vector<std::string>& columns){
  std::vector<std::string> names;
  for (auto col : columns){
    names.push_back(qualifyColumnName(tablename, col));
  }
  return names;
}

struct JoinColumns {
  std::string table1Col;
  std::string table2Col;
};

bool columnNameInHeader(std::vector<std::string> header, std::string qualifiedColname){
  for (auto col : header){
    if (qualifiedColname == col){
      return true;
    }
  }
  return false;
}

std::string qualifyNameOrLeaveQualified(std::string table, std::string col1){
  // if is qualified leave
  if (tableFromQualifiedName(col1) != ""){
    return col1;
  }
  return qualifyColumnName(table, col1);
}


bool assignQualifiedColIfBelongs(std::string table, TableData& data, std::string col, std::string* outValue){
  auto colTable = qualifyNameOrLeaveQualified(table, col);
  for (auto col : data.header.columns){
    if (col == colTable){
      *outValue = colTable;
      return true;
    }
  }
  return false;
}

JoinColumns figureOutJoinCols(std::string table1, TableData& data1, std::string table2, TableData& data2, std::string col1, std::string col2){
  std::string table1Col = "";
  std::string table2Col = "";

  auto col1IsTable1 = assignQualifiedColIfBelongs(table1, data1, col1, &table1Col);
  auto col1IsTable2 = assignQualifiedColIfBelongs(table2, data2, col1, &table2Col);

  auto col2IsTable1 = assignQualifiedColIfBelongs(table1, data1, col2, &table1Col);
  auto col2IsTable2 = assignQualifiedColIfBelongs(table2, data2, col2, &table2Col);

  assert(table1Col != "");
  assert(table2Col != "");

  return JoinColumns {
    .table1Col = table1Col,
    .table2Col = table2Col,
  };
}

TableData joinTableData(std::string table1, TableData& data1, std::string table2, TableData& data2, std::string col1, std::string col2, OperatorType op){
  auto joinCols = figureOutJoinCols(table1, data1, table2, data2, col1, col2);
  auto columnIndex1 = getColumnIndexs(data1.header.columns,  { joinCols.table1Col }).at(0);
  auto columnIndex2 = getColumnIndexs(data2.header.columns,  { joinCols.table2Col }).at(0);
  auto columnType1 = data1.header.types.at(columnIndex1);
  auto columnType2 = data2.header.types.at(columnIndex2);

  if (columnType1 != columnType2){
    throw std::logic_error(std::string("type mismatch on join: ") + joinCols.table1Col  + "[" + headerTypeStr(columnType1) + "] cannot be joined with " + joinCols.table2Col  + "[" + headerTypeStr(columnType2) + "]");
  }
  //std::cout << "join types: " << headerTypeStr(columnType1) << ", " << headerTypeStr(columnType2) << std::endl;
  std::vector<std::string> header;
  std::vector<TypeTokenType> types;
  for (auto col : fullQualifiedNames(table1, data1.header.columns)){
    header.push_back(col);
    auto colIndex = getColumnIndexs(data1.header.columns, { col }).at(0);
    types.push_back(data1.header.types.at(colIndex));
  }
  for (auto col : fullQualifiedNames(table2, data2.header.columns)){
    header.push_back(col);
    auto colIndex = getColumnIndexs(data2.header.columns, { col }).at(0);
    types.push_back(data2.header.types.at(colIndex));
  }

  std::vector<std::vector<std::string>> rows;
  for (int i = 0; i < data1.rows.size(); i++){
    bool hasMatch = false;
    auto row2Length = data2.rows.at(0).size();
    for (int j = 0; j < data2.rows.size(); j++){
      auto colOneValue = data1.rows.at(i).at(columnIndex1);
      auto colTwoValue = data2.rows.at(j).at(columnIndex2);
      auto matches =  colOneValue == colTwoValue;
      
      //std::cout << "matches? : " << matches << std::endl;
      //std::cout <<" Comparing: (" << col1 << " - " << col2 << ") -> " << colOneValue << " = " << colTwoValue << std::endl; 

      if (matches){
        hasMatch = true;
      }
      if (!matches){
        continue;
      }
      std::vector<std::string> row;
      for (auto colValue : data1.rows.at(i)){
        row.push_back(colValue);
      }
      for (auto colValue : data2.rows.at(j)){
        row.push_back(colValue);
      }
      rows.push_back(row);
    }
    if (!hasMatch){
      std::vector<std::string> row;
      for (auto colValue : data1.rows.at(i)){
        row.push_back(colValue);
      }
      for (int j = 0; j < row2Length; j++){
        row.push_back("NULL");
      }
      rows.push_back(row);
    }
  }

  HeaderData headerData {
    .columns = header,
    .types = types,
  };
  return TableData {
    .header = headerData,
    .rows = rows,
  };
}

std::vector<std::vector<std::string>> select(std::string tableName, std::vector<std::string> columns, SqlJoin join, SqlFilter filter, SqlOrderBy orderBy, std::vector<std::string> groupBy, int limit, int offset, std::string basePath){
  auto tableData = readTableData(tableName, basePath);

  if (join.hasJoin){
    if (tableName == join.table){
      std::cout << "cannot join a table on itself" << std::endl;
      assert(false);
    }
    TableData additionalTableData = readTableData(join.table, basePath);
    tableData = joinTableData(tableName, tableData, join.table, additionalTableData, join.col1, join.col2, join.type);
  }

  auto qualifiedColumns = fullQualifiedNames(tableName, columns);
  auto qualifiedOrderBy = fullQualifiedNames(tableName, orderBy.cols);
  auto qualifiedGroupBy = fullQualifiedNames(tableName, groupBy);

  std::vector<std::string> qualifiedFilter;
  if (filter.hasFilter){
    std::vector<std::string> filterColumns = { filter.column };
    auto qualedNames = fullQualifiedNames(tableName, filterColumns); 
    qualifiedFilter = qualedNames;
  }


  auto orderIndexs = getColumnIndexs(tableData.header.columns, qualifiedOrderBy);
  std::sort (tableData.rows.begin(), tableData.rows.end(), [&orderIndexs, &orderBy](std::vector<std::string>& row1, std::vector<std::string>& row2) -> bool {
    for (int i = 0; i < orderIndexs.size(); i++){
      auto index = orderIndexs.at(i);
      auto value = strcmp(row1.at(index).c_str(), row2.at(index).c_str()); // this is wrong because row is already the new one 
      
      auto isDesc = orderBy.isDesc.at(i);
      if (value > 0){
        return isDesc ? true : false;
      }else if (value < 0){
        return isDesc ? false : true;
      }
    }
    return false;
  });

  std::vector<std::vector<std::string>> finalRows;
  auto filterIndex = -1;
  if (filter.hasFilter){
    filterIndex = getColumnIndexs(tableData.header.columns, qualifiedFilter).at(0);
  }

  auto indexs = getColumnsStarSelection(tableData.header.columns, qualifiedColumns);
  auto groupingIndexs = getColumnIndexs(tableData.header.columns, qualifiedGroupBy);
  std::set<std::string> groupingKeysHash;

  for (int i = offset; i < tableData.rows.size(); i++){
    auto row = tableData.rows.at(i);
    if (filter.hasFilter){
      auto columnValue = row.at(filterIndex);
      auto passFilter = passesFilter(tableData.header, columnValue, filterIndex, filter);
      if (!passFilter){
        continue;
      }
    }
    if (limit >= 0 && finalRows.size() >= limit){
      break;
    }

    std::vector<std::string> organizedRow;

    if (groupBy.size() > 0){
      auto groupingKey = createGroupingKey(row, groupingIndexs);
      auto groupKeyHash = hashGroupingKey(groupingKey);
      auto alreadyHasKey = groupingKeysHash.find(groupKeyHash) != groupingKeysHash.end();
      if (!alreadyHasKey){
        groupingKeysHash.insert(groupKeyHash);
        for (auto index : indexs){
          organizedRow.push_back(row.at(index));
        }
        finalRows.push_back(organizedRow);
      }
    }else{
      for (auto index : indexs){
        organizedRow.push_back(row.at(index));
      }   
      finalRows.push_back(organizedRow);
    }
  }

  return finalRows;
}


std::optional<std::string> findValue(std::string columnToFind, std::vector<std::string>& columns, std::vector<std::string>& values){
  for (int i = 0; i < columns.size(); i++){
    if (columnToFind == columns.at(i)){
      return values.at(i);
    }
  }
  return std::nullopt;
}

std::string createRow(std::vector<std::string> values){
  return join(values, ',') + "\n";
}

bool checkValidType(std::optional<std::string>& value, TypeTokenType type){
  if (type == TYPE_STRING){
    return true;
  }
  if (type == TYPE_INT){
    if (!value.has_value()){
      return true;
    }

    int _;
    bool validInt = maybeParseInt(value.value(), &_);
    return validInt;
  }
  return false;
}

std::string serializeValueType(std::optional<std::string> value, TypeTokenType type){
  bool isValidType = checkValidType(value, type);
  if (!isValidType){
    throw std::logic_error(std::string("Invalid type for value: ") + (value.has_value() ? value.value() : "NULL") + ", expected type = " + headerTypeStr(type));
  }
  auto headerType = headerTypeStr(type);
  std::cout << "serializing value: " << value.value() << ", type is = " << headerType << std::endl;

  return value.has_value() ? value.value() : "NULL";
}

void insert(std::string tableName, std::vector<std::string> columns, std::vector<std::vector<std::string>> values, std::string basePath){
  auto header = readHeader(tableName, basePath);
  auto qualifiedColumns = fullQualifiedNames(tableName, columns);
  auto indexs = getColumnIndexs(header.columns, qualifiedColumns);

  std::string newContent = "";
  for (auto value : values){
    std::vector<std::string> valuesToInsert;
    for (int i = 0; i < header.columns.size(); i++){
      assert(columns.size() == value.size());
      auto columnName = header.columns.at(i);
      auto columnType = header.types.at(i);
      auto columnValue = findValue(header.columns.at(i), qualifiedColumns, value);
      valuesToInsert.push_back(serializeValueType(columnValue, columnType));
    }
    newContent = newContent + createRow(valuesToInsert);
  }
  appendFile(tablePath(tableName, basePath), newContent);
}

std::string print(std::vector<std::string>& values){
  std::string valueStr = "";
  for (auto &value : values){
    valueStr += value + " ";
  }
  return valueStr;
}


void update(std::string tableName, std::vector<std::string>& columns, std::vector<std::string>& values, SqlFilter& filter, std::string basePath){
  auto header = readHeader(tableName, basePath);
  auto allRows = select(tableName, header.columns, {}, SqlFilter{ .hasFilter = false }, SqlOrderBy{}, {}, -1, 0, basePath);
  std::string content = createHeader(header);
  auto columnIndexesToUpdate = getColumnIndexs(header.columns, fullQualifiedNames(tableName, columns));

  for (auto row : allRows){
    bool applyUpdate = true;
    if (filter.hasFilter){
      std::vector<std::string> cols = { filter.column };
      auto fullColnames = getColumnIndexs(header.columns, fullQualifiedNames(tableName, cols));
      auto column = row.at(fullColnames.at(0)); // only checking agianst the first element?
      auto passFilter = passesFilter(header, column, fullColnames.at(0), filter);
      if (!passFilter){
        applyUpdate = false;
      }
    }

    if (applyUpdate){
      for (int i = 0; i < columnIndexesToUpdate.size(); i++){
        auto columnType = header.types.at(columnIndexesToUpdate.at(i));
        auto columnValue = values.at(i);
        row.at(columnIndexesToUpdate.at(i)) = serializeValueType(columnValue, columnType);
      }
  
      content = content + createRow(row);
    }else{
      content = content + createRow(row);
    }
    
  }
  saveFile(tablePath(tableName, basePath), content);
}

void deleteRows(std::string tableName, SqlFilter& filter, std::string basePath){
  auto header = readHeader(tableName, basePath);
  auto rowsToKeep = select(tableName, header.columns, {}, SqlFilter{}, SqlOrderBy{}, {}, -1, 0, basePath);
  std::string content = createHeader(header);
  for (auto row : rowsToKeep){
    if (filter.hasFilter){
      std::vector<std::string> filterColumns = { filter.column };
      auto filterIndex = getColumnIndexs(header.columns, fullQualifiedNames(tableName, filterColumns)).at(0);
      auto column = row.at(filterIndex);
      auto passFilter = passesFilter(header, column, filterIndex, filter);
      if (passFilter){
        continue;
      }
    }
    content = content + createRow(row);
  }
  saveFile(tablePath(tableName, basePath), content);
}

std::vector<std::vector<std::string>> executeSqlQuery(SqlQuery& query, std::string dataDir, bool* valid, std::string* error){
  assert(query.validQuery);
  *valid = true;
  try {
    if (query.type == SQL_SELECT){
      auto selectData = std::get_if<SqlSelect>(&query.queryData);
      assert(selectData != NULL);
      return select(query.table, selectData -> columns, selectData -> join, selectData -> filter, selectData -> orderBy, selectData -> groupby, selectData -> limit, selectData -> offset, dataDir);
    }else if (query.type == SQL_INSERT){
      auto insertData = std::get_if<SqlInsert>(&query.queryData);
      assert(insertData != NULL);
      insert(query.table, insertData -> columns, insertData -> values, dataDir);
      return {};
    }else if (query.type == SQL_UPDATE){
      auto updateData = std::get_if<SqlUpdate>(&query.queryData);
      assert(updateData != NULL);
      update(query.table, updateData -> columns, updateData -> values, updateData -> filter, dataDir);
      return {};
    }else if (query.type == SQL_DELETE){
      auto deleteData = std::get_if<SqlDelete>(&query.queryData);
      assert(deleteData != NULL);
      deleteRows(query.table, deleteData -> filter, dataDir);
      return {};
    }else if (query.type == SQL_CREATE_TABLE){
      auto createData = std::get_if<SqlCreate>(&query.queryData);
      assert(createData != NULL);
      createTable(query.table, createData -> columns, createData -> types, dataDir);
      return {};
    }else if (query.type == SQL_DELETE_TABLE){
      deleteTable(query.table, dataDir);
      return {};
    }else if (query.type == SQL_DESCRIBE){
      return describeTable(query.table, dataDir);
    }else if (query.type == SQL_SHOW_TABLES){
      return showTables(dataDir);
    }
    assert(false);
    return {};
  }catch (const std::exception & ex){
    *valid = false;
    *error = ex.what();
    return {};
  }catch (...){
    *valid = false;
    *error = "unknown sql error";
    return {};
  }
}

}