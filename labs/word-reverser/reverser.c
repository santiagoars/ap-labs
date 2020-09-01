#include <stdio.h>
#include <string.h>

int main(){
    // Place your magic here
    int i = 0;
    char x;
    char str[256] = "";
    while((x = getchar()) != EOF){
        if(x == '\n'){
            for(int j = strlen(str); j >= 0; j--){
                printf(" %c ",str[j]);
            }
            printf("\n");
            memset(str, 0, sizeof str);
            i = 0;
            continue;
        }
        str[i++] = x;
    }

    return 0;
}
