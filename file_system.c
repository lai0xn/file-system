#include "file_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void initializeMemory(FileSystem* fs) {
    // Clean up existing memory if any
    clearMemory(fs);

    printf("Enter total number of blocks: ");
    scanf("%d", &fs->totalBlocks);
    printf("Enter block size (records per block): ");
    scanf("%d", &fs->blockSize);

    // Allocate memory
    fs->secondaryMemory = (Block*)malloc(fs->totalBlocks * sizeof(Block));
    fs->blockAllocationTable = (bool*)calloc(fs->totalBlocks, sizeof(bool));
    fs->fileMetadata = (Metadata*)malloc(MAX_FILES * sizeof(Metadata));

    // Initialize structures
    for (int i = 0; i < fs->totalBlocks; i++) {
        fs->secondaryMemory[i].records = (Record*)malloc(fs->blockSize * sizeof(Record));
        fs->secondaryMemory[i].maxRecords = fs->blockSize;
        initializeBlock(fs, i);
    }

    for (int i = 0; i < MAX_FILES; i++) {
        fs->fileMetadata[i].isActive = false;
    }

    fs->numFiles = 0;
    printf("Memory initialized successfully!\n");
}

void initializeBlock(FileSystem* fs, int blockIndex) {
    for (int i = 0; i < fs->secondaryMemory[blockIndex].maxRecords; i++) {
        fs->secondaryMemory[blockIndex].records[i].id = -1;
        fs->secondaryMemory[blockIndex].records[i].isDeleted = false;
        strcpy(fs->secondaryMemory[blockIndex].records[i].data, "");
    }
    fs->secondaryMemory[blockIndex].nextBlock = -1;
    fs->secondaryMemory[blockIndex].numRecords = 0;
    strcpy(fs->secondaryMemory[blockIndex].filename, "");
}

void createFile(FileSystem* fs) {
    if (!fs->secondaryMemory) {
        printf("Memory not initialized. Please initialize memory first.\n");
        return;
    }

    if (fs->numFiles >= MAX_FILES) {
        printf("Maximum number of files reached!\n");
        return;
    }

    Metadata newFile;
    printf("Enter filename: ");
    scanf("%s", newFile.filename);

    // Check if file already exists
    if (findFile(fs, newFile.filename) != NULL) {
        printf("File with this name already exists!\n");
        return;
    }

    printf("Enter number of records: ");
    scanf("%d", &newFile.numRecords);

    if (newFile.numRecords <= 0) {
        printf("Number of records must be positive!\n");
        return;
    }

    printf("Choose global organization (0 for Contiguous, 1 for Chained): ");
    int orgChoice;
    scanf("%d", &orgChoice);
    newFile.globalOrg = orgChoice == 0 ? CONTIGUOUS : CHAINED;

    printf("Choose internal organization (0 for Sorted, 1 for Unsorted): ");
    scanf("%d", &orgChoice);
    newFile.internalOrg = orgChoice == 0 ? SORTED : UNSORTED;

    // Calculate required blocks
    newFile.numBlocks = calculateRequiredBlocks(newFile.numRecords, fs->blockSize);

    // Find free blocks
    int firstBlock = findFreeBlocks(fs, newFile.numBlocks, newFile.globalOrg == CONTIGUOUS);
    if (firstBlock == -1) {
        printf("Not enough space available!\n");
        return;
    }

    newFile.firstBlockAddress = firstBlock;
    newFile.isActive = true;

    // Store metadata
    fs->fileMetadata[fs->numFiles++] = newFile;

    // Initialize blocks for the file
    int currentBlock = firstBlock;
    for (int i = 0; i < newFile.numBlocks; i++) {
        fs->blockAllocationTable[currentBlock] = true;
        strcpy(fs->secondaryMemory[currentBlock].filename, newFile.filename);

        if (newFile.globalOrg == CHAINED && i < newFile.numBlocks - 1) {
            // For chained organization, find next free block
            int nextBlock = findFreeBlocks(fs, 1, false);
            if (nextBlock == -1) {
                printf("Error: Couldn't allocate all blocks!\n");
                return;
            }
            fs->secondaryMemory[currentBlock].nextBlock = nextBlock;
            currentBlock = nextBlock;
        } else if (newFile.globalOrg == CONTIGUOUS) {
            if (i < newFile.numBlocks - 1) {
                fs->secondaryMemory[currentBlock].nextBlock = currentBlock + 1;
            }
            currentBlock++;
        }
    }

    printf("File created successfully!\n");
}

