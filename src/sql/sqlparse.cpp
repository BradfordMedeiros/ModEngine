#include "./sqlparse.h"

namespace sql {
std::string toUpper(std::string s){
  transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return toupper(c); });
  return s;
}

std::string tokenTypeStr(LexTokens token, bool includeContent){
  auto symbolToken = std::get_if<SymbolToken>(&token);
  if (symbolToken != NULL){
    return symbolToken -> name; 
  }
  auto operatorToken = std::get_if<OperatorToken>(&token);
  if (operatorToken != NULL){
    std::string result = "OPERATOR";
    if (includeContent){
      std::string op = "";
      if (operatorToken -> type == GREATER_THAN){
        op = ">";
      }else if (operatorToken -> type == LESS_THAN){
        op = "<";
      }else if (operatorToken -> type == EQUAL){
        op = "=";
      }else if (operatorToken -> type == NOT_EQUAL){
        op = "!=";
      }else if (operatorToken -> type == GREATER_THAN_OR_EQUAL){
        op = ">=";
      }else if (operatorToken -> type == LESS_THAN_OR_EQUAL){
        op = "<=";
      }
      result = result + "(" + op + ")";
    }
    return result; 
  }

  auto identifierToken = std::get_if<IdentifierToken>(&token);
  if (identifierToken != NULL){
    std::string result =  "IDENTIFIER_TOKEN";
    if (includeContent){
      result = result + "(" + identifierToken -> content + ")";
    }
    return result;
  }

  auto invalidToken = std::get_if<InvalidToken>(&token);
  if (invalidToken != NULL){
    return "INVALID_TOKEN";
  }
  assert(false);
  return "";
}
std::string tokenTypeStr(std::vector<LexTokens> tokens, bool includeContent){
  std::string content = "";
  for (int i = 0; i < tokens.size(); i++){
    auto token = tokens.at(i);
    content = content + tokenTypeStr(token, includeContent) + (i < (tokens.size() - 1) ? " " : "");
  }
  return content;
}

bool isIdentifier(std::string token){
  return (token.find(',') == std::string::npos) && (token.find('\n') == std::string::npos);
}

std::string tokenTypeStr(TokenResult token){
  if (token.isDelimiter){
    return std::string("DEL(") + token.delimiter + ")";
  }
  return std::string("TOKEN(" + token.token + ")");
}
std::string tokenizeTypeStr(std::vector<TokenResult> tokens){
  std::string content = "";
  for (int i = 0; i < tokens.size(); i++){
    auto token = tokens.at(i);
    content = content + tokenTypeStr(token) + (i < (tokens.size() - 1) ? " " : "");
  }
  return content;
}

std::vector<TokenResult> tokenize(std::string str, std::vector<char> delimiters){
  std::vector<TokenResult> result;

  int lowIndex = 0;
  for (int i = 0; i < str.size(); i++){
    auto currentChar = str.at(i);
    bool isDelimiter = std::count(delimiters.begin(), delimiters.end(), currentChar) > 0;
    if (isDelimiter){
      auto token = str.substr(lowIndex, (i - lowIndex));
      lowIndex = i + 1;
      if (token.size() > 0){
        result.push_back(TokenResult{
          .isDelimiter = false,
          .delimiter = ' ',
          .token = token,
        });
      }
      result.push_back(TokenResult{
        .isDelimiter = true,
        .delimiter = currentChar,
        .token = "",
      });
    }
  }
  if (lowIndex < str.size()){
    auto token = str.substr(lowIndex, str.size() - lowIndex);
    result.push_back(TokenResult{
      .isDelimiter = false,
      .delimiter = ' ',
      .token = token,
    });
  }
  return result;
}


std::vector<const char*> validSymbols = {
  "SELECT", "FROM", "CREATE", "DROP", "TABLE", "SHOW", "TABLES", "VALUES", 
  "INSERT", "INTO", "DESCRIBE", "GROUP", "BY", "LIMIT", "WHERE", "UPDATE", "SET",
  "ORDER", "ASC", "DESC", "DELETE", "LEFT", "JOIN", "ON", "OFFSET",
}; 

