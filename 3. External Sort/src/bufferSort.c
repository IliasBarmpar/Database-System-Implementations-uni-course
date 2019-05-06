#include "bufferSort.h"
#include <string.h>

int sortFindMin(char **dataArr,  InputInfo *inputInfoArr, int bufferSize, int fieldNo, int fieldOffset){
	// Find minimum
	int i, min = 0;
	while( inputInfoArr[min].recordCount==0 ){
		min++;
	}
	if(min>=bufferSize-1)
		return -1;

	if(fieldNo == 0){
		i = min+1;
		for(; i< bufferSize-1; i++){
			if( inputInfoArr[i].recordCount>0 )
				if( *( (int *)(dataArr[i]+fieldOffset) ) < *( (int *)(dataArr[min]+fieldOffset) ) )
					min = i;
		}
	}else if( 1 <= fieldNo && fieldNo <= 3){
		i = min+1;
		for(; i < bufferSize-1; i++)
			if( inputInfoArr[i].recordCount > 0 )
				if( strcmp( (char *)(dataArr[i]+fieldOffset), (char *)(dataArr[min]+fieldOffset) ) < 0 )
					min = i;
	}
	return min;
}

void sortCheckInputBlock(BF_Block **buffer, char **dataArr, InputInfo *inputInfoArr, int temp_fd, int min){
	//Case 1: there are enough blocks in this group so grab the next one.
	if(inputInfoArr[min].groupSize>0){
		// Decrease groupSize, unpin the previous block and get the next one!
		inputInfoArr[min].groupSize--;
		BF_UnpinBlock(buffer[min]);
		inputInfoArr[min].blockLocation++;
		BF_GetBlock(temp_fd , inputInfoArr[min].blockLocation, buffer[min]);
		// Initialize data pointer by pointing data to the new block, getting the record count and moving the data pointer to the first record.
		dataArr[min] = BF_Block_GetData(buffer[min]);
		memcpy(&inputInfoArr[min].recordCount, dataArr[min], sizeof(int));
		dataArr[min] = (char *)(dataArr[min] + sizeof(int));
	}
	//Case 2: there aren't enough blocks in this group so...
	else{
		// Unpin our block and set values to 0
		BF_UnpinBlock(buffer[min]);
		dataArr[min] = 0;
		inputInfoArr[min].recordCount = 0;
		inputInfoArr[min].blockLocation = 0;
		inputInfoArr[min].groupSize = 0;
	}
}

void sortCheckOutputBlock(BF_Block **buffer, char **dataArr, int bufferSize, int temp2_fd, int blockCount,
					  int *nextOutputBlock, int *outputRecordCount, int *outputOffset)
{
	memcpy( dataArr[bufferSize-1], outputRecordCount, sizeof(int) );
	*outputRecordCount=0;
	*outputOffset = 0 ;
	BF_Block_SetDirty(buffer[bufferSize-1]);
	BF_UnpinBlock( buffer[bufferSize-1]);
	if( *nextOutputBlock <= blockCount){
		BF_GetBlock(temp2_fd, *nextOutputBlock, buffer[bufferSize-1]);
		(*nextOutputBlock)++;
		dataArr[bufferSize-1] = BF_Block_GetData(buffer[bufferSize-1]);
	}
}
