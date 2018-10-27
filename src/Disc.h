//
// Created by Alper Çakan on 9.04.2018.
//

#ifndef STGMGR_DISC_H
#define STGMGR_DISC_H

#include <string>
#include "constants.h"

class Disc {
 public:
  static char *readPage(const std::string &fileName, size_t locPageAddr);

  static bool writePage(const std::string &fileName, size_t locPageAddr,
                        const char *content);

  static bool appendPage(const std::string &fileName);

  static uint_t getPageCount(const std::string &fileName);

  static bool discFull;

  /**
   * The global address of the first newly created page will be this
   */
  static uint_t newPageAddr;
};

#endif  // STGMGR_DISC_H
