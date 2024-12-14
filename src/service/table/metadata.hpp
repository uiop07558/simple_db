#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "src/engine/page/page.hpp"

using std::string;

enum class FieldType: uint8_t {
  Null = 0,
  Int = 1,
  Float = 2,
  Bool = 3,
  String = 4,
};

struct Field {
  FieldType type{};
  uint16_t number{};
  string name;
  bool isKey;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Field, type, number, name);
};

struct TableMetadata {
  pageptr_t rootId{};
  string name;
  vector<Field> fields;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TableMetadata, rootId, name, fields);
};