void displayMemoryState(FileSystem* fs) {
    if (!fs->secondaryMemory) {
        printf("Memory not initialized.\n");
        return;
    }

    printf("\nMemory State (Total Blocks: %d, Block Size: %d records):\n", fs->totalBlocks, fs->blockSize);
    printf("--------------------------------------------------------------------------------\n");
    for (int i = 0; i < fs->totalBlocks; i++) {
        if (fs->blockAllocationTable[i]) {
            printf("%s[%d]%s %s (%d/%d records) -> %d\n", 
                   RED, i, RESET,
                   fs->secondaryMemory[i].filename,
                   fs->secondaryMemory[i].numRecords,
                   fs->secondaryMemory[i].maxRecords,
                   fs->secondaryMemory[i].nextBlock);
        } else {
            printf("%s[%d]%s FREE\n", GREEN, i, RESET);
        }
    }
}

void displayMetadata(FileSystem* fs) {
    if (!fs->secondaryMemory) {
        printf("Memory not initialized.\n");
        return;
    }

    printf("\nFile Metadata:\n");
    printf("+----------------------+---------+---------+---------+----------------+----------------+----------------+\n");
    printf("| Filename             | Blocks  | Records | Active  | First Block    | Global Org     | Internal Org   |\n");
    printf("+----------------------+---------+---------+---------+----------------+----------------+----------------+\n");

    for (int i = 0; i < fs->numFiles; i++) {
        if (fs->fileMetadata[i].isActive) {
            Metadata* file = &fs->fileMetadata[i];
            int activeRecords = countActiveRecordsInFile(fs, file);

            printf("| %-20s | %-7d | %-7d | %-7d | %-14d | %-14s | %-14s |\n",
                   file->filename,
                   file->numBlocks,
                   file->numRecords,
                   activeRecords,
                   file->firstBlockAddress,
                   file->globalOrg == CONTIGUOUS ? "Contiguous" : "Chained",
                   file->internalOrg == SORTED ? "Sorted" : "Unsorted");
        }
    }

    printf("+----------------------+---------+---------+---------+----------------+----------------+----------------+\n");
}

void searchRecord(FileSystem* fs) {
    if (!fs->secondaryMemory) {
        printf("Memory not initialized.\n");
        return;
    }

    int id;
    printf("Enter record ID to search: ");
    scanf("%d", &id);

    bool found = false;
    for (int i = 0; i < fs->numFiles; i++) {
        if (!fs->fileMetadata[i].isActive) continue;

        int blockIndex = fs->fileMetadata[i].firstBlockAddress;
        while (blockIndex != -1) {
            for (int j = 0; j < fs->secondaryMemory[blockIndex].numRecords; j++) {
                Record* record = &fs->secondaryMemory[blockIndex].records[j];
                if (record->id == id && !record->isDeleted) {
                    printf("Record found in file: %s\n", fs->fileMetadata[i].filename);
                    printf("  Block: %d, Position: %d\n", blockIndex, j);
                    printf("  ID: %d, Data: %s\n", record->id, record->data);
                    found = true;
                    return;
                }
            }
            blockIndex = fs->secondaryMemory[blockIndex].nextBlock;
        }
    }

    if (!found) {
        printf("Record with ID %d not found in any file.\n", id);
    }
}

int countActiveRecordsInFile(FileSystem* fs, Metadata* file) {
    int activeRecords = 0;
    int blockIndex = file->firstBlockAddress;
    while (blockIndex != -1) {
        for (int i = 0; i < fs->secondaryMemory[blockIndex].numRecords; i++) {
            if (!fs->secondaryMemory[blockIndex].records[i].isDeleted) {
                activeRecords++;
            }
        }
        blockIndex = fs->secondaryMemory[blockIndex].nextBlock;
    }
    return activeRecords;
}

