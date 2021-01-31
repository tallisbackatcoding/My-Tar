#ifndef INCLUDED
#define INCLUDED  1

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct
{
    int size;
    int capacity;
    char** array;
} string_array;

void free_all(string_array *s);

void new_string(string_array *s,char *a);

void add_string(string_array *s,char *a);

int my_strlen(char *s);

void my_strcpy(char *s1, char *s2);

void print_array(string_array *a);

string_array * new_string_array(int capacity);

void ensure_capacity(string_array * a);

void add_string(string_array *s,char *a);

void new_string(string_array *s,char *a);

void free_all(string_array *s);

int equal(char *s1,char *s2);

void my_concat(char *s1, char *s2);

//returns 0 if date1 is lexicographically less than date2
int isBigger(char *date1,char *date2);

int isLess(char *date1,char *date2);

int isSubstring(char *str, char *subStr);

void add_slash_and_filename_to_path(char *new_path,char *path, char *fileName);

int isSubsequence(char *t, char *s);

#endif
