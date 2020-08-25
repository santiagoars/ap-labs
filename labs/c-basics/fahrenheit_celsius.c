#include<stdlib.h>
#include <stdio.h>

#define   LOWER  0       /* lower limit of table */
#define   UPPER  300     /* upper limit */
#define   STEP   20      /* step size */

/* print Fahrenheit-Celsius table */

int main(int argc, char ** argv)
{
    int fahr;
    if(argc == 2){
        printf("Fahrenheit: %3d, Celcius: %6.1f\n", atoi(argv[1]), (5.0/9.0) * (atoi(argv[1])-32));
    }else if(argc == 3){
        printf("Fahrenheit: %3d, Celcius: %6.1f\n", atoi(argv[1]), (5.0/9.0) * (atoi(argv[1])-32));
        printf("Fahrenheit: %3d, Celcius: %6.1f\n", atoi(argv[2]), (5.0/9.0) * (atoi(argv[2])-32));
    }else if(argc == 4) {
        if(atoi(argv[3]) <= 0){
            printf("Invalid parameters!!\n");
        }else{
            for (fahr = atoi(argv[1]); fahr <= atoi(argv[2]); fahr = fahr + atoi(argv[3]))
            printf("Fahrenheit: %3d, Celcius: %6.1f\n", fahr, (5.0/9.0)*(fahr-32));
        }
    }else{
        for (fahr = LOWER; fahr <= UPPER; fahr = fahr + STEP)
	    printf("Fahrenheit: %3d, Celcius: %6.1f\n", fahr, (5.0/9.0)*(fahr-32));
    }
    return 0;
}
