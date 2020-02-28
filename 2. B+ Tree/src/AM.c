#include "AM.h"
#include "bf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int AM_errno = AME_OK;
int num_of_scans = 0;
Scan* curScans = NULL;

int num_of_open_files = 0;

file_info* open_files = NULL;

void file_info_Init(file_info* fi, char* ft, int fd, char at1, int al1, char at2, int al2, int rbl)
{
    //Initialization of struct, element of open files array
    fi->file_type = ft;
    fi->file_desc = fd;
    fi->attrType1 = at1;
    fi->attrLength1 = al1;
    fi->attrType2 = at2;
    fi->attrLength2 = al2;
    fi->root_block_num = rbl;
}

int add_open_file(file_info fi)
{
    //Adds file info to array of open files
    int i = 0;
    int f = 1;
    if(num_of_open_files<AM_MAX_OPEN_FILES)
    {
        while(i<AM_MAX_OPEN_FILES && f == 1)
        {
            //finds the first index that is free
            if(open_files[i].file_type == NULL)
            {
                open_files[i] = fi;
                num_of_open_files++;

                f=0;
            }
            i++;
        }
        //returns index of array of open files
        return i-1;
    }
    else
        return AM_OPEN_FILES_LIMIT_ERROR;
}

int delete_open_file(int fd)
{
    //Deletes file info from array of open files
    if(num_of_open_files>=0 && fd<AM_MAX_OPEN_FILES)
    {
        if(open_files[fd].file_type != NULL)
        {
            file_info_Init(&open_files[fd], NULL, 0, 0, 0, 0, 0, 0);
            num_of_open_files--;
            return AME_OK;
        }
        else
            return AM_INVALID_FILE_ERROR;
    }
    else
        return AM_INVALID_FILE_ERROR;
}

int key_comparison(char keyType, void* insertKey, void* key)
{
    //Compares two void* variabls deepending on their type
    char *cinsertKey, *ckey;
    int iinsertKey, ikey;
    float finsertKey, fkey;

    switch (keyType)
    {
        case 'c':
            cinsertKey = (char*)insertKey;
            ckey = (char*)key;
            if(strcmp(cinsertKey, ckey)<0) return 1;
            else return 0;
        case 'i':
            iinsertKey = *((int*)insertKey);
            ikey = *((int*)key);
            if(iinsertKey<ikey) return 1;
            else return 0;
        case 'f':
            finsertKey = *((float*)insertKey);
            fkey = *((float*)key);
            if(finsertKey<fkey) return 1;
            else return 0;
    }

}

int key_equal(char keyType, void* insertKey, void* key)
{
    char *cinsertKey, *ckey;
    int iinsertKey, ikey;
    float finsertKey, fkey;

    switch (keyType)
    {
        case 'c':

            cinsertKey = (char*)insertKey;
            ckey = (char*)key;
            //           printf("\n%s=", cinsertKey);
            //           printf("%s\n", ckey);
            if(strcmp(cinsertKey, ckey)==0) return 1;
            else return 0;
        case 'i':
            iinsertKey = *((int*)insertKey);
            ikey = *((int*)key);
            //           printf("\n%d=", iinsertKey);
            //           printf("%d\n", ikey);
            if(iinsertKey==ikey) return 1;
            else return 0;
        case 'f':
            finsertKey = *((float*)insertKey);
            fkey = *((float*)key);
            //           printf("\n%f=", finsertKey);
            //           printf("%f\n", fkey);
            if(finsertKey==fkey) return 1;
            else return 0;
    }

}

/******************************************************************/
//Implementation of Stack

void stackInit(Stack* stack, int maxSize)
{
    int* newelements;

    newelements = (int*)malloc(maxSize* sizeof(int));

    stack->elements = newelements;
    stack->numofElements = 0;
    stack->maxSize = maxSize;
}

int stackEmpty(Stack* stack)
{
    return stack->numofElements == 0;
}

void stackPush(Stack* stack, int newelement)
{
    stack->elements[stack->numofElements] = newelement;
    stack->numofElements++;
}

int stackPop(Stack* stack)
{
    if(stackEmpty(stack))
    {
        return -1;
    }
    stack->numofElements--;
    return stack->elements[stack->numofElements];
}

void stackDestroy(Stack* stack)
{
    free(stack->elements);

    stack->elements = NULL;
    stack->numofElements = 0;
    stack->maxSize = 0;
}

/******************************************************************/

int createIndexBlock(int fileDesc, int blockptr, int newblockptr, void* key, int keyLength)
{
    // Creates an Index Block and returns its block number
    BF_Block *block;
    BF_Block_Init(&block);
    char* data;
    int blocks_num, block_num;


    if (BF_AllocateBlock(fileDesc, block) != BF_OK) return AM_BF_LEVEL_ERROR;
    data = BF_Block_GetData(block);
    BF_GetBlockCounter(fileDesc, &blocks_num);
    block_num = blocks_num -1;
    char type = 'i';
    int keyCount = 1;
    int offset = 0;
    memcpy(data, &type, sizeof(char));
    offset += sizeof(char);
    memcpy(data+offset, &keyCount, sizeof(int));
    offset += sizeof(int);
    memcpy(data+offset, &blockptr, sizeof(int));
    offset += sizeof(int);
    memcpy(data+offset, key, keyLength);
    offset += keyLength;
    memcpy(data+offset, &newblockptr, sizeof(int));

    BF_Block_SetDirty(block);
    BF_UnpinBlock(block);

    return block_num;
}

