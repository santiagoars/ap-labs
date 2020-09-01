#include <stdio.h>
#include <stdlib.h>

static char *months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
static char endDays[2][13] = {{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

/* month_day function's prototype*/
void month_day(int year, int yearday, int *pmonth, int *pday){
    int b = (year % 100 != 0 || year% 400 == 0) && (year % 4);
    if((b == 1 && yearday > 366) || (b == 0 && yearday > 365)){
        *pday = 0;
        *pmonth = 0;
        return;
    }

    for(size_t i = 1; i < 13; i++){
        if(yearday > endDays[b][i]){
            *pmonth = i;
            yearday = yearday - (int)endDays[b][i];
        }
    }
    *pday = yearday;
}

int main(int argc, char *argv[]) {
    if(argc == 3){
        int yearday = atoi(argv[2]);
        int pday, pmonth = 0;
        int year = atoi(argv[1]);
        month_day(year, yearday, &pmonth, &pday);
        if(pday == 0 && pmonth == 0){
            printf("Invalid");
        }else{
            printf("%s, %d, %d\n", months[pmonth], pday, year);
        }
    }
    return 0;
}
