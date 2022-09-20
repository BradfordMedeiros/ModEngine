#include "./sqlparse_test.h"

namespace sql {

void assertTokenization(std::string str, std::vector<char> delimitors, std::string expectedTokenStr){
  std::string tokString = tokenizeTypeStr(tokenize(str, delimitors));
  if (expectedTokenStr != tokString){
    throw std::logic_error("Incorrect tok string\ngot: " + tokString + " \nwanted: " + expectedTokenStr);
  }  
}
void tokenize1(){
  assertTokenization("hello-world-yo", {'-'}, "TOKEN(hello) DEL(-) TOKEN(world) DEL(-) TOKEN(yo)");
}
void tokenize2(){
  assertTokenization("hello+world-yo", {'+'}, "TOKEN(hello) DEL(+) TOKEN(world-yo)");
}
void tokenize3(){
  assertTokenization("hello+world-yo", {'+','-'}, "TOKEN(hello) DEL(+) TOKEN(world) DEL(-) TOKEN(yo)");
}
void tokenize4(){
  assertTokenization("", {'-'}, "");
}
void tokenize5(){
  assertTokenization("-", {'-'}, "DEL(-)");
}
void tokenize6(){
  assertTokenization("--", {'-'}, "DEL(-) DEL(-)");
}
void tokenize7(){
  assertTokenization("+-+", {'-'}, "TOKEN(+) DEL(-) TOKEN(+)");
}

void assertLex(std::string sqlQuery, std::string expectedLex){
  auto lexString = tokenTypeStr(lex(sqlQuery), true);
  if (lexString != expectedLex){
    throw std::logic_error("Incorrect lex string\ngot: " + lexString + " \nwanted: " + expectedLex);
  }  
}
void lexTestSelect1(){
  assertLex(
    "select name from users",
    "SELECT IDENTIFIER_TOKEN(name) FROM IDENTIFIER_TOKEN(users)"
  );
}
void lexTestSelect2(){
  assertLex(
    "    select  name  from users    ",
    "SELECT IDENTIFIER_TOKEN(name) FROM IDENTIFIER_TOKEN(users)"
  );
}
void lexTestSelect3(){
  assertLex(
    "select style from fashions",
    "SELECT IDENTIFIER_TOKEN(style) FROM IDENTIFIER_TOKEN(fashions)"
  );
}

void lexTestSelect4(){
  assertLex(
    "select count(style) from fashions group by name",
    "SELECT IDENTIFIER_TOKEN(count) LEFTP IDENTIFIER_TOKEN(style) RIGHTP FROM IDENTIFIER_TOKEN(fashions) GROUP BY IDENTIFIER_TOKEN(name)"
  );
}


void lexTestSelectSplice(){
  assertLex(
    "select name,age from users",
    "SELECT IDENTIFIER_TOKEN(name) SPLICE IDENTIFIER_TOKEN(age) FROM IDENTIFIER_TOKEN(users)"
  );
}

void lexTestSelectSpliceWeirdSpacing(){
  assertLex(
    "select  name ,  age from users create table drop table ",
    "SELECT IDENTIFIER_TOKEN(name) SPLICE IDENTIFIER_TOKEN(age) FROM IDENTIFIER_TOKEN(users) CREATE TABLE DROP TABLE"
  );
}

void lexTestInsert1(){
  assertLex(
    "insert into users (name, age) values ('bob', 20)",
    "INSERT INTO IDENTIFIER_TOKEN(users) LEFTP IDENTIFIER_TOKEN(name) SPLICE IDENTIFIER_TOKEN(age) RIGHTP VALUES LEFTP IDENTIFIER_TOKEN('bob') SPLICE IDENTIFIER_TOKEN(20) RIGHTP"
  );
}

void lexTestOperators(){
  assertLex(
    "some = operator > < !=",
    "IDENTIFIER_TOKEN(some) EQUAL IDENTIFIER_TOKEN(operator) OPERATOR(>) OPERATOR(<) OPERATOR(!=)"
  );
}

void assertComplete(std::string expression, bool expected){
  bool complete = parseTokens(lex(expression)).validQuery;
  if (complete != expected){
    throw std::logic_error(std::string("Incorrect completeness\ngot: ") + std::to_string(complete) + " \nwanted: " + std::to_string(expected));
  }
}

void testParserComplete(){
  assertComplete("create table sometable", true);
  assertComplete("create table anothertable", true);
  assertComplete("drop table anothertable", true);
}

void testParserIncomplete(){
  assertComplete("create", false);
  assertComplete("table anothertable", false);
  assertComplete("blob table anothertable", false);
}

void testCompileSqlCreateTable(){
  auto sqlQuery1 = compileSqlQuery("create table testtable");
  assert(sqlQuery1.validQuery);
  assert(sqlQuery1.type == SQL_CREATE_TABLE);
  assert(sqlQuery1.table == "testtable");

  auto sqlQuery2 = compileSqlQuery(" create    table another  ");
  assert(sqlQuery2.validQuery);
  assert(sqlQuery2.type == SQL_CREATE_TABLE);
  assert(sqlQuery2.table == "another");
}

void testCompileSqlDropTable(){
  auto sqlQuery1 = compileSqlQuery("drop table testtable");
  assert(sqlQuery1.validQuery);
  assert(sqlQuery1.type == SQL_DELETE_TABLE);
  assert(sqlQuery1.table == "testtable");

  auto sqlQuery2 = compileSqlQuery("drop table another");
  assert(sqlQuery2.validQuery);
  assert(sqlQuery2.type == SQL_DELETE_TABLE);
  assert(sqlQuery2.table == "another");
}

void testCompileSqlSelect(){
  auto sqlQuery1 = compileSqlQuery("select name, age from testtable");
  assert(sqlQuery1.validQuery);
  assert(sqlQuery1.type == SQL_SELECT);
  assert(sqlQuery1.table == "testtable");
  auto queryData = std::get_if<SqlSelect>(&(sqlQuery1.queryData));
  assert(queryData != NULL);
  assert(!queryData -> filter.hasFilter);
  assert(queryData -> columns.at(0) == "name");
  assert(queryData -> columns.at(1) == "age");
}

void testCompileSqlSelectJoin(){
  auto sqlQuery1 = compileSqlQuery("select name, anothertable.age from testtable left join anothertable on testtable.name = anothertable.name");
  assert(sqlQuery1.validQuery);
  assert(sqlQuery1.type == SQL_SELECT);
  assert(sqlQuery1.table == "testtable");
  auto queryData = std::get_if<SqlSelect>(&(sqlQuery1.queryData));
  assert(queryData != NULL);
  assert(queryData -> columns.at(0) == "name");
  assert(queryData -> columns.at(1) == "anothertable.age");
  assert(queryData -> join.hasJoin);
  assert(queryData -> join.table == "anothertable");
  assert(queryData -> join.col1 == "testtable.name");
  assert(queryData -> join.col2 == "anothertable.name");
  assert(queryData -> join.type == EQUAL);
}

void testCompileSqlUpdate(){
  auto sqlQuery1 = compileSqlQuery("update atable set name = nonamehere");
  assert(sqlQuery1.validQuery);
  assert(sqlQuery1.type == SQL_UPDATE);
  assert(sqlQuery1.table == "atable");
  auto queryData = std::get_if<SqlUpdate>(&(sqlQuery1.queryData));
  assert(queryData != NULL);
  assert(queryData -> columns.at(0) == "name");
}

void testCompileSqlUpdateWhere(){
  auto sqlQuery1 = compileSqlQuery("update atable set name = nonamehere where age = 50");
 // assert(sqlQuery1.validQuery);
 
  /*
  struct SqlUpdate {
  std::vector<std::string> columns;
  std::vector<std::string> values;
  SqlFilter filter;
};
struct SqlQuery {
  bool validQuery;
  SQL_QUERY_TYPE type;
  std::string table;
  std::variant<SqlSelect, SqlInsert, SqlCreate, SqlDropTable, SqlUpdate, SqlDelete, SqlShowTables, SqlDescribe> queryData;
};

enum OperatorType { GREATER_THAN, LESS_THAN, GREATER_THAN_OR_EQUAL, LESS_THAN_OR_EQUAL, EQUAL, NOT_EQUAL };
struct OperatorToken {
  OperatorType type;
};
struct SqlFilter {
  bool hasFilter;
  std::string column;
  std::string value;
  OperatorType type;
};


*/
  throw std::logic_error("where update not yet implemented");
}

void testCompileSqlOffset(){
  auto sqlQuery1 = compileSqlQuery("select name from testtable offset 15");
  assert(sqlQuery1.validQuery);
  assert(sqlQuery1.type == SQL_SELECT);
  assert(sqlQuery1.table == "testtable");
  auto queryData = std::get_if<SqlSelect>(&(sqlQuery1.queryData));
  assert(queryData != NULL);
  assert(queryData -> columns.at(0) == "name");
  assert(queryData -> offset == 15);
}

void testCompileSqlOffsetWithLimit(){
  auto sqlQuery1 = compileSqlQuery("select name from testtable limit 5 offset 5");
  assert(sqlQuery1.validQuery);
  assert(sqlQuery1.type == SQL_SELECT);
  assert(sqlQuery1.table == "testtable");
  auto queryData = std::get_if<SqlSelect>(&(sqlQuery1.queryData));
  assert(queryData != NULL);
  assert(queryData -> columns.at(0) == "name");
  assert(queryData -> limit == 5);
  assert(queryData -> offset == 5);
}

}