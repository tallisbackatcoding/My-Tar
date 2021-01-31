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

char *my_itoa_base(size_t n, int base){
    if(n == 0){
        char *r = malloc(2*sizeof(char));
        r[0] = '0';
        r[1] = 0;
        return r;
    }
    char str[100];
    int i = 0;
    while(n!=0){
        int temp  = 0;
        temp = n % base;
        if(temp < 10){
            str[i] = temp + 48;
        }
        else{
            str[i] = temp + 87;
        }
        n = n/base;
        i++;
    }
    char *result = malloc((100)*sizeof(char));
    int k = 0;
    for(int j=i-1; j>=0; j--,k++){
        result[k] = str[j];
    }
    result[k] = 0;
    return result;
}

void my_puts(char *s, int new_line_needed){
    write(1, s, my_strlen(s));
    if(new_line_needed){
        write(1, "\n", 1);
    }
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

void expand(int fd, int size, char null)
{
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
//Function writes specific attribute of header into tar file
int fill_header(int fd_tar, char *header_struct, int size, size_t data){

    char str[100];
    int checksum_zeroes = 0;
    char *field = my_itoa_base(data, 8);
    my_strcpy(str, field);
    my_strcpy(header_struct,str);

    expand(fd_tar, size - 1 - my_strlen(str), '0');
    write(fd_tar, str, my_strlen(str)+1);
    checksum_zeroes += size - 1 - my_strlen(str);
    free(field);
    return checksum_zeroes;
}

int count_bytes(char *str){
    int bytes = 0;
    for(int i = 0; str[i]; i++){
        bytes += str[i];
    }
    return bytes;
}
//Function writes header parameters for file "file_name"
void write_header(int fd_tar, struct stat file_info, char *file_name)
{
    tar_header header;

    //name
    char file_type = get_type(file_info);
    char name[100];
    my_strcpy(name, file_name);
    if(file_type == '5')
    {
        my_concat(name,"/");
    }
    int checksum_zeroes = 0;

    write(fd_tar, name, my_strlen(name));
    expand(fd_tar,100 - my_strlen(name),0);

    memset( &header, 0, sizeof( tar_header ) );
    strcpy(header.name, name);

    //mode
    checksum_zeroes += fill_header(fd_tar, header.mode, 8, file_info.st_mode%512);
    //user id
    checksum_zeroes += fill_header(fd_tar, header.owner, 8, file_info.st_uid);
    //owner id
    checksum_zeroes += fill_header(fd_tar, header.group, 8, file_info.st_gid);
    //size
    checksum_zeroes += fill_header(fd_tar, header.size, 12, file_info.st_size);
    //last modified

    checksum_zeroes += fill_header(fd_tar, header.modified, 12, file_info.st_mtim.tv_sec);
    //type
    char file_type_str[2];
    file_type_str[0] = file_type;
    file_type_str[1] = 0;

    my_strcpy(header.type, file_type_str);

    if(file_type == '2')
    {
        readlink(file_name, header.link, 100);
    }

    size_t checksum = 0;
    checksum += count_bytes(header.name);
    checksum += count_bytes(header.group);
    checksum += count_bytes(header.mode);
    checksum += count_bytes(header.modified);
    checksum += count_bytes(header.owner);
    checksum += count_bytes(header.size);
    checksum += count_bytes(header.type);
    checksum += 256; //8 spaces
    checksum += checksum_zeroes*48;

    //checksum
    fill_header(fd_tar, header.checksum, 8, checksum);

    //type
    write(fd_tar, file_type_str, 1);
}

void add_file(char *file_name, int fd_tar)
{
    int fd_input = open(file_name, O_RDONLY);
    int file_size = 0;

    struct stat file_info;

    if (lstat(file_name, &file_info) != 0)
    {
        my_puts("tar: ", 0);
        my_puts(file_name, 0);
        my_puts(": Cannot stat: No such file or directory", 1);
        my_puts("tar: Exiting with failure status due to previous errors", 1);
        return;
    }

    write_header(fd_tar, file_info, file_name);

    //fill or not to fill link name
    if(get_type(file_info) == '2')
    {
        char linked_file[100];
        int len;

        if ((len = readlink(file_name, linked_file, 99)) != -1)
        {
            linked_file[len] = 0;
            write(fd_tar, linked_file, my_strlen(linked_file));
            expand(fd_tar, 100 - my_strlen(linked_file), 0);
        }
        else{
            my_puts("tar: ", 0);
            my_puts(file_name, 0);
            my_puts(": Cannot stat: No such file or directory", 1);
            my_puts("tar: Exiting with failure status due to previous errors", 1);
        }
    }
    else
    {
        expand(fd_tar, 100, 0);
    }

    //padding up to 512 bytes
    expand(fd_tar,255,0);

    //if it is folder, recursively adding files from folder to tar
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
        if(file_size != 0){
            expand(fd_tar,512 - file_size % 512,0);
        }
    }
    close(fd_input);
}

//returns string at position "begin" to "end"
char* get_string_range(int fd, int begin, int end)
{
    char *result = malloc(512*sizeof(char));
    lseek(fd, begin, SEEK_SET);
    int null_check = read(fd, result, end - begin);
    if(!null_check)
    {
        free(result);
        return NULL;
    }
    return result;
}

//string to long long
long long my_atoi(char *str)
{
    long long result = 0;
    for(int i = 0; str[i]; i++)
    {
        result = result*10 + str[i] - 48;
    }
    return result;
}

//assigns dates to files after extraction
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

        //adding filenames and its date to array to assign them lately
        new_string(file_names, file_name);
        new_string(file_dates, file_mtime);

        int skip = 0;
        if(file_type[0] == '5')
        {
            mkdir(file_name, 0755);
            skip = 1;
        }
        else if(file_type[0] == '0')
        {
            fd = open(file_name, O_CREAT | O_TRUNC | O_WRONLY, 0755);
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

        //if it is not dir or link it fills content of file
        if(fd != -1)
        {
            lseek(fd_tar, i, SEEK_SET);
            while(1)
            {
                read(fd_tar, &buffer, 1);
                if(buffer == 0)
                    break;

                write(fd, &buffer, 1);
                i++;
            }
            close(fd);
            i = i + 512 - i % 512;
        }else{
            my_puts("tar: file descriptor is negative", 1);
            return;
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

        my_puts(file_name, 1);

        free(file_name);
        free(file_type);
        i += 512;
        if(skip)
            continue;

        lseek(fd_tar, i, SEEK_SET);
        while(true)
        {
            read(fd_tar, &buffer, 1);
            if(buffer == 0)
            {
                break;
            }
            i++;
        }
        i = i + 512 - i % 512;

    }
}
//-u option
void add_file_if_newer(char *new_file_name, int fd_tar)
{
    int fd_new = open(new_file_name, O_RDONLY);

    struct stat file_info;
    if (lstat(new_file_name, &file_info) != 0)
    {
        my_puts("tar: ", 0);
        my_puts(new_file_name, 0);
        my_puts(": Cannot stat: No such file or directory", 1);
        my_puts("tar: Exiting with failure status due to previous errors", 1);
        return;
    }
    close(fd_new);

    char new_file_date[12];
    char *num = my_itoa_base(file_info.st_mtim.tv_sec, 8);
    my_strcpy(new_file_date, num);
    free(num);

    int file_should_be_added = 1;
    int i = 0;
    char buffer;
    lseek(fd_tar, 0, SEEK_SET);


    while(true)
    {
        int skip = 0;
        char *check = get_string_range(fd_tar, i, i+2);
        if(check == NULL || check[0] <= 0)
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
                read(fd_tar, &buffer, 1);
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
        if(isSubsequence(argv[i], "-u")){
            if_newer_flag = 1;
        }
    }
    if(print_flag + create_flag + append_flag + extract_flag + if_newer_flag == 0)
    {
        free_all(files);
        my_puts("tar: You must specify one of the '-ctrux' options", 1);
        return 1;
    }
    char tar_name[100];
    if(argc > i)
    {
        strcpy(tar_name, argv[i]);
    }
    else
    {
        free_all(files);
        my_puts("tar: option requires an argument -- 'f'", 1);
        return 1;
    }
    i++;
    if(print_flag + create_flag + append_flag + extract_flag > 1)
    {
        my_puts("tar: You may not specify more than one '-ctrux' option", 1);
        free_all(files);
        return 1;
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
            my_puts("tar: ", 0);
            my_puts(tar_name, 0);
            my_puts(": Cannot open: No such file or directory", 1);
            my_puts("tar: Error is not recoverable: exiting now", 1);

            free_all(files);
            return 1;
        }
        print_tar(fd_tar);
        close(fd_tar);
        free_all(files);
        return 0;
    }
    if(create_flag && f_flag)
    {
        int fd_tar = open(tar_name, O_CREAT | O_TRUNC | O_RDWR, 0755);
        if(!files->size)
        {
            my_puts("tar: Cowardly refusing to create an empty archive", 1);
            free_all(files);
            close(fd_tar);
            return 1;
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
        int fd_tar = open(tar_name, O_CREAT | O_RDWR);
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
    int error_occured = 0;
    if(print_flag | extract_flag)
    {
        my_puts("tar: Refusing to read archive contents from terminal (missing -f option?)", 1);
        my_puts("tar: Error is not recoverable: exiting now", 1);
        error_occured = 1;
    }
    else if(create_flag)
    {
        my_puts("tar: Refusing to write archive contents to terminal (missing -f option?)", 1);
        my_puts("tar: Error is not recoverable: exiting now", 1);
        error_occured = 1;
    }
    else if(append_flag | if_newer_flag)
    {
        my_puts("tar: Options '-ru' are incompatible with '-f'", 1);
        my_puts("tar: Error is not recoverable: exiting now", 1);
        error_occured = 1;
    }
    if(f_flag)
    {
        my_puts("tar: You must specify one of the '-ctrux' options", 1);
        error_occured = 1;
    }

    free_all(files);
    if(error_occured){
        return 1;
    }
    return 0;
}
