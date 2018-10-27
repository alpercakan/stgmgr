//
// Created by Alper Çakan on 9.04.2018.
//

#ifndef STGMGR_PAGE_H
#define STGMGR_PAGE_H

#include <string>
#include "constants.h"

class Page final {
 public:
  /**
   * Constructs an empty page which does not correspond to a page on the disc.
   */
  Page();

  /**
   * Constructs a page from the disc.
   *
   * @param fileName The file name of the file in which the requested page
   * resides
   * @param pageAddr Teh local page address of the requested page
   */
  Page(std::string fileName, uint_t pageAddr);

  /**
   * The destructor.
   */
  ~Page();

  /**
   * Gives "Is Used" field of the page header.
   *
   * @return Whether the page has meaningful, not-deleted data or not.
   */
  bool isUsed();

  /**
   * Sets "Is Used" field of the page header.
   *
   * @param isUsed Whether the page has meaningful, not-deleted data or not.
   */
  void setIsUsed(bool isUsed);

  /**
   * Gives "Page Category" field of the page header.
   *
   * @return An integer representing the category (catalogue, data etc.) of the
   * page
   */
  uint_t pageCategory();

  /**
   * Sets "Page Category" field of the page header.
   *
   * @param An integer representing the category (catalogue, data etc.) of the
   * page
   */
  void setPageCategory(uint_t pageCategory);

  /**
   * Gives the global address of the page.
   *
   * The global address of a page is unique among the pages in the whole disk;
   * unlike the local address, which is unique only in the file.
   *
   * @return The global address of the page
   */
  uint_t globAddr();

  /**
   * Gives the pointer to the start of the page content
   * @return The pointer to the start of the page content
   */
  const char* content();

  /**
   * Writes bytes to the content of the page.
   *
   * @param data Data to be written
   * @param len The length of the byte array "data"
   * @param pos The start position/byte index of the writing operation
   * @return Success/failure
   */
  bool writeContent(const char* const data, uint_t len, uint_t pos = 0);

  /**
   * The number of elements in the page header. All of them are 8-byte integers.
   */
  static const uint_t PAGE_HEADER_ELEM_COUNT = 3;

  /**
   * The size left to actual content of a page. It is simply the size of the
   * page minus the size of the page header.
   */
  static const uint_t CONTENT_SIZE =
      PAGE_SIZE - PAGE_HEADER_ELEM_COUNT * sizeof(uint_t);

  /**
   * Gives the 8-byte unsigned integer which starts at the given byte position.
   *
   * @param pos The index of the first byte of the requested integer in the byte
   * array of the page content
   * @return The integer described above
   */
  uint_t getUIntAtPos(uint_t pos);

  /**
   * Gives the local address of the page.
   *
   * The local address of a page simply means the 1-based index of the page in
   * the array of pages of the file. Hence, a local address of 0 means that the
   * page does not exist on the disk; it exists only on the memory.
   *
   * @return The local address of the page
   */
  uint_t getLocAddr();

  /**
   * Writes the page to the disk (only if the page did not exist in the disc
   * before or has been updated after being read from the disk).
   *
   * If you do not supply fileName and locAddr, the page will be written to its
   * current file and local address. Note that this is only possible for the
   * Page objects constructed from a physical page (i.e., constructed with a
   * file name and a local page address.
   *
   * @param fileName File name of the file in which this page is to be written
   * @param locAddr Local address of the page
   * @return Success/failure
   */
  bool persist(std::string fileName = "", uint_t locAddr = 0);

  /**
   * Gets (or creates) the consecutive page in the file.
   *
   * @param forceGet
   * @return If an error occurs or there is no consecutive page (this is only if
   * forceGet is false), null. Otherwise, a pointer to a dynamically allocated
   * Page object representing this physical page. Note that no pointer to that
   * object is kept by the class, hence the user is responsible for freeing it.
   */
  Page* getConsecPage(bool forceGet = false);

  /**
   * Finds the first empty cell in the page.
   *
   * A "cell" is a user-define logical unit in a page. This class is not
   * interested in how you store your content in a page, hence the size of a
   * cell is required for looping over the cells.
   *
   * @param cellSize Size of one cell
   * @return The index of the first empty cell in the page, or -1 if all cells
   * are full
   */
  int firstEmptyCellIndex(size_t cellSize);

  /**
   * Nullifies the whole page, including the header; preserving only the global
   * page address header field.
   */
  void reset();

  /**
   * Nullifies a byte range in the content area of the page.
   *
   * @param pos The byte index of the start position
   * @param len The byte count of the range
   */
  void resetRange(size_t pos, size_t len);

  /**
   * Casts the Page into a boolean value, which represents the validness of the
   * page.
   *
   * A page is invalid if and only if it has been tried to construct it from a
   * physical page and the read operation failed.
   *
   * @return Validness of the page
   */
  operator bool() const;

 private:
  void setGlobAddr(uint_t);

  char* whole();
  char* const data;

  static const uint_t PAGE_HEADER_IS_USED_INDEX = 0;
  static const uint_t PAGE_HEADER_PAGE_CAT_INDEX = 1;
  static const uint_t PAGE_HEADER_GLOB_ADDR_INDEX = 2;

  char* contentAddr();

  bool isModified;

  uint_t locAddr = 0;
  std::string fileName;
};

#endif  // STGMGR_PAGE_H
