#include "./transactional_pager.hpp"

#include <sys/mman.h>
#include <sys/stat.h>
#include <cstdio>

using std::max;

#define EXPAND_RATE (100)


pageptr_t TransactionalPagerLocal::addPage(const Page& page) {
  return this->manager.addPage(page, txid);
}

Page TransactionalPagerLocal::getPage(pageptr_t id) {
  return this->manager.getPage(id, txid);
}

void TransactionalPagerLocal::delPage(pageptr_t id) {
  this->manager.delPage(id, txid);
}

void TransactionalPagerLocal::saveMetaPage(const MetaPage& metaPage) {
  this->manager.saveMetaPage(metaPage, txid);
}

MetaPage TransactionalPagerLocal::getMetaPage() {
  return this->manager.getMetaPage(txid);
};

pageptr_t TransactionalPager::addPage(const Page& page, txid_t txid) {
  txLock.lock();
  if (!txInfo[txid].writeMode) {
    txLock.unlock();
    return 0;
  }

  pageptr_t writeTo = findPlace(txid);
  actions[txid].emplace_front(PageActionType::Write, writeTo, page);
  txLock.unlock();
  return writeTo;
}

Page TransactionalPager::getPage(pageptr_t id, txid_t txid) {
  Page page;
  bool foundInMem = false;
  txLock.lock();
  if (txInfo[txid].writeMode) {
    for (int i = actions[txid].size() - 1; i >= 0; i--) {
      PageAction action = actions[txid][i];
      if (action.type == PageActionType::Write && action.pageId == id) {
        page = action.page.value();
        foundInMem = true;
        break;
      }
    }
  }

  if (!foundInMem) {
    page = loadPage(id);
  }
  txLock.unlock();

  return page;
}

void TransactionalPager::delPage(pageptr_t id, txid_t txid) {
  txLock.lock();
  if (!txInfo[txid].writeMode) {
    txLock.unlock();
    return;
  }
  actions[txid].emplace_front(PageActionType::Delete, id, nullopt);
  txLock.unlock();
}

pageptr_t TransactionalPager::findPlace(txid_t txid) {
  pageptr_t writeTo = 0;
  if (newFreeList.empty()) {
    writeTo = meta.getCursize() + appendNum;
    appendNum++;
  }
  else {
    writeTo = newFreeList.back();
    newFreeList.pop_back();
  }
  return writeTo;
}

void TransactionalPager::allocate(size_t appendNum) {
  if ((meta.getCursize() + appendNum) * PAGE_SIZE > fileLen) {
    fileLock.unlock_upgrade_and_lock();
    // expand file
    size_t expandLen = max((uint64_t) EXPAND_RATE * PAGE_SIZE, (uint64_t) appendNum * PAGE_SIZE);
    ftruncate(fd, fileLen + expandLen);
    fileLen += fileLen + expandLen;
    munmap(mmapPtr, mmapLen);
    mmapPtr = reinterpret_cast<byte*>(mmap(nullptr, fileLen, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0));
    mmapLen = fileLen;

    fileLock.unlock_and_lock_upgrade();
  }
}

void TransactionalPager::syncFreeList() {
  pageptr_t oldHeadId = meta.getFreeListHead();
  pageptr_t oldTailId = meta.getFreeListTail();
  if (oldHeadId != 0 && oldTailId != 0) {
    pageptr_t oldListCurId = oldHeadId;
    Page oldListCurPage = loadPage(oldListCurId);
    while (oldListCurId != oldTailId) {
      freeList.push_front(oldListCurId);
      DeletedPage oldListCur(oldListCurPage);
      oldListCurPage = loadPage(oldListCur.getNext());
    }
  }

  pageptr_t listAppendNum = (freeList.size() / MAX_DELETED_COUNT) + ((freeList.size() % MAX_DELETED_COUNT) != 0);
  allocate(listAppendNum);

  for (pageptr_t i = 0; i < listAppendNum; i++) {
    Page page = Page::createDeleted();
    DeletedPage newDeleted(page);
    size_t curFreeListSize = freeList.size();
    for (size_t j = 0; j < min((size_t) MAX_DELETED_COUNT, curFreeListSize); j++) {
      newDeleted.putPtr(freeList[MAX_DELETED_COUNT * i + j]);
    }

    if (i < listAppendNum - 1) {
      newDeleted.setNext(meta.getCursize() + i + 1);
    }
    else {
      newDeleted.setNext(0);
    }

    writePageToMmap(newDeleted.page, meta.getCursize() + i);
  }

  fsync(fd);

  if (listAppendNum == 0) {
    meta.setFreeListHead(0);
    meta.setFreeListTail(0);
  }
  else {
    meta.setFreeListHead(meta.getCursize());
    meta.setFreeListTail(meta.getCursize() + listAppendNum - 1);
  }

  meta.setCursize(meta.getCursize() + listAppendNum);
}

