#include "./colors.h"

void printRed(std::string str){
	std::cout << "\033[1;31m" << str << "\033[0m\n";
}

void printGreen(std::string str){
	std::cout << "\033[1;32m" << str << "\033[0m\n";
}