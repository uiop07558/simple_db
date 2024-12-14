#include <cassert>
#include <iostream>
#include <vector>
#include <map>

#include "../../bptree/bptree.hpp"
#include "../transactional_pager.hpp"

using std::byte;
using std::vector;
using std::map;
using std::cout;
using std::endl;

TransactionalPager thePager("./data.db");

#define NUM_SMALL_INSERTS 100
#define NUM_LARGE_INSERTS 250
#define LARGE_KEY_SIZE 512
#define LARGE_VALUE_SIZE 1500

#define RUN_TEST(test) \
  cout << "Running " << #test << "... "; \
  test(); \
  cout << "PASSED\n";

vector<byte> generateBytes(size_t size, byte start = byte{0}) {
  vector<byte> result(size);
  for (size_t i = 0; i < size; ++i) {
    result[i] = static_cast<byte>((static_cast<int>(start) + i) % 256);
  }
  return result;
}

void testInsertSingleElement() {
  txid_t txidWrite = thePager.startTransaction(true, 0);
  TransactionalPagerLocal pagerWrite = thePager.getLocal(txidWrite);

  Bptree treeWrite = Bptree::createTree(pagerWrite);

  vector<byte> key = generateBytes(1, byte{'a'});
  vector<byte> value = generateBytes(1, byte{'1'});
  treeWrite.insert(key, value);

  MetaPage meta = pagerWrite.getMetaPage();
  meta.setMetaTableRoot(treeWrite.getRootId());
  pagerWrite.saveMetaPage(meta);

  thePager.commit(txidWrite);

  txid_t txidRead = thePager.startTransaction(false, 0);
  auto pagerRead = thePager.getLocal(txidRead);

  auto treeRead = Bptree(pagerRead, pagerRead.getMetaPage().getMetaTableRoot());

  auto result = treeRead.search(key);
  assert(result.has_value());
  assert(result.value() == value);

  thePager.commit(txidRead);
}

void testLeafSplit() {
  txid_t txidInsert = thePager.startTransaction(true, 0);
  TransactionalPagerLocal pagerInsert = thePager.getLocal(txidInsert);

  Bptree treeInsert = Bptree::createTree(pagerInsert);

  for (int i = 0; i < NUM_LARGE_INSERTS; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto value = generateBytes(LARGE_VALUE_SIZE, byte{i + 50});
    treeInsert.insert(key, value);
  }

  MetaPage meta = pagerInsert.getMetaPage();
  meta.setMetaTableRoot(treeInsert.getRootId());
  pagerInsert.saveMetaPage(meta);

  thePager.commit(txidInsert);

  txid_t txidRead = thePager.startTransaction(false, 0);
  auto pagerRead = thePager.getLocal(txidRead);

  auto treeRead = Bptree(pagerRead, pagerRead.getMetaPage().getMetaTableRoot());

  for (int i = 0; i < NUM_LARGE_INSERTS; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto result = treeRead.search(key);
    assert(result.has_value());
    assert(result.value() == generateBytes(LARGE_VALUE_SIZE, byte{i + 50}));
  }

  thePager.commit(txidRead);
}

void testLeafMerge() {
  txid_t txidInsert = thePager.startTransaction(true, 0);
  TransactionalPagerLocal pagerInsert = thePager.getLocal(txidInsert);

  Bptree treeInsert = Bptree::createTree(pagerInsert);

  for (int i = 0; i < NUM_LARGE_INSERTS; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto value = generateBytes(LARGE_VALUE_SIZE, byte{i + 50});
    treeInsert.insert(key, value);
  }

  MetaPage meta = pagerInsert.getMetaPage();
  meta.setMetaTableRoot(treeInsert.getRootId());
  pagerInsert.saveMetaPage(meta);

  thePager.commit(txidInsert);

  txid_t txidDelete = thePager.startTransaction(true, 0);
  TransactionalPagerLocal pagerDelete = thePager.getLocal(txidDelete);

  auto treeDelete = Bptree(pagerDelete, pagerDelete.getMetaPage().getMetaTableRoot());

  for (int i = NUM_LARGE_INSERTS / 2; i < NUM_LARGE_INSERTS; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    treeDelete.remove(key);
  }

  meta = pagerDelete.getMetaPage();
  meta.setMetaTableRoot(treeDelete.getRootId());
  pagerDelete.saveMetaPage(meta);

  thePager.commit(txidDelete);

  txid_t txidRead = thePager.startTransaction(false, 0);
  auto pagerRead = thePager.getLocal(txidRead);

  auto treeRead = Bptree(pagerRead, pagerRead.getMetaPage().getMetaTableRoot());

  for (int i = 0; i < NUM_LARGE_INSERTS / 2; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto result = treeRead.search(key);
    assert(result.has_value());
    assert(result.value() == generateBytes(LARGE_VALUE_SIZE, byte{i + 50}));
  }

  thePager.commit(txidRead);
}

// void testBptreeIterator() {
//   MockPager pager;
//   initBptree(pager);
//   Bptree tree(pager, 1);

//   vector<pair<vector<byte>, vector<byte>>> keyValues;
//   for (int i = 0; i < NUM_LARGE_INSERTS; ++i) {
//     auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
//     auto value = generateBytes(LARGE_VALUE_SIZE, byte{i + 50});
//     keyValues.emplace_back(key, value);
//     tree.insert(key, value);
//   }

//   BptreeIterator it = tree.iterate();
//   size_t index = 0;

//   while (it.hasNext()) {
//     auto [key, value] = it.next();
//     assert(key == keyValues[index].first);
//     assert(value == keyValues[index].second);
//     index += 1;
//   }

//   assert(index == keyValues.size());
// }


int main() {
  RUN_TEST(testInsertSingleElement);
  RUN_TEST(testLeafSplit);
  RUN_TEST(testLeafMerge);
  // RUN_TEST(testBptreeIterator);

  cout << "All tests passed" << endl;
  return 0;
}