void indexAddandSort(char* data, int offset, void* newkey, int newptr, char keyType, int keyLength)
{
    //Adds a key and a pointer/block number to a block and sorts it when needed
    void* key;
    int ptr;
    int offsetnew;
    int keyCount;
    key = data+offset;

    if(!key_comparison(keyType,newkey,key))
    {
        //insert at the end
        offset = offset + keyLength+ sizeof(int);
    }
    else
    {
        //move data until the new key sorted
        do{
            offsetnew = offset;
            key = data+offsetnew;
            offsetnew += keyLength;
            memcpy(&ptr, data+offsetnew, sizeof(int));
            offsetnew += sizeof(int);
            memcpy(data+offsetnew, key, keyLength);
            offsetnew += keyLength;
            memcpy(data+offsetnew, &ptr, sizeof(int));

            offset = offset - keyLength - sizeof(int);
            key = data+offset;

        }while(key_comparison(keyType,newkey,key));

        offset = offset + keyLength + sizeof(int);
    }
    memcpy(data+offset, newkey, keyLength);
    offset += keyLength;
    memcpy(data+offset, &newptr, sizeof(int));

    memcpy(&keyCount ,data+ sizeof(char), sizeof(int));
    keyCount++;
    memcpy(data+ sizeof(char), &keyCount, sizeof(int));
}

void indexSplit(char* data, char* newdata, int offset, int keyCount, void* newkey, int newptr, char keyType, int keyLength, int c)
{
    //Moves data to the new index block
    void* key;
    int ptr;
    int offsetnew=0;
    int i, count, newcount;
    int f = 0;
    memcpy(&count, data+ sizeof(char), sizeof(int));
    newcount=0;
    char newtype='i';

    memcpy(newdata+offsetnew,&newtype, sizeof(char));
    offsetnew += sizeof(char);
    memcpy(newdata+offsetnew,&newcount, sizeof(int));
    offsetnew += sizeof(int);

    offsetnew += sizeof(int);

    printf("%d  %d\n", offset, offsetnew);
    int loops = keyCount/2;
    if(keyCount%2 == 0)
    {
        if (c==3) loops--; //CASE 3
    }
    else
    {
        if(c==1 || c==2) loops++; //CASE 1 and 2
    }
    for(i=0; i<loops; i++)
    {
        key = data+offset;

        if(newkey!=NULL)
        {
            //If there is a new key to be added to the new block it is added at the right place
            if(key_comparison(keyType, newkey, key))
            {
                memcpy(newdata+offsetnew,newkey, keyLength);
                offsetnew+= keyLength;
                memcpy(newdata+offsetnew,&newptr, sizeof(int));
                offsetnew += sizeof(int);
                newcount++;
                memcpy(newdata+ sizeof(char), &newcount, sizeof(int));
                newkey = NULL;
                f=1;
            }
        }
        //Data transfer to the new index block
        memcpy(newdata+offsetnew,key, keyLength);
        memset(data+offset,0,keyLength);
        offset+= keyLength;
        offsetnew+= keyLength;
        memcpy(&ptr,data+offset, sizeof(int));
        memcpy(newdata+offsetnew,&ptr, sizeof(int));

        memset(data+offset,0, sizeof(int));
        offset += sizeof(int);
        offsetnew += sizeof(int);

        count--;
        newcount++;
        memcpy(data+ sizeof(char),&count, sizeof(int));
        memcpy(newdata+ sizeof(char), &newcount, sizeof(int));
    }
    if (c==1 || c==3)
    {
        // decrease key count by one more because of the key that leaves the block and climbs the index
        count--;
        memcpy(data+ sizeof(char),&count, sizeof(int));
    }

    if(newkey!=NULL)
    {
        if(!f)
        {
            //If the new key hasn't been added yet its bigger than all the others and it's added at the end
            memcpy(newdata+offsetnew,newkey, keyLength);
            offsetnew+= keyLength;
            memcpy(newdata+offsetnew,&newptr, sizeof(int));
            offsetnew += sizeof(int);
            newcount++;
            memcpy(newdata+ sizeof(char), &newcount, sizeof(int));
        }
    }
}

void AM_Init() {
    BF_Init(LRU);

    if(!open_files) {
        open_files = malloc(AM_MAX_OPEN_FILES * sizeof(file_info));
    }
    return;
}


int AM_CreateIndex(char *fileName, 
	               char attrType1, 
	               int attrLength1, 
	               char attrType2, 
	               int attrLength2) {
    int fd;
    char* data;
    BF_Block *metadatablock;
    BF_Block_Init(&metadatablock);
    int offset = 0;
    int root_block_num = -1;

    //Created file and adds the block with the metadata

    if (BF_CreateFile(fileName) != BF_OK) return AME_EOF;
    BF_OpenFile(fileName, &fd);
    BF_AllocateBlock(fd, metadatablock);
    data = BF_Block_GetData(metadatablock);
    memcpy(data, "AM_index", sizeof("AM_index"));
    offset += sizeof("AM_index");
    memcpy(data+offset, &attrType1, sizeof(attrType1));
    offset += sizeof(attrType1);
    memcpy(data+offset, &attrLength1, sizeof(attrLength1));
    offset += sizeof(attrLength1);
    memcpy(data+offset, &attrType2, sizeof(attrType2));
    offset += sizeof(attrType2);
    memcpy(data+offset, &attrLength2, sizeof(attrLength2));
    offset += sizeof(attrLength2);
    memcpy(data+offset, &root_block_num, sizeof(root_block_num));

    BF_Block_SetDirty(metadatablock);
    BF_UnpinBlock(metadatablock);
    BF_CloseFile(fd);
  return AME_OK;
}