int findFreeBlocks(FileSystem* fs, int numBlocksNeeded, bool contiguous) {
    if (contiguous) {
        int consecutiveCount = 0;
        int startBlock = -1;

        for (int i = 0; i < fs->totalBlocks; i++) {
            if (!fs->blockAllocationTable[i]) {
                if (consecutiveCount == 0) startBlock = i;
                consecutiveCount++;

                if (consecutiveCount == numBlocksNeeded) {
                    return startBlock;
                }
            } else {
                consecutiveCount = 0;
            }
        }
    } else {
        // For chained organization, just find first free block
        for (int i = 0; i < fs->totalBlocks; i++) {
            if (!fs->blockAllocationTable[i]) return i;
        }
    }

    return -1;
}

void copyBlock(FileSystem* fs, int dest, int src) {
    // Copy all records from source to destination
    for (int i = 0; i < fs->blockSize; i++) {
        fs->secondaryMemory[dest].records[i] = fs->secondaryMemory[src].records[i];
    }

    // Copy block metadata
    fs->secondaryMemory[dest].numRecords = fs->secondaryMemory[src].numRecords;
    fs->secondaryMemory[dest].nextBlock = fs->secondaryMemory[src].nextBlock;
    strncpy(fs->secondaryMemory[dest].filename, fs->secondaryMemory[src].filename, MAX_FILENAME);
    fs->secondaryMemory[dest].maxRecords = fs->secondaryMemory[src].maxRecords;
}

Metadata* findFile(FileSystem* fs, const char* filename) {
    for (int i = 0; i < fs->numFiles; i++) {
        if (fs->fileMetadata[i].isActive && strcmp(fs->fileMetadata[i].filename, filename) == 0) {
            return &fs->fileMetadata[i];
        }
    }
    return NULL;
}

int findNextFreeRecordSlot(Block* block) {
    for (int i = 0; i < block->maxRecords; i++) {
        if (block->records[i].id == -1 || block->records[i].isDeleted) {
            return i;
        }
    }
    return -1;
}

int calculateRequiredBlocks(int numRecords, int blockSize) {
    return (numRecords + blockSize - 1) / blockSize;
}

void insertRecord(FileSystem* fs) {
    if (!fs->secondaryMemory) {
        printf("Memory not initialized.\n");
        return;
    }

    char filename[MAX_FILENAME];
    printf("Enter filename: ");
    scanf("%s", filename);

    Metadata* file = findFile(fs, filename);
    if (!file) {
        printf("File not found.\n");
        return;
    }

    Record newRecord;
    printf("Enter record ID: ");
    scanf("%d", &newRecord.id);
    printf("Enter record data: ");
    scanf("%s", newRecord.data);
    newRecord.isDeleted = false;

    // Check if ID already exists
    int blockIndex = file->firstBlockAddress;
    while (blockIndex != -1) {
        for (int i = 0; i < fs->secondaryMemory[blockIndex].numRecords; i++) {
            if (fs->secondaryMemory[blockIndex].records[i].id == newRecord.id && 
                !fs->secondaryMemory[blockIndex].records[i].isDeleted) {
                printf("Record with ID %d already exists!\n", newRecord.id);
                return;
            }
        }
        blockIndex = fs->secondaryMemory[blockIndex].nextBlock;
    }

    // Find a block with free space
    blockIndex = file->firstBlockAddress;
    while (blockIndex != -1) {
        if (fs->secondaryMemory[blockIndex].numRecords < fs->blockSize) {
            int slot = findNextFreeRecordSlot(&fs->secondaryMemory[blockIndex]);
            if (slot != -1) {
                fs->secondaryMemory[blockIndex].records[slot] = newRecord;
                fs->secondaryMemory[blockIndex].numRecords++;
                printf("Record inserted successfully.\n");
                return;
            }
        }
        blockIndex = fs->secondaryMemory[blockIndex].nextBlock;
    }

    printf("No space available in existing blocks!\n");
}

void deleteRecord(FileSystem* fs) {
    if (!fs->secondaryMemory) {
        printf("Memory not initialized.\n");
        return;
    }

    char filename[MAX_FILENAME];
    int id;

    printf("Enter filename: ");
    scanf("%s", filename);
    printf("Enter record ID to delete: ");
    scanf("%d", &id);

    Metadata* file = findFile(fs, filename);
    if (!file) {
        printf("File not found.\n");
        return;
    }

    int blockIndex = file->firstBlockAddress;
    while (blockIndex != -1) {
        for (int i = 0; i < fs->secondaryMemory[blockIndex].numRecords; i++) {
            if (fs->secondaryMemory[blockIndex].records[i].id == id && 
                !fs->secondaryMemory[blockIndex].records[i].isDeleted) {
                fs->secondaryMemory[blockIndex].records[i].isDeleted = true;
                file->numRecords--;
                printf("Record deleted successfully.\n");
                return;
            }
        }
        blockIndex = fs->secondaryMemory[blockIndex].nextBlock;
    }

    printf("Record with ID %d not found.\n", id);
}

