#ifndef STEP1_FUNCTIONS_H
#define STEP1_FUNCTIONS_H

#include "bf.h"
#include "sort_file.h"

typedef struct pair{
    int block_num;
    int offset;
} pair;

int record_cmp(Record*, Record*, int);

int swap(char*, char*, Record, int, int);

pair partition(char **, int, int, int, int, int, int);

void quickSort(char **, int, int, int, int, int, int);

void printGroup(int, int, int);

#endif //STEP1_FUNCTIONS_H