int AM_DestroyIndex(char *fileName) {
    int fd;
    int i = 0;
    if (BF_OpenFile(fileName,&fd) != BF_OK) return AME_EOF;
    BF_CloseFile(fd);
    int f = 1;
    while (f == 1 && i<AM_MAX_OPEN_FILES)
    {
        if(open_files+i != NULL)
        {
            if(open_files[i].file_desc == fd)
                f = 0;
        }
        i++;
    }
    if(f)
        remove(fileName);
    else
        return AM_OPEN_FILE_DISTRUCTION_ERROR;

  return AME_OK;
}


int AM_OpenIndex (char *fileName) {
    int fd;
    char* data;
    BF_Block *block;
    BF_Block_Init(&block);
    int offset = 0;
    file_info fi;

    if (BF_OpenFile(fileName,&fd) != BF_OK) return AME_EOF;

    BF_GetBlock(fd, 0, block);
    data = BF_Block_GetData(block);
    BF_UnpinBlock(block);

    char* ft;
    char at1, at2;
    int al1, al2, rbn;
    ft = data;
    offset += 9;
    memcpy(&at1, data+offset, 1);
    offset += 1;
    memcpy(&al1, data+offset, 4);
    offset += 4;
    memcpy(&at2, data+offset, 1);
    offset += 1;
    memcpy(&al2, data+offset, 4);
    offset += 4;
    memcpy(&rbn, data+offset, 4);

    file_info_Init(&fi, ft, fd, at1, al1, at2, al2, rbn);

    return add_open_file(fi);

  //return AME_OK;
}


int AM_CloseIndex (int fileDesc) {

    int fd;
    if(open_files[fileDesc].file_type != NULL)
    {
        fd = open_files[fileDesc].file_desc;

        if (BF_CloseFile(fd) != BF_OK) return AM_BF_LEVEL_ERROR;
        return delete_open_file(fileDesc);
    }
    else
        return AM_INVALID_FILE_ERROR;
    //    return AME_OK;
}


