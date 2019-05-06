#include "buffer.h"
#include "general_use_functions.h"
#include "step1_functions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


SR_ErrorCode SR_Init() {
  // Your code goes here
	maxRecordsPerBlock = (BF_BLOCK_SIZE-sizeof(int))/sizeof(Record);
    return SR_OK;
}

SR_ErrorCode SR_CreateFile(const char *fileName) {
  // Your code goes here
    int fd;
    char* data;
    BF_Block *metadatablock;
    BF_Block_Init(&metadatablock);

    if (BF_CreateFile(fileName) != BF_OK) return SR_ERROR;
    BF_OpenFile(fileName, &fd);
    BF_AllocateBlock(fd, metadatablock);
    data = BF_Block_GetData(metadatablock);
    memcpy(data,"SRF", 3);
    BF_Block_SetDirty(metadatablock);

    BF_UnpinBlock(metadatablock);
    BF_CloseFile(fd);
    return SR_OK;
}

SR_ErrorCode SR_OpenFile(const char *fileName, int *fileDesc) {
  // Your code goes here
    int fd;
    char* data;
    BF_Block *block;
    BF_Block_Init(&block);

    if (BF_OpenFile(fileName,&fd) != BF_OK) return SR_ERROR;

    BF_GetBlock(fd, 0, block);
    data = BF_Block_GetData(block);
    BF_UnpinBlock(block);
    if(strcmp(data,"SRF")) return SR_ERROR;

    *fileDesc = fd;
    return SR_OK;
}

SR_ErrorCode SR_CloseFile(int fileDesc) {
  // Your code goes here
    if (BF_CloseFile(fileDesc) != BF_OK) return SR_ERROR;
    return SR_OK;
}

SR_ErrorCode SR_InsertEntry(int fileDesc,	Record record) {
  // Your code goes here
    int blocks_num;
    int block_num;
    int records_num;
    int used_space;
    int offset;
    char* data;

    BF_Block *block;
    BF_Block_Init(&block);
    BF_GetBlockCounter(fileDesc, &blocks_num);

    if (blocks_num == 1)
    {
        if (BF_AllocateBlock(fileDesc, block) != BF_OK) return SR_ERROR;
        data = BF_Block_GetData(block);
        records_num = 1;
        memcpy(data, &records_num, sizeof(records_num));
        offset = sizeof(records_num);
    }
    else if (blocks_num > 1)
    {
        block_num = blocks_num - 1;
        if (BF_GetBlock(fileDesc, block_num, block) != BF_OK) return SR_ERROR;
        data = BF_Block_GetData(block);
        memcpy(&records_num, data, 4);
        //printf("records: %d ", records_num);
        used_space = sizeof(records_num) + (records_num * sizeof(Record));

        if(used_space + sizeof(Record) <= BF_BLOCK_SIZE)
        {
            offset = used_space;
            records_num++;
            memcpy(data, &records_num, sizeof(records_num));
        }
        else
        {
            BF_UnpinBlock(block);
            if (BF_AllocateBlock(fileDesc, block) != BF_OK) return SR_ERROR;
            data = BF_Block_GetData(block);
            records_num = 1;
            memcpy(data, &records_num, sizeof(records_num));
            offset = sizeof(records_num);

        }

    }
    memcpy(data + offset, &record.id, sizeof(record.id));
    offset += sizeof(record.id);
    memcpy(data + offset, record.name, sizeof(record.name));
    offset += sizeof(record.name);
    memcpy(data + offset, record.surname, sizeof(record.surname));
    offset += sizeof(record.surname);
    memcpy(data + offset, record.city, sizeof(record.city));


    BF_Block_SetDirty(block);
    BF_UnpinBlock(block);

    BF_Block_Destroy(&block);
    return SR_OK;
}

