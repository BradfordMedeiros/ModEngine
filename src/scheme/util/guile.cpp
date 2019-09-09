#include "./guile.h"

void* startGuile(void* data){
	return NULL;
}

SCM printTestDbl (void){
	std::cout << "test print yo" << std::endl;
	return SCM_UNSPECIFIED;
}
SCM getNumber(SCM arg1, SCM arg2){
	std::cout << "arg2 is: "  << scm_to_double(arg2) << std::endl;
	return scm_from_double(scm_to_double(arg1) + 10.0);
}

// want to do:
// call a function dynamically
// load some code. 
// figure out how to create separate execution context such that the scripts can have isolation

SCM testprint(){
	std::cout << "hello world" << std::endl;
	return SCM_UNSPECIFIED;
}

SCM callFunc(){
	return SCM_UNSPECIFIED;
}

void initGuile(){
  std::cout << "hello world from init guile" << std::endl;
  
  scm_with_guile(&startGuile, NULL);
  
  scm_c_primitive_load("./res/scripts/test.scm"); // throws exception if file doesn't exist

  scm_c_define("testint32", scm_from_int(10));
  scm_c_define("testdbl", scm_from_double(22.0));

  scm_c_define_gsubr("testprint", 0, 0, 0, (void *)&testprint);
  scm_c_define_gsubr("testnum", 2, 0, 0, (void*) getNumber);
  SCM func_symbol = scm_variable_ref(scm_c_lookup("print-test"));
  
  scm_call_0(func_symbol);
  scm_c_eval_string("(define evaleddata \"some evaled data\")");
  //scm_call (func_symbol, SCM_UNDEFINED);

  //scm_c_define_gsubr ("tortoise-reset", 0, 0, 0, &tortoise_reset);
}


void startShellForNewThread(){
  scm_with_guile(&startGuile, NULL);

  int argc = 0;
  char* argv[] = { { } };
  scm_shell(argc, argv);
}

void registerFunction(const char* name,  SCM (*callback)(SCM arg)){
  scm_c_define_gsubr(name, 1, 0, 0, (void *)callback);
}
