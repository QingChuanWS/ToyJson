#include "json_toy_node.h"

#include <assert.h>

#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>
#include <stack>

#include "basic.h"

namespace jst {
template <typename JND_Type>
static JND_Type& jst_node_data_mem_get(JNodeData& data) {
  return dynamic_cast<JND_Type&>(data);
}

static shared_ptr<JNodeData> jst_node_data_copy(JType type, shared_ptr<JNodeData> jnd) {
  switch (type) {
    case JST_STR:
      return std::make_shared<String>(String(jst_node_data_mem_get<String>(*jnd)));
    case JST_NUM:
      return std::make_shared<Number>(Number(jst_node_data_mem_get<Number>(*jnd)));
    case JST_ARR:
      return std::make_shared<Array>(Array(jst_node_data_mem_get<Array>(*jnd)));
    case JST_OBJ:
      return std::make_shared<Object>(Object(jst_node_data_mem_get<Object>(*jnd)));
    default:
      return nullptr;
  }
}

JNode::JNode(JType t, const char* str, size_t len) : type(JST_NULL), data(nullptr) {
  JST_DEBUG(t == JST_NUM || t == JST_STR);
  type = t;
  if (type == JST_NUM) {
    if (len == 0)
      jst_node_parser_num(std::string(str));
    else
      jst_node_parser_num(std::string(str, len));
  } else if (type == JST_STR)
    jst_node_parser_str(str, len);
}

JNode::JNode(JType t, double num) : type(JST_NULL), data(nullptr) {
  JST_DEBUG(t == JST_NUM);
  type = t;
  this->data = std::make_shared<Number>(Number(num));
}

JNode::JNode(JType t, Array& arr) : type(JST_ARR), data(nullptr) {
  JST_DEBUG(t == JST_ARR);
  this->data = std::make_shared<Array>(Array(arr));
}

JNode::JNode(JType t, Object& obj) : type(JST_OBJ), data(nullptr) {
  JST_DEBUG(t == JST_ARR);
  this->data = std::make_shared<Object>(Object(obj));
}

// copy construct
JNode::JNode(const JNode& node) : type(node.type) {
  if (node.data == nullptr) {
    this->data = nullptr;
    return;
  }
  this->data = jst_node_data_copy(node.type, node.data);
}

// assigment construct
JNode& JNode::operator=(const JNode& node) {
  type = node.type;
  this->data = jst_node_data_copy(node.type, node.data);
  return *this;
}

// move copy construct
JNode::JNode(JNode&& node) noexcept : type(node.type), data(std::move(node.data)) {
  node.data = nullptr;
  node.type = JST_NULL;
}

// move assigment construct
JNode& JNode::operator=(JNode&& node) noexcept {
  this->type = node.type;
  this->data = std::move(node.data);
  node.data = nullptr;
  node.type = JST_NULL;

  return *this;
}

// deconstruct
JNode::~JNode() {
  type = JST_NULL;
  this->data = nullptr;
}

// reset the node type as JST_NULL
void JNode::jst_node_type_reset() {
  this->data = nullptr;
  type = JST_NULL;
}

void JNode::jst_node_data_reset(const bool data) {
  this->type = data ? JST_TRUE : JST_FALSE;
  this->data = nullptr;
}

void JNode::jst_node_data_reset(const String& data) {
  this->type = JST_STR;
  this->data = std::make_shared<String>(data);
}
void JNode::jst_node_data_reset(const double data) {
  this->type = JST_NUM;
  this->data = std::make_shared<Number>(data);
}
void JNode::jst_node_data_reset(const Array& data) {
  this->type = JST_ARR;
  this->data = std::make_shared<Array>(data);
}
void JNode::jst_node_data_reset(const Object& data) {
  this->type = JST_OBJ;
  this->data = std::make_shared<Object>(data);
}

// get the data of number node;
void JNode::jst_node_data_get(double& num) const {
  JST_DEBUG(type == JST_NUM);
  auto& this_data = jst_node_data_mem_get<Number>(*(this->data));
  num = this_data.num;
}

// get the data of string node;
void JNode::jst_node_data_get(std::string& str) const {
  JST_DEBUG(type == JST_STR);
  auto& this_data = jst_node_data_mem_get<String>(*(this->data));
  str = std::string(this_data.c_str(), this_data.size());
}

void JNode::jst_node_data_get(char** node_str, size_t& len) const {
  JST_DEBUG(type == JST_STR);
  auto& this_data = jst_node_data_mem_get<String>(*(this->data));
  *node_str = this_data.c_str();
  len = this_data.size();
}

void JNode::jst_node_data_get(bool& b) const {
  JST_DEBUG(type == JST_FALSE || type == JST_TRUE);
  b = type == JST_TRUE ? true : false;
}

void JNode::jst_node_data_get(Array& arr) const {
  JST_DEBUG(type == JST_ARR);
  auto& this_data = jst_node_data_mem_get<Array>(*(this->data));
  arr = this_data;
}

void JNode::jst_node_data_get(Object& obj) const {
  JST_DEBUG(type == JST_OBJ);
  auto& this_data = jst_node_data_mem_get<Object>(*(this->data));
  obj = this_data;
}

size_t JNode::jst_node_data_length_get() const {
  JST_DEBUG(type == JST_STR);
  auto& this_data = jst_node_data_mem_get<String>(*(this->data));
  return this_data.size();
}

JRType JNode::jst_node_parser_num(const std::string& str) {
  JST_CONDATION_STATE(str[0] == '0' && str.size() > 1, (*this), JST_PARSE_SINGULAR);

  JRType ret = JST_PARSE_OK;
  NumberExp exp_state;
  NumberPoint point_state;

  int index = 0;
  if (str[0] == '-') index += 1;

  while (index < str.size()) {
    if (str[index] == ' ') break;
    if (std::isdigit(str[index])) {
      JST_CONDATION_STATE(
          exp_state.is_have && point_state.is_have && exp_state.exp_index < point_state.point_index,
          (*this), JST_PARSE_INVALID_VALUE);
    } else if (str[index] == 'E' || str[index] == 'e') {
      JST_CONDATION_STATE(exp_state.is_have || index + 1 >= str.size(), (*this),
                          JST_PARSE_INVALID_VALUE);
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
      JST_CONDATION_STATE(point_state.is_have || index + 1 >= str.size(), (*this),
                          JST_PARSE_INVALID_VALUE);
      point_state.is_have = true;
      point_state.point_index = index;
    } else {
      ret = JST_PARSE_INVALID_VALUE;
      break;
    }
    index++;
  }

  JST_CONDATION_STATE(ret != JST_PARSE_OK, (*this), ret);
  std::string n_str = str.substr(0, index);
  double n = std::strtod(n_str.c_str(), NULL);
  JST_CONDATION_STATE(n == HUGE_VAL || n == -HUGE_VAL, (*this), JST_PARSE_NUMBER_TOO_BIG)
  this->data = std::make_shared<Number>(Number(n));

  return ret;
}

JRType JNode::jst_node_parser_str(const char* str, size_t len) {
  this->data = std::make_shared<String>(String(str, len));
  return JST_PARSE_OK;
}

JRType JNode::jst_node_data_set(JType t, const char* str, const size_t len) {
  JST_DEBUG(t != JST_ARR);
  JRType ret = JST_PARSE_OK;
  type = t;
  if (t == JST_NULL || t == JST_TRUE || t == JST_FALSE)
    ret = JST_PARSE_OK;
  else if (type == JST_NUM)
    ret = jst_node_parser_num(std::string(str, len));
  else if (type == JST_STR)
    ret = jst_node_parser_str(str, len);
  if (ret != JST_PARSE_OK) this->data = nullptr;
  return ret;
}

JRType JNode::jst_node_data_set(JType t, String&& s) {
  JST_DEBUG(t == JST_STR);
  JRType ret = JST_PARSE_OK;
  type = t;
  this->data = std::make_shared<String>(String(s));
  return ret;
}

JRType JNode::jst_node_data_set(JType t, Array&& arr) {
  JST_DEBUG(t == JST_ARR);
  type = t;
  this->data = std::make_shared<Array>(Array(arr));
  return JST_PARSE_OK;
}

JRType JNode::jst_node_data_set(JType t, Object&& obj) {
  JST_DEBUG(t == JST_OBJ);
  type = t;
  this->data = std::make_shared<Object>(Object(obj));
  return JST_PARSE_OK;
}

JRType JNode::jst_node_data_set(JType t, double num) {
  JST_DEBUG(t == JST_NUM);
  type = t;
  this->data = std::make_shared<Number>(Number(num));
  return JST_PARSE_OK;
}

bool operator==(const JNode& jn_1, const JNode& jn_2) {
  if (jn_1.jst_node_type_get() != jn_2.jst_node_type_get()) return false;
  switch (jn_1.jst_node_type_get()) {
    case JST_STR:
      return jst_node_data_mem_get<String>(*jn_1.data) == jst_node_data_mem_get<String>(*jn_2.data);
    case JST_NUM:
      return jst_node_data_mem_get<Number>(*jn_1.data) == jst_node_data_mem_get<Number>(*jn_2.data);
    case JST_ARR:
      return jst_node_data_mem_get<Array>(*jn_1.data) == jst_node_data_mem_get<Array>(*jn_2.data);
    case JST_OBJ:
      return jst_node_data_mem_get<Object>(*jn_1.data) == jst_node_data_mem_get<Object>(*jn_2.data);
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