#ifndef BUFFERSORT_H
#define BUFFERSORT_H

#include "buffer.h"

int sortFindMin(char **,  InputInfo *, int , int , int );

void sortCheckInputBlock(BF_Block **, char **, InputInfo *, int , int);

void sortCheckOutputBlock(BF_Block **, char **, int , int, int , int* , int *, int *);

#endif // BUFFERSORT_H
