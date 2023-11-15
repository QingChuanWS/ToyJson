#include "node.h"

#include <assert.h>

#include <cctype>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <stack>

#include "basic.h"
#include "enum.h"

namespace jst {

static shared_ptr<JData> jst_node_data_copy(JNType type, const JData& node) {
  switch (type) {
    case JST_STR:
      return std::make_shared<JString>(node.as<JString>());
    case JST_NUM:
      return std::make_shared<JNumber>(node.as<JNumber>());
    case JST_ARR:
      return std::make_shared<JArray>(node.as<JArray>());
    case JST_OBJ:
      return std::make_shared<JObject>(node.as<JObject>());
    default:
      return nullptr;
  }
}

JNode::JNode(JNType t, const char* str, size_t len) : _type(JST_NULL), _data(nullptr) {
  _type = t;
  if (_type == JST_NUM) {
    if (len == 0) {
      jst_node_parser_num(std::string(str));
    } else {
      jst_node_parser_num(std::string(str, len));
    }
  } else if (_type == JST_STR){
    _data = std::make_shared<JString>(JString(str, len));
  }
}

// copy construct
JNode::JNode(const JNode& node) {
  _type = node._type;
  _data = jst_node_data_copy(node._type, *node._data);
}

// assigment construct
JNode& JNode::operator=(const JNode& node) {
  _type = node._type;
  _data = jst_node_data_copy(node._type, *node._data);
  return *this;
}

// move copy construct
JNode::JNode(JNode&& node) noexcept {
  _type = node._type;
  _data = node._data;
  node._data = nullptr;
  node._type = JST_NULL;
}

// move assigment construct
JNode& JNode::operator=(JNode&& node) noexcept {
  _type = node._type;
  _data = node._data;
  node._data = nullptr;
  node._type = JST_NULL;

  return *this;
}

// deconstruct
JNode::~JNode() {
  _type = JST_NULL;
  _data = nullptr;
}

JRetType JNode::jst_node_parser_num(const std::string& str) {
  if (str[0] == '0' && str.size() > 1) {
    _type = JST_NULL;
    return JST_PARSE_SINGULAR;
  }

  auto ret = JST_PARSE_OK;
  NumberExp exp_state;
  NumberPoint point_state;

  int index = 0;
  if (str[0] == '-') index += 1;

  while (index < str.size()) {
    if (str[index] == ' ') {
      break;
    }
    if (std::isdigit(str[index])) {
      if (exp_state.is_have && point_state.is_have &&
          exp_state.exp_index < point_state.point_index) {
        ret = JST_PARSE_INVALID_VALUE;
        break;
      }
    } else if (str[index] == 'E' || str[index] == 'e') {
      if (exp_state.is_have || index + 1 >= str.size()) {
        ret = JST_PARSE_INVALID_VALUE;
        break;
      }
      exp_state.is_have = true;
      if (str[index + 1] == '+' || str[index + 1] == '-') {
        exp_state.exp_index = index + 2;
        index++;
      } else if (std::isdigit(str[index + 1]))
        exp_state.exp_index = index + 1;
      else {
        ret = JST_PARSE_INVALID_VALUE;
        break;
      }
    } else if (str[index] == '.') {
      if (point_state.is_have || index + 1 >= str.size()) {
        ret = JST_PARSE_INVALID_VALUE;
        break;
      }
      point_state.is_have = true;
      point_state.point_index = index;
    } else {
      ret = JST_PARSE_INVALID_VALUE;
      break;
    }
    index++;
  }
  if (ret != JST_PARSE_OK) {
    _type = JST_NULL;
    return ret;
  }
  std::string n_str = str.substr(0, index);
  double n = std::strtod(n_str.c_str(), NULL);

  if (n == HUGE_VAL || n == -HUGE_VAL) {
    _type = JST_NULL;
    return JST_PARSE_NUMBER_TOO_BIG;
  }
  _data = std::make_shared<JNumber>(JNumber(n));
  return ret;
}


JRetType JNode::data_set(JNType t, const char* str, const size_t len) {
  JST_DEBUG(t != JST_ARR);
  JRetType ret = JST_PARSE_OK;
  _type = t;
  if (t == JST_NULL || t == JST_TRUE || t == JST_FALSE)
    ret = JST_PARSE_OK;
  else if (_type == JST_NUM) {
    ret = jst_node_parser_num(std::string(str, len));
  } else if (_type == JST_STR) {
    _data = std::make_shared<JString>(JString(str, len));
  }
  if (ret != JST_PARSE_OK) {
    _type = JST_NULL;
  }
  return ret;
}

bool operator==(const JNode& left, const JNode& right) {
  if (left._type != right._type) {
    return false;
  }

  switch (left.type()) {
    case JST_STR:
      return left._data->as<JString>() == right._data->as<JString>();
    case JST_NUM:
      return left._data->as<JNumber>() == right._data->as<JNumber>();
    case JST_ARR:
      return left._data->as<JArray>() == right._data->as<JArray>();
    case JST_OBJ:
      return left._data->as<JObject>() == right._data->as<JObject>();
    default:
      return true;
  }
}

bool operator!=(JNode& jn_1, JNode& jn_2) { return !(jn_1 == jn_2); }

void swap(JNode& jn_1, JNode& jn_2) {
  JNode tmp = std::move(jn_1);
  jn_1 = std::move(jn_2);
  jn_2 = std::move(tmp);
}

}  // namespace jst