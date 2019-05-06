#include <stdio.h>
#include <string.h>
#include "step1_functions.h"

int record_cmp(Record* record, Record* pivot, int fieldNo)
{
    switch(fieldNo){
        case 0:
            if(record->id < pivot->id) return 1;
            else return 0;
        case 1:
            if(strcmp(record->name, pivot->name) < 0) return 1;
            else return 0;
        case 2:
            if(strcmp(record->surname, pivot->surname) < 0) return 1;
            else return 0;
        case 3:
            if(strcmp(record->city, pivot->city) < 0) return 1;
            else return 0;
    }

}

int swap(char* data, char* idata, Record record, int offset, int index_offset)
{
    if(idata == NULL) idata = data;

    memcpy(data + offset, idata + index_offset, sizeof(record.id));
    offset += sizeof(record.id); index_offset += sizeof(record.id);
    memcpy(data + offset, idata + index_offset, sizeof(record.name));
    offset += sizeof(record.name); index_offset += sizeof(record.name);
    memcpy(data + offset, idata + index_offset, sizeof(record.surname));
    offset += sizeof(record.surname); index_offset += sizeof(record.surname);
    memcpy(data + offset, idata + index_offset, sizeof(record.city));

    index_offset = index_offset - sizeof(record.id) - sizeof(record.name)-sizeof(record.surname);
    memcpy(idata + index_offset, &record.id, sizeof(record.id));
    index_offset += sizeof(record.id);
    memcpy(idata + index_offset, record.name, sizeof(record.name));
    index_offset += sizeof(record.name);
    memcpy(idata + index_offset, record.surname, sizeof(record.surname));
    index_offset += sizeof(record.surname);
    memcpy(idata + index_offset, record.city, sizeof(record.city));
}

pair partition(char **data, int first_block, int low, int last_block, int high, int total_blocks, int fieldNo)
{
    pair ipair;
    int records_num;
    int offset, record_offset, i_record_offset;
    Record record;
    Record pivot;

    //Finding the last record of the group and setting it as the pivot to be compered with all the records of the group
    memcpy(&pivot.id, data[last_block] + high, sizeof(pivot.id));
    high += 4;
    memcpy(pivot.name, data[last_block] + high, sizeof(pivot.name));
    high += sizeof(pivot.name);
    memcpy(pivot.surname, data[last_block] + high, sizeof(pivot.surname));
    high += sizeof(pivot.surname);
    memcpy(pivot.city, data[last_block] + high, sizeof(pivot.city));
    high = high - 4 - sizeof(pivot.name) - sizeof(pivot.surname);

    //Setting an index where the the lesser than the pivot records will go
    int index = low - sizeof(Record);
    int index_block = first_block;

    int current_block;
    int records;

    //Checking every block in the group
    for (int i = 0; i < total_blocks; ++i) {
        current_block = first_block+i;
        memcpy(&records_num, data[current_block], 4);
        if (i==0)
            offset = low;
        else
            offset = 4;

        records = records_num;
        if (i == total_blocks-1 && high != sizeof(records_num) + ((records_num-1) * sizeof(Record)))
            //if it is the last block and "high" is not at the last record of the block
            records = ((high-4)/ sizeof(Record));
        if (i == 0 && low != 4)
            //if it is the first block "low" is not at the first record of the block
            records = records - ((low-4)/ sizeof(Record));

        //Checking every record in the block
        for (int j = 0; j < records; ++j) {

            record_offset = offset;
            memcpy(&record.id, data[current_block] + record_offset, sizeof(record.id));
            record_offset += sizeof(record.id);
            memcpy(record.name, data[current_block] + record_offset, sizeof(record.name));
            record_offset += sizeof(record.name);
            memcpy(record.surname, data[current_block] + record_offset, sizeof(record.surname));
            record_offset += sizeof(record.surname);
            memcpy(record.city, data[current_block] + record_offset, sizeof(record.city));

            //Comparing every record with the pivot
            if(record_cmp(&record, &pivot, fieldNo))
            {
                if(index + sizeof(Record) < BF_BLOCK_SIZE)
                {
                    //if there is space in the current block where the index is, move the index to the next record
                    //swap the record we checked with the index record
                    index += sizeof(Record);
                    swap(data[current_block], data[index_block], record, offset, index);
                } else
                {
                    //if there isn't any space left move to the next block make it the index block and index the first record of that block
                    index_block++;
                    index = 4;
                    swap(data[current_block], data[index_block], record, offset, index);
                }
            }
            offset += sizeof(Record);
        }
    }

    //at the end swap the pivot and the index just so all the lesser records are on the left of the pivot and all the greater on the right

    if(last_block == index_block)
    {
        if(index + sizeof(Record) < BF_BLOCK_SIZE)
        {
            index += sizeof(Record);
            swap(data[last_block], NULL, pivot, high, index);
        }

    } else
    {
        if(index + sizeof(Record) < BF_BLOCK_SIZE)
        {
            index += sizeof(Record);
            swap(data[last_block], data[index_block] , pivot, high, index);
        }
        else
        {
            index_block++;
            index = 4;
            swap(data[last_block], data[index_block], pivot, high, index);
        }
    }

    //Return the pair of the block and the offset of the index
    ipair.block_num = index_block;
    ipair.offset = index;
    return ipair;

}

