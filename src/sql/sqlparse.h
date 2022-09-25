#ifndef MODPLUGIN_SQLPARSE
#define MODPLUGIN_SQLPARSE

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <functional>
#include <variant>
#include <stdio.h>
#include <iostream>
#include "./util.h"

namespace sql {
struct SymbolToken{
  std::string name;
};
struct IdentifierToken{
  std::string content;
};
struct InvalidToken{};

enum OperatorType { GREATER_THAN, LESS_THAN, GREATER_THAN_OR_EQUAL, LESS_THAN_OR_EQUAL, EQUAL, NOT_EQUAL };
struct OperatorToken {
  OperatorType type;
};

enum TypeTokenType { TYPE_INT, TYPE_STRING };
struct TypeToken {
  TypeTokenType type;
};

typedef std::variant<SymbolToken, IdentifierToken, OperatorToken, TypeToken, InvalidToken> LexTokens;

std::string tokenTypeStr(std::vector<LexTokens> tokens, bool includeContent);
std::vector<LexTokens> lex(std::string value);

struct TokenResult {
  bool isDelimiter;
  char delimiter;
  std::string token;
};

std::string tokenizeTypeStr(std::vector<TokenResult> tokens);
std::vector<TokenResult> tokenize(std::string str, std::vector<char> delimiters);


enum SQL_QUERY_TYPE { SQL_SELECT, SQL_INSERT, SQL_UPDATE, SQL_DELETE, SQL_CREATE_TABLE, SQL_DELETE_TABLE, SQL_SHOW_TABLES, SQL_DESCRIBE };

struct SqlFilter {
  bool hasFilter;
  std::string column;
  std::string value;
  OperatorType type;
};
struct SqlOrderBy {
  std::vector<std::string> cols;
  std::vector<bool> isDesc;
};

struct SqlJoin {
  bool hasJoin;
  std::string table;
  std::string col1;
  std::string col2;
  OperatorType type;
};

struct SqlSelect {
  std::vector<std::string> columns;
  SqlJoin join;
  int limit;
  int offset;
  SqlFilter filter;
  SqlOrderBy orderBy;
  std::vector<std::string> groupby;
};
struct SqlInsert {
  std::vector<std::string> columns;
  std::vector<std::vector<std::string>> values;
  SqlFilter filter;
};

struct SqlCreate {
  std::vector<std::string> columns;
  std::vector<TypeTokenType> types;
};
struct SqlDropTable {};
struct SqlUpdate {
  std::vector<std::string> columns;
  std::vector<std::string> values;
  SqlFilter filter;
};
struct SqlDelete {
  SqlFilter filter;
};
struct SqlShowTables{};
struct SqlDescribe{};

struct SqlQuery {
  bool validQuery;
  SQL_QUERY_TYPE type;
  std::string table;
  std::variant<SqlSelect, SqlInsert, SqlCreate, SqlDropTable, SqlUpdate, SqlDelete, SqlShowTables, SqlDescribe> queryData;
};

SqlQuery parseTokens(std::vector<LexTokens> lexTokens);

SqlQuery compileSqlQuery(std::string queryString);

std::string drawDotGraph();

}

#endif