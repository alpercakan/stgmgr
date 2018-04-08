#ifndef STGMGR_TEXTS_H
#define STGMGR_TEXTS_H

#include <string>


// General
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

// Related to commands
#define ILLEGAL_COMMAND_LINE_OPTION_MESSAGE(OPTION) \
(std::string("Illegal command line option: ") + (OPTION))

#define COMMAND_FAILED_MESSAGE(OPTION) \
(std::string("Command line option ") + (OPTION) + " has failed.")

#endif // STGMGR_TEXTS_H
