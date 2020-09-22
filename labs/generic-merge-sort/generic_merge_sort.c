#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 10000

void mergeSort(void *array[], int l, int r, int (*cmpFun)(void *, void *));
void merge(void *array[], int l, int m, int r, int (*cmpFun)(void *, void *));
int bigger(char *, char *);
char *lines[MAX];

int bigger(char *s1, char *s2) {
    double x1, x2;
    x1 = atof(s1);
    x2 = atof(s2);
    if (x1 < x2)
        return -1;
    else if(x1 > x2)
        return 1;
    else
        return 0;
}

void merge(void* array[], int l, int m, int r, int (*cmpFun)(void *, void *)){
    int i, j, k; 
    int n1 = m-l + 1; 
    int n2 = r - m; 

    void *L[n1], *R[n2]; 

    for (i = 0; i < n1; i++)
        L[i] = array[l + i];

    for (j = 0; j < n2; j++) 
        R[j] = array[m + 1+ j]; 

    i = 0; 
    j = 0; 
    k = l; 
    while (i < n1 && j < n2){ 
        if (cmpFun(L[i],R[j])<=0) { 
            array[k] = L[i]; 
            i++; 
        } 
        else
        { 
            array[k] = R[j]; 
            j++; 
        } 
        k++; 
    } 
    while (i < n1) 
    { 
        array[k] = L[i]; 
        i++; 
        k++; 
    } 
    while (j < n2) 
    { 
        array[k] = R[j]; 
        j++; 
        k++; 
    }
}

void mergeSort(void* array[], int l, int r, int (*cmpFun)(void *, void *)){
    if (l < r) 
    { 
        int m = l+(r-l)/2; 

        mergeSort(array, l, m, cmpFun); 
        mergeSort(array, m+1, r, cmpFun); 

        merge(array, l, m, r, cmpFun); 
    } 
}

int main(int argc, char** argv)
{
    int type = 0;
    if(argc < 2) {
        printf("Error: Incorrect parameters.\n");
        printf("Usage: ./generic_merge_sort <type (default is string)> <filename>\n");
        return 1;
    }

    if(argc == 2 && strcmp(argv[1],"-n") == 0){
        printf("Error, wrong parameters.\n");
        printf("Usage: ./generic_merge_sort <type (default is string)> <filename>\n");
        return 1;
    }

    if(argc == 3 && strcmp(argv[1],"-n") == 0){
        type = 1;
    }

    FILE *file;

    if(type){
        if((file = fopen(argv[2],"r")) == NULL){
            printf("Error, can't open %s",argv[2]);
            return 1;
        }
    }else{
        if((file = fopen(argv[1],"r")) == NULL){
            printf("Error, can't open %s",argv[1]);
            return 1;
        }
    }
    
    char line[256] = {0};
    int index = 0;
    while(fgets(line,100,file)){
        lines[index] = (char*)malloc(strlen(line) + sizeof(char*));
        strcpy(lines[index],line);
        index++;
    }

    if(fclose(file)){
        printf("Error: couldn't close the file.\n");
    }


    mergeSort((void *) lines, 0, index-1, (int (*)(void*, void*)) (type ? bigger : strcmp));
    for (int j = 0; j < index; j++){
        printf("%s", lines[j]);
    }
    return 0;
}