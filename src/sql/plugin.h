#ifndef MOD_SQL
#define MOD_SQL

namespace sql {

typedef std::map<std::string, std::string> (*func_map)();
void registerGetArgs(func_map getArgsFn);
void registerGuileFns();
void registerGuileTypes();

};

#endif
