#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Disc.h"
#include "Page.h"

using namespace std;

/**
 * Creates a type. The first field will be the primary key.
 *
 * @param typeName The name of the type to be created
 * @param fieldNames The names of the fields. Must be nonempty, since at least
 * the type must have a primary key.
 * @return Success/failure
 */
bool createType(const string &typeName, const vector<string> &fieldNames) {
  if (fieldNames.empty()) {
    return false;
  }

  // First, register the field names to the system catalogue

  Page *fieldPage = new Page(SYS_CATALOGUE_FIELDS_FILE_NAME, 1);

  if (!(*fieldPage)) {
    delete fieldPage;
    return false;
  }

  // Advance to the first unused page
  while (fieldPage->isUsed()) {
    auto tmp = fieldPage;
    fieldPage = fieldPage->getConsecPage(true);
    delete tmp;

    if (!fieldPage) {
      return false;
    }
  }

  // Clean the garbage
  fieldPage->reset();

  // Write the names of the fields
  for (size_t i = 0; i < fieldNames.size(); ++i) {
    auto name = fieldNames[i];

    if (!(fieldPage->writeContent(name.c_str(), name.length(),
                                  i * FIELD_NAME_SIZE))) {
      delete fieldPage;
      return false;
    }
  }

  // Save the page
  fieldPage->setIsUsed(true);
  fieldPage->setPageCategory(PAGE_CATEGORY_FIELD_NAMES);

  if (!(fieldPage->persist())) {
    delete fieldPage;
    return false;
  }

  delete fieldPage;

  // Now, onto registering the type into the system catalogue

  Page *typePage = new Page(SYS_CATALOGUE_TYPES_FILE_NAME, 1);

  if (!(*typePage)) {
    delete typePage;
    return false;
  }

  int emptyCellIndex;

  // Advance to the first page with empty cell, or create one if there is none
  while ((emptyCellIndex = typePage->firstEmptyCellIndex(TYPE_DATA_SIZE)) < 0) {
    auto tmp = typePage;
    typePage = typePage->getConsecPage(true);
    delete tmp;

    if (!typePage) {
      return false;
    }
  }

  uint_t fieldCount = fieldNames.size();
  uint_t fieldPageAddr = fieldPage->getLocAddr();
  uint_t useMark = 1;
  size_t cellStart = emptyCellIndex * TYPE_DATA_SIZE;

  typePage->resetRange(cellStart, TYPE_DATA_SIZE);

  typePage->writeContent(reinterpret_cast<char *>(&useMark), sizeof(uint_t),
                         cellStart);

  typePage->writeContent(typeName.c_str(), typeName.length(),
                         sizeof(uint_t) + cellStart);

  typePage->writeContent(reinterpret_cast<char *>(&fieldCount), sizeof(uint_t),
                         sizeof(uint_t) + TYPE_NAME_SIZE + cellStart);

  typePage->writeContent(reinterpret_cast<char *>(&fieldPageAddr),
                         sizeof(uint_t),
                         cellStart + TYPE_NAME_SIZE + 2 * sizeof(uint_t));

  typePage->setIsUsed(true);
  typePage->setPageCategory(PAGE_CATEGORY_TYPES);
  if (!(typePage->persist())) {
    return false;
  }

  // Prepare an empty data file for the new type
  remove(typeName.c_str());
  return Disc::appendPage(typeName);
}

/**
 * Returns the list of the "matching" types
 *
 * @param all If true, all types will match. If false, only the type with the
 * given name will match
 * @param typeName The name of the type to be matched
 * @return The vector of pairs (Type Name, The Number of Fields in the Type)
 */
vector<pair<string, vector<string>>> getTypeList(bool all = true,
                                                 const string &typeName = "") {
  vector<pair<string, vector<string>>> typeNames;
  Page *typePage = new Page(SYS_CATALOGUE_TYPES_FILE_NAME, 1);

  while (typePage && *typePage) {
    if (typePage->isUsed()) {
      for (int i = 0; i < typePage->CONTENT_SIZE / TYPE_DATA_SIZE; ++i) {
        auto cell = typePage->content() + i * TYPE_DATA_SIZE;

        if (*(reinterpret_cast<const uint_t *>(cell)) == 1) {
          string name = cell + sizeof(uint_t);
          auto fieldNameCount = *reinterpret_cast<const sint_t *>(
              cell + sizeof(uint_t) + TYPE_NAME_SIZE);
          auto fieldPageAddr = *reinterpret_cast<const uint_t *>(
              cell + TYPE_NAME_SIZE + 2 * sizeof(uint_t));
          Page fieldNamesPage(SYS_CATALOGUE_FIELDS_FILE_NAME, fieldPageAddr);

          if (!fieldNamesPage) {
            delete typePage;
            return typeNames;
          }

          vector<string> fieldNames(fieldNameCount);

          for (size_t i = 0; i < fieldNameCount; ++i) {
            fieldNames[i] = fieldNamesPage.content() + i * FIELD_NAME_SIZE;
          }

          if (all || name == typeName) typeNames.push_back({name, fieldNames});

          if (!all && name == typeName) {
            delete typePage;
            return typeNames;
          }
        }
      }
    }

    auto tmp = typePage;
    typePage = typePage->getConsecPage();
    delete tmp;
  }

  delete typePage;
  return typeNames;
}