int AM_InsertEntry(int fileDesc, void *value1, void *value2) {

    char *data;
    char *newdata;
    int blocks_num;
    int block_num;
    int newRootNum = -1;
    char type;
    int keyCount;
    int next_data_block;
    int offset;
    int offsetnew;
    int endFlag;
    int i;
    int prevBlock;
    int blockNum;
    int newBlockNum;
    void* key;
    void* value;
    void* climbingKey;
    void* newclimbingKey;

    int fd = open_files[fileDesc].file_desc;


    Stack trail;
    stackInit(&trail, 10);

    BF_Block *block;
    BF_Block_Init(&block);

    BF_Block *newblock;
    BF_Block_Init(&newblock);
    BF_GetBlockCounter(fd, &blocks_num);

    char keyType = open_files[fileDesc].attrType1;
    int keyLength = open_files[fileDesc].attrLength1;
    char valueType = open_files[fileDesc].attrType2;
    int valueLength = open_files[fileDesc].attrLength2;

    //find root
    int nextBlock = open_files[fileDesc].root_block_num;

    //determine how many records can fit
    int max_records_in_data_blocks = ( BF_BLOCK_SIZE - sizeof(char) - 2*sizeof(int) ) /(keyLength + valueLength);
    int max_recrods_in_index_blocks = ( BF_BLOCK_SIZE - sizeof(char) - 2*sizeof(int) ) / (keyLength + sizeof(int));

    void* lol;
    char *lol1;
    int lol2;
    float lol3;

    if (blocks_num == 1)
    {
        if (BF_AllocateBlock(fd, block) != BF_OK) return AM_BF_LEVEL_ERROR;
        data = BF_Block_GetData(block);
        type = 'd';
        keyCount = 1;
        next_data_block = -1;
        memcpy(data, &type, sizeof(char));
        offset = sizeof(char);
        memcpy(data+offset, &keyCount, sizeof(int));
        offset += sizeof(int);
        memcpy(data+offset, &next_data_block, sizeof(int)); // next data block pointer/block num
        offset += sizeof(int);
        memcpy(data+offset, value1, keyLength);
        offset += keyLength;
        memcpy(data+offset, value2, valueLength);

        BF_GetBlockCounter(fd, &blocks_num);
        block_num = blocks_num - 1;
        newRootNum = block_num; //The root has changed

        BF_Block_SetDirty(block);
        BF_UnpinBlock(block);
    }
    else if(blocks_num>1) {
        do {
            BF_GetBlock(fd, nextBlock, block);
            data = BF_Block_GetData(block);

            offset = 0;
            keyCount = 0;
            memcpy(&type, data, sizeof(char));
            offset += sizeof(char);      //Get type + move pointer
            memcpy(&keyCount, data+offset, sizeof(int));
            offset += sizeof(int);    //Get keyCount + move pointer

            if (type == 'd')
            {
                blockNum = nextBlock;
                break;                     //If we reach a data block break.
            }

            stackPush(&trail, nextBlock);
            endFlag = 0;
            i = 1;
            data = data+offset;
            while (i <= keyCount && endFlag == 0) {
                key = data + sizeof(int);
                if (key_comparison(keyType, value1, key)) {
                    endFlag = 1;
                    break;
                }
                data = data + sizeof(int);         //skip the preceding 'pointer'
                data = data + keyLength;           // Compare keys and move pointer
                ++i;
            }

            // Parsing tree
            if (endFlag == 1) {
                //go left
                memcpy(&nextBlock, data, sizeof(int));
            } else {
                //go right
                // This case only takes place when value1 is greater than all the keys in a block.
                memcpy(&nextBlock, data, sizeof(int));
            }

            BF_UnpinBlock(block);
        } while (type == 'i');
        /* Now that it's finished. Which it did using that "if(type == 'd') statement */
        /* our block is pointing to the desired data block and so is our data.  */



        BF_GetBlockCounter(fd, &blocks_num);

        //is there enough space?
        if(keyCount < max_records_in_data_blocks)
        {
            //yes
            //find last key
            offset = sizeof(char) + 2*sizeof(int) + ((keyCount-1)*(keyLength+valueLength));

            key = data+offset;

            if(!key_comparison(keyType,value1,key))
            {
                //add at the end
                offset = offset + keyLength+valueLength;

                memcpy(data+offset, value1, keyLength);
                offset += keyLength;
                memcpy(data+offset, value2, valueLength);
            }
            else
            {
                //the new key is lesser than the last key so it needs to be sorted
                do{
                    offsetnew = offset;
                    key = data+offsetnew;
                    offsetnew += keyLength;
                    value = data+offsetnew;
                    offsetnew += valueLength;
                    memcpy(data+offsetnew, key, keyLength);
                    offsetnew += keyLength;
                    memcpy(data+offsetnew, value, valueLength);

                    offset = offset - keyLength - valueLength;
                    key = data+offset;

                }while(key_comparison(keyType,value1,key));
                offset = offset + keyLength+valueLength;
                memcpy(data+offset, value1, keyLength);
                offset += keyLength;
                memcpy(data+offset, value2, valueLength);
            }
            keyCount++;
            memcpy(data+ sizeof(char), &keyCount, sizeof(int));


        }else {
            //no
            /* Split Data Block */
            /* We've got 'type' and 'keycount' as well as 'data' and 'block' for block1 already */
            /* Second Block */
            BF_Block *block2;                            /* Second block */
            BF_Block_Init(&block2);                        /* gets initialized. */
            BF_AllocateBlock(fd, block2);            /* Allocate block */
            char *data2;                                /* Pointer on */
            data2 = BF_Block_GetData(block2);            /* block's data. */
            int blockNum2;
            BF_GetBlockCounter(fd, &blockNum2);
            --blockNum2;
            char *data2Save;
            char *dataSave;
            dataSave = data;
            data2Save = data2;

            /*  PROCESS EXPLANATION
                What we do now is: Find the middle key of the block.
                Is the key to be inserted lesser?
                  -Copy all keys including the middle one to the second block and as for the
                   first block move all key-value combos till the new key is in it's proper place
                  Is it greater?
                  -Do stuff.
            */

            /* Now let's find this infamous middle key. */
            /* data and data2 are both pointing at the very start of their own blocks */
            void *middleKey;
            int middle = (keyCount - 1) / 2;
            middleKey = data + sizeof(char) + 2 * sizeof(int) + middle * (keyLength + valueLength);
            /* Block2 init */
            void *dS;
            dS = data;
            memcpy(data2, data, sizeof(char));
            data2 = (void *) (data2 + sizeof(char)); /* Copy type from block1                 */
            int x;
            x = keyCount -
                (keyCount - 1) / 2;                                          /* Calculate block2's keyCount		    */
            memcpy(data2, &x, sizeof(int));
            data2 = (void *) (data2 + sizeof(int));  /* and add it.              			    */
            int y;
            y = keyCount + 1 - x;
            memcpy((void *) (data + sizeof(char)), &y, sizeof(int));
            data = (void *) (data + sizeof(char) +
                             sizeof(int));            /* Skipping type and keyCount for block1 			*/
            memcpy(data2, data, sizeof(int));
            data2 = (void *) (data2 + sizeof(int)); /* and copying its 'next block' content  			*/
            memcpy(data, &blockNum2,
                   sizeof(int));                    /* now make sure to add second block's number to the firstblock */


            /* data2 is ready to copy so we need to get data up to speed */
            data = (void *) (data + sizeof(int) + middle * (keyLength +
                                                            valueLength)); /* Works for case 1, case 2 needs an additional key-value skip*/

            /* Now let's compare it with our 'key to be inserted' */
            if (key_comparison(keyType, value1, middleKey)) {
                /* Let's start copying */
                int i;
                for (i = middle; i < keyCount; ++i) {
                    memcpy(data2, data, (keyLength + valueLength));
                    memset(data, 0, (keyLength + valueLength)); //QUESTION which value is a good initializer?
                    data2 = (void *) (data2 + (keyLength + valueLength));
                    data = (void *) (data + (keyLength + valueLength));
                }

                /* Return first block's pointer to the correct address and begin */
                /* Calculating new keyCount for block1 */
                data = (void *) (data - (keyCount - middle) * (keyLength + valueLength));
                keyCount = middle;
                if (keyCount == 0) {
                    memcpy(data, value1, keyLength);
                    memcpy(data + keyLength, value2, valueLength);
                } else if (keyCount > 0) {
                    /* Explanation */
                    int j = 0;
                    do {
                        if (key_comparison(keyType, value1, (void *) (data - (keyLength + valueLength)))) {
                            memmove(data, (void *) (data - (keyLength + valueLength)), (keyLength + valueLength));
                            data = (void *) (data - (keyLength + valueLength));
                        } else {
                            break;
                        }
                        ++j;
                    } while (j < keyCount);
                    memcpy(data, value1, keyLength);
                    memcpy((void *) (data + keyLength), value2, valueLength);
                } else {

                }
            } else {
                /* Previous block keeps all key/value combos including the middlekey     */
                /* and the rest go to the next block including the 'key to be inserted'  */
                int doneDeal = 0;
                int j = middle;
                do {
                    data = (void *) (data + (keyLength + valueLength));
                    if (key_comparison(keyType, value1, (void *) (data + (keyLength + valueLength)))) {
                        memcpy(data2, value1, keyLength);
                        memcpy((void *) (data2 + keyLength), value2, valueLength);
                        data2 = (void *) (data2 + (keyLength + valueLength));
                        doneDeal = 1;
                    }
                    memcpy(data2, data, (keyLength + valueLength));
                    memset(data, 0, (keyLength + valueLength)); //QUESTION which value is a good initializer?
                    data2 = (void *) (data2 + (keyLength + valueLength));
                    ++j;
                } while (j < keyCount - 1);
                if (doneDeal == 0) {
                    memcpy(data2, value1, keyLength);
                    memcpy((void *) (data2 + keyLength), value2, valueLength);
                    data2 = (void *) (data2 + (keyLength + valueLength));
                }
            }

            BF_Block_SetDirty(block);
            BF_Block_SetDirty(block2);
            BF_UnpinBlock(block2);
            BF_UnpinBlock(block);


            BF_GetBlock(fd, blockNum2, block);
            data = BF_Block_GetData(block);
            climbingKey = data + sizeof(char) + 2 * sizeof(int);
            BF_UnpinBlock(block);

            endFlag = 1;
            int lastptr;
            newBlockNum = blockNum2;
            BF_GetBlockCounter(fd, &blocks_num);

            if (blocks_num == 3) {
                //There is no Index Block, only two Data blocks, so one needs to be created
                block_num = createIndexBlock(fd, blockNum, newBlockNum, climbingKey, keyLength);
                if (block_num <= 0) return AM_INDEX_CREATION_ERROR;

                newRootNum = block_num; //The root has changed
            } else {
                do {
                    //Follow the trail to the root block by block
                    prevBlock = stackPop(&trail);

                    BF_GetBlock(fd, prevBlock, block);
                    data = BF_Block_GetData(block);
                    memcpy(&type, data, sizeof(char));
                    offset = sizeof(type);
                    memcpy(&keyCount, data + offset, sizeof(int));
                    offset += sizeof(keyCount);

                    if (keyCount < max_recrods_in_index_blocks) {
                        //There is enough space in the index block for another key
                        offset = offset + (keyCount - 1) * keyLength + keyCount * sizeof(int);
                        indexAddandSort(data, offset, climbingKey, newBlockNum, keyType, keyLength);

                        endFlag = 0; //stop the search, an idex block of the trail with enough space has been found
                    }
                    else {
                        //There is not enough space
                        //find the previous of the middle key
                        offset = offset + ((keyCount / 2) - 1 ) * keyLength + (keyCount / 2) * sizeof(int);

                        key = data + offset;

                        if (BF_AllocateBlock(fd, newblock) != BF_OK) return AM_BF_LEVEL_ERROR;
                        newdata = BF_Block_GetData(newblock);
                        BF_GetBlockCounter(fd, &blocks_num);

                        char* cclimbingKey;
                        int iclimbingKey;
                        float fclimbingKey;
                        if (key_comparison(keyType, climbingKey, key)) {

                            switch (keyType)
                            {
                                case 'c':
                                    cclimbingKey = (char *) key;
                                case 'i':
                                    iclimbingKey = (*(int*)key);
                                case 'f':
                                    fclimbingKey = (*(float*)key);
                            }

                            //memset(data + offset, 0, keyLength);


                            offset += keyLength;
                            memcpy(&lastptr, data+offset, sizeof(int));
                            memcpy(newdata + sizeof(char) + sizeof(int), &lastptr, sizeof(int));
                            //memset(data+offset, 0, sizeof(int));

                            offset += sizeof(int);
                            indexSplit(data, newdata, offset, keyCount, NULL, newBlockNum, keyType, keyLength, 1);

                            memcpy(&keyCount, data + sizeof(char), sizeof(int));
                            offset = sizeof(char) + sizeof(int) + (keyCount - 1) * keyLength + keyCount * sizeof(int);
                            indexAddandSort(data, offset, climbingKey, newBlockNum, keyType, keyLength);
                        } else {
                            offset = offset + keyLength + sizeof(int);
                            printf("%d \n", offset);
                            key = data+offset;

                            if (key_comparison(keyType, climbingKey, key)) {

                                switch (keyType)
                                {
                                    case 'c':
                                        cclimbingKey = (char *) climbingKey;
                                    case 'i':
                                        iclimbingKey = (*(int*)climbingKey);
                                    case 'f':
                                        fclimbingKey = (*(float*)climbingKey);
                                }
                                memcpy(newdata + sizeof(char) + sizeof(int), &newBlockNum, sizeof(int));

                                indexSplit(data, newdata, offset, keyCount, NULL, newBlockNum, keyType, keyLength, 2);
                            } else {

                                switch (keyType)
                                {
                                    case 'c':
                                        cclimbingKey = (char *) key;
                                    case 'i':
                                        iclimbingKey = (*(int*)key);
                                        printf("iclimbingkey %d \n", iclimbingKey);
                                    case 'f':
                                        fclimbingKey = (*(float*)key);
                                }

                                if(keyType == 'i') printf("%d \n", (*(int*)key));

                                //memset(data+offset, 0, keyLength);
                                offset += keyLength;
                                memcpy(&lastptr, data+offset, sizeof(int));
                                //memset(data+offset, 0, sizeof(int));
                                memcpy(newdata + sizeof(char) + sizeof(int), &lastptr, sizeof(int));

                                offset += sizeof(int);
                                indexSplit(data, newdata, offset, keyCount, climbingKey, newBlockNum, keyType, keyLength, 3);
                            }
                        }

                        newBlockNum = blocks_num - 1;

                        switch (keyType)
                        {
                            case 'c':
                                climbingKey = cclimbingKey;
                            case 'i':
                                (*(int*)climbingKey)= iclimbingKey;
                            case 'f':
                                (*(float*)climbingKey)= fclimbingKey;
                        }


                        BF_Block_SetDirty(newblock);
                        BF_UnpinBlock(newblock);
                    }

                    BF_Block_SetDirty(block);
                    BF_UnpinBlock(block);
                } while (!stackEmpty(&trail) && endFlag);

                if (endFlag) {
                    //The root has been splitted so a new one must be created

                    block_num = createIndexBlock(fd, prevBlock, newBlockNum, climbingKey, keyLength);
                    if (block_num <= 0) return AM_INDEX_CREATION_ERROR;

                    newRootNum = block_num;
                }
            }
        }
    }

    

    if(newRootNum != -1)
    {
        //Change the root on the metadata and open files array
        BF_GetBlock(fd, 0, block);
        data = BF_Block_GetData(block);
        memcpy(data+19, &newRootNum, sizeof(int));

        BF_Block_SetDirty(block);
        BF_UnpinBlock(block);

        open_files[fileDesc].root_block_num = newRootNum;
    }

    stackDestroy(&trail);
    BF_Block_Destroy(&block);
    //BF_Block_Destroy(&newblock);

    return AME_OK;
}




