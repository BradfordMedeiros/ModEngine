#ifndef MOD_SQL
#define MOD_SQL

#include <string>
#include <vector>
#include "./common/util.h"

enum SQL_QUERY_TYPE { SQL_SELECT, SQL_INSERT, SQL_UPDATE, SQL_DELETE, SQL_CREATE_TABLE, SQL_DELETE_TABLE };

struct SqlFilter {
  bool hasFilter;
  std::string column;
  std::string value;
  bool invert;
};

struct SqlQuery {
  SQL_QUERY_TYPE type;
  std::string table;
};

void executeSqlQuery(SqlQuery& query);
void testQuery();

#endif