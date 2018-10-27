//
// Created by Alper Çakan on 9.04.2018.
//

#ifndef STGMGR_CONSTANTS_H
#define STGMGR_CONSTANTS_H

#include <cstdint>

// Page categories
#define PAGE_CATEGORY_FIELD_NAMES 1
#define PAGE_CATEGORY_TYPES 2
#define PAGE_CATEGORY_DATA 3

// Sizes
#define PAGE_SIZE 2048
#define MAX_STORAGE_SIZE 10485760  // bytes = 10 MB
#define MAX_PAGE_COUNT (MAX_STORAGE_SIZE / PAGE_SIZE)

#define FIELD_NAME_SIZE 32
#define TYPE_DATA_SIZE 56
#define TYPE_NAME_SIZE 32

// Typedefs
typedef int64_t sint_t;   // Signed integer type
typedef uint64_t uint_t;  // Unsigned integer type
#define INT_SIZE 8;       // bytes

typedef sint_t field_t;  // Field type
typedef uint_t addr_t;   // Page address type

// File names
#define SYS_CATALOGUE_GENERAL_FILE_NAME "syscatalgen"
#define SYS_CATALOGUE_TYPES_FILE_NAME "syscatalt"
#define SYS_CATALOGUE_FIELDS_FILE_NAME "syscatalf"

// Messages
#define HELP_MESSAGE \
  "Storage Manager v0.1\n\
\n\
The storage manager for a very basic DBMS.\n\
\n\
Usage: ./stgmr <option>\n\
\n\
Options:\n\
    --help, -h      Prints this message\n\
\n\
    --format, -f    Formats the current directory to be as an empty DB\n\
\n\
    --console, -c   Starts the stgmgr console, which you can use for DDL and DML operations\n\
\n\
Author: Alper Çakan\n\
"

#define ERR_MSG_DISK_FULL "The disk is full"
#define ERR_MSG_FAIL_READ_PAGE "Could not read page"
#define ERR_MSG_NO_PRIMARY_KEY "There should be a primary key field"

#endif  // STGMGR_CONSTANTS_H