/**
 * Deletes a type.
 *
 * @param typeName The name of the type to be deleted
 * @return Success/failure
 */
bool deleteType(const string &typeName) {
  remove(typeName.c_str());

  Page *typePage = new Page(SYS_CATALOGUE_TYPES_FILE_NAME, 1);

  while (typePage && *typePage) {
    if (typePage->isUsed()) {
      for (int i = 0; i < typePage->CONTENT_SIZE / TYPE_DATA_SIZE; ++i) {
        const char *cell = typePage->content() + i * TYPE_DATA_SIZE;

        if (*(reinterpret_cast<const uint_t *>(cell)) == 1 &&
            string(cell + sizeof(uint_t)) == typeName) {
          Page fieldPage(SYS_CATALOGUE_FIELDS_FILE_NAME,
                         *reinterpret_cast<const uint_t *>(
                             cell + TYPE_NAME_SIZE + 2 * sizeof(uint_t)));

          if (!fieldPage) {
            delete typePage;
            return false;
          }

          fieldPage.setIsUsed(false);

          uint_t markFree = 0;
          typePage->writeContent(reinterpret_cast<char *>(&markFree),
                                 sizeof(uint_t), i * TYPE_DATA_SIZE);

          if (!(fieldPage.persist() && typePage->persist())) {
            delete typePage;
            return false;
          }

          delete typePage;
          return true;
        }
      }
    }

    auto tmp = typePage;
    typePage = typePage->getConsecPage();
    delete tmp;
  }

  delete typePage;
  return false;
}

/**
 * Formats the current directory to be an empty database.
 *
 * Note that this operation may remove and/or overwrite your existing files.
 * Therefore, it should be run only if the directory contains only the
 * executable of this program.
 */
bool format() {
  // Just remove all system catalogue files and initialize each of them with a
  // single null page.

  remove(SYS_CATALOGUE_GENERAL_FILE_NAME);
  remove(SYS_CATALOGUE_TYPES_FILE_NAME);
  remove(SYS_CATALOGUE_FIELDS_FILE_NAME);

  return Disc::appendPage(SYS_CATALOGUE_GENERAL_FILE_NAME) &&
         Disc::appendPage(SYS_CATALOGUE_TYPES_FILE_NAME) &&
         Disc::appendPage(SYS_CATALOGUE_FIELDS_FILE_NAME);
}

/**
 * Creates a record.
 *
 * @param typeName The name of the type of the record to be created
 * @param values The field values of the record to be created. Must be nonempty.
 * @return The pair (Glob. Page Addr., Loc. Page Addr.) for the page in which
 * the newly created record is placed, or (0, 0) on failure.
 */
pair<uint_t, uint_t> createRecord(const string &typeName,
                                  const vector<sint_t> &values) {
  Page *page = new Page(typeName, 1);

  if (!(*page)) {
    delete page;
    return {0, 0};
  }

  int emptyCellIndex;
  const auto recSize = (values.size() + 1) * sizeof(sint_t);

  while ((emptyCellIndex = page->firstEmptyCellIndex(recSize)) < 0) {
    auto tmp = page;
    page = page->getConsecPage(true);
    delete tmp;

    if (!page) {
      return {0, 0};
    }
  }

  size_t cellStart = emptyCellIndex * recSize;

  uint_t useMark = 1;
  page->resetRange(cellStart, recSize);
  page->writeContent(reinterpret_cast<char *>(&useMark), sizeof(uint_t),
                     cellStart);

  for (size_t i = 0; i < values.size(); ++i) {
    page->writeContent(reinterpret_cast<const char *>(&values[i]),
                       sizeof(uint_t),
                       cellStart + sizeof(uint_t) + i * sizeof(sint_t));
  }

  page->setIsUsed(true);
  page->setPageCategory(PAGE_CATEGORY_DATA);

  if (!(page->persist())) {
    delete page;
    return {0, 0};
  }

  delete page;
  return {page->globAddr(), page->getLocAddr()};
}