std::vector<LexTokens> lex(std::string value){
  std::vector<LexTokens> lexTokens;
  std::vector<TokenResult> filteredTokens;
  for (auto token : tokenize(value, {' ', ',', '(', ')', '=', '"', '<', '>', '!', '\n', '\r' })){
    if (token.isDelimiter && token.delimiter == ' '){
      continue;
    }
    filteredTokens.push_back(token);
  }

  for (int i = 0; i < filteredTokens.size(); i++){
    auto token = filteredTokens.at(i);
    if (token.isDelimiter){
      if (token.delimiter == ','){
        lexTokens.push_back(SymbolToken { .name = "SPLICE" });
      }else if (token.delimiter == '('){
        lexTokens.push_back(SymbolToken { .name = "LEFTP" });
      }else if (token.delimiter == ')'){
        lexTokens.push_back(SymbolToken { .name = "RIGHTP" });
      }else if (token.delimiter == '='){
        lexTokens.push_back(SymbolToken { . name = "EQUAL" });
      }else if (token.delimiter == '!'){
        auto hasNextToken = (i + 1) < filteredTokens.size();
        auto nextTokenEqual = hasNextToken && (filteredTokens.at(i + 1).isDelimiter && filteredTokens.at(i + 1).delimiter == '=');
        if (nextTokenEqual){
          lexTokens.push_back(OperatorToken { .type = NOT_EQUAL });
          i = i + 1;
        }else{
          lexTokens.push_back(InvalidToken{});
        }
      }else if (token.delimiter == '>'){
        auto hasNextToken = (i + 1) < filteredTokens.size();
        auto nextTokenEqual = hasNextToken && (filteredTokens.at(i + 1).isDelimiter && filteredTokens.at(i + 1).delimiter == '=');
        if (nextTokenEqual){
          lexTokens.push_back(OperatorToken { .type = GREATER_THAN_OR_EQUAL });
          i = i + 1; // skip the next token
        }else{
          lexTokens.push_back(OperatorToken { .type = GREATER_THAN });
        }
      }else if (token.delimiter == '<'){
        auto hasNextToken = (i + 1) < filteredTokens.size();
        auto nextTokenEqual = hasNextToken && (filteredTokens.at(i + 1).isDelimiter && filteredTokens.at(i + 1).delimiter == '=');
        if (nextTokenEqual){
          lexTokens.push_back(OperatorToken { .type = LESS_THAN_OR_EQUAL });
          i = i + 1; // skip the next token
        }else{
          lexTokens.push_back(OperatorToken { .type = LESS_THAN });
        }
      }else if (token.delimiter == '\"'){
        lexTokens.push_back(SymbolToken { .name = "QUOTE" });
      }else if (token.delimiter == '\n' || token.delimiter == '\r'){
        // do nothing, this just gets ignored
      }else{
        std::cout << "delimiter: " << token.delimiter << std::endl;
        assert(false);
      }
    }else{
      bool isSymbol = false;
      for (auto validSymbol : validSymbols){
        if (toUpper(token.token) == validSymbol){
          lexTokens.push_back(SymbolToken { .name = validSymbol });
          isSymbol = true;
          break;
        }
      }
      if (!isSymbol){
        if (isIdentifier(token.token)){
          lexTokens.push_back(IdentifierToken{
            .content = token.token,
          });
        }else{
          assert(false);
        }
      }
    }
  }
  return lexTokens;
}

struct NextState {
  std::string token;
  std::string stateSuffix;
};
struct TokenState {
  std::vector<NextState> nextStates;
  std::function<void(SqlQuery&, LexTokens*)> fn;
};