SR_ErrorCode SR_SortedFile(
  const char* input_filename,
  const char* output_filename,
  int fieldNo,
  int bufferSize
) {
    // Your code goes here

    //Dimitris

    //Buffer Size Check
    if(bufferSize<3 || bufferSize>BF_BUFFER_SIZE) return SR_ERROR;

    //Open input file with file descriptor input_fd
    int input_fd;
    SR_OpenFile(input_filename, &input_fd);

    Record firstRecord;
    Record lastRecord;

    int blocks_num;
    BF_GetBlockCounter(input_fd, &blocks_num);

    //Step 1

    //Create temporary file for step 1
    int temp_fd;
    SR_CreateFile("step1.db");
    SR_OpenFile("step1.db", &temp_fd);

    int i;
    int offset, record_offset, records_num;
    Record record;

    //Passing data to temp file for sorting
    BF_Block *block_t;
    BF_Block_Init(&block_t);
    char* data_t;
    for (i=1; i<blocks_num; i++)
    {
        if (BF_GetBlock(input_fd, i, block_t) != BF_OK) return SR_ERROR;
        data_t = BF_Block_GetData(block_t);

        memcpy(&records_num, data_t,4);
        offset = sizeof(int);

        for (int j = 0; j < records_num; ++j)
        {
            record_offset = offset;
            memcpy(&record.id, data_t + record_offset, sizeof(record.id));
            record_offset += sizeof(record.id);
            memcpy(record.name, data_t + record_offset, sizeof(record.name));
            record_offset += sizeof(record.name);
            memcpy(record.surname, data_t + record_offset, sizeof(record.surname));
            record_offset += sizeof(record.surname);
            memcpy(record.city, data_t + record_offset, sizeof(record.city));

            SR_InsertEntry(temp_fd, record);

            offset += sizeof(Record);
        }

        BF_UnpinBlock(block_t);
    }

    //SR_PrintAllEntries(temp_fd);

    BF_GetBlockCounter(temp_fd, &blocks_num);

    int records_per_block = BF_BLOCK_SIZE/ sizeof(Record);

    int first_block_num, last_block_num;
    int low, high;
    int total_records;

    blocks_num--; // minus the metadata block
    int groups = blocks_num/bufferSize; // the number of groups if the blocks are perfectly divided by the buffer size
    if(blocks_num%bufferSize!=0) groups++; // plus one group of the remaining blocks if the blocks are not perfectly divided by the buffer size

    int first_block, last_block, total_blocks;

    //Creating two arrays of pointers to blocks and their data with size=bufferSize
    BF_Block **block;
    char **data;
    block = malloc(bufferSize*sizeof(BF_Block*));
    data = malloc(bufferSize*sizeof(char *));
    for (i = 0; i <bufferSize ; ++i) {
        BF_Block_Init(&block[i]);
    }

    //Working on each group of blocks
    for (i = 0; i < groups; ++i) {
        //Getting the block_num of the first block and last block of the group
        first_block_num = i*bufferSize + 1;

        if(i < groups-1)
            last_block_num = (i+ 1)*bufferSize ;
        else
            last_block_num = blocks_num;

        total_blocks = last_block_num - first_block_num+1;

        //Pinning all the blocks of the group to the memory and getting their data
        for (int j = 0; j <total_blocks ; ++j) {
            BF_GetBlock(temp_fd, first_block_num+j, block[j]);
            data[j] = BF_Block_GetData(block[j]);
        }

        //Setting the offset for the first(low) and the last(high) record of the group
        low = 4; // first record of first block

        memcpy(&records_num, data[total_blocks-1], 4);
        high = sizeof(records_num) + ((records_num-1) * sizeof(Record)); // last record of last block

        //Setting the locations of the first and last blocks in the array
        first_block = 0;
        last_block = total_blocks-1;

        //Calling Quicksort depending on the number of blocks
        if(!(blocks_num%bufferSize))
            quickSort(data, first_block, low, last_block, high, bufferSize, fieldNo);
        else
        {
            if(i<groups-1)
                quickSort(data, first_block, low, last_block, high, bufferSize, fieldNo);
            else
                quickSort(data, first_block, low, last_block, high, blocks_num%bufferSize, fieldNo);
        }

        for (int j = 0; j <total_blocks ; ++j){
            BF_Block_SetDirty(block[j]);
            BF_UnpinBlock(block[j]);
        }

        //If needed print sorted data by group of blocks with "bufferSize" blocks
        //printGroup(temp_fd, first_block_num, last_block_num);
    }
    for (i = 0; i <bufferSize ; ++i) {
        BF_Block_Destroy(&block[i]);
    }
    free(block);
    free(data);

    //If needed print the whole temp file with sorted data of step 1
    //SR_PrintAllEntries(temp_fd); getchar();



	// Ilias-Elias
	int l, y, q, j;
	int save, internalCondition, blockCount, fieldOffset, externalCondition;
	int nextOutputBlock, meter , temp2_fd, groupsOf;
	double temp, groupCount;

	//Keeping a temp_fd save
	save = temp_fd;

	// Initializing
	BF_GetBlockCounter(temp_fd, &blockCount); blockCount--;
	setFieldOffSet(&fieldOffset,fieldNo);
	if(fieldOffset<0){
		fprintf(stderr, "Incorrect fieldNo.\n");
		return SR_ERROR;
	}
	groupsOf  = bufferSize;
	setExternalCondition(&externalCondition, blockCount, bufferSize);

	// Create a second file and allocate as many blocks as the first one has //
	SR_CreateFile("step2.db");
	SR_OpenFile("step2.db", &temp2_fd);
	BF_Block *tempBlock;
	BF_Block_Init(&tempBlock);
	for(q=0; q<blockCount; q++){
		BF_AllocateBlock(temp2_fd, tempBlock);
		BF_UnpinBlock(tempBlock);
	}
	BF_Block_Destroy(&tempBlock);

	// Creating arrays for the blocks and the info necessary for their operations //
    BF_Block **buffer;
    char **dataArr;
	InputInfo *inputInfoArr;
	buffer = malloc(bufferSize*sizeof(BF_Block *));
	dataArr = malloc(bufferSize*sizeof(char *));
	inputInfoArr = malloc(bufferSize*sizeof(InputInfo));
	for(y=0; y<bufferSize; ++y)
		BF_Block_Init(&buffer[y]);

	// Merging process
	for(l = 0; l < externalCondition; ++l){
		setInternalCondition( &internalCondition, blockCount, groupsOf, bufferSize);
		BF_GetBlock(temp2_fd, 1,  buffer[bufferSize-1]);
		dataArr[bufferSize-1] = BF_Block_GetData(buffer[bufferSize-1]);

		meter = 1;
		nextOutputBlock = 2;
		// Internal Loop //
		for(j = 0; j < internalCondition; ++j){
			// Setup Buffer
			bufferSetup( buffer, dataArr, inputInfoArr , bufferSize, temp_fd, blockCount, groupsOf, &meter);
			// Sort
			bufferSort( buffer, dataArr, inputInfoArr, &nextOutputBlock, bufferSize, fieldNo, fieldOffset, temp_fd, temp2_fd, blockCount);
		}
		groupsOf = groupsOf*(bufferSize-1);
		switchIntegers( &temp_fd, &temp2_fd);
    }

    SR_CloseFile(temp_fd);
    SR_CloseFile(temp2_fd);

    if(temp_fd==save){
		remove("step2.db");
		rename("step1.db", output_filename);
	}else{
		remove("step1.db");
		rename("step2.db", output_filename);
	}
	
	free(buffer);
	free(dataArr);
	free(inputInfoArr);
    return SR_OK;
}

SR_ErrorCode SR_PrintAllEntries(int fileDesc) {
  // Your code goes here
    int i, id;
    int blocks_num;
    int records_num;
    int offset;
    char* data;
    BF_Block *block;
    BF_Block_Init(&block);

    BF_GetBlockCounter(fileDesc, &blocks_num);
    for (i=1; i<blocks_num; i++)
    {
        if (BF_GetBlock(fileDesc, i, block) != BF_OK) return SR_ERROR;
        data = BF_Block_GetData(block);

        memcpy(&records_num, data,4);
        offset = sizeof(int);

        printf("Block Number %d: \n", i);

        for (int j = 0; j < records_num; ++j)
        {
            memcpy(&id, data+offset,4);
            printf("ID: %d \t\t", id);
            printf("Name: %s \t\t", data+offset+4);
            printf("Surname: %s \t\t", data+offset+19);
            printf("City: %s \n", data+offset+39);

            offset += sizeof(Record);
        }
        BF_UnpinBlock(block);
    }

    BF_Block_Destroy(&block);
    return SR_OK;
}
