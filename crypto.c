#include <stdio.h>
#include <stdlib.h>

#define DATA_SIZE 1000

int main()
{
    char data[DATA_SIZE] = "I can!!";
    FILE * fPtr;
    fPtr = fopen("/home/parallels/Desktop/Operating_systems_coursework/file.txt", "w");

    if(fPtr == NULL)
    {
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }

    fputs(data, fPtr);
    fclose(fPtr);

    return 0;
}
