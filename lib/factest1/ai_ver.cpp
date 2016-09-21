/*************************************************************************
	> File Name: ai_ver.cpp
	> Author: 
	> Mail:  
	> Created Time: 2016年09月08日 星期四 09时58分30秒
 ************************************************************************/

#include <stdio.h>

#include <string>

using namespace std;

extern string nlu_get_version();

int get_nlu_version(char *buf, int len)
{
    int size;
    string version = nlu_get_version();
    if (version.empty()) return -1;

    size = version.size();
    strncpy(buf, version.c_str(), (len > size ? size : len));
    return 0;
}