/**
 * Searches for (and deletes) for a record or all records.
 *
 * Can be used for:
 * - Querying a specific record of a type
 * - Deleting a specific record of a type
 * - Deleting all records of a type
 * - Querying all records of a type
 *
 * @param typeName The name of the type of the record(s) to be searched/deleted
 * @param keyValue The key value of the record to be searched/deleted
 * @param all If true, keyValue will be ignored and all records of the given
 * type will "match"
 * @param del If true, the matched records will be deleted
 * @param suc A reference to a boolean variable. This will contain the
 * success/failure status. Note that finding no matching records is not a
 * failure.
 * @return The pair (Record Values, (Global Page Addr. of the Record), (Local
 * Page Addr. of the Record)).
 */
pair<vector<vector<sint_t>>, pair<uint_t, uint_t>> searchRecord(
    const string &typeName, sint_t keyValue, bool all, bool del, bool &suc) {
  suc = true;
  vector<vector<sint_t>> res;
  Page *page = new Page(typeName, 1);
  uint_t glob = 0, loc = 0;

  if (!(*page)) {
    delete page;
    suc = false;
    return {res, {0, 0}};
  }

  auto typeList = getTypeList(false, typeName);

  if (typeList.empty()) {
    delete page;
    suc = false;
    return {res, {0, 0}};
  }

  auto fieldNames = typeList[0].second;
  int recSize = sizeof(uint_t) * (1 + fieldNames.size());

  while (page) {
    if (page->isUsed()) {
      for (int i = 0; i < page->CONTENT_SIZE / recSize; ++i) {
        const char *cell = page->content() + i * recSize;

        if (*(reinterpret_cast<const uint_t *>(cell)) == 1 &&
            (*(reinterpret_cast<const sint_t *>(cell + sizeof(uint_t))) ==
                 keyValue ||
             all)) {
          vector<sint_t> record;

          for (int j = 0; j < fieldNames.size(); ++j) {
            record.push_back(*(reinterpret_cast<const sint_t *>(
                cell + sizeof(uint_t) * (j + 1))));
          }

          res.push_back(record);

          if (del) {
            uint_t markEmpty = 0;
            page->writeContent(reinterpret_cast<char *>(&markEmpty),
                               sizeof(uint_t), i * recSize);

            if (!(page->persist())) {
              delete page;
              suc = false;
              return {res, {0, 0}};
            }
          }

          glob = page->globAddr();
          loc = page->getLocAddr();

          if (!all) {
            delete page;
            return {res, {glob, loc}};
          }
        }
      }
    }

    auto tmp = page;
    page = page->getConsecPage();
    delete tmp;
  }

  delete page;
  return {res, {glob, loc}};
}

/**
 * Gives the "actual" arguments of the program as a vector of strings.
 *
 * "Actual" simply means that the first argument is ignored, which is the
 * path of the executable.
 *
 * @param argc Argument count
 * @param argv Arguments, as passed on to the executable entry point by the
 * system
 * @return The vector of the actual arguments as strings
 */
vector<string> argsToVec(int argc, char *argv[]) {
  vector<string> res(argc);

  for (int i = 1; i < argc; ++i) {
    res[i - 1] = argv[i];
  }

  return res;
}

string join(const vector<string> &arr, const string &delim) {
  string joined;

  if (!arr.empty()) {
    joined += arr[0];

    for (size_t i = 1; i < arr.size(); ++i) {
      joined += delim + arr[i];
    }
  }

  return joined;
}

/**
 * Gives a string representation of a record in human-readable format.
 *
 * The format is as the following:
 * TypeName(fieldVal1, fieldVal2, ...)
 *
 * @param typeName Name of the record type
 * @param rec The field values of the record. Must be nonempty.
 * @return Human-readable string representation of the record
 */
string recToStr(const string &typeName, const vector<sint_t> &rec) {
  vector<string> valsAsStr;
  valsAsStr.reserve(rec.size());

  for (const auto val : rec) {
    valsAsStr.push_back(to_string(val));
  }

  return typeName + "(" + join(valsAsStr, ", ") + ")";
}

void persistGlobPageAddr() {
  Page genSysCat(SYS_CATALOGUE_GENERAL_FILE_NAME, 1);

  genSysCat.writeContent(reinterpret_cast<char *>(&Disc::newPageAddr),
                         sizeof(uint_t), 0);

  genSysCat.persist();
}

