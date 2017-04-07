#define  _CRT_SECURE_NO_WARNINGS

#include <iostream>
using namespace std;

#include "dynamic_api_helper.hpp"

int main()
{
	char szPath[MAX_PATH] = { 0 };
	_API_(GetTempPathA)(MAX_PATH, szPath);
	printf("%s \n", szPath);
	system("pause");
	return 0;
}