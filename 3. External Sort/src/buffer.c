#include "bufferSort.h"
#include <string.h>

void bufferSetup(BF_Block **buffer, char **dataArr, InputInfo *inputInfoArr, int bufferSize, int temp_fd, int blockCount, int groupsOf, int *meter){
	int i;
	for(i=0; i < bufferSize-1; ++i){
		if((*meter) <= blockCount){
			BF_GetBlock( temp_fd, (*meter), buffer[i] );
			dataArr[i] = BF_Block_GetData( buffer[i] );
			memcpy(&inputInfoArr[i].recordCount, dataArr[i], sizeof(int));
			dataArr[i] = (char*)(dataArr[i] + sizeof(int));
			inputInfoArr[i].blockLocation = (*meter);
			if(blockCount-(*meter) < groupsOf){
				inputInfoArr[i].groupSize = (blockCount-(*meter));
			}else{
				inputInfoArr[i].groupSize = groupsOf-1;
			}
		}else{
			dataArr[i] = 0;
			inputInfoArr[i].recordCount = 0;
			inputInfoArr[i].blockLocation = 0;
			inputInfoArr[i].groupSize = 0;
		}
		(*meter) += groupsOf;
	}
}

void bufferSort(BF_Block **buffer, char **dataArr, InputInfo *inputInfoArr, int *nextOutputBlock, int bufferSize, int fieldNo, int fieldOffset, int temp_fd, int temp2_fd, int blockCount){
	int i, min, outputRecordCount,outputOffset, noRecordsLeft;
	outputRecordCount = 0;
	outputOffset = 0;
	noRecordsLeft = 1;

	while(noRecordsLeft){
		//Find minimum the record that is min
		min = sortFindMin( dataArr, inputInfoArr, bufferSize, fieldNo, fieldOffset);
		if( min < 0 ) break;
		outputRecordCount++;

		//Copy record
		memcpy((void *)(dataArr[bufferSize-1]+sizeof(int)+outputOffset), dataArr[min], sizeof(Record));
		outputOffset += sizeof(Record);
		inputInfoArr[min].recordCount--;

		//If there are no more records in this input block check input block
		if(inputInfoArr[min].recordCount==0){
			sortCheckInputBlock(buffer, dataArr, inputInfoArr, temp_fd, min);
		}else{
			dataArr[min] = (char *)(dataArr[min] + sizeof(Record));
		}

		//Check output block
		if(outputRecordCount >= maxRecordsPerBlock){
			sortCheckOutputBlock( buffer, dataArr, bufferSize, temp2_fd, blockCount, nextOutputBlock, &outputRecordCount, &outputOffset);

		}
	}
	// If the ending condition has been met and we still have a block open we write this one as well
	if(outputRecordCount>0){
		memcpy(dataArr[bufferSize-1], &outputRecordCount, sizeof(int));
		BF_Block_SetDirty(buffer[bufferSize-1]);
		BF_UnpinBlock( buffer[bufferSize-1]);
	}
}