std::string generateWhere(std::string suffix){
  std::string content = "";
  content = content + 
    "WHERE:" + suffix + " IDENTIFIER_TOKEN " + suffix +"\n" + 
    "IDENTIFIER_TOKEN:" + suffix + " EQUAL " + suffix + "\n" + 
    "IDENTIFIER_TOKEN:" + suffix + " OPERATOR " + suffix + "\n" + 
    "EQUAL:" + suffix + " IDENTIFIER_TOKEN " + suffix +"2\n" + 
    "OPERATOR:" + suffix + " IDENTIFIER_TOKEN " + suffix + "2\n" +
    "IDENTIFIER_TOKEN:" + suffix + "2 *END*\n";
  return content;
}

auto machineTransitions = ""
"start CREATE\n"
"start DROP\n"
"start SELECT\n"
"start SHOW\n"
"start DESCRIBE\n"
"start INSERT\n"
"start UPDATE\n"
"start DELETE\n"

"CREATE TABLE\n"
"TABLE IDENTIFIER_TOKEN table\n"
"IDENTIFIER_TOKEN:table *END*\n"
"IDENTIFIER_TOKEN:table LEFTP table\n"
"LEFTP:table IDENTIFIER_TOKEN create_tablecol\n"
"IDENTIFIER_TOKEN:create_tablecol SPLICE create_tablecol\n"
"SPLICE:create_tablecol IDENTIFIER_TOKEN create_tablecol\n"
"IDENTIFIER_TOKEN:create_tablecol RIGHTP create_tablecol\n"
"RIGHTP:create_tablecol *END*\n"

"DROP TABLE droptable\n"
"TABLE:droptable IDENTIFIER_TOKEN droptable\n"
"IDENTIFIER_TOKEN:droptable *END*\n"

"SHOW TABLES\n"
"TABLES *END*\n"

"SELECT IDENTIFIER_TOKEN select\n"
"IDENTIFIER_TOKEN:select SPLICE\n"
"IDENTIFIER_TOKEN:select FROM\n"
"SPLICE IDENTIFIER_TOKEN select\n"
"FROM IDENTIFIER_TOKEN tableselect\n"
"FROM *SUBQUERY* tableselect\n"
"IDENTIFIER_TOKEN:tableselect LEFT tableselect\n"
"LEFT:tableselect JOIN tableselect\n"
"JOIN:tableselect IDENTIFIER_TOKEN tablejoin\n"
"IDENTIFIER_TOKEN:tablejoin ON tablejoin\n"
"ON:tablejoin IDENTIFIER_TOKEN tablejoinc\n"
"IDENTIFIER_TOKEN:tablejoinc EQUAL tablejoinc\n"
"EQUAL:tablejoinc IDENTIFIER_TOKEN tablejoinv\n"
"IDENTIFIER_TOKEN:tablejoinv *END*\n"
"IDENTIFIER_TOKEN:tableselect LIMIT tableselect\n"
"IDENTIFIER_TOKEN:tableselect WHERE whereselect\n"
"IDENTIFIER_TOKEN:tableselect ORDER\n"
"ORDER BY tableorderby\n"
"BY:tableorderby IDENTIFIER_TOKEN orderby\n"
"IDENTIFIER_TOKEN:orderby *END*\n"

"IDENTIFIER_TOKEN:orderby LIMIT tableselect\n"
"IDENTIFIER_TOKEN:orderby SPLICE orderbycomma\n"
"IDENTIFIER_TOKEN:orderby ASC orderby\n"
"IDENTIFIER_TOKEN:orderby DESC orderby\n"

"ASC:orderby SPLICE orderbycomma\n"
"ASC:orderby LIMIT tableselect\n"
"ASC:orderby *END*\n"
"DESC:orderby SPLICE orderbycomma\n"
"DESC:orderby LIMIT tableselect\n"
"DESC:orderby *END*\n"

"SPLICE:orderbycomma IDENTIFIER_TOKEN orderby\n"
"IDENTIFIER_TOKEN:tableselect *END*\n"
"IDENTIFIER_TOKEN:tableselect GROUP whereselect2\n" + 
generateWhere("whereselect") + 
"IDENTIFIER_TOKEN:tableselect OFFSET tableselect_offset\n"

