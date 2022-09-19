#ifndef MODPLUGIN_SQL
#define MODPLUGIN_SQL

#include <string.h>
#include <set>
#include "./sqlparse.h"
#include "./util.h"

namespace sql {
	std::vector<std::vector<std::string>> executeSqlQuery(SqlQuery& query, std::string dataDir, bool* valid, std::string* error);
}

#endif