#include "general_use_functions.h"
#include <stdio.h>
#include <math.h>

/* General-use Functions */
void switchIntegers( int *a, int *b){
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

void strictRoundUp(double *value){
	if(*value> (int)(*value))
		*value = (int)((*(value))+1);
}

/* Setters */
void setFieldOffSet(int *fieldOffset, int fieldNo){
	if(fieldNo==0){
		*fieldOffset = 0;
	}else if(fieldNo==1){
		*fieldOffset = sizeof(int);
	}else if(fieldNo==2){
		*fieldOffset = sizeof(int)+15;
	}else if(fieldNo==3){
		*fieldOffset = sizeof(int)+35;
	}else{
		*fieldOffset = -1;
	}
}

void setExternalCondition(int *target,int blockCount, int bufferSize){
	double eOS;
	eOS = log((blockCount/bufferSize))/log(bufferSize-1);
	strictRoundUp(&eOS);
	*target = (int)(eOS);
	// If the number is round then that is exactly the
	// steps we need, but if it is even slightly greater
	// that means that there will be some blocks left
	// and therefore an additional step is needed.
}

void setInternalCondition(int *target, int blockCount, int groupsOf, int bufferSize ){
	double internalCondition, groupCount;
	groupCount = (double)blockCount/groupsOf;
	strictRoundUp(&groupCount);
	internalCondition = (double)groupCount/(bufferSize-1);
	strictRoundUp(&internalCondition);
	*target = (int)internalCondition;
}


/* Printers */
void printRec(void*add){
	printf("%d|",*((int*)(add)));
	printf("%s|",(char*)(add+sizeof(int)));
	printf("%s|",(char*)(add+sizeof(int)+15));
	printf("%s|\n",(char*)(add+sizeof(int)+35));
}

void printfDataBlock(void *add, int outputOffset){
	char ch = getchar();
	int b = sizeof(Record);
	printf("RecordCount=%d|outputOffset=%d|soR=%d",*((int*)(add)),outputOffset,b);
	int l, q = outputOffset/sizeof(Record);
	printf(" |q=%d\n",q);
	add = (void*)(add+sizeof(int));
	for(l=0; l<q; l++){
		printRec(add);
		add = (void*)(add+sizeof(Record));
	}
	ch = getchar();
}
