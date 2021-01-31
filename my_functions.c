#include "my_functions.h"

int my_strlen(char *s)
{
    int i = 0;
    while(s[i]){
        i++;
    }
    return i;
}
void my_strcpy(char *s1, char *s2)
{
    int i;
    for(i = 0; s2[i]; i++){
        s1[i] = s2[i];
    }
    s1[i] = 0;
}
//function which prints elements of string_array
void print_array(string_array *a)
{
    for(int i = 0; i < a->size; i++)
    {
        printf("%s", a->array[i]);
        if(i != a->size - 1){
            printf("  ");
        }
    }
}
//function which creates new stuct string_array object with needed capacity
string_array * new_string_array(int capacity)
{
    string_array * str_array = malloc(sizeof(string_array));
    str_array->size = 0;
    str_array->capacity = capacity;
    str_array->array = malloc(sizeof(char*)*capacity);
    return str_array;
}
//this function keeps track of whether our string_array is full and needs a capacity increase
void ensure_capacity(string_array * a)
{
    if(a->size >= a->capacity)
    {
        string_array *new_arr = new_string_array(a->capacity*2);
        for(int i = 0; i < a->size; i++){
            add_string(new_arr,a->array[i]);
            free(a->array[i]);
        }
        free(a->array);
        a->array = malloc(sizeof(char*)*new_arr->capacity);
        a->size = 0;
        for(int i = 0; i < new_arr->size; i++){
            add_string(a,new_arr->array[i]);
        }
        a->capacity *= 2;
        free_all(new_arr);
    }
}
//function adds string 'a' to string_array 's'
void add_string(string_array *s,char *a)
{
    s->array[s->size] = malloc(sizeof(char)*(my_strlen(a)+1));
    my_strcpy(s->array[s->size],a);
    s->size++;
}
//this function calls add_string() and ensure_capacity()
void new_string(string_array *s,char *a)
{
    ensure_capacity(s);
    add_string(s,a);
}
//hand-made destructor
void free_all(string_array *s)
{
    for(int i = 0; i < s->size; i++){
        free(s->array[i]);
    }
    free(s->array);
    free(s);
}
//returns 1 if two strings are equal, 0 otherwise
int equal(char *s1,char *s2)
{
    int i = 0;
    for(; s1[i] != 0 && s2[i] != 0; i++){
        if(s1[i] != s2[i])
            return 0;
    }
    if(s1[i] != s2[i])
        return 0;
    return 1;
}
void my_concat(char *s1, char *s2)
{
    int i = 0;
    while(s1[i])
        i++;
    int j = 0;
    while(s2[j]){
        s1[i] = s2[j];
        i++;
        j++;
    }
    s1[i] = s2[j];
}

//this function saves date of a file up to nanoseconds inside a string and returns it



//returns 1 if date1 is lexicographically less than date2
int isBigger(char *date1,char *date2)
{
    int i = 0;
    for(; date1[i] && date2[i]; i++){
        if(date1[i] > date2[i]){
            return 0;
        }
        else if(date1[i] < date2[i]){
            return 1;
        }
    }
    if(date1[i] != 0 && date2[i] == 0){
        return 0;
    }
    else if(date1[i] == 0 && date2[i] != 0){
        return 1;
    }
    return 0;


}
int isLess(char *date1,char *date2){
    return !isBigger(date1,date2);
}
//yes
int isSubstring(char *str, char *subStr)
{
    int j = 0;
    for(int i = 0; str[i];){
        if(subStr[j] == 0){
            return 1;
        }
        if(str[i] == subStr[j]){
            j++;
            i++;
        }
        else{
            i = i - j + 1;
            j = 0;
        }
    }
    if(subStr[j] == 0){
        return 1;
    }
    return 0;
}

void add_slash_and_filename_to_path(char *new_path,char *path, char *fileName){
    my_strcpy(new_path,path);
    my_concat(new_path,"/");
    my_concat(new_path,fileName);
}

int isSubsequence(char *t, char *s) {
    int count = 0;
    for(int i = 0; t[i]; i++) {
        if(s[count] == t[i]) count++;
    }
    return count == my_strlen(s);
}
