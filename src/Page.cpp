//
// Created by Alper Çakan on 9.04.2018.
//

#include "Page.h"
#include "Disc.h"
#include <cstring>

using std::exception;
using std::string;

Page::Page() : data(new char[PAGE_SIZE]), isModified(true) {
  memset(this->data, 0, PAGE_SIZE);
}

Page::Page(string fileName, uint_t pageAddr)
    : data(Disc::readPage(fileName, pageAddr)),
      locAddr(pageAddr),
      fileName(fileName),
      isModified(false) {}

bool Page::isUsed() {
  return *(reinterpret_cast<const uint_t *>(whole()) +
           PAGE_HEADER_IS_USED_INDEX);
}

uint_t Page::pageCategory() {
  return *(reinterpret_cast<const uint_t *>(whole()) +
           PAGE_HEADER_PAGE_CAT_INDEX);
}

uint_t Page::globAddr() {
  return *(reinterpret_cast<const uint_t *>(whole()) +
           PAGE_HEADER_GLOB_ADDR_INDEX);
}

void Page::setIsUsed(bool isUsed) {
  isModified = true;
  *(reinterpret_cast<uint_t *>(whole()) + PAGE_HEADER_IS_USED_INDEX) = isUsed;
}

void Page::setPageCategory(uint_t pageCategory) {
  isModified = true;
  *(reinterpret_cast<uint_t *>(whole()) + PAGE_HEADER_PAGE_CAT_INDEX) =
      pageCategory;
}

void Page::setGlobAddr(uint_t globAddr) {
  isModified = true;
  *(reinterpret_cast<uint_t *>(whole()) + PAGE_HEADER_GLOB_ADDR_INDEX) =
      globAddr;
}

char *Page::contentAddr() {
  return whole() + sizeof(uint_t) * PAGE_HEADER_ELEM_COUNT;
}

const char *Page::content() { return contentAddr(); }

char *Page::whole() {
  if (!(*this)) {
    throw exception();
  }

  return data;
}

uint_t Page::getUIntAtPos(uint_t pos) {
  return *reinterpret_cast<const uint_t *>(contentAddr() + pos);
}

bool Page::writeContent(const char *const data, uint_t len, uint_t pos) {
  if (len > PAGE_SIZE - PAGE_HEADER_ELEM_COUNT - pos) return false;

  isModified = true;

  memcpy(contentAddr() + pos, data, len);

  return true;
}

Page::~Page() { delete[] data; }

bool Page::persist(string fileName, uint_t locAddr) {
  if (fileName.empty()) {
    fileName = this->fileName;
  } else {
    this->fileName = fileName;
  }

  if (locAddr == 0) {
    locAddr = this->locAddr;
  } else {
    this->locAddr = locAddr;
  }

  if (fileName.empty() || locAddr == 0) {
    return false;
  }

  if (isModified) {
    return Disc::writePage(fileName, locAddr, whole());
  }

  return true;
}

Page *Page::getConsecPage(bool forceGet) {
  Page *consecPage = nullptr;

  if (locAddr >= Disc::getPageCount(fileName)) {
    if (forceGet) {
      if (!Disc::appendPage(fileName)) {
        return nullptr;
      }

      consecPage = new Page(fileName, locAddr + 1);
    }
  } else {
    consecPage = new Page(fileName, locAddr + 1);
  }

  if (!consecPage || !(*consecPage)) {
    delete consecPage;
    return nullptr;
  }

  return consecPage;
}

uint_t Page::getLocAddr() { return locAddr; }

int Page::firstEmptyCellIndex(size_t cellSize) {
  if (!isUsed()) {
    reset();
    return 0;
  }

  for (int i = 0; i < CONTENT_SIZE / cellSize; ++i) {
    if (getUIntAtPos(cellSize * i) == 0) {
      return i;
    }
  }

  return -1;  // No empty cell in this page
}

void Page::reset() {
  auto glob = globAddr();
  memset(whole(), 0, PAGE_SIZE);
  setGlobAddr(glob);
}

void Page::resetRange(size_t pos, size_t len) {
  memset((void *)(contentAddr() + pos), 0, len);
}

Page::operator bool() const { return data; }
