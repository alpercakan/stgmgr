//
// Created by Alper Çakan on 9.04.2018.
//

#include "Disc.h"

#include <fstream>
#include <iostream>
#include <cstring>

using std::cout;
using std::endl;
using std::fstream;
using std::ifstream;
using std::ofstream;
using std::string;

uint_t Disc::newPageAddr = 1;
bool Disc::discFull = false;

char *Disc::readPage(const string &fileName, const size_t locPageAddr) {
  ifstream file(fileName, ifstream::binary);

  file.seekg(PAGE_SIZE * (locPageAddr - 1));

  char *data = new char[PAGE_SIZE];

  file.read(data, PAGE_SIZE);

  file.close();

  if (!file) return nullptr;

  cout << "-- Reading page #" << *(reinterpret_cast<uint_t *>(data) + 2) << ":"
       << locPageAddr << " (file: " << fileName << ")" << endl;

  return data;
}

bool Disc::writePage(const string &fileName, const size_t locPageAddr,
                     const char *const content) {
  fstream file(fileName, fstream::binary | fstream::in | fstream::out);

  file.seekp(PAGE_SIZE * (locPageAddr - 1));
  file.write(content, PAGE_SIZE);

  file.close();

  if (!file) return false;

  cout << "-- Writing to page #"
       << *(reinterpret_cast<const uint_t *>(content) + 2) << ":" << locPageAddr
       << " (file: " << fileName << ")" << endl;

  return true;
}

uint_t Disc::getPageCount(const string &fileName) {
  ifstream file(fileName, ifstream::binary);

  file.seekg(0, file.end);
  auto size = file.tellg();

  file.close();

  return size / PAGE_SIZE;
}

bool Disc::appendPage(const string &fileName) {
  if (newPageAddr == MAX_PAGE_COUNT) {
    discFull = true;
    return false;
  }

  auto emptyPageData = new char[PAGE_SIZE];
  memset(emptyPageData, 0, PAGE_SIZE);
  *(reinterpret_cast<uint_t *>(emptyPageData) + 2) = newPageAddr++;

  ofstream file(fileName, ofstream::app | ofstream::binary);

  file.write(emptyPageData, PAGE_SIZE);

  file.close();

  delete[] emptyPageData;

  if (!file) {
    return false;
  }

  return true;
}
