#include <cassert>
#include <iostream>
#include <vector>
#include <map>

#include "../bptree.hpp"
#include "../../pager/pager.hpp"

using std::byte;
using std::vector;

// Mock Pager for Testing
class MockPager : public Pager {
public:
  std::map<pageptr_t, Page> pages;
  pageptr_t nextId = 1;

  pageptr_t addPage(const Page& page) override {
    pageptr_t id = nextId++;
    pages[id] = page;
    return id;
  }

  Page getPage(pageptr_t id) const override {
    return pages.at(id);
  }

  void delPage(pageptr_t id) override {
    pages.erase(id);
  }
};

#define NUM_SMALL_INSERTS 100
#define NUM_LARGE_INSERTS 10
#define LARGE_KEY_SIZE 512
#define LARGE_VALUE_SIZE 1530

#define RUN_TEST(test) \
  std::cout << "Running " << #test << "... "; \
  test(); \
  std::cout << "PASSED\n";

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

  assert(pager.pages.size() > 10);
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
    tree.remove(generateBytes(LARGE_KEY_SIZE, byte{i}));
  }

  for (int i = 0; i < NUM_LARGE_INSERTS / 2; ++i) {
    auto key = generateBytes(LARGE_KEY_SIZE, byte{i});
    auto result = tree.search(key);
    assert(result.has_value());
    assert(result.value() == generateBytes(LARGE_VALUE_SIZE, byte{i + 50}));
  }

  assert(pager.pages.size() < NUM_LARGE_INSERTS / 2);
}

void testStress() {
  MockPager pager;
  initBptree(pager);
  Bptree tree(pager, 1);

  const int NUM_KEYS = 500;
  for (int i = 0; i < NUM_KEYS; ++i) {
    tree.insert(generateBytes(LARGE_KEY_SIZE, byte{i}),
          generateBytes(LARGE_VALUE_SIZE, byte{i}));
  }

  for (int i = 0; i < NUM_KEYS; ++i) {
    auto result = tree.search(generateBytes(LARGE_KEY_SIZE, byte{i}));
    assert(result.has_value());
    assert(result.value() == generateBytes(LARGE_VALUE_SIZE, byte{i}));
  }

  for (int i = 0; i < NUM_KEYS / 2; ++i) {
    tree.remove(generateBytes(LARGE_KEY_SIZE, byte{i}));
  }

  for (int i = NUM_KEYS / 2; i < NUM_KEYS; ++i) {
    auto result = tree.search(generateBytes(LARGE_KEY_SIZE, byte{i}));
    assert(result.has_value());
    assert(result.value() == generateBytes(LARGE_VALUE_SIZE, byte{i}));
  }
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

  std::cout << "All tests passed" << std::endl;
  return 0;
}