void defragmentFile(FileSystem* fs) {
    if (!fs->secondaryMemory) {
        printf("Memory not initialized.\n");
        return;
    }

    char filename[MAX_FILENAME];
    printf("Enter filename to defragment: ");
    scanf("%s", filename);

    Metadata* file = findFile(fs, filename);
    if (!file) {
        printf("File not found.\n");
        return;
    }

    if (file->globalOrg != CHAINED) {
        printf("Only chained files can be defragmented.\n");
        return;
    }

    // Count active records
    int activeRecords = 0;
    int blockIndex = file->firstBlockAddress;
    while (blockIndex != -1) {
        for (int i = 0; i < fs->secondaryMemory[blockIndex].numRecords; i++) {
            if (!fs->secondaryMemory[blockIndex].records[i].isDeleted) {
                activeRecords++;
            }
        }
        blockIndex = fs->secondaryMemory[blockIndex].nextBlock;
    }

    // Calculate required blocks after defragmentation
    int requiredBlocks = (activeRecords + fs->blockSize - 1) / fs->blockSize;

    // Find continuous space
    int newStartBlock = findFreeBlocks(fs, requiredBlocks, true);
    if (newStartBlock == -1) {
        printf("Not enough continuous space for defragmentation.\n");
        return;
    }

    // Create temporary array for active records
    Record* tempRecords = (Record*)malloc(activeRecords * sizeof(Record));
    int tempIndex = 0;

    // Collect all active records
    blockIndex = file->firstBlockAddress;
    while (blockIndex != -1) {
        for (int i = 0; i < fs->secondaryMemory[blockIndex].numRecords; i++) {
            if (!fs->secondaryMemory[blockIndex].records[i].isDeleted) {
                tempRecords[tempIndex++] = fs->secondaryMemory[blockIndex].records[i];
            }
        }
        blockIndex = fs->secondaryMemory[blockIndex].nextBlock;
    }

    // Free old blocks
    blockIndex = file->firstBlockAddress;
    while (blockIndex != -1) {
        int nextBlock = fs->secondaryMemory[blockIndex].nextBlock;
        fs->blockAllocationTable[blockIndex] = false;
        initializeBlock(fs, blockIndex);
        blockIndex = nextBlock;
    }

    // Redistribute records to new blocks
    tempIndex = 0;
    for (int i = 0; i < requiredBlocks; i++) {
        int currentBlock = newStartBlock + i;
        fs->blockAllocationTable[currentBlock] = true;
        strcpy(fs->secondaryMemory[currentBlock].filename, file->filename);
        fs->secondaryMemory[currentBlock].numRecords = 0;

        // Fill block with records
        for (int j = 0; j < fs->blockSize && tempIndex < activeRecords; j++) {
            fs->secondaryMemory[currentBlock].records[j] = tempRecords[tempIndex++];
            fs->secondaryMemory[currentBlock].numRecords++;
        }

        // Set next block pointer
        if (i < requiredBlocks - 1) {
            fs->secondaryMemory[currentBlock].nextBlock = currentBlock + 1;
        } else {
            fs->secondaryMemory[currentBlock].nextBlock = -1;
        }
    }

    // Update file metadata
    file->firstBlockAddress = newStartBlock;
    file->numBlocks = requiredBlocks;
    file->numRecords = activeRecords;

    free(tempRecords);
    printf("File defragmented successfully.\n");
}

void deleteFile(FileSystem* fs) {
    if (!fs->secondaryMemory) {
        printf("Memory not initialized.\n");
        return;
    }

    char filename[MAX_FILENAME];
    printf("Enter filename to delete: ");
    scanf("%s", filename);

    Metadata* file = findFile(fs, filename);
    if (!file) {
        printf("File not found.\n");
        return;
    }

    // Free all blocks
    int blockIndex = file->firstBlockAddress;
    while (blockIndex != -1) {
        int nextBlock = fs->secondaryMemory[blockIndex].nextBlock;
        fs->blockAllocationTable[blockIndex] = false;
        initializeBlock(fs, blockIndex);
        blockIndex = nextBlock;
    }

    // Mark file as inactive
    file->isActive = false;
    printf("File deleted successfully.\n");
}

