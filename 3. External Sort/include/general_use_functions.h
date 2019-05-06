#ifndef GUF_H
#define GUF_H

#include "sort_file.h"

/* General-use Functions */
void switchIntegers( int *, int *);

void strictRoundUp(double *);


/* Setters */
void setExternalCondition(int *,int , int);

void setInternalCondition(int *, int , int , int);

void setFieldOffSet(int *, int);


/* Printers */
void printRec(void* );

void printfDataBlock(void *, int);

#endif /* INCLUDE_BUFFER_H_ */
