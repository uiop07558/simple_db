#pragma once

#include <mutex>
#include <unordered_map>
#include <optional>
#include <deque>
#include <utility>
#include <filesystem>
#include <cstddef>
#include <string>
#include <cstdint>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>

#include <boost/thread/shared_mutex.hpp>

#include "./pager.hpp"
#include "../page/page.hpp"
#include "../page/meta_page.hpp"

using std::mutex;
using std::unordered_map;
using std::optional;
using std::deque;
using std::nullopt;
using std::move;
using std::pair;
using std::string;
using std::filesystem::path;

using boost::upgrade_mutex;

typedef uint64_t txid_t;

enum class PageActionType: uint8_t {
  Write,
  Delete,
};

struct PageAction {
  PageActionType type;
  pageptr_t pageId;
  optional<Page> page;
};

class TransactionalPager;

class TransactionalPagerLocal: public Pager {
 private:
  txid_t txid;
  TransactionalPager& manager;

  TransactionalPagerLocal(TransactionalPager& manager, txid_t txid): txid(txid), manager(manager) {}
 public:
  friend class TransactionalPager;

  pageptr_t addPage(const Page& page) override;
  Page getPage(pageptr_t id) override;
  void delPage(pageptr_t id) override;

  void saveMetaPage(const MetaPage& metaPage) override;
  MetaPage getMetaPage() override;
};

namespace {
  struct TxInfo {
    bool writeMode;
    string tableId;
  };
};

class TransactionalPager {
 private:
  byte* mmapPtr{};
  size_t mmapLen{};
  int64_t fd{};
  size_t fileLen{};
  upgrade_mutex fileLock; // fileLock protects only info about mapping, not the mapping itself

  unordered_map<string, upgrade_mutex> tableLocks;
  txid_t txidSeq{};
  unordered_map<txid_t, TxInfo> txInfo; 
  unordered_map<txid_t, deque<PageAction>> actions;
  deque<pageptr_t> newFreeList;
  pageptr_t appendNum{};
  MetaPage newMeta;
  mutex txLock; // txLock protects info about transactions

  MetaPage meta;
  deque<pageptr_t> freeList;
  upgrade_mutex metaLock; // metaLock protects info about meta page and meta page itself

  void writePageToMmap(Page& page, pageptr_t writeTo) {
    if (writeTo == 0) {
      return;
    }

    page.setPageType(page.byteSize());
    page.data.resize(PAGE_SIZE);

    fileLock.lock_shared();
    memcpy(mmapPtr + writeTo * PAGE_SIZE, page.data.data(), PAGE_SIZE);
    fileLock.unlock_shared();
  }

  void cleanTransaction(txid_t txid) {
    actions.erase(txid);
    upgrade_mutex& tableLock = tableLocks[txInfo[txid].tableId];
    if (txInfo[txid].writeMode) { 
      newMeta = MetaPage();
      newFreeList = deque<pageptr_t>{};
      appendNum = 0;

      tableLock.unlock_upgrade();
      metaLock.unlock_upgrade();
    }
    else {
      tableLock.unlock_shared();
      metaLock.unlock_shared();
    }
    txInfo.erase(txid);
  }

  void syncMeta() {
    MetaPage writePage = meta;
    writePage.data.resize(PAGE_SIZE);
    
    fileLock.lock_shared();
    memcpy(mmapPtr, writePage.data.data(), PAGE_SIZE);
    fsync(fd);
    fileLock.unlock_shared();
  }

  void loadMeta() {
    assert(fileLen >= PAGE_SIZE);
    metaLock.lock_shared();
    fileLock.lock_shared();
    unsafe_buf<byte> buf = {
      ptr: mmapPtr,
      len: PAGE_SIZE,
    };

    MetaPage page(buf);
    fileLock.unlock_shared();
    pagesize_t dataSize = page.getByteSize();
    page.data.resize(dataSize);
    meta = page;
    metaLock.unlock_shared();
  }

  Page loadPage(pageptr_t id) {
    fileLock.lock_shared();
    assert(id * PAGE_SIZE < fileLen);
    assert(fileLen % PAGE_SIZE == 0);

    unsafe_buf<byte> buf = {
      ptr: mmapPtr + (id * PAGE_SIZE),
      len: PAGE_SIZE,
    };

    Page page(buf);
    fileLock.unlock_shared();
    pagesize_t dataSize = page.getByteSize();
    page.data.resize(dataSize);
    return page;
  }

  void syncFreeList();
  void loadFreeList();

  pageptr_t findPlace(txid_t txid);
  void allocate(size_t appendNum);
 public:
  friend class TransactionalPagerLocal;

  pageptr_t addPage(const Page& page, txid_t txid);
  Page getPage(pageptr_t id, txid_t txid) ;
  void delPage(pageptr_t id, txid_t txid);

  txid_t startTransaction(bool writable, string tableId);
  inline TransactionalPagerLocal getLocal(txid_t txid) { return TransactionalPagerLocal(*this, txid); }

  void commit(txid_t txid);
  inline void rollback(txid_t txid);
  
  void saveMetaPage(const MetaPage& metaPage, txid_t txid);
  MetaPage getMetaPage(txid_t txid) ;

  TransactionalPager(path path);

  TransactionalPager(const TransactionalPager&) = delete;
  TransactionalPager& operator=(const TransactionalPager&) = delete;
  TransactionalPager(TransactionalPager&&) = default;
  TransactionalPager& operator=(TransactionalPager&&) = default;
  ~TransactionalPager() = default;
};
