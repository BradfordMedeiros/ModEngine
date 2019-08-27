#ifndef MOD_GUILE
#define MOD_GUILE

#include <iostream>
#include <libguile.h>

void initGuile();
void startShellForNewThread();

void registerFunction(const char* name,  SCM (*callback)(SCM arg));
#endif
