#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "my_functions.h"

char *str_base_unsigned(size_t n, int base){
    if(n == 0){
        char *r = malloc(2*sizeof(char));
        r[0] = '0';
        r[1] = 0;
        return r;
    }
    char hexaDeciNum[100];
    int i = 0;
    while(n!=0){
        int temp  = 0;
        temp = n % base;
        if(temp < 10){
            hexaDeciNum[i] = temp + 48;
        }
        else{
            hexaDeciNum[i] = temp + 87;
        }
        n = n/base;
        i++;
    }
    char * result = malloc((i+1)*sizeof(char));
    int k = 0;
    for(int j=i-1; j>=0; j--,k++){
        result[k] = hexaDeciNum[j];
    }
    result[k] = 0;
    return result;
}

int power(int base, int degree)
{
    int result = 1;
    for(int i = 0; i < degree; i++)
    {
        result *= base;
    }
    return result;
}
long long convertOctalToDecimal(long long octalNumber)
{
    long long decimalNumber = 0, i = 0;

    while(octalNumber != 0)
    {
        decimalNumber += (octalNumber%10) * power(8,i);
        ++i;
        octalNumber/=10;
    }

    i = 1;

    return decimalNumber;
}

typedef struct
{
    char name[100];
    char mode[8];
    char owner[8];
    char group[8];
    char size[12];
    char modified[12];
    char checksum[8];
    char type[1];
    char link[100];
} tar_header;

void expand(int fd, int size)
{
    char null = 0;
    for(int i = 0; i < size; i++)
        write(fd, &null, 1);
}

int copy_from(int fd_to, int fd_from)
{
    int input_length = 0, file_size = 0;
    while(true)
    {
        char buffer[2];
        input_length = read(fd_from,buffer,1);
        if(!input_length)
            break;
        write(fd_to, buffer, 1);
        file_size++;
    }
    return file_size;
}

char get_type(struct stat file_info)
{
    char file_type = 0;
    switch (file_info.st_mode & S_IFMT)
    {
    case S_IFBLK:
        file_type = '4';
        break;
    case S_IFCHR:
        file_type = '3';
        break;
    case S_IFDIR:
        file_type = '5';
        break;
    case S_IFIFO:
        file_type = '6';
        break;
    case S_IFLNK:
        file_type = '2';
        break;
    case S_IFREG:
        file_type = '0';
        break;
    default:
        file_type = 0;
        break;
    }
    return file_type;
}

void write_header(int fd_tar, struct stat file_info, char *file_name)
{
    char file_type = get_type(file_info);
    char str[100];
    my_strcpy(str, file_name);
    if(file_type == '5')
    {
        my_concat(str,"/");
    }
    int str_size = my_strlen(str);
    write(fd_tar, str, str_size);

    expand(fd_tar,100 - str_size);

    tar_header header;
    memset( &header, 0, sizeof( tar_header ) );
    strcpy(header.name, str);

    sprintf(str, "%07o",file_info.st_mode);
    int file_mode = 0;
    for(int i = 4; i < 7; i++)
    {
        file_mode = file_mode*10 + str[i] - 48;
    }
    //mode
    sprintf(str, "%07d", file_mode);
    strcpy(header.mode,str);
    write(fd_tar, str, 8);
    //user id
    sprintf(str, "%07o", file_info.st_uid);
    strcpy(header.owner,str);
    write(fd_tar, str, 8);
    //owner id
    sprintf(str, "%07o", file_info.st_gid);
    strcpy(header.group,str);
    write(fd_tar, str, 8);
    //size
    sprintf(str, "%011o", file_info.st_size);
    strcpy(header.size,str);
    write(fd_tar, str, 12);

    //last modified
    sprintf(str, "%011o", file_info.st_mtim.tv_sec);
    strcpy(header.modified,str);
    write(fd_tar, str, 12);
    //type
    sprintf(str, "%c", file_type);
    strcpy(header.type,str);

    if(file_type == '2')
    {
        char *buf = malloc(100*sizeof(char));
        readlink(file_name, buf, 100);
        strcpy(header.link,buf);
        free(buf);
    }

    size_t checksum = 0;
    int i;

    const unsigned char* bytes = &header;
    for( i = 0; i < sizeof( tar_header ); ++i )
    {
        checksum += bytes[i];
    }
    checksum += 256;

    //checksum
    sprintf(str, "%07o", checksum );
    write(fd_tar, str, 8);
    //type
    sprintf(str, "%c", file_type);
    write(fd_tar,str, 1);

    //expand(fd_tar,1);
}