"IDENTIFIER_TOKEN:whereselect2 GROUP whereselect2\n"
"GROUP:whereselect2 BY whereselect2\n"
"BY:whereselect2 IDENTIFIER_TOKEN groupbyselect\n"
"IDENTIFIER_TOKEN:groupbyselect *END*\n"
"IDENTIFIER_TOKEN:groupbyselect SPLICE groupbyselect\n"
"SPLICE:groupbyselect IDENTIFIER_TOKEN groupbyselect\n"
"IDENTIFIER_TOKEN:groupbyselect LIMIT tableselect\n"
"IDENTIFIER_TOKEN:whereselect2 LIMIT tableselect\n"
"LIMIT:tableselect IDENTIFIER_TOKEN limit_tableselect\n"
"IDENTIFIER_TOKEN:limit_tableselect *END*\n"
"IDENTIFIER_TOKEN:limit_tableselect OFFSET tableselect_offset\n"
"OFFSET:tableselect_offset IDENTIFIER_TOKEN tableselect_offset\n"
"IDENTIFIER_TOKEN:tableselect_offset *END*\n"

"INSERT INTO\n"
"INTO IDENTIFIER_TOKEN tableinsert\n"
"IDENTIFIER_TOKEN:tableinsert LEFTP tableinsert\n"
"LEFTP:tableinsert IDENTIFIER_TOKEN tableinsertcolname\n"
"IDENTIFIER_TOKEN:tableinsertcolname SPLICE tableinsertcolname\n"
"SPLICE:tableinsertcolname IDENTIFIER_TOKEN tableinsertcolname\n"
"IDENTIFIER_TOKEN:tableinsertcolname RIGHTP tableinsertcolname\n"
"RIGHTP:tableinsertcolname VALUES\n"
"VALUES LEFTP tableinsert_v\n"
"LEFTP:tableinsert_v IDENTIFIER_TOKEN tableinsert_v\n"
"IDENTIFIER_TOKEN:tableinsert_v RIGHTP tableinsert_v\n"
"IDENTIFIER_TOKEN:tableinsert_v SPLICE tableinsert_v\n"
"SPLICE:tableinsert_v IDENTIFIER_TOKEN tableinsert_v\n"
"RIGHTP:tableinsert_v *END*\n"
"RIGHTP:tableinsert_v SPLICE tableinsert_new\n"
"SPLICE:tableinsert_new LEFTP tableinsert_v\n"


"DESCRIBE IDENTIFIER_TOKEN describe\n"
"IDENTIFIER_TOKEN:describe *END*\n"

"UPDATE IDENTIFIER_TOKEN tableupdate\n"
"IDENTIFIER_TOKEN:tableupdate SET\n"
"SET IDENTIFIER_TOKEN tableupdate_col\n"
"IDENTIFIER_TOKEN:tableupdate_col EQUAL tableupdate_val\n"
"EQUAL:tableupdate_val IDENTIFIER_TOKEN tableupdate_val\n"
"IDENTIFIER_TOKEN:tableupdate_val *END*\n"
"IDENTIFIER_TOKEN:tableupdate_val SPLICE tableupdate_val\n"
"SPLICE:tableupdate_val IDENTIFIER_TOKEN tableupdate_col\n"
"IDENTIFIER_TOKEN:tableupdate_val WHERE tableupdate\n" + 
generateWhere("tableupdate_where") +

"DELETE FROM delete_vals\n"
"FROM:delete_vals IDENTIFIER_TOKEN delete_vals\n"
"IDENTIFIER_TOKEN:delete_vals WHERE delete_where_val\n" + 
"IDENTIFIER_TOKEN:delete_vals *END*\n" + 
generateWhere("delete_where_val");

