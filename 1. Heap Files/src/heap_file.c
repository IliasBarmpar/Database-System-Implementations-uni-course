#include <string.h>
#include "heap_file.h"
#include "bf.h"
#include <stdio.h>

HP_ErrorCode HP_Init() {
  // write your code here

  return HP_OK;
}

HP_ErrorCode HP_CreateIndex(const char *filename) {
//  BF_PrintError(BF_Init(LRU)); 
  
  BF_PrintError(BF_CreateFile(filename));

  int fileDesc;
  BF_PrintError(BF_OpenFile(filename,&fileDesc));

  BF_Block *block;
  BF_Block_Init(&block);
  BF_PrintError(BF_AllocateBlock(fileDesc,block));

  {
    char *data;
    data = BF_Block_GetData(block);
    //char type[2] = type[0] = 'H'; type[1] = 'P'; 
    char *type; type = "HP";
    memcpy(data, type, sizeof(type));
    BF_Block_SetDirty(block);
  }

  BF_PrintError(BF_UnpinBlock(block));

  BF_PrintError(BF_CloseFile(fileDesc));
  return HP_OK;
}

HP_ErrorCode HP_OpenFile(const char *fileName, int *fileDesc){
  BF_PrintError(BF_OpenFile(fileName,fileDesc));

  BF_Block *block;
  BF_Block_Init(&block);
  BF_PrintError(BF_GetBlock(*fileDesc,0,block));
  char *data;
  data = BF_Block_GetData(block);
  char type[2];
  strncpy(type, data, 2);
  if(strcmp(type,"HP")){
	printf("This is not a heap file.\n");
	return HP_ERROR;
  }
  return HP_OK;
}

HP_ErrorCode HP_CloseFile(int fileDesc){
  BF_PrintError(BF_CloseFile(fileDesc));
  return HP_OK;
}

HP_ErrorCode HP_InsertEntry(int fileDesc, Record record){
  int counter = (BF_BLOCK_SIZE-sizeof(int))/sizeof(record);
  int numberOfBlocks;
  BF_GetBlockCounter(fileDesc, &numberOfBlocks);
  BF_Block *block;
  BF_Block_Init(&block);
  int *data;
  if( numberOfBlocks == 1 ){
	printf("1\n");
	BF_PrintError(BF_AllocateBlock(fileDesc,block));
	data = (int *)BF_Block_GetData(block);
	*data = 1;
	data = (int*)((char*)data+ sizeof(int));
  }else{
	BF_GetBlock(fileDesc, numberOfBlocks-1, block);
	data = (int *)BF_Block_GetData(block);
	int numberOfRecs = *data;
	if(numberOfRecs < counter-1){
		*data = ++numberOfRecs;
		data = (int*)((char*)data + sizeof(int) + numberOfRecs*sizeof(Record));
	}else{
		BF_PrintError(BF_AllocateBlock(fileDesc,block));
		data = (int *)BF_Block_GetData(block);
		*data = 1;
		data = (int*)((char*)data+ sizeof(int));
	}
  }
  Record *r = data;
  memcpy(r,&record,sizeof(Record));
  BF_Block_SetDirty(block);
  BF_PrintError(BF_UnpinBlock(block));
  return HP_OK;
}

HP_ErrorCode HP_PrintAllEntries(int fileDesc) {
  int blockCount;
  BF_PrintError( BF_GetBlockCounter(fileDesc, &blockCount));
  if(blockCount > 1){
	int i;
	BF_Block *block;
	BF_Block_Init(&block);
	for(i = 1; i < blockCount; ++i){
		BF_GetBlock(fileDesc, i, block);
		int *data;
		data = (int *)BF_Block_GetData(block);
		int numberOfRecs = *data;
		data = (int*)((char*)data+ sizeof(int));
		Record *r = data;
		int y;		
		for(y = 1; y <= numberOfRecs; y++){
			printf("Id:       %d\n",r->id);
			printf("Name      %s\n",r->name);
			printf("Surname:  %s\n",r->surname);
			printf("City:     %s\n",r->city);
			r = (Record *)((char*)r + sizeof(Record));
		}
	}
  }
  return HP_OK;
}

HP_ErrorCode HP_GetEntry(int fileDesc, int rowId, Record *record) {
  int count, whichBlock, whichRecord;
  count = (BF_BLOCK_SIZE-sizeof(int))/sizeof(record);
  whichBlock = rowId/count;
  whichRecord = rowId%count;
  whichRecord++;
  BF_Block *block;
  BF_Block_Init(&block);
  BF_GetBlock(fileDesc, whichBlock+2, block);
  char *data;
  data = BF_Block_GetData(block);
  data = (char*)data + sizeof(int);
  record = data;
  record = (Record *)((char*)record + (whichRecord-1)*sizeof(Record));
  return HP_OK;
}





































