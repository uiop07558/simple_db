#include <cassert>
#include <iostream>
#include <vector>
#include <map>

#include "../bptree.hpp"
#include "../../pager/pager.hpp"

using std::byte;
using std::vector;
using std::map;
using std::cout;
using std::endl;

// Mock Pager for Testing
class MockPager : public Pager {
 public:
  map<pageptr_t, Page> pages;
  pageptr_t nextId = 1;

  pageptr_t addPage(const Page& page) override {
    pageptr_t id = nextId++;
    pages[id] = page;
    return id;
  }

  Page getPage(pageptr_t id) override {
    return pages.at(id);
  }

  void delPage(pageptr_t id) override {
    pages.erase(id);
  }
};

#define NUM_SMALL_INSERTS 100
#define NUM_LARGE_INSERTS 256
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

void initBptree(Pager& pager) {
  Page leafPage = Page::createLeaf();
  pager.addPage(leafPage);
}

void testInsertSingleElement() {
  MockPager pager;
  initBptree(pager);
  Bptree tree(pager, 1);

  vector<byte> key = generateBytes(1, byte{'a'});
  vector<byte> value = generateBytes(1, byte{'1'});
  tree.insert(key, value);

  auto result = tree.search(key);
  assert(result.has_value());
  assert(result.value() == value);
}

void testInsertMultipleElements() {
  MockPager pager;
  initBptree(pager);
  Bptree tree(pager, 1);

  for (int i = 0; i < NUM_SMALL_INSERTS; ++i) {
    auto key = generateBytes(10, byte{i});
    auto value = generateBytes(20, byte{i + 100});
    tree.insert(key, value);
  }

  for (int i = 0; i < NUM_SMALL_INSERTS; ++i) {
    auto key = generateBytes(10, byte{i});
    auto result = tree.search(key);
    assert(result.has_value());
    assert(result.value() == generateBytes(20, byte{i + 100}));
  }
}

void testTriggerLeafSplit() {
  MockPager pager;
  initBptree(pager);
  Bptree tree(pager, 1);

  for (int i = 0; i < NUM_LARGE_INSERTS; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto value = generateBytes(LARGE_VALUE_SIZE, byte{i + 100});
    tree.insert(key, value);
  }

  for (int i = 0; i < NUM_LARGE_INSERTS; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto result = tree.search(key);
    assert(result.has_value());
    assert(result.value() == generateBytes(LARGE_VALUE_SIZE, byte{i + 100}));
  }

  assert(pager.pages.size() > 1);
}

void testTriggerInternalSplit() {
  MockPager pager;
  initBptree(pager);
  Bptree tree(pager, 1);

  for (int i = 0; i < NUM_LARGE_INSERTS * 5; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE / 2, byte{i});
    auto value = generateBytes(LARGE_VALUE_SIZE / 2, byte{i + 50});
    tree.insert(key, value);
  }

  for (int i = 0; i < NUM_LARGE_INSERTS * 5; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE / 2, byte{i});
    auto result = tree.search(key);
    assert(result.has_value());
    assert(result.value() == generateBytes(LARGE_VALUE_SIZE / 2, byte{i + 50}));
  }

  assert(pager.pages.size() >= 10);
}

void testDeleteLeafKey() {
  MockPager pager;
  initBptree(pager);
  Bptree tree(pager, 1);

  auto key = generateBytes(10, byte{'a'});
  auto value = generateBytes(20, byte{'1'});
  tree.insert(key, value);

  tree.remove(key);
  auto result = tree.search(key);
  assert(!result.has_value());
}

void testDeleteNonExistentKey() {
  MockPager pager;
  initBptree(pager);
  Bptree tree(pager, 1);

  tree.remove(generateBytes(10, byte{'z'}));
  assert(pager.pages.size() == 1);
}

void testLeafMerge() {
  MockPager pager;
  initBptree(pager);
  Bptree tree(pager, 1);

  for (int i = 0; i < NUM_LARGE_INSERTS; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto value = generateBytes(LARGE_VALUE_SIZE, byte{i + 50});
    tree.insert(key, value);
  }

  for (int i = NUM_LARGE_INSERTS / 2; i < NUM_LARGE_INSERTS; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    tree.remove(key);
  }

  for (int i = 0; i < NUM_LARGE_INSERTS / 2; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto result = tree.search(key);
    assert(result.has_value());
    assert(result.value() == generateBytes(LARGE_VALUE_SIZE, byte{i + 50}));
  }

  assert(pager.pages.size() <= NUM_LARGE_INSERTS);
}

void testStress() {
  MockPager pager;
  initBptree(pager);
  Bptree tree(pager, 1);

  const int NUM_KEYS = 256;
  for (int i = 0; i < NUM_KEYS; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto value = generateBytes(LARGE_VALUE_SIZE, byte{i+59});
    tree.insert(key, value);
  }

  for (int i = 0; i < NUM_KEYS; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto result = tree.search(key);
    assert(result.has_value());
    assert(result.value() == generateBytes(LARGE_VALUE_SIZE, byte{i+59}));
  }

  for (int i = NUM_KEYS / 2; i < NUM_KEYS; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    tree.remove(key);
  }

  for (int i = 0; i < NUM_KEYS / 2; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto result = tree.search(key);
    assert(result.has_value());
    assert(result.value() == generateBytes(LARGE_VALUE_SIZE, byte{i+59}));
  }
}

void testBptreeIterator() {
  MockPager pager;
  initBptree(pager);
  Bptree tree(pager, 1);

  vector<pair<vector<byte>, vector<byte>>> keyValues;
  for (int i = 0; i < NUM_LARGE_INSERTS; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto value = generateBytes(LARGE_VALUE_SIZE, byte{i + 50});
    keyValues.emplace_back(key, value);
    tree.insert(key, value);
  }

  BptreeIterator it = tree.iterate();
  size_t index = 0;

  while (it.hasNext()) {
    auto [key, value] = it.next();
    assert(key == keyValues[index].first);
    assert(value == keyValues[index].second);
    index += 1;
  }

  assert(index == keyValues.size());
}


int main() {
  RUN_TEST(testInsertSingleElement);
  RUN_TEST(testInsertMultipleElements);
  RUN_TEST(testTriggerLeafSplit);
  RUN_TEST(testTriggerInternalSplit);
  RUN_TEST(testDeleteLeafKey);
  RUN_TEST(testDeleteNonExistentKey);
  RUN_TEST(testLeafMerge);
  RUN_TEST(testStress);
  RUN_TEST(testBptreeIterator);

  cout << "All tests passed" << endl;
  return 0;
}