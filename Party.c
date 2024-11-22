#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(){

    ///Variables
    const char *Filename = "Data folder/Party.txt";


    //Code
    FILE *file = fopen(Filename,"r");
    if (file==NULL){
        printf("Party file not found, creating one..\n");
        _sleep(3);
        file=fopen(Filename,"w");
        if (file!=NULL){
        printf("File created successfully.");
        } else {
            printf("Error when creating file");
            exit(1);
        }
    }







}