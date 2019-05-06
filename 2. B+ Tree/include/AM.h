#ifndef AM_H_
#define AM_H_

#define AM_MAX_OPEN_FILES 20
#define AM_MAX_FILE_SCANS 20

typedef struct file_info{
    char* file_type;
    int file_desc;
    char attrType1;
    int attrLength1;
    char attrType2;
    int attrLength2;
    int root_block_num;
}file_info;

typedef struct Scan{
	int fileDesc;
	int fd;
	int blockNum;  
	int offset;
	int availableKeysInCurBlock;
	int nextBlock;
	int op;
	void* compKey;
	int completed;
	void* curScanValue;
}Scan;

void file_info_Init(file_info*, char*, int, char, int, char, int, int);

extern file_info* open_files;

int add_open_file(file_info);

int delete_open_file(int);

int key_comparison(char, void*, void*);

int key_equal(char, void*, void*);

typedef struct Stack{
    int* elements;
    int numofElements;
    int maxSize;
}Stack;


void printer(int );

void dataprinter(int , int );

void stackInit(Stack*, int);

int stackEmpty(Stack*);

void stackPush(Stack*, int);

int stackPop(Stack*);

void stckDestroy(Stack*);

int createIndexBlock(int, int, int, void*, int);

void indexSplit(char*, char*, int, int, void*, int, char, int, int);

void indexAddandSort(char*, int, void*, int, char, int);

/* Error codes */

extern int AM_errno;

#define AME_OK 0
#define AME_EOF -1
#define AM_OPEN_FILES_LIMIT_ERROR -2
#define AM_OPEN_FILE_DISTRUCTION_ERROR -3
#define AM_INVALID_FILE_ERROR -4
#define AM_BF_LEVEL_ERROR -5
#define AM_INDEX_CREATION_ERROR -6
#define AM_OPEN_SCAN_LIMIT_ERROR -7
#define AM_ABNORMAL_ERROR -10

#define EQUAL 1
#define NOT_EQUAL 2
#define LESS_THAN 3
#define GREATER_THAN 4
#define LESS_THAN_OR_EQUAL 5
#define GREATER_THAN_OR_EQUAL 6

void AM_Init( void );


int AM_CreateIndex(
  char *fileName, /* όνομα αρχείου */
  char attrType1, /* τύπος πρώτου πεδίου: 'c' (συμβολοσειρά), 'i' (ακέραιος), 'f' (πραγματικός) */
  int attrLength1, /* μήκος πρώτου πεδίου: 4 γιά 'i' ή 'f', 1-255 γιά 'c' */
  char attrType2, /* τύπος πρώτου πεδίου: 'c' (συμβολοσειρά), 'i' (ακέραιος), 'f' (πραγματικός) */
  int attrLength2 /* μήκος δεύτερου πεδίου: 4 γιά 'i' ή 'f', 1-255 γιά 'c' */
);


int AM_DestroyIndex(
  char *fileName /* όνομα αρχείου */
);


int AM_OpenIndex (
  char *fileName /* όνομα αρχείου */
);


int AM_CloseIndex (
  int fileDesc /* αριθμός που αντιστοιχεί στο ανοιχτό αρχείο */
);


int AM_InsertEntry(
  int fileDesc, /* αριθμός που αντιστοιχεί στο ανοιχτό αρχείο */
  void *value1, /* τιμή του πεδίου-κλειδιού προς εισαγωγή */
  void *value2 /* τιμή του δεύτερου πεδίου της εγγραφής προς εισαγωγή */
);


int AM_OpenIndexScan(
  int fileDesc, /* αριθμός που αντιστοιχεί στο ανοιχτό αρχείο */
  int op, /* τελεστής σύγκρισης */
  void *value /* τιμή του πεδίου-κλειδιού προς σύγκριση */
);


void *AM_FindNextEntry(
  int scanDesc /* αριθμός που αντιστοιχεί στην ανοιχτή σάρωση */
);


int AM_CloseIndexScan(
  int scanDesc /* αριθμός που αντιστοιχεί στην ανοιχτή σάρωση */
);


void AM_PrintError(
  char *errString /* κείμενο για εκτύπωση */
);

void AM_Close();


#endif /* AM_H_ */