/**
 * Gives a string representation of a type in human-readable format.
 *
 * The format is as the following:
 * TypeName(FieldName1, FieldName2, ...)
 *
 * @param typeName Name of the type
 * @param fieldNames Names of the fields
 * @return Human-readable string representation of the type
 */
string typeToStr(const string &typeName, const vector<string> &fieldNames) {
  return typeName + "(" + join(fieldNames, ", ") + ")";
}

bool execCmd(const string &line) {
  string cmd;
  istringstream ss(line);  // "Parse" the read line
  ss >> cmd;

  if (cmd == "exit") {
    persistGlobPageAddr();
    exit(EXIT_SUCCESS);
  }

  if (cmd == "create_type") {
    string typeName, fieldName;
    vector<string> fieldNames;

    ss >> typeName;

    while (ss) {
      if (ss >> fieldName) fieldNames.push_back(fieldName);
    }

    if (!createType(typeName, fieldNames)) return false;

    cout << typeToStr(typeName, fieldNames) << " is created!" << endl;

  } else if (cmd == "delete_type") {
    string typeName;
    ss >> typeName;

    if (!deleteType(typeName)) {
      return false;
    }

    cout << typeName << " is deleted!" << endl;
  } else if (cmd == "list_types") {
    auto vec = getTypeList();

    for (const auto &t : vec) {
      cout << typeToStr(t.first, t.second) << endl;
    }
  } else if (cmd == "create_record") {
    string typeName;
    vector<sint_t> values;

    ss >> typeName;

    while (ss) {
      sint_t value;

      if (ss >> value) values.push_back(value);
    }

    auto pageNumber = createRecord(typeName, values);

    if (pageNumber.first == 0 && pageNumber.second == 0) {
      return false;
    }

    cout << recToStr(typeName, values) << " is created in page #"
         << pageNumber.first << ":" << pageNumber.second << endl;
  } else if (cmd == "delete_record") {
    string typeName;
    sint_t key;
    bool suc;

    ss >> typeName >> key;

    auto res = searchRecord(typeName, key, false, true, suc);

    if (!suc) {
      return false;
    }

    if (res.first.empty()) {
      cout << "No such record found" << endl;
    } else {
      cout << "The record is deleted from page #" << res.second.first << ":"
           << res.second.second << endl;
    }
  } else if (cmd == "search_record") {
    string typeName;
    sint_t key;
    bool suc;

    ss >> typeName >> key;

    auto res = searchRecord(typeName, key, false, false, suc);

    if (!suc) {
      return false;
    }

    if (res.first.empty()) {
      cout << "No such record found" << endl;
    } else {
      cout << "The record is found in page #" << res.second.first << ":"
           << res.second.second << endl;
      cout << recToStr(typeName, res.first[0]) << endl;
    }
  } else if (cmd == "list_records") {
    string typeName;
    ss >> typeName;
    bool suc;

    auto res = searchRecord(typeName, 0, true, false, suc);

    if (!suc) {
      return false;
    }

    for (const auto &rec : res.first) {
      cout << recToStr(typeName, rec) << endl;
    }
  }

  return true;
}

/**
 * Read-eval-print loop mode for DML and DDL commands.
 */
void repl() {
  while (true) {
    string line;
    cout << "> ";        // Classic REPL line start output
    getline(cin, line);  // Read line by line

    try {
      if (!execCmd(line)) {
        cout << "Command failed!" << endl;

        if (Disc::discFull) {
          cout << "The disc is full" << endl;
        }
      }
    } catch (exception e) {
      cout << "Command execution error!" << endl;
    }
  }
}

/**
 * Prints the help message.
 */
void printHelp() { cout << HELP_MESSAGE << endl; }

void initGlobPageAddr() {
  Page genSysCat(SYS_CATALOGUE_GENERAL_FILE_NAME, 1);

  Disc::newPageAddr = (*reinterpret_cast<const uint_t *>(genSysCat.content()));
}

/**
 * The entry point.
 */
int main(int argc, char *argv[]) {
  auto args = argsToVec(argc, argv);

  if (args.empty() || args[0] == "--help" || args[0] == "-h") {
    printHelp();
  } else if (args[0] == "--format" || args[0] == "-f") {
    cout << "Formatting..." << endl;

    if (format()) {
      persistGlobPageAddr();
      cout << "Formatted successfully." << endl;
    } else {
      cout << "Formatting failed!" << endl;
      return EXIT_FAILURE;
    }
  } else if (args[0] == "--console" || args[0] == "-c") {
    initGlobPageAddr();

    cout << "Console mode" << endl
         << "Type DDL or DML command and press enter." << endl
         << endl;

    repl();
  }

  return EXIT_SUCCESS;
}
