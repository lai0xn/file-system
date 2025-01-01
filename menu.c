#include "file_system.h"

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void displayMenu() {
    printf("\n=== File Management System Simulator ===\n");
    printf("1. Initialize Memory\n");
    printf("2. Create File\n");
    printf("3. Display Memory State\n");
    printf("4. Display Metadata\n");
    printf("5. Search Record\n");
    printf("6. Insert Record\n");
    printf("7. Delete Record\n");
    printf("8. Defragment File\n");
    printf("9. Delete File\n");
    printf("10. Rename File\n");
    printf("11. Compact Memory\n");
    printf("12. Clear Memory\n");
    printf("0. Exit\n");
}
