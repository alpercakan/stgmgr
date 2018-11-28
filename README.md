## Build Instructions:

  1) cd into the directory stgmgr/
     (You are probably there now, since you are reading this)

  2) Run:
      cmake CMakeLists.txt && make

  3) The program will be created as "stgmgr" in the directory stgmgr/bin

## Running:
  1) cd into the directory stgmgr/bin/

  2) Run "./stgmgr --format" to initialize the directory as an empty DB

  3) Run "./stgmgr --console" to be able to run DML and DDL commands on the DB

## Output format: Page Addresses
  The page address outputted by the program are in the format #Global:Local
  where the global address of a page is unique among all the files and the local
  address is unique just inside the invidivual file.

## DML and DDL Commands:
### Creating a Type
    Syntax: create_type <field-name> {, <field-name> }

    The command name for creating a type is create_type. After that, you list the field names, separated by whitespace. Since the only allowed type is 64-bit signed integers, this command is not taking field types. The first field will be the primary key. Hence, you should list at least one field. Further information about valid file names can be found in the first project’s report.

    It is recommended, but not required, that you use InitialCapsCamelCase for type and field names.

    Note that this command will fail if the disc drive is full.

### Deleting a Type
    Syntax: delete_type <type-name>

    The command name for deleting a type is delete_type. The only argument is the name of the type.

### Listing All Types
    Syntax: list_types

    The command name for listing all the types is list_types. It takes no argument.

### Creating a Record
    Syntax: create_record <type-name> <field-value> {, <field-value> }

    The command name for creating a record is create_record. The first argument is the type of the record to be created, and the other arguments are the values of the fields (in the order that was given when creating the type).

### Deleting a Record
    Syntax: delete_record <type-name> <field-value>

    The command name for deleting a record is delete_record. The first argument is the type of the record to be deleted, and the second argument is the primary key value of the record to be deleted.

### Searching for a Record
    Syntax: search_record <type-name> <field-value>

    The command name for searching for a record is search_record. The first argument is the type of the record to be searched, and the second argument is the primary key value of the record to be searched.

### Listing All Records of a Type
    Syntax: list_records <type-name>

    The command name for listing all the records of a type is list_records. The only argument is the name of the type whose records are to be listed.