void setTableName(SqlQuery& query, LexTokens* token){
  auto identifier = std::get_if<IdentifierToken>(token);
  assert(identifier != NULL);
  query.table = identifier -> content;
}

std::map<std::string, std::function<void(SqlQuery&, LexTokens* token)>> machineFns {
  {"CREATE", [](SqlQuery& query, LexTokens* token) -> void {
      query.type = SQL_CREATE_TABLE;
      query.queryData = SqlCreate{};
  }},
  {"DROP", [](SqlQuery& query, LexTokens* token) -> void {
      query.type = SQL_DELETE_TABLE;
      query.queryData = SqlDropTable{};
  }},
  {"SHOW", [](SqlQuery& query, LexTokens* token) -> void {
      query.type = SQL_SHOW_TABLES;
      query.queryData = SqlShowTables{};
  }},
  {"IDENTIFIER_TOKEN:table", setTableName},
  {"IDENTIFIER_TOKEN:create_tablecol", [](SqlQuery& query, LexTokens* token) -> void {
      auto identifier = std::get_if<IdentifierToken>(token);
      assert(identifier != NULL);
      auto createQuery = std::get_if<SqlCreate>(&query.queryData);
      assert(createQuery != NULL);
      createQuery -> columns.push_back(identifier -> content);
  }},
  {"IDENTIFIER_TOKEN:droptable", setTableName},
  {"SELECT", [](SqlQuery& query, LexTokens* token) -> void {
      query.type = SQL_SELECT;
      query.queryData = SqlSelect{
        .limit = -1,
        .offset = 0,
      };
  }},
  {"IDENTIFIER_TOKEN:select", [](SqlQuery& query, LexTokens* token) -> void {
      auto identifierToken = std::get_if<IdentifierToken>(token);
      assert(identifierToken != NULL);
      SqlSelect* selectQuery = std::get_if<SqlSelect>(&query.queryData);
      assert(selectQuery != NULL);
      selectQuery -> columns.push_back(identifierToken -> content);
  }},
  {"IDENTIFIER_TOKEN:tableselect", setTableName},
  {"IDENTIFIER_TOKEN:limit_tableselect", [](SqlQuery& query, LexTokens* token) -> void {
      auto identifierToken = std::get_if<IdentifierToken>(token);
      assert(identifierToken != NULL);
      SqlSelect* selectQuery = std::get_if<SqlSelect>(&query.queryData);
      assert(selectQuery != NULL);
      selectQuery -> limit = std::atoi(identifierToken -> content.c_str()); // strong typing should occur earlier
  }},
  {"IDENTIFIER_TOKEN:tableselect_offset", [](SqlQuery& query, LexTokens* token) -> void {
      auto identifierToken = std::get_if<IdentifierToken>(token);
      assert(identifierToken != NULL);
      SqlSelect* selectQuery = std::get_if<SqlSelect>(&query.queryData);
      assert(selectQuery != NULL);
      selectQuery -> offset = std::atoi(identifierToken -> content.c_str());
  }},
  {"IDENTIFIER_TOKEN:groupbyselect", [](SqlQuery& query, LexTokens* token) -> void {
    auto identifierToken = std::get_if<IdentifierToken>(token);
    assert(identifierToken != NULL);
    SqlSelect* selectQuery = std::get_if<SqlSelect>(&query.queryData);
    assert(selectQuery != NULL);
    selectQuery -> groupby.push_back(identifierToken -> content);
  }},
  {"IDENTIFIER_TOKEN:orderby", [](SqlQuery& query, LexTokens* token) -> void {
    auto identifierToken = std::get_if<IdentifierToken>(token);
    assert(identifierToken != NULL);
    SqlSelect* selectQuery = std::get_if<SqlSelect>(&query.queryData);
    assert(selectQuery != NULL);
    selectQuery -> orderBy.cols.push_back(identifierToken -> content);
    selectQuery -> orderBy.isDesc.push_back(false);
  }},
  {"DESC:orderby", [](SqlQuery& query, LexTokens* token) -> void {
    SqlSelect* selectQuery = std::get_if<SqlSelect>(&query.queryData);
    assert(selectQuery != NULL);
    auto lastIndex = selectQuery -> orderBy.isDesc.size() - 1;
    selectQuery -> orderBy.isDesc.at(lastIndex) = true;
  }},
  {"IDENTIFIER_TOKEN:tablejoin", [](SqlQuery& query, LexTokens* token) -> void {
    auto identifierToken = std::get_if<IdentifierToken>(token);
    assert(identifierToken != NULL);
    SqlSelect* selectQuery = std::get_if<SqlSelect>(&query.queryData);
    assert(selectQuery != NULL);
    selectQuery -> join = SqlJoin {
      .hasJoin = true,
      .table = identifierToken -> content,
    };
  }},
  {"IDENTIFIER_TOKEN:tablejoinc", [](SqlQuery& query, LexTokens* token) -> void {
    auto identifierToken = std::get_if<IdentifierToken>(token);
    assert(identifierToken != NULL);
    SqlSelect* selectQuery = std::get_if<SqlSelect>(&query.queryData);
    assert(selectQuery != NULL);
    selectQuery -> join.col1 = identifierToken -> content;
  }},
  {"EQUAL:tablejoinc", [](SqlQuery& query, LexTokens* token) -> void {
    SqlSelect* selectQuery = std::get_if<SqlSelect>(&query.queryData);
    assert(selectQuery != NULL);
    selectQuery -> join.type = EQUAL;
  }},
  {"IDENTIFIER_TOKEN:tablejoinv", [](SqlQuery& query, LexTokens* token) -> void {
    auto identifierToken = std::get_if<IdentifierToken>(token);
    assert(identifierToken != NULL);
    SqlSelect* selectQuery = std::get_if<SqlSelect>(&query.queryData);
    assert(selectQuery != NULL);
    selectQuery -> join.col2 = identifierToken -> content;
  }},
  {"DESCRIBE", [](SqlQuery& query, LexTokens* token) -> void {
      query.type = SQL_DESCRIBE;
      query.queryData = SqlDescribe{};
  }},
  {"IDENTIFIER_TOKEN:describe", setTableName},
  {"INSERT", [](SqlQuery& query, LexTokens* token) -> void {
      query.type = SQL_INSERT;
      query.queryData = SqlInsert{};
  }},

  {"IDENTIFIER_TOKEN:tableinsert", setTableName},
  {"IDENTIFIER_TOKEN:tableinsertcolname", [](SqlQuery& query, LexTokens* token) -> void {
      auto insertQuery = std::get_if<SqlInsert>(&query.queryData);
      assert(insertQuery != NULL);
      auto identifierToken = std::get_if<IdentifierToken>(token);
      assert(identifierToken != NULL);
      insertQuery -> columns.push_back(identifierToken -> content);   
  }},
  {"IDENTIFIER_TOKEN:tableinsert_v", [](SqlQuery& query, LexTokens* token) -> void {
      auto insertQuery = std::get_if<SqlInsert>(&query.queryData);
      assert(insertQuery != NULL);
      auto identifierToken = std::get_if<IdentifierToken>(token);
      assert(identifierToken != NULL);
      bool hasSpace = true;
      auto hasValue = insertQuery -> values.size() > 0;
      if (!hasValue){
        hasSpace = false;
      }else{
        auto lastValue = insertQuery -> values.at(insertQuery -> values.size() - 1);
        hasSpace = lastValue.size() <= insertQuery -> columns.size() - 1;       
      }
      if (!hasSpace){
        std::vector<std::string> value;
        insertQuery -> values.push_back(value);
      }
      insertQuery -> values.at(insertQuery -> values.size() - 1).push_back(identifierToken -> content);
  }},

  {"IDENTIFIER_TOKEN:tableupdate", [](SqlQuery& query, LexTokens* token) -> void {
    auto identifierToken = std::get_if<IdentifierToken>(token);
    assert(identifierToken != NULL);
    query.table = identifierToken -> content;
    query.type = SQL_UPDATE;
    query.queryData = SqlUpdate{};
  }},
  {"IDENTIFIER_TOKEN:tableupdate_col", [](SqlQuery& query, LexTokens* token) -> void {
    auto updateQuery = std::get_if<SqlUpdate>(&query.queryData);
    assert(updateQuery != NULL);
    auto identifierToken = std::get_if<IdentifierToken>(token);
    assert(identifierToken != NULL);
    updateQuery -> columns.push_back(identifierToken -> content);   
  }},
  {"IDENTIFIER_TOKEN:tableupdate_val", [](SqlQuery& query, LexTokens* token) -> void {
    auto updateQuery = std::get_if<SqlUpdate>(&query.queryData);
    assert(updateQuery != NULL);
    auto identifierToken = std::get_if<IdentifierToken>(token);
    assert(identifierToken != NULL);
    updateQuery -> values.push_back(identifierToken -> content);   
  }},
  {"IDENTIFIER_TOKEN:tableupdatef_col", [](SqlQuery& query, LexTokens* token) -> void {
    auto updateQuery = std::get_if<SqlUpdate>(&query.queryData);
    assert(updateQuery != NULL);
    auto identifierToken = std::get_if<IdentifierToken>(token);
    assert(identifierToken != NULL);
    updateQuery -> values.push_back(identifierToken -> content);   
    updateQuery -> filter.hasFilter = true;
    updateQuery -> filter.column = identifierToken -> content;
  }},
  {"IDENTIFIER_TOKEN:tableupdatef_val", [](SqlQuery& query, LexTokens* token) -> void {
    auto updateQuery = std::get_if<SqlUpdate>(&query.queryData);
    assert(updateQuery != NULL);
    auto identifierToken = std::get_if<IdentifierToken>(token);
    assert(identifierToken != NULL);
    updateQuery -> values.push_back(identifierToken -> content);   
    updateQuery -> filter.hasFilter = true;
    updateQuery -> filter.value = identifierToken -> content;
  }},
  {"IDENTIFIER_TOKEN:delete_vals", [](SqlQuery& query, LexTokens* token) -> void {
    auto identifierToken = std::get_if<IdentifierToken>(token);
    assert(identifierToken != NULL);
    query.table = identifierToken -> content;
    query.type = SQL_DELETE;
    query.queryData = SqlDelete{};
  }},
};

