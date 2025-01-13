# File Management System Simulator

A C-based simulation of a file management system that demonstrates various file organization and memory management concepts.

## Overview

This project implements a file system simulator that supports both contiguous and chained file organization methods. It provides functionality for:

-   Memory initialization and management
-   File creation and deletion
-   Record insertion and deletion
-   Memory defragmentation and compaction
-   File metadata management
-   Memory state visualization

## Features

-   **Flexible Memory Configuration**: Users can specify total blocks and block size
-   **Dual File Organization**:
    -   Contiguous allocation
    -   Chained allocation
-   **Record Management**:
    -   Support for sorted and unsorted record organization
    -   Record insertion with unique IDs
    -   Record deletion with space reclamation
-   **Memory Optimization**:
    -   File defragmentation
    -   Memory compaction
    -   Block allocation tracking
-   **System Visualization**:
    -   Color-coded memory state display
    -   Detailed metadata information
    -   Block allocation status

## Project Structure

```
file_systeme_sfsd/
├── file_system.c    # Core file system implementation
├── file_system.h    # Header file with data structures
├── main.c          # Program entry point
├── menu.c          # User interface implementation
├── menu.h          # Menu function declarations
├── build/          # Compiled objects and executable
├── Makefile        # Build configuration
└── README.md       # Project documentation
```

## Prerequisites

-   GCC compiler
-   Make build system
-   Unix-like environment (Linux/macOS) or Windows with MinGW

## Building the Project

1. Clone the repository:

    ```bash
    git clone https://github.com/wailbentafat/file_systeme_sfsd.git
    cd file_systeme_sfsd
    ```

2. Build the project:

    ```bash
    make
    ```

3. Run the simulator:
    ```bash
    make run
    ```

## Usage Guide

1. **Initialize Memory**:

    - Select option 1
    - Enter total number of blocks
    - Enter block size (records per block)

2. **Create a File**:

    - Select option 2
    - Enter filename
    - Specify number of records
    - Choose organization method:
        - Global: Contiguous (0) or Chained (1)
        - Internal: Sorted (0) or Unsorted (1)

3. **Manage Records**:

    - Insert records with option 6
    - Delete records with option 7
    - Search records with option 5

4. **Optimize Memory**:

    - Defragment individual files with option 8
    - Compact entire memory with option 11

5. **Monitor System State**:
    - View memory state with option 3
    - Check file metadata with option 4

## Implementation Details

### Memory Management

-   Dynamic allocation of blocks and records
-   Efficient block allocation tracking
-   Automatic memory cleanup

### File Organization

-   **Contiguous**: Files stored in consecutive blocks
-   **Chained**: Blocks linked using next-block pointers

### Record Management

-   Unique record IDs
-   Soft deletion for efficient space reuse
-   Support for both sorted and unsorted organizations

## Cleanup

To remove built files:

```bash
make clean
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

MIT License
