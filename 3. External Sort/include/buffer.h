#ifndef INCLUDE_BUFFER_H_
#define INCLUDE_BUFFER_H_s

#include "bf.h"
#include "sort_file.h"

extern int maxRecordsPerBlock;

typedef struct InputInfo{
	int blockLocation;
	int groupSize;
	int recordCount;
} InputInfo;


/*Buffer Functions*/
void bufferSetup(BF_Block **, char **, InputInfo *, int , int , int , int , int *);

void bufferSort(BF_Block **, char **,  InputInfo *, int*, int , int , int , int , int, int );

#endif /* INCLUDE_BUFFER_H_ */