void add_file(char *file_name, int fd_tar)
{
    int fd_input = open(file_name, O_RDONLY);
    int file_size = 0;
    struct stat file_info;

    if (lstat(file_name, &file_info) != 0)
    {
        printf("No such file\n");
        return;
    }

    write_header(fd_tar, file_info, file_name);
    if(get_type(file_info) == '2')
    {
        char *buf = malloc(100*sizeof(char));
        readlink(file_name, buf, 100);
        int len;
        if ((len = readlink(file_name, buf, 100)) != -1)
        {
            write(fd_tar, buf, my_strlen(buf));
            expand(fd_tar, 100 - my_strlen(buf));
        }
        else
            printf("%s doesn't exist or not a link\n", file_name);
        free(buf);
    }
    else
    {
        expand(fd_tar,100);
    }
    expand(fd_tar,255);
    if(get_type(file_info) == '5')
    {
        DIR *folder;
        struct dirent *dir;
        folder = opendir(file_name);
        if(folder)
        {
            while ((dir = readdir(folder)) != NULL)
            {
                if(dir->d_name[0] != '.')
                {
                    char new_path[256];
                    add_slash_and_filename_to_path(new_path,file_name, dir->d_name);
                    add_file(new_path, fd_tar);
                }
            }
            closedir(folder);
        }
    }
    else if(get_type(file_info) == '0')
    {
        file_size = copy_from(fd_tar, fd_input);
        if(file_size!=0){
            expand(fd_tar,512 - file_size % 512);
        }
    }
    close(fd_input);
}

char* get_string_range(int fd, int begin, int end)
{
    char *result = malloc(512*sizeof(char));
    lseek(fd, begin, SEEK_SET);
    int i = 0;
    int null_check = read(fd, result, end - begin);
    if(!null_check)
    {
        return NULL;
    }
    return result;
}

long long my_atoi(char *str)
{
    long long result = 0;
    for(int i = 0; str[i]; i++)
    {
        result = result*10 + str[i] - 48;
    }
    return result;
}

