//
// Created by aone on 2021/5/14.
//

#include <stdio.h>

/*!
 * 判断文件是否存在
 * Check if a file exist using fopen() function
 * return 1 if the file exist otherwise return 0
 */
int cfileexists(const char * filename){
    /* try to open file to read */
    FILE *file;
    if (file = fopen(filename, "r")){
        fclose(file);
        return 1;
    }
    return 0;
}