void quickSort(char **data, int first_block, int low, int last_block, int high, int total_blocks, int fieldNo)
{
    //Using a recursive algorithm

    pair pi;
    int first_half_blocks, second_half_blocks;
    int new_first_block, new_last_block, new_low, new_high;

    if((first_block<last_block) || ((first_block==last_block) && (low<high)))
    {
        //Call the partition function
        pi = partition(data, first_block, low, last_block, high, total_blocks, fieldNo);

        //Calling Quicksort for the records before the index (those that are lesser than the index)

        //Making the last(high) record to the one before the index
        if(pi.offset > 4)
        {
            new_high = pi.offset - sizeof(Record);
            new_last_block = pi.block_num;
        }
        else
        {
            new_last_block = pi.block_num -1;
            new_high = BF_BLOCK_SIZE - sizeof(Record);
        }
        first_half_blocks = new_last_block - first_block +1;
        quickSort(data, first_block, low, new_last_block, new_high, first_half_blocks, fieldNo);

        //Calling Quicksort for the records after the index (those that are greater than the index)

        //Making the first(low) record to the one after the index
        if(pi.offset < BF_BLOCK_SIZE - sizeof(Record))
        {
            new_low= pi.offset + sizeof(Record);
            new_first_block = pi.block_num;
        }
        else
        {
            new_first_block = pi.block_num +1;
            new_low = 4;
        }
        second_half_blocks = last_block - new_first_block +1;
        quickSort(data, new_first_block, new_low, last_block, high, second_half_blocks, fieldNo);
    }
}

void printGroup(int temp_fd, int first_block, int last_block)
{
    BF_Block *block;
    BF_Block_Init(&block);
    char* data;
    int newoffset, id, records_num;
    for (int k = 0; k <= last_block-first_block ; ++k) {
        BF_GetBlock(temp_fd, first_block+k, block) != BF_OK;
        data = BF_Block_GetData(block);

        memcpy(&records_num, data,4);
        newoffset = sizeof(int);

        printf("Block Number %d: \n", first_block+k);

        for (int j = 0; j < records_num; ++j)
        {
            memcpy(&id, data+newoffset,4);
            printf("ID: %d \t\t", id);
            printf("Name: %s \t\t", data+newoffset+4);
            printf("Surname: %s \t\t", data+newoffset+19);
            printf("City: %s \n", data+newoffset+39);

            newoffset += sizeof(Record);
        }

        BF_UnpinBlock(block);
    }
    getchar();
}