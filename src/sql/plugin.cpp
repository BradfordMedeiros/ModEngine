#include <iostream>
#include <libguile.h>
#include "./sql.h"
#include "./sqlparse.h"

using namespace sql;

std::string dataDir = "./res/state/";

SCM nestedVecToSCM(std::vector<std::vector<std::string>>& list){
  SCM scmList = scm_make_list(scm_from_unsigned_integer(list.size()), scm_from_unsigned_integer(0));
  for (int i = 0; i < list.size(); i++){
    auto sublist = list.at(i);
    SCM scmSubList = scm_make_list(scm_from_unsigned_integer(sublist.size()), scm_from_unsigned_integer(0));
    for (int j = 0; j < sublist.size(); j++){
      scm_list_set_x (scmSubList, scm_from_unsigned_integer(j), scm_from_locale_string(sublist.at(j).c_str()));
    }
    scm_list_set_x(scmList, scm_from_unsigned_integer(i), scmSubList);
  }
  return scmList;
}

struct sqlQueryHolder {
  SqlQuery* query;
};

SCM sqlObjectType;  

SqlQuery* queryFromForeign(SCM sqlQuery){
  sqlQueryHolder* query;
  scm_assert_foreign_object_type(sqlObjectType, sqlQuery);
  query = (sqlQueryHolder*)scm_foreign_object_ref(sqlQuery, 0); 
  return query -> query;
}

SCM scmSql(SCM sqlQuery){
  auto query = queryFromForeign(sqlQuery);
  auto sqlResult = executeSqlQuery(*query, dataDir);
  return nestedVecToSCM(sqlResult);
} 

SCM scmSqlCompile(SCM sqlQueryString){
  auto queryObj = (sqlQueryHolder*)scm_gc_malloc(sizeof(sqlQueryHolder), "sqlquery");
  SqlQuery* query = new SqlQuery;
  *query = compileSqlQuery(scm_to_locale_string(sqlQueryString));
  queryObj -> query = query;
  return scm_make_foreign_object_1(sqlObjectType, queryObj);
}

void finalizeSqlObjectType(SCM sqlQuery){
  auto query = queryFromForeign(sqlQuery);
  delete query;
}

void registerGuileFns() asm ("registerGuileFns");
void registerGuileFns() { 
 scm_c_define_gsubr("sql", 1, 0, 0, (void*)scmSql);
 scm_c_define_gsubr("sql-compile", 1, 0, 0, (void*)scmSqlCompile);
}

void registerGuileTypes() asm("registerGuileTypes");
void registerGuileTypes(){
  sqlObjectType = scm_make_foreign_object_type(scm_from_utf8_symbol("sqlquery"), scm_list_1(scm_from_utf8_symbol("data")), finalizeSqlObjectType);
}

typedef std::map<std::string, std::string> (*func_map)();
void registerGetArgs(func_map getArgsFn) asm("registerGetArgs");
void registerGetArgs(func_map getArgsFn){
  std::cout << "SQL_INFO: registered get args fn: " << std::endl;
  auto args = getArgsFn();
  if (args.find("sqldir") != args.end()){
    dataDir = args.at("sqldir");
  }
}


#ifdef BINARY_MODE

#include "./sqlparse_test.h"

typedef void (*func_t)();
struct TestCase {
  const char* name;
  func_t test;
};