template <typename T>
int addWhereStateFns(std::string suffix, std::map<std::string, std::function<void(SqlQuery&, LexTokens* token)>>& machineFns){
  machineFns[std::string("EQUAL:") + suffix] =  [](SqlQuery& query, LexTokens* token) -> void {
      T* queryWithFilter = std::get_if<T>(&query.queryData);
      assert(queryWithFilter != NULL);
      queryWithFilter -> filter.hasFilter = true;
      queryWithFilter -> filter.type = EQUAL;
  };
  machineFns[std::string("OPERATOR:") + suffix] = [](SqlQuery& query, LexTokens* token) -> void {
      auto operatorToken = std::get_if<OperatorToken>(token);
      assert(operatorToken != NULL);
      T* queryWithFilter = std::get_if<T>(&query.queryData);
      assert(queryWithFilter != NULL);
      queryWithFilter -> filter.hasFilter = true;
      queryWithFilter -> filter.type = operatorToken -> type;
  };
  machineFns[std::string("IDENTIFIER_TOKEN:") + suffix] = [](SqlQuery& query, LexTokens* token) -> void {
      auto identifierToken = std::get_if<IdentifierToken>(token);
      assert(identifierToken != NULL);
      T* queryWithFilter = std::get_if<T>(&query.queryData);
      assert(queryWithFilter != NULL);
      queryWithFilter -> filter.column = identifierToken -> content;
  };
  machineFns[std::string("IDENTIFIER_TOKEN:") + suffix + "2"] = [](SqlQuery& query, LexTokens* token) -> void {
      auto identifierToken = std::get_if<IdentifierToken>(token);
      assert(identifierToken != NULL);
      T* queryWithFilter = std::get_if<T>(&query.queryData);
      assert(queryWithFilter != NULL);
      queryWithFilter -> filter.hasFilter = true;
      queryWithFilter -> filter.value = identifierToken -> content;
  };
  return 0;
}

