#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_FILES 100
#define MAX_FILENAME 50
#define MAX_BLOCKS 1000

// Colors for visualization
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

// Enums for organization types
typedef enum {
    CONTIGUOUS,
    CHAINED
} GlobalOrganization;

typedef enum {
    SORTED,
    UNSORTED
} InternalOrganization;

// Structure for a record
typedef struct {
    int id;
    char data[50];
    bool isDeleted;
} Record;

// Structure for file metadata
typedef struct {
    char filename[MAX_FILENAME];
    int numBlocks;
    int numRecords;
    int firstBlockAddress;
    GlobalOrganization globalOrg;
    InternalOrganization internalOrg;
    bool isActive;
} Metadata;

// Structure for a block
typedef struct {
    Record* records;  // Dynamic array based on blockSize
    int nextBlock;    // For chained organization
    int numRecords;
    int maxRecords;   // Store block capacity
    char filename[MAX_FILENAME];
} Block;

// Structure for the file system
typedef struct {
    Block* secondaryMemory;
    Metadata* fileMetadata;
    bool* blockAllocationTable;
    int totalBlocks;
    int blockSize;
    int numFiles;
} FileSystem;

// Function declarations
void initializeMemory(FileSystem* fs);
void createFile(FileSystem* fs);
void displayMemoryState(FileSystem* fs);
void displayMetadata(FileSystem* fs);
void searchRecord(FileSystem* fs);
void insertRecord(FileSystem* fs);
void deleteRecord(FileSystem* fs);
void defragmentFile(FileSystem* fs);
void deleteFile(FileSystem* fs);
void renameFile(FileSystem* fs);
void compactMemory(FileSystem* fs);
void clearMemory(FileSystem* fs);

// Helper function declarations
int findFreeBlocks(FileSystem* fs, int numBlocksNeeded, bool contiguous);
void initializeBlock(FileSystem* fs, int blockIndex);
int calculateRequiredBlocks(int numRecords, int blockSize);
void copyBlock(FileSystem* fs, int dest, int src);
Metadata* findFile(FileSystem* fs, const char* filename);
int findNextFreeRecordSlot(Block* block);
int countActiveRecordsInFile(FileSystem* fs, Metadata* file);

#endif // FILE_SYSTEM_H
