#include "file_system.h"
#include "menu.h"

int main() {
    FileSystem fs = {NULL, NULL, NULL, 0, 0, 0};
    int choice;

    do {
        displayMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        clearScreen();

        switch(choice) {
            case 1:
                initializeMemory(&fs);
                break;
            case 2:
                createFile(&fs);
                break;
            case 3:
                displayMemoryState(&fs);
                break;
            case 4:
                displayMetadata(&fs);
                break;
            case 5:
                searchRecord(&fs);
                break;
            case 6:
                insertRecord(&fs);
                break;
            case 7:
                deleteRecord(&fs);
                break;
            case 8:
                defragmentFile(&fs);
                break;
            case 9:
                deleteFile(&fs);
                break;
            case 10:
                renameFile(&fs);
                break;
            case 11:
                compactMemory(&fs);
                break;
            case 12:
                clearMemory(&fs);
                break;
            case 0:
                printf("Exiting program...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }

        if (choice != 0) {
            printf("\nPress Enter to continue...");
            getchar();  // Consume newline
            getchar();
            clearScreen();
        }

    } while(choice != 0);

    // Clean up
    clearMemory(&fs);

    return 0;
}
