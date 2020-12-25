#ifndef MOD_SQL
#define MOD_SQL

#include <string>
#include <vector>
#include <variant>
#include "./common/util.h"

enum SQL_QUERY_TYPE { SQL_SELECT, SQL_INSERT, SQL_UPDATE, SQL_DELETE, SQL_CREATE_TABLE, SQL_DELETE_TABLE };

struct SqlFilter {
  bool hasFilter;
  std::string column;
  std::string value;
  bool invert;
};

struct SqlSelect {
  std::vector<std::string> columns;
  SqlFilter filter;
};
struct SqlInsert {
  std::vector<std::string> columns;
  std::vector<std::string> values;
};
struct SqlCreate {
  std::vector<std::string> columns;
};
struct SqlUpdate {
  std::vector<std::string> columns;
  std::vector<std::string> values;
  SqlFilter filter;
};
struct SqlDelete {
  SqlFilter filter;
};

struct SqlQuery {
  SQL_QUERY_TYPE type;
  std::string table;
  std::variant<SqlSelect, SqlInsert, SqlCreate, SqlUpdate, SqlDelete> queryData;
};

std::vector<std::vector<std::string>> executeSqlQuery(SqlQuery& query);

#endif