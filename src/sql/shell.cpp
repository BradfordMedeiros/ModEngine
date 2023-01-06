#include "./shell.h"

using namespace sql;

int loopSqlShell(std::string dataDir){
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
        auto sqlQuery = compileSqlQuery(value, {});
        if(sqlQuery.validQuery){
          if (executeQuery){
            bool valid = false;
            std::string error;
            auto rows = executeSqlQuery(sqlQuery, dataDir, &valid, &error);
            for (auto row : rows){
              std::cout << join(row, ' ') << std::endl;
            }
            if (!valid){
              std::cout << "error executing query: " << error << std::endl;
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