void renameFile(FileSystem* fs) {
    if (!fs->secondaryMemory) {
        printf("Memory not initialized.\n");
        return;
    }

    char oldFilename[MAX_FILENAME], newFilename[MAX_FILENAME];
    printf("Enter current filename: ");
    scanf("%s", oldFilename);
    printf("Enter new filename: ");
    scanf("%s", newFilename);

    // Check if new filename already exists
    if (findFile(fs, newFilename) != NULL) {
        printf("A file with the new name already exists!\n");
        return;
    }

    Metadata* file = findFile(fs, oldFilename);
    if (!file) {
        printf("File not found.\n");
        return;
    }

    // Update filename in metadata
    strcpy(file->filename, newFilename);

    // Update filename in all blocks
    int blockIndex = file->firstBlockAddress;
    while (blockIndex != -1) {
        strcpy(fs->secondaryMemory[blockIndex].filename, newFilename);
        blockIndex = fs->secondaryMemory[blockIndex].nextBlock;
    }

    printf("File renamed successfully.\n");
}

void compactMemory(FileSystem* fs) {
    if (!fs->secondaryMemory) {
        printf("Memory not initialized.\n");
        return;
    }

    // Count total active files and blocks
    int activeFiles = 0;
    for (int i = 0; i < fs->numFiles; i++) {
        if (fs->fileMetadata[i].isActive) {
            activeFiles++;
        }
    }

    if (activeFiles == 0) {
        printf("No active files to compact.\n");
        return;
    }

    // Process each active file
    int nextFreeBlock = 0;
    for (int i = 0; i < fs->numFiles; i++) {
        if (!fs->fileMetadata[i].isActive) continue;

        Metadata* file = &fs->fileMetadata[i];
        int oldFirstBlock = file->firstBlockAddress;

        // Move blocks to beginning of memory
        int currentBlock = oldFirstBlock;
        int blocksProcessed = 0;

        while (currentBlock != -1 && blocksProcessed < file->numBlocks) {
            if (currentBlock != nextFreeBlock) {
                // Copy block to new location
                copyBlock(fs, nextFreeBlock, currentBlock);

                // Clear old block
                fs->blockAllocationTable[currentBlock] = false;
                initializeBlock(fs, currentBlock);
            }

            // Update next pointers
            if (blocksProcessed < file->numBlocks - 1) {
                fs->secondaryMemory[nextFreeBlock].nextBlock = nextFreeBlock + 1;
            } else {
                fs->secondaryMemory[nextFreeBlock].nextBlock = -1;
            }

            fs->blockAllocationTable[nextFreeBlock] = true;
            currentBlock = fs->secondaryMemory[currentBlock].nextBlock;
            nextFreeBlock++;
            blocksProcessed++;
        }

        // Update file metadata
        file->firstBlockAddress = oldFirstBlock == nextFreeBlock - blocksProcessed ? 
                                oldFirstBlock : nextFreeBlock - blocksProcessed;
    }

    // Clear remaining blocks
    for (int i = nextFreeBlock; i < fs->totalBlocks; i++) {
        fs->blockAllocationTable[i] = false;
        initializeBlock(fs, i);
    }

    printf("Memory compacted successfully.\n");
}

void clearMemory(FileSystem* fs) {
    if (fs->secondaryMemory != NULL) {
        // Free record arrays in each block
        for (int i = 0; i < fs->totalBlocks; i++) {
            free(fs->secondaryMemory[i].records);
        }

        // Free main structures
        free(fs->secondaryMemory);
        free(fs->blockAllocationTable);
        free(fs->fileMetadata);

        // Reset pointers and counters
        fs->secondaryMemory = NULL;
        fs->blockAllocationTable = NULL;
        fs->fileMetadata = NULL;
        fs->numFiles = 0;
        fs->totalBlocks = 0;

        printf("Memory cleared successfully.\n");
    } else {
        printf("Memory not initialized.\n");
    }
}
