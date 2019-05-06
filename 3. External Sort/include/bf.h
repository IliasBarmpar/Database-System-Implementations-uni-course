#ifndef BF_H
#define BF_H

#ifdef __cplusplus
extern "C" {
#endif

#define BF_BLOCK_SIZE 1024    /* Î¤Î¿ Î¼Î­Î³ÎµÎ¸Î¿Ï‚ ÎµÎ½ÏŒÏ‚ block ÏƒÎµ bytes */
#define BF_BUFFER_SIZE 64     /* ÎŸ Î¼Î­Î³Î¹ÏƒÏ„Î¿Ï‚ Î±Ï�Î¹Î¸Î¼ÏŒÏ‚ block Ï€Î¿Ï… ÎºÏ�Î±Ï„Î¬Î¼Îµ ÏƒÏ„Î·Î½ Î¼Î½Î®Î¼Î· */
#define BF_MAX_OPEN_FILES 100 /* ÎŸ Î¼Î­Î³Î¹ÏƒÏ„Î¿Ï‚ Î±Ï�Î¹Î¸Î¼ÏŒÏ‚ Î±Î½Î¿Î¹ÎºÏ„ÏŽÎ½ Î±Ï�Ï‡ÎµÎ¯Ï‰Î½ */

typedef enum BF_ErrorCode {
  BF_OK,
  BF_OPEN_FILES_LIMIT_ERROR,     /* Î¥Ï€Î¬Ï�Ï‡Î¿Ï…Î½ Î®Î´Î· BF_MAX_OPEN_FILES Î±Ï�Ï‡ÎµÎ¯Î± Î±Î½Î¿Î¹ÎºÏ„Î¬ */
  BF_INVALID_FILE_ERROR,         /* ÎŸ Î±Î½Î±Î³Î½Ï‰Ï�Î¹ÏƒÏ„Î¹ÎºÏŒÏ‚ Î±Ï�Î¹Î¸Î¼ÏŒÏ‚ Î±Ï�Ï‡ÎµÎ¯Î¿Ï… Î´ÎµÎ½ Î±Î½Ï„Î¹ÏƒÏ„Î¹Ï‡ÎµÎ¯ ÏƒÎµ ÎºÎ¬Ï€Î¿Î¹Î¿ Î±Î½Î¿Î¹Ï‡Ï„ÏŒ Î±Ï�Ï‡ÎµÎ¯Î¿ */
  BF_ACTIVE_ERROR,               /* Î¤Î¿ ÎµÏ€Î¯Ï€ÎµÎ´Î¿ BF ÎµÎ¯Î½Î±Î¹ ÎµÎ½ÎµÏ�Î³ÏŒ ÎºÎ±Î¹ Î´ÎµÎ½ Î¼Ï€Î¿Ï�ÎµÎ¯ Î½Î± Î±Ï�Ï‡Î¹ÎºÎ¿Ï€Î¿Î¹Î·Î¸ÎµÎ¯ */
  BF_FILE_ALREADY_EXISTS,        /* Î¤Î¿ Î±Ï�Ï‡ÎµÎ¯Î¿ Î´ÎµÎ½ Î¼Ï€Î¿Ï�ÎµÎ¯ Î½Î± Î´Î·Î¼Î¹Î¿Ï…Ï�Î³Î¹Î¸ÎµÎ¯ Î³Î¹Î±Ï„Î¹ Ï…Ï€Î¬Ï�Ï‡ÎµÎ¹ Î®Î´Î· */
  BF_FULL_MEMORY_ERROR,          /* Î— Î¼Î½Î®Î¼Î· Î­Ï‡ÎµÎ¹ Î³ÎµÎ¼Î¯ÏƒÎµÎ¹ Î¼Îµ ÎµÎ½ÎµÏ�Î³Î¬ block */
  BF_INVALID_BLOCK_NUMBER_ERROR, /* Î¤Î¿ block Ï€Î¿Ï… Î¶Î·Ï„Î®Î¸Î·ÎºÎµ Î´ÎµÎ½ Ï…Ï€Î¬Ï�Ï‡ÎµÎ¹ ÏƒÏ„Î¿ Î±Ï�Ï‡ÎµÎ¯Î¿ */
  BF_AVAILABLE_PIN_BLOCKS_ERROR,
  BF_ERROR
} BF_ErrorCode;

typedef enum ReplacementAlgorithm {
  LRU,
  MRU
} ReplacementAlgorithm;

typedef struct BF_Block BF_Block;

void BF_Block_Init(BF_Block **block);

void BF_Block_Destroy(BF_Block **block);

void BF_Block_SetDirty(BF_Block *block);

char* BF_Block_GetData(const BF_Block *block);

BF_ErrorCode BF_Init(const ReplacementAlgorithm repl_alg);

BF_ErrorCode BF_CreateFile(const char* filename);

BF_ErrorCode BF_OpenFile(const char* filename, int *file_desc);

BF_ErrorCode BF_CloseFile(const int file_desc);

BF_ErrorCode BF_GetBlockCounter(const int file_desc, int *blocks_num);

BF_ErrorCode BF_AllocateBlock(const int file_desc, BF_Block *block);

BF_ErrorCode BF_GetBlock(const int file_desc, const int block_num, BF_Block *block);

BF_ErrorCode BF_UnpinBlock(BF_Block *block);

void BF_PrintError(BF_ErrorCode err);

BF_ErrorCode BF_Close();

#ifdef __cplusplus
}
#endif
#endif // BF_H