int main(int argc, char *argv[]){
  bool shellMode = false;
  if (argc >= 2){
    if (strcmp(argv[1], "shell") == 0){
      shellMode = true;
    }else if (strcmp(argv[1], "script") == 0){
      assert(argc >= 3);
      auto sqlQuery = compileSqlQuery(argv[2]);
      auto rows = executeSqlQuery(sqlQuery, dataDir);
      for (auto row : rows){
        std::cout << join(row, ' ') << std::endl;
      }
      return 0;
    }else if (strcmp(argv[1], "file") == 0){
      assert(argc >= 3);
      auto lines = filterWhitespace(split(loadFile(argv[2]), ';'));
      std::vector<SqlQuery> queries;

      bool hasInvalidQuery = false;
      for (auto content : lines){
        auto sqlQuery = compileSqlQuery(content);
        if (!sqlQuery.validQuery){
          std::cout << "invalid syntax: " << content << std::endl;
          hasInvalidQuery = true;
        }
        queries.push_back(sqlQuery);
      }
      if (hasInvalidQuery){
        return -1;
      }
      for (auto query : queries){
        auto rows = executeSqlQuery(query, dataDir);
        for (auto row : rows){
          std::cout << join(row, ' ') << std::endl;
        }
      }
      return 0;
    }else if (strcmp(argv[1], "dotviz") == 0){
      std::cout << drawDotGraph() << std::endl;
      return 0;
    }
  }

  if (shellMode){
    bool lexMode = false;
    bool executeQuery = true;
    while(true){
      std::string value;
      std::getline(std::cin, value);
      if (value == "quit"){
        return 0;
      }else if (value == "lex"){
        std::cout << "lex mode enabled" << std::endl;
        lexMode = true;
        continue;
      }else if (value == "sql"){
        std::cout << "sql mode enabled" << std::endl;
        lexMode = false; 
        executeQuery = true;
        continue;
      }else if (value == "sql-check"){
        std::cout << "sql-check mode enabled" << std::endl;
        lexMode = false; 
        executeQuery = false;
        continue;      
      }

      if (!lexMode){
        auto sqlQuery = compileSqlQuery(value);
        if(sqlQuery.validQuery){
          if (executeQuery){
            auto rows = executeSqlQuery(sqlQuery, dataDir);
            for (auto row : rows){
              std::cout << join(row, ' ') << std::endl;
            }
          }else{
            std::cout << "Valid query" << std::endl;
          }
        }else{
          std::cout << "Invalid query: " << value << std::endl;
        }
      }else{
        auto lexString = tokenTypeStr(lex(value), true);
        std::cout << lexString << std::endl;
      }
    }
  }

  std::vector<TestCase> tests = { 
    TestCase {
      .name = "tokenize1",
      .test = tokenize1,
    },
    TestCase {
      .name = "tokenize2",
      .test = tokenize2,
    },
    TestCase {
      .name = "tokenize3",
      .test = tokenize3,
    },
    TestCase {
      .name = "tokenize4",
      .test = tokenize4,
    },
    TestCase {
      .name = "tokenize5",
      .test = tokenize5,
    },
    TestCase {
      .name = "tokenize6",
      .test = tokenize6,
    },
    TestCase {
      .name = "tokenize7",
      .test = tokenize7,
    },
    TestCase {
      .name = "lexTestSelect1",
      .test = lexTestSelect1,
    },
    TestCase {
      .name = "lexTestSelect2",
      .test = lexTestSelect2,
    },
    TestCase {
      .name = "lexTestSelect3",
      .test = lexTestSelect3,
    },
    TestCase {
      .name = "lexTestSelect4",
      .test = lexTestSelect4,
    },
    TestCase {
      .name = "lexTestSelectSplice",
      .test = lexTestSelectSplice,
    },
    TestCase {
      .name = "lexTestSelectSpliceWeirdSpacing",
      .test = lexTestSelectSpliceWeirdSpacing,
    },
    TestCase {
      .name = "lexTestInsert1",
      .test = lexTestInsert1,
    },
    TestCase {
      .name = "lexTestOperators",
      .test = lexTestOperators,
    },
    TestCase {
      .name = "testParserComplete",
      .test = testParserComplete,
    },
    TestCase {
      .name = "testParserIncomplete",
      .test = testParserIncomplete,
    },
    TestCase {
      .name = "testCompileSqlCreateTable",
      .test = testCompileSqlCreateTable,
    },
    TestCase {
      .name = "testCompileSqlDropTable",
      .test = testCompileSqlDropTable,
    },
    TestCase {
      .name = "testCompileSqlSelect",
      .test = testCompileSqlSelect,
    },
    TestCase {
      .name = "testCompileSqlSelectJoin",
      .test = testCompileSqlSelectJoin,
    },
    TestCase {
      .name = "testCompileSqlUpdate",
      .test = testCompileSqlUpdate,
    },
    TestCase {
      .name = "testCompileSqlOffset",
      .test = testCompileSqlOffset,
    },
    TestCase {
      .name = "testCompileSqlOffsetWithLimit",
      .test = testCompileSqlOffsetWithLimit,
    },
  };
  int totalTests = tests.size();
  int numFailures = 0;
  for (int i = 0; i < tests.size(); i++){
    auto test = tests.at(i);
    try {
      test.test();
      std::cout << i << " : " << test.name << " : pass" << std::endl;
    }catch(std::logic_error ex){
      std::cout << i << " : " << test.name << " : fail - " << ex.what() << std::endl;
      numFailures++;
    }catch(...){
      std::cout << i << " : " << test.name << " : fail -- error unknown" << std::endl;
      numFailures++;
    }
  }
  std::cout << "Tests passing: " << (totalTests - numFailures) << " / " << totalTests << std::endl;
}

#endif