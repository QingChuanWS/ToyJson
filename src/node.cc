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

JNode::JNode(JNType t, const char* str, size_t len) : ty(JST_NULL), data(nullptr) {
  JST_DEBUG(t == JST_NUM || t == JST_STR);
  ty = t;
  if (ty == JST_NUM) {
    if (len == 0) {
      jst_node_parser_num(std::string(str));
    } else {
      jst_node_parser_num(std::string(str, len));
    }
  } else if (ty == JST_STR)
    jst_node_parser_str(str, len);
}

JNode::JNode(JNType t, double num) {
  JST_DEBUG(t == JST_NUM);
  ty = JST_NUM;
  data = std::make_shared<JNumber>(JNumber(num));
}

JNode::JNode(JNType t, JArray& arr) {
  JST_DEBUG(t == JST_ARR);
  ty = JST_ARR;
  data = std::make_shared<JArray>(JArray(arr));
}

JNode::JNode(JNType t, JObject& obj) {
  JST_DEBUG(t == JST_OBJ);
  ty = JST_OBJ;
  data = std::make_shared<JObject>(JObject(obj));
}

// copy construct
JNode::JNode(const JNode& node) {
  ty = node.ty;
  data = jst_node_data_copy(node.ty, *node.data);
}

// assigment construct
JNode& JNode::operator=(const JNode& node) {
  ty = node.ty;
  data = jst_node_data_copy(node.ty, *node.data);
  return *this;
}

// move copy construct
JNode::JNode(JNode&& node) noexcept {
  ty = node.ty;
  data = node.data;
  node.data = nullptr;
  node.ty = JST_NULL;
}

// move assigment construct
JNode& JNode::operator=(JNode&& node) noexcept {
  ty = node.ty;
  data = node.data;
  node.data = nullptr;
  node.ty = JST_NULL;

  return *this;
}

// deconstruct
JNode::~JNode() {
  ty = JST_NULL;
  data = nullptr;
}

void JNode::jst_node_data_reset(const bool val) {
  ty = val ? JST_TRUE : JST_FALSE;
  data = nullptr;
}

void JNode::jst_node_data_reset(const JString& val) {
  ty = JST_STR;
  data = std::make_shared<JString>(val);
}
void JNode::jst_node_data_reset(const double val) {
  ty = JST_NUM;
  data = std::make_shared<JNumber>(val);
}
void JNode::jst_node_data_reset(const JArray& val) {
  ty = JST_ARR;
  data = std::make_shared<JArray>(val);
}
void JNode::jst_node_data_reset(const JObject& val) {
  ty = JST_OBJ;
  data = std::make_shared<JObject>(val);
}

// get the data of number node;
void JNode::jst_node_data_get(double& num) const {
  JST_DEBUG(ty == JST_NUM);
  num = data->as<JNumber>().get();
}

// get the data of string node;
void JNode::jst_node_data_get(std::string& str) const {
  JST_DEBUG(ty == JST_STR);
  auto d = data->as<JString>();
  str = std::string(d.c_str(), d.size());
}

void JNode::jst_node_data_get(const char** node_str, size_t& len) const {
  JST_DEBUG(ty == JST_STR);
  *node_str = data->as<JString>().c_str();
  len = data->as<JString>().size();
}

void JNode::jst_node_data_get(bool& b) const {
  JST_DEBUG(ty == JST_FALSE || ty == JST_TRUE);
  b = ty == JST_TRUE ? true : false;
}

void JNode::jst_node_data_get(JArray& arr) const {
  JST_DEBUG(ty == JST_ARR);
  arr = data->as<JArray>();
}

void JNode::jst_node_data_get(JObject& obj) const {
  JST_DEBUG(ty == JST_OBJ);
  obj = data->as<JObject>();
}

size_t JNode::jst_node_data_length_get() const {
  JST_DEBUG(ty == JST_STR);
  return data->as<JString>().size();
}

JRetType JNode::jst_node_parser_num(const std::string& str) {
  if (str[0] == '0' && str.size() > 1) {
    ty = JST_NULL;
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
    ty = JST_NULL;
    return ret;
  }
  std::string n_str = str.substr(0, index);
  double n = std::strtod(n_str.c_str(), NULL);

  if (n == HUGE_VAL || n == -HUGE_VAL) {
    ty = JST_NULL;
    return JST_PARSE_NUMBER_TOO_BIG;
  }
  data = std::make_shared<JNumber>(JNumber(n));
  return ret;
}

JRetType JNode::jst_node_parser_str(const char* str, size_t len) {
  data = std::make_shared<JString>(JString(str, len));
  return JST_PARSE_OK;
}

JRetType JNode::jst_node_data_set(JNType t, const char* str, const size_t len) {
  JST_DEBUG(t != JST_ARR);
  JRetType ret = JST_PARSE_OK;
  ty = t;
  if (t == JST_NULL || t == JST_TRUE || t == JST_FALSE)
    ret = JST_PARSE_OK;
  else if (ty == JST_NUM) {
    ret = jst_node_parser_num(std::string(str, len));
  } else if (ty == JST_STR) {
    ret = jst_node_parser_str(str, len);
  }
  if (ret != JST_PARSE_OK) {
    ty = JST_NULL;
  }
  return ret;
}

JRetType JNode::jst_node_data_set(JNType t, JString&& s) {
  JST_DEBUG(t == JST_STR);
  JRetType ret = JST_PARSE_OK;
  ty = t;
  data = std::make_shared<JString>(JString(s));
  return ret;
}

JRetType JNode::jst_node_data_set(JNType t, JArray&& arr) {
  JST_DEBUG(t == JST_ARR);
  ty = t;
  data = std::make_shared<JArray>(JArray(arr));
  return JST_PARSE_OK;
}

JRetType JNode::jst_node_data_set(JNType t, JObject&& obj) {
  JST_DEBUG(t == JST_OBJ);
  ty = t;
  data = std::make_shared<JObject>(JObject(obj));
  return JST_PARSE_OK;
}

JRetType JNode::jst_node_data_set(JNType t, double num) {
  JST_DEBUG(t == JST_NUM);
  ty = t;
  data = std::make_shared<JNumber>(JNumber(num));
  return JST_PARSE_OK;
}

bool operator==(const JNode& left, const JNode& right) {
  if (left.ty != right.ty) {
    return false;
  }

  switch (left.type()) {
    case JST_STR:
      return left.data->as<JString>() == right.data->as<JString>();
    case JST_NUM:
      return left.data->as<JNumber>() == right.data->as<JNumber>();
    case JST_ARR:
      return left.data->as<JArray>() == right.data->as<JArray>();
    case JST_OBJ:
      return left.data->as<JObject>() == right.data->as<JObject>();
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