void TransactionalPager::loadFreeList() {
  if (meta.getFreeListHead() == 0 || meta.getFreeListTail() == 0) {
    assert(meta.getFreeListHead() == meta.getFreeListTail());
    return;
  }

  pageptr_t freeListHead = meta.getFreeListHead();
  pageptr_t freeListCur = freeListHead;
  while (freeListCur != 0) {
    Page deletedPage = loadPage(freeListCur);
    DeletedPage deleted(deletedPage);
    for (pagesize_t i = 0; i < deleted.getCount(); i++) {
      freeList.push_front(deleted.getPtr(i));
      freeListCur = deleted.getNext();
    }
  }
}

txid_t TransactionalPager::startTransaction(bool writable, tableid_t tableId) {
  txid_t retId = 0;
  txLock.lock();
  actions[txidSeq] = deque<PageAction>();
  txInfo[txidSeq] = TxInfo {
    writeMode: writable,
    tableId: tableId,
  };
  retId = txidSeq;
  txidSeq++;
  upgrade_mutex& tableLock = tableLocks[tableId];
  if (writable) {
    newMeta = meta;
    newFreeList = freeList;
    appendNum = 0;
  }
  txLock.unlock();
  if (writable) {
    tableLock.lock_upgrade();
    metaLock.lock_upgrade();
  } 
  else {
    tableLock.lock_shared();
    metaLock.lock_shared();
  }
  
  return retId;
}

void TransactionalPager::commit(txid_t txid) {
  txLock.lock();
  auto actions = this->actions[txid];
  upgrade_mutex& tableLock = tableLocks[txInfo[txid].tableId];
  bool writeMode = txInfo[txid].writeMode;
  txLock.unlock();

  if (writeMode) {
    tableLock.unlock_upgrade_and_lock();
    metaLock.unlock_upgrade_and_lock();

    meta = newMeta;
    freeList = newFreeList;
    fileLock.lock_upgrade();
    allocate(appendNum);

    unordered_map<pageptr_t, PageAction> writeTable;
    while (!actions.empty()) {
      PageAction action = move(actions.front());
      actions.pop_front();

      switch (action.type) {
        case PageActionType::Write: {
          // Page page = action.page.value();
          // writePageToMmap(page, action.pageId);
          writeTable[action.pageId] = action;

          break;
        }
        case PageActionType::Delete: {
          // freeList.push_front(action.pageId);
          if (!writeTable.contains(action.pageId) || writeTable[action.pageId].type == PageActionType::Delete) {
            writeTable[action.pageId] = action;
          }
          else {
            writeTable.erase(action.pageId);
          }

          break;
        }
      }
    }
    for (auto item: writeTable) {
      PageAction action = item.second;
      switch (action.type) {
        case PageActionType::Write: {
          Page page = action.page.value();
          writePageToMmap(page, action.pageId);

          break;
        }
        case PageActionType::Delete: {
          freeList.push_front(action.pageId);

          break;
        }
      }
    }

    fsync(fd);
    meta.setCursize(meta.getCursize() + appendNum);
    syncFreeList();
    syncMeta();

    fileLock.unlock_upgrade();

    metaLock.unlock_and_lock_upgrade();
    tableLock.unlock_and_lock_upgrade();
  }

  txLock.lock();
  cleanTransaction(txid);
  txLock.unlock();
}

inline void TransactionalPager::rollback(txid_t txid) {
  txLock.lock();
  cleanTransaction(txid);
  txLock.unlock();
}

inline MetaPage TransactionalPager::getMetaPage(txid_t txid) {
  return meta;
}

inline void TransactionalPager::saveMetaPage(const MetaPage &metaPage, txid_t txid) {
  txLock.lock();
  if (!txInfo[txid].writeMode) {
    return;
  }
  newMeta = metaPage;
  txLock.unlock();
}

TransactionalPager::TransactionalPager(path path) {
  mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;

  int dirfd = open(path.parent_path().c_str(), O_RDONLY | O_DIRECTORY, S_IRWXU);
  if (dirfd < 0) {
    perror("bad path");
    exit(errno);
  }

  int filefd = openat(dirfd, path.filename().c_str(), O_RDWR | O_CREAT | O_EXCL, mode);
  if (filefd < 0) {
    if (errno == EEXIST) {
      filefd = openat(dirfd, path.filename().c_str(), O_RDWR);
    }

    if (filefd < 0) {
      perror("bad path");
      exit(errno);
    }
  }

  fsync(dirfd);

  struct stat statbuf;
  int status = fstat(filefd, &statbuf);
  if (status < 0) {
    perror("fstat");
    exit(errno);
  }

  fd = filefd;
  if (statbuf.st_size < PAGE_SIZE) { // init meta
    ftruncate(filefd, PAGE_SIZE);
    meta.setCursize(1);
    fileLen = PAGE_SIZE;
    mmapLen = PAGE_SIZE;
    void* mmapRet = mmap(nullptr, fileLen, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (mmapRet == MAP_FAILED) {
      perror("mmap");
      exit(errno);
    }
    mmapPtr = reinterpret_cast<byte*>(mmapRet);
    syncMeta();
  }
  else {
    fileLen = statbuf.st_size;
    mmapLen = statbuf.st_size;
    void* mmapRet = mmap(nullptr, fileLen, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (mmapRet == MAP_FAILED) {
      perror("mmap");
      exit(errno);
    }
    mmapPtr = reinterpret_cast<byte*>(mmapRet);
    loadMeta();
    loadFreeList();
  }
}