void assign_dates(string_array *file_names, string_array* file_dates)
{
    for(int i = 0; i < file_names->size; i++)
    {
        struct utimbuf ut;
        time(&ut.actime);
        ut.modtime = convertOctalToDecimal(my_atoi(file_dates->array[i]));
        utime(file_names->array[i],&ut);
    }
}
void extract_files(int fd_tar)
{
    int i = 0;
    lseek(fd_tar, 0, SEEK_SET);
    char buffer;
    int file_end = 0;

    string_array *file_names = new_string_array(4);
    string_array *file_dates = new_string_array(4);

    while(true)
    {

        char *check = get_string_range(fd_tar, i, i+2);

        if(check == NULL)
        {
            break;
        }
        free(check);
        int fd = -1;
        char *file_name = get_string_range(fd_tar, i,       i + 100);
        char *file_type = get_string_range(fd_tar, i + 156, i + 157);
        char *file_mode = get_string_range(fd_tar, i + 100, i + 108);
        char *file_mtime = get_string_range(fd_tar, i + 136, i + 148);

        new_string(file_names, file_name);
        new_string(file_dates, file_mtime);

        int skip = 0;
        if(file_type[0] == '5')
        {
            mkdir(file_name, my_atoi(file_mode));
            skip = 1;
        }
        else if(file_type[0] == '0')
        {
            fd = open(file_name, O_CREAT | O_TRUNC | O_WRONLY);
        }
        else if(file_type[0] == '2')
        {
            char *file_link = get_string_range(fd_tar, i + 157, i + 257);
            symlink(file_link, file_name);
            free(file_link);
            skip = 1;
        }
        free(file_name);
        free(file_type);
        free(file_mode);
        free(file_mtime);


        i += 512;

        if(skip)
            continue;

        if(fd != -1)
        {
            int count = 0;
            lseek(fd_tar, i, SEEK_SET);
            while(1)
            {
                int length = read(fd_tar, &buffer, 1);

                if(buffer == 0)
                    break;

                write(fd, &buffer, 1);
                i++;
            }
            close(fd);
            i = i + 512 - i % 512;

        }
    }
    assign_dates(file_names, file_dates);
    free_all(file_names);
    free_all(file_dates);

}
void print_tar(int fd_tar)
{

    int i = 0;
    lseek(fd_tar, 0, SEEK_SET);
    char buffer;
    while(true)
    {
        int skip = 0;
        char *check = get_string_range(fd_tar, i, i+2);
        if(check == NULL)
            break;
        free(check);

        char *file_name = get_string_range(fd_tar, i,       i + 100);
        char *file_type = get_string_range(fd_tar, i + 156, i + 157);

        if(file_type[0] == '5')
            skip = 1;

        puts(file_name);

        free(file_name);
        free(file_type);
        i += 512;
        if(skip)
            continue;

        lseek(fd_tar, i, SEEK_SET);
        while(true)
        {
            int length = read(fd_tar, &buffer, 1);
            if(buffer == 0)
            {
                break;
            }
            i++;
        }
        i = i + 512 - i % 512;

    }
}
void add_file_if_newer(char *new_file_name, int fd_tar)
{

    int fd_new = open(new_file_name, O_RDONLY);

    struct stat file_info;
    if (lstat(new_file_name, &file_info) != 0)
    {
        printf("No such file\n");
        return;
    }
    close(fd_new);
    char new_file_date[12];
    sprintf(new_file_date, "%011o", file_info.st_mtim.tv_sec);

    int file_should_be_added = 1;
    int i = 0;
    lseek(fd_tar, 0, SEEK_SET);
    char buffer;
    while(true)
    {
        int skip = 0;
        char *check = get_string_range(fd_tar, i, i+2);
        if(check == NULL)
            break;
        free(check);

        char *file_name = get_string_range(fd_tar, i,       i + 100);
        char *file_type = get_string_range(fd_tar, i + 156, i + 157);
        char *file_date = get_string_range(fd_tar, i + 136, i + 148);

        if(file_type[0] == '5')
            skip = 1;

        if(equal(file_name, new_file_name) && !isBigger(file_date, new_file_date))
        {
            file_should_be_added = 0;
        }

        free(file_name);
        free(file_type);
        free(file_date);

        i += 512;

        if(skip)
            continue;

        lseek(fd_tar, i, SEEK_SET);
        while(1)
        {
            int length = read(fd_tar, &buffer, 1);
            if(buffer == 0)
            {
                break;
            }
            i++;
        }
        i = i + 512 - i % 512;

    }
    if(file_should_be_added)
    {
        lseek(fd_tar, 0, SEEK_END);
        add_file(new_file_name, fd_tar);
    }
}
int main( int argc, char* argv[] )
{
    string_array *files = new_string_array(4);
    int create_flag = 0, f_flag = 0, print_flag = 0,
        append_flag = 0, extract_flag = 0, if_newer_flag = 0;
    int i;
    for(i = 1; i < argc && argv[i][0] == '-'; i++)
    {
        if(isSubsequence(argv[i],"-t"))
        {
            print_flag = 1;
        }
        if(isSubsequence(argv[i],"-c"))
        {
            create_flag = 1;
        }
        if(isSubsequence(argv[i],"-f"))
        {
            f_flag = 1;
        }
        if(isSubsequence(argv[i],"-r"))
        {
            append_flag = 1;
        }
        if(isSubsequence(argv[i],"-x"))
        {
            extract_flag = 1;
        }
    }
    if(print_flag + create_flag + append_flag + extract_flag == 0)
    {
        free_all(files);
        printf("tar: You must specify one of the '-ctrux' options\n");
        return 0;
    }
    char tar_name[100];
    if(argc > i)
    {
        strcpy(tar_name, argv[i]);
    }
    else
    {
        free_all(files);
        printf("tar: option requires an argument -- 'f'\n");
        return 0;
    }
    i++;
    if(print_flag + create_flag + append_flag + extract_flag > 1)
    {
        printf("tar: You may not specify more than one '-ctrux' option\n");
        free_all(files);
        return 0;
    }
    for( ; i < argc; i++)
    {
        new_string(files, argv[i]);

    }
    if(print_flag && f_flag)
    {
        int fd_tar = open(tar_name, O_RDONLY);
        if(fd_tar == -1)
        {
            printf("tar: %s: Cannot open: No such file or directory\n",tar_name);
            printf("tar: Error is not recoverable: exiting now\n");
            free_all(files);
            return 0;
        }
        print_tar(fd_tar);
        close(fd_tar);
        return 0;
    }
    if(create_flag && f_flag)
    {
        int fd_tar = open(tar_name, O_CREAT | O_TRUNC | O_WRONLY);
        if(!files->size)
        {
            printf("tar: Cowardly refusing to create an empty archive\n");
            free_all(files);
            close(fd_tar);
            return 0;
        }
        for(int j = 0; j < files->size; j++)
        {
            add_file(files->array[j], fd_tar);
        }
        close(fd_tar);
        free_all(files);
        return 0;
    }
    if(append_flag && f_flag)
    {
        int fd_tar = open(tar_name, O_CREAT | O_WRONLY);
        lseek(fd_tar, 0, SEEK_END);
        for(int j = 0; j < files->size; j++)
        {
            add_file(files->array[j], fd_tar);
        }
        close(fd_tar);
        free_all(files);
        return 0;
    }
    if(if_newer_flag && f_flag)
    {
        int fd_tar = open(tar_name, O_CREAT | O_WRONLY);
        for(int j = 0; j < files->size; j++)
        {
            add_file_if_newer(files->array[j], fd_tar);
        }
        close(fd_tar);
        free_all(files);
        return 0;
    }
    if(extract_flag && f_flag)
    {
        int fd_tar = open(tar_name, O_RDONLY);

        extract_files(fd_tar);
        close(fd_tar);
        free_all(files);
        return 0;
    }
    if(print_flag | extract_flag)
    {
        printf("tar: Refusing to read archive contents from terminal (missing -f option?)\n");
        printf("tar: Error is not recoverable: exiting now\n");
    }
    else if(create_flag)
    {
        printf("tar: Refusing to write archive contents to terminal (missing -f option?)\n");
        printf("tar: Error is not recoverable: exiting now\n");
    }
    else if(append_flag | if_newer_flag)
    {
        printf("tar: Options '-ru' are incompatible with '-f -'\n");
        printf("tar: Error is not recoverable: exiting now\n");
    }
    if(f_flag)
    {
        printf("tar: You must specify one of the '-ctrux' options\n");
    }
    free_all(files);
    return 0;
}