auto _1 = addWhereStateFns<SqlDelete>("delete_where_val", machineFns);
auto _2 = addWhereStateFns<SqlSelect>("whereselect", machineFns);

std::map<std::string, TokenState> createMachine(std::string transitionsStr, std::map<std::string, std::function<void(SqlQuery&, LexTokens* token)>>& fns){
  std::map<std::string, TokenState> machine;
  auto transitions = filterWhitespace(split(transitionsStr, '\n'));
  for (auto transition : transitions){
    auto allTransitions = split(transition, ' ');
    assert(allTransitions.size() == 2 || allTransitions.size() == 3);
    auto machineName = allTransitions.at(0);
    if (machine.find(machineName) == machine.end()){
      machine[machineName] = TokenState{ 
        .nextStates = {},
        .fn = fns.find(machineName) == fns.end() ? [](SqlQuery& query, LexTokens* token) -> void {} : fns.at(machineName),
      };
    }
    machine.at(machineName).nextStates.push_back(NextState{
      .token = allTransitions.at(1),
      .stateSuffix = allTransitions.size() > 2 ? allTransitions.at(2) : "",
    });
  }
  return machine;
}

std::map<std::string, TokenState> machine = createMachine(machineTransitions, machineFns);


SqlQuery parseTokens(std::vector<LexTokens> lexTokens){
  //std::cout << "machine: " << std::endl << machineTransitions << std::endl;
  SqlQuery query {
    .validQuery = false,
    .type = SQL_SELECT,
    .table = "",
  };
  std::string currState = "start";
  std::string stateSuffix = "";

  LexTokens* lastToken = NULL;
  for (auto &lexToken : lexTokens){
    if (machine.find(currState) == machine.end()){
      break;
    }
    auto nextStates = machine.at(currState).nextStates;
    machine.at(currState).fn(query, lastToken);
    lastToken = &lexToken;

    // maybe we detect if the next expression is a subquery, if so, we run that expression
    // and then feed it into the machine, advance by x tokens

    auto tokenAsStr = tokenTypeStr(lexToken, false);
    bool nextStateValid = false;
    for (auto state : nextStates){
      if (state.token == tokenAsStr){
        nextStateValid = true;
        stateSuffix = state.stateSuffix;
        break;
      }
    }
    if (!nextStateValid){
      return query;
    }
    currState = tokenAsStr + (stateSuffix.size() == 0 ? "" : (":" + stateSuffix));
  }

  if (machine.find(currState) != machine.end()){
    machine.at(currState).fn(query, lastToken);
    auto finalNextStates = machine.at(currState).nextStates;
    auto completeExpression = false;
    for (auto state : finalNextStates){
      if (state.token == "*END*"){
        completeExpression = true;
        break;
      }
    }
    query.validQuery = completeExpression;
  }
  return query;
}

SqlQuery compileSqlQuery(std::string queryString){
  return parseTokens(lex(queryString));
}

std::string drawDotGraph(){
  std::string prefix = "\n strict digraph { \n";
  std::string content = "";
  std::string suffix = "\n}\n";

  for (auto &[machineName, tokenState] : machine){
    for (auto nextState : tokenState.nextStates){
      auto rootContent = "\"" + machineName + "\" -> \"" + nextState.token;
      auto suffixContent = nextState.stateSuffix == "" ? "" : ( ":" + nextState.stateSuffix);
      content = content + rootContent + suffixContent + "\"\n" ;
    }
  }
  return prefix + content + suffix;
}

}