void printer(int fileDesc){
    int fd = open_files[fileDesc].file_desc;
    printf("\nWEVEMADEITFELLAS\n");
    BF_Block *block;
    BF_Block_Init(&block);
    char *data;
    char type;
    int offset, keyCount,left, keyLength;
    keyLength = open_files[fileDesc].attrLength1;
    /* Printer */
    BF_GetBlock(fd, 0, block);
    data = BF_Block_GetData(block);
    data = (void*)(data + sizeof("AM_index"));
    data = (void*)(data + 2*sizeof(char) + 2*sizeof(int));
    int root;
    memcpy(&root,data,sizeof(int));
    printf("Root:%d\n",root);
    BF_GetBlock(fd, root, block);
    data = BF_Block_GetData(block);
    memcpy(&type, data, sizeof(char));
    printf("%c/ ", type);
    offset = sizeof(char);
    memcpy(&keyCount, data + offset, sizeof(int));
    printf("%d/ ", keyCount);
    offset += sizeof(int);
    memcpy(&left, data + offset, sizeof(int));
    printf("%d/ ", left);
    offset += sizeof(int);
    int i,right;
    data = (void*)(data + offset);

    for (i = 0; i < keyCount; i++) {
        printf("%f/ ", *((float *) data));
        data = (void*)(data + keyLength);
        memcpy(&right, data, sizeof(int));
        printf("%d/ ", right);
        data = (void*)(data + sizeof(int));
    }
/*
    printf("\n\n\n\n");
    dataprinter( fileDesc,  left);
printf("\n\n\n\n");
    dataprinter( fileDesc,  right);
printf("\n\n\n\n");
*/
}

void dataprinter(int fileDesc, int blockNum){
    int fd = open_files[fileDesc].file_desc;
    BF_Block *block;
    BF_Block_Init(&block);
    char *data;
    BF_GetBlock(fd, blockNum, block);
    data = BF_Block_GetData(block);

    int type, keyCount, keyLength, valueLength, next, offset;
    keyLength = open_files[fileDesc].attrLength1;
    valueLength = open_files[fileDesc].attrLength2;
    memcpy(&type, data, sizeof(char));
    printf("%c/ ", type);
    offset = sizeof(char);
    memcpy(&keyCount, (void*)(data + offset), sizeof(int));
    printf("%d/ ", keyCount);
    offset += sizeof(int);
    memcpy(&next, (void*)(data + offset), sizeof(int));
    printf("%d/ ", next);
    offset += sizeof(int);

    int i;
    data = (void*)(data+offset);
    for (i = 0; i < keyCount; i++) {
        printf("%f/ ", *((float *) data));
        data = (void*)(data + keyLength);
        printf("%s/ ", data);
        data = (void*)(data + valueLength);
    }
}




int AM_OpenIndexScan(int fileDesc, int op, void *compKey) {
    // Part 1: Checks
    // 1. Is there enough space to scan?
    if(num_of_scans>=AM_MAX_FILE_SCANS){
        return AM_OPEN_SCAN_LIMIT_ERROR; }
    // 2. Does such a file exist?
    int fd = open_files[fileDesc].file_desc;
    // 3. Find a spot to place the new scan
    int spot = -1;
    int found = 0;
    while(spot < AM_MAX_FILE_SCANS && found == 0){
        ++spot;
        if(curScans[spot].fd == NULL)
        {
            found = 1;
            ++num_of_scans;
        }
    }
    if(found == 0)
        return AM_ABNORMAL_ERROR;

    //Part 2: Scanning Process
    /* Initialization */
    int root = open_files[fileDesc].root_block_num;
    char type;
    char keyType = open_files[fileDesc].attrType1;
    int keyLength = open_files[fileDesc].attrLength1;
    int valueLength = open_files[fileDesc].attrLength2;
    int dataBlockNum,nBlock;
    nBlock = root;
    printf("\nROOT:%d\nnBlock:%d\n", root, nBlock);
    BF_Block *block;
    BF_Block_Init(&block);
    char *data;
    int endFlag;
    //Start scanning
    Scan newScan;
    int offset, keyCount;
    void *key;
    if( op == 1 || op == 4 || op == 6){
        do {
            BF_GetBlock(fd, nBlock, block);
            data = BF_Block_GetData(block);
            offset = 0;
            keyCount = 0;
            memcpy(&type, data, sizeof(char)); data = (void*)(data + sizeof(char));
            memcpy(&keyCount, data, sizeof(int)); data = (void*)(data + sizeof(int));
            if (type == 'd'){
                data = (void*)(data - sizeof(char) - sizeof(int) );
                dataBlockNum = nBlock;
                break;                     //If we reach a data block break.
            }
            endFlag = 0;
            int i = 1;
            //	data = (void*)(data + sizeof(int));
            while (i <= keyCount && endFlag == 0) {
                if (key_comparison(keyType, compKey, (void*)(data+sizeof(int))) || key_equal(keyType, compKey, (void*)(data+sizeof(int)))) {
                    //data = data + sizeof(keyLength);
                    endFlag = 1;
                    break;
                }
                data = data + sizeof(int);         //skip the preceding 'pointer'
                data = data + keyLength;           // Compare keys and move pointer
                ++i;
            }
            // Parsing tree
            if (endFlag == 1) {
                //go left
                memcpy(&nBlock, data, sizeof(int));
                printf("nBlock:%d\n",nBlock);
            } else {
                //go right
                // This case only takes place when value1 is greater than all the keys in a block.
                //data = data + sizeof(int) + sizeof(keyLength);
                memcpy(&nBlock, data, sizeof(int));
                printf("nBlock:%d\n",nBlock);
            }

            BF_UnpinBlock(block);
        } while (type == 'i');
    }else if( op == 2 || op == 3 || op == 5){
        do{
            BF_GetBlock(fd, nBlock, block);
            data = BF_Block_GetData(block);
            memcpy(&type, data, sizeof(char)); data = (void *)(data + sizeof(char));  //Get type + move pointer.
            data = (void *)(data + sizeof(int)); 					  //Skip keyCount / move pointer
            if(type == 'd'){							  //If we reach a data block breal.
                data = (void*)(data - sizeof(char) - sizeof(int));		  //Return pointer to the start of the block
                dataBlockNum = nBlock;
                break;
            }
            memcpy(&nBlock, data, sizeof(int));
        }while(type== 'i');
        /*
        if(keyType=='f'){
            BF_Block *a;
            BF_Block_Init(&a);
            char *q;
            BF_GetBlock(fd,nBlock,a);
            q = BF_Block_GetData(a);
            int keyCount, nextBlock;
            char type;
            memcpy(&type, q, sizeof(char)); q = (void*)(q+sizeof(char));
            memcpy(&keyCount, q, sizeof(int)); q = (void*)(q+sizeof(int));
            memcpy(&nextBlock, q, sizeof(int)); q = (void*)(q+sizeof(int));
            printf("%c/ %d/ %d/ ", type,keyCount, nextBlock);

            int i;
            for (i = 0; i < keyCount; i++) {
                printf("%s/ ", ((char *)q)); q = q + keyLength;
                printf("%d/ ", *((int *)q)); q = q + valueLength;
            }
        }*/
    }else{
        //TODO error
    }

    data = (void *)(data + sizeof(char));		/* Skip type */
    int nextBlock;
    memcpy(&keyCount, data,sizeof(int));    data = data + sizeof(int);
    memcpy(&nextBlock, data, sizeof(int));  data = data + sizeof(int);

    newScan.completed = 0;
    newScan.offset = 0;
    newScan.fileDesc = fileDesc;
    newScan.fd = fd;
    newScan.blockNum = dataBlockNum;
    newScan.availableKeysInCurBlock = keyCount;
    newScan.nextBlock = nextBlock;
    newScan.op = op;
    newScan.compKey = compKey;
    /*********Calculate offset for operations that don't parse all  the way to the leftmost data block ***************/
    if(op==4||op==6){
        int flag = 0;
        int j = 0;
        while( j < keyCount){
            if(op==4){
                if(key_comparison(keyType, data, compKey) || key_equal(keyType,data,compKey) ){
                    newScan.offset += open_files[fileDesc].attrLength1 + open_files[fileDesc].attrLength2;
                    data = (void*)(data + open_files[fileDesc].attrLength1 + open_files[fileDesc].attrLength2);
                    --newScan.availableKeysInCurBlock;
                }else{
                    flag = 1;
                    break;
                }
            }else{
                if(key_comparison(keyType, data, compKey) ){
                    newScan.offset += open_files[fileDesc].attrLength1 + open_files[fileDesc].attrLength2;
                    data = (void*)(data + open_files[fileDesc].attrLength1 + open_files[fileDesc].attrLength2);
                    --newScan.availableKeysInCurBlock;
                }else{
                    flag = 1;
                    break;
                }
            }
            ++j;
        }
        if(newScan.availableKeysInCurBlock==0){
            if( newScan.nextBlock < 0 ){
                newScan.completed = 1;
            }else{
                /* Just send the next block and all the values there will be greater */
                BF_GetBlock(fd, nextBlock, block);
                data = BF_Block_GetData(block);
                data = data + sizeof(char);		/* Skip type */
                memcpy(&keyCount, data,sizeof(int));    data = data + sizeof(int);
                memcpy(&nextBlock, data, sizeof(int));  data = data + sizeof(int);
                newScan.offset = 0;
                newScan.availableKeysInCurBlock = keyCount;
                newScan.nextBlock = nextBlock;
            }
        }
    }else if(op==1){
        int j = 0;
        while( j < keyCount){
            if(key_equal(keyType, data, compKey)){
                break;
            }else if(key_comparison(keyType, data, compKey)){
                newScan.offset += keyLength + valueLength;
                data = (void*)(data + keyLength + valueLength);
                --newScan.availableKeysInCurBlock;
            }else{
                newScan.completed = 1;
                break;
            }
            ++j;
        }
        if(newScan.availableKeysInCurBlock==0){
            newScan.completed = 1;
        }
    }
    curScans[spot] = newScan;
    return spot;
}


void *AM_FindNextEntry(int scanDesc) {
    if (curScans[scanDesc].completed){
        AM_errno = AME_EOF;
        return NULL;
    }
    /* Initialization */
    BF_Block *block;
    BF_Block_Init(&block);
    char *data;
    int fileDesc = curScans[scanDesc].fileDesc;
    int fd = curScans[scanDesc].fd;
    int dataBlock = curScans[scanDesc].blockNum;
    int nextBlock = curScans[scanDesc].nextBlock;
    BF_GetBlock( fd, dataBlock, block);
    data = BF_Block_GetData(block);
    data = data + sizeof(char) + 2*sizeof(int);
    int op = curScans[scanDesc].op;
    void *compKey = curScans[scanDesc].compKey;

    /* Process */
    if(curScans[scanDesc].availableKeysInCurBlock <= 0){
        if(curScans[scanDesc].nextBlock <= 0){
            AM_errno = AME_EOF;
            return NULL;
        }else{
            BF_GetBlock( fd, nextBlock, block);
            data = BF_Block_GetData(block);
            data = data + sizeof(char);
            memcpy(&curScans[scanDesc].availableKeysInCurBlock, data,sizeof(int)); data = data + sizeof(int);
            memcpy(&curScans[scanDesc].nextBlock, data, sizeof(int)); data = data + sizeof(int);

            curScans[scanDesc].offset = 0;
        }
    }
    /****************************************************************************/

    int keyLength, valueLength;
    keyLength = open_files[fileDesc].attrLength1;
    valueLength = open_files[fileDesc].attrLength2;
    int type = open_files[fileDesc].attrType1;

    data = (void*)(data + curScans[scanDesc].offset);
    switch(op)
    {
        case 1:
            if(key_equal(type,compKey,data)){
                curScans[scanDesc].offset += (keyLength + valueLength);
                --curScans[scanDesc].availableKeysInCurBlock;
                return (void*)(data+keyLength);
            }else{
                curScans[scanDesc].completed = 1;
                AM_errno = AME_EOF;
                return NULL;
            }
            break;
        case 2:
            if(!key_equal(type,data,compKey)){
                curScans[scanDesc].offset += (keyLength + valueLength);
                --curScans[scanDesc].availableKeysInCurBlock;
                return (void*)(data+keyLength);
            }else{
                int i, keyCount;
                i = 0;
                keyCount = curScans[scanDesc].availableKeysInCurBlock;
                while(i < keyCount){
                    if(key_equal(type,data,compKey)){
                        data = (void*)(data + (keyLength + valueLength));
                        curScans[scanDesc].offset += (keyLength + valueLength);
                        --curScans[scanDesc].availableKeysInCurBlock;
                    }else{
                        curScans[scanDesc].offset += (keyLength + valueLength);
                        --curScans[scanDesc].availableKeysInCurBlock;
                        return (void*)(data+keyLength);
                    }
                    ++i;
                }
                if(curScans[scanDesc].availableKeysInCurBlock == 0){
                    if(curScans[scanDesc].nextBlock <= 0 ){
                        curScans[scanDesc].completed = 1;
                        AM_errno = AME_EOF;
                        return NULL;
                    }else{
                        BF_GetBlock( fd, curScans[scanDesc].nextBlock, block);
                        data = BF_Block_GetData(block);
                        data = data + sizeof(char); /* skip type */
                        memcpy(&curScans[scanDesc].availableKeysInCurBlock, data,sizeof(int));  data = (void*)(data + sizeof(int));
                        memcpy(&curScans[scanDesc].nextBlock, data, sizeof(int)); 				data = (void*)(data + sizeof(int));
                        curScans[scanDesc].offset = (keyLength + valueLength); /* = and not += in order to reset it */
                        --curScans[scanDesc].availableKeysInCurBlock;
                        return (void*)(data+keyLength);
                    }
                }
            }
            break;
        case 3:
            if(key_comparison(type,data,compKey)){
                curScans[scanDesc].offset += (keyLength + valueLength);
                --curScans[scanDesc].availableKeysInCurBlock;
                return (void*)(data+keyLength);
            }else{
                curScans[scanDesc].completed = 1;
                AM_errno = AME_EOF;
                return NULL;
            }
            break;
        case 4:
            if(key_comparison(type,compKey,data)){
                curScans[scanDesc].offset += (keyLength + valueLength);
                --curScans[scanDesc].availableKeysInCurBlock;
                return (void*)(data+keyLength);
            }else{
                curScans[scanDesc].completed = 1;
                AM_errno = AME_EOF;
                return NULL;
            }
            break;
        case 5:
            if( key_comparison(type, data, compKey ) || key_equal(type,compKey,data) ){
                curScans[scanDesc].offset += (keyLength + valueLength);
                --curScans[scanDesc].availableKeysInCurBlock;
                return (void*)(data+keyLength);
            }else{
                curScans[scanDesc].completed = 1;
                AM_errno = AME_EOF;
                return NULL;
            }
            break;
        case 6:
            if( key_comparison(type, compKey, data ) || key_equal(type,compKey,data) ){
                curScans[scanDesc].offset += (keyLength + valueLength);
                --curScans[scanDesc].availableKeysInCurBlock;
                return (void*)(data+keyLength);
            }else{
                curScans[scanDesc].completed = 1;
                AM_errno = AME_EOF;
                return NULL;
            }
            break;
    }
}


int AM_CloseIndexScan(int scanDesc) {


  return AME_OK;
}


void AM_PrintError(char *errString) {
  
}

void AM_Close() {
    //BF_Close();
    //free(open_files);
}
