#include <iostream>
#include <libguile.h>
#include "./sql.h"
#include "./sqlparse.h"

#ifdef BINARY_MODE

#include "./sqlparse_test.h"
#include "./shell.h"
using namespace sql;

std::string dataDir  = "./res/state/";

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
      bool valid = false;
      std::string error;
      auto rows = executeSqlQuery(sqlQuery, dataDir, &valid, &error);
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
        bool valid = false;
        std::string error;
        auto rows = executeSqlQuery(query, dataDir, &valid, &error);
        for (auto row : rows){
          std::cout << join(row, ' ') << std::endl;
        }
      }
      return 0;
    }else if (strcmp(argv[1], "dotviz") == 0){
      std::cout << drawDotGraph() << std::endl;
      return 0;
    }else if (strcmp(argv[1], "help") == 0){
      std::cout << "usage: " << argv[0] << " <command>, where command is shell, script, file, dotviz, or help" << std::endl;
      return 0;
    }
  }

  if (shellMode){
    return loopSqlShell(dataDir);
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
      .name = "testCompileSqlCreateTableWithTypes",
      .test = testCompileSqlCreateTableWithTypes,
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
      .name = "testCompileSqlUpdateWhere",
      .test = testCompileSqlUpdateWhere,
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