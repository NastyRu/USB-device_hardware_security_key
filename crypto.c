#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "crypto_config.h"

typedef unsigned char byte;

byte get_next_byte(byte *in, int length)
{
    static byte *bytes = NULL;
    static int count = 0;
    static int ocount;

    if (!in)
    {
	    if(!count)
	        count = ocount;
	    return bytes[count--];
    }
    else
    {
	    bytes = in;
	    count = ocount = length - 1;
	    return (byte)0;
    }
}

byte* load_file(const char *filename, int *size)
{
    int file_size = 0;
    FILE *file = fopen(filename, "rb+");
    byte *result = NULL;

    if (file)
    {
	    fseek(file, 0, SEEK_END);
	    file_size = ftell(file);
	    result = malloc(sizeof(byte) * file_size);
	    fseek(file, 0, SEEK_SET);
	    fread(result, sizeof(byte), file_size, file);
	    *size = file_size;
	    fclose(file);
    }

    return result;
}

void write_file(const char *name, byte *src, int len)
{
    FILE *result = fopen(name, "wb+");
    int bytes_out = 0;

    bytes_out = fwrite(src, sizeof(byte), len, result);
    if(bytes_out != len)
    {
	    fprintf(stderr, "Out bytes did not match length!\n");
    }

    fclose(result);
}

void xor_bytes(byte *arr, int arr_len, byte *password, int password_len)
{
    byte cur;
    get_next_byte(password, password_len);
    int i;

    for (i = 0; i < arr_len; i++)
    {
	    cur = get_next_byte(NULL, 0);
	    arr[i] ^= cur;
    }
}

int main(int argc, char **argv)
{
    int flsz, i = 0;
    byte* filemem;
    char* fltmp;
    
    while (secret_apps[i] != NULL)
    {
        fltmp = malloc(sizeof(char)*(strlen(secret_apps[i]) + 1));
	    strcpy(fltmp, secret_apps[i]);
	    fltmp[strlen(secret_apps[i])] = '\0';

        filemem = load_file(fltmp, &flsz);

        if (argc == 1)
        {
            char temp[strlen(PASSWORD) + 1];
            strcpy(temp, PASSWORD);
            temp[strlen(PASSWORD)] = '\0';
            xor_bytes(filemem, flsz, temp, strlen(temp));
            
        }
        if (argc == 2)
        {
            xor_bytes(filemem, flsz, argv[1], strlen(argv[1]));  
        }
	
	    write_file(fltmp, filemem, flsz);

	    free(fltmp);
        i++;
    }

    return 0;
}
