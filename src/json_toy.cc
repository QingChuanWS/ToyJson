#include "json_toy.h"

#include <assert.h>

#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <stack>
#include <string>

#include "enum.h"

namespace jst {
JNdParser::JNdParser(const JNdParser& context)
    : str(context.str), str_index(context.str_index), root(context.root) {
  if (context.stack == nullptr) {
    stack = nullptr, size = 0, top = 0;
    return;
  }
  stack = (char*)malloc(context.size * sizeof(char));
  size = context.size;
  memset(this->stack, 0, this->size);
  top = context.top;
  std::memcpy(stack, context.stack, size);
}

JNdParser& JNdParser::operator=(const JNdParser& context) {
  this->str = context.str;
  this->root = context.root;
  this->str_index = context.str_index;

  if (this->stack != nullptr) free(this->stack);
  this->stack = (char*)malloc(context.size * sizeof(char));
  this->size = context.size;
  memset(this->stack, 0, this->size);
  this->top = context.top;
  std::memcpy(stack, context.stack, size);
  return *this;
}

JNdParser::JNdParser(JNdParser&& context)
    : str(std::move(context.str)), root(std::move(context.root)) {
  this->stack = context.stack;
  this->size = context.size;
  this->top = context.top;
  this->str_index = context.str_index;

  context.stack = nullptr;
  context.size = 0;
  context.top = 0;
  context.str_index = 0;
}

JNdParser& JNdParser::operator=(JNdParser&& context) {
  str = std::move(context.str);
  root = std::move(context.root);

  if (this->stack != nullptr) free(this->stack);
  this->stack = context.stack;
  this->size = context.size;
  this->top = context.top;
  this->str_index = context.str_index;

  context.stack = nullptr;
  context.size = 0;
  context.top = 0;
  context.str_index = 0;
  return *this;
}

JNdParser::~JNdParser() {
  JST_DEBUG(top == 0);
  if (this->stack != nullptr) free(stack);
  this->stack = 0;
  this->str_index = 0;
  this->size = 0;
  this->top = 0;
}

void* JNdParser::stack_push(size_t p_size) {
  JST_DEBUG(p_size > 0);
  if (size == 0) {
    this->size = init_stack_size;
    this->stack = (char*)calloc(this->size, sizeof(char));
  }
  if (this->top + p_size > this->size) {
    while (top + p_size >= size) this->size += (this->size >> 1);
    this->stack = (char*)realloc(this->stack, this->size);
    memset(this->stack, 0, this->size);
  }
  void* ret = this->stack + this->top;
  this->top += p_size;
  return ret;
}

void* JNdParser::stack_pop(size_t p_size) {
  JST_DEBUG(this->top >= p_size);
  top -= p_size;
  return this->stack + top;
}

void JNdParser::reset(const std::string& j_str) {
  JST_DEBUG(top == 0);
  this->str = j_str;
  if (this->stack != nullptr) free(stack);
  this->stack = 0;
  this->str_index = 0;
  this->size = 0;
  this->top = 0;
}

JRType JNdParser::jst_parser(JNode* node) {
  if (node == nullptr)
    return jst_val_parser(root);
  else
    return jst_val_parser(*node);
}

JRType JNdParser::jst_ws_parser(jst_ws_state state, JType t) {
  int ws_count = 0;
  int index = this->str_index;
  JRType ret = JST_PARSE_OK;
  for (int i = index; i < this->str.size(); i++) {
    if (this->str[i] == ' ' || this->str[i] == '\n' || this->str[i] == '\t' ||
        this->str[i] == '\r') {
      ws_count++;
      continue;
    }
    break;
  }
  this->str_index += ws_count;

  if (state == JST_WS_BEFORE && this->str_index == this->str.size()) {
    ret = JST_PARSE_EXCEPT_VALUE;
  } else if (state == JST_WS_AFTER && this->str_index != this->str.size()) {
    if (t == JST_ARR) {
      if (this->str[this->str_index] != ',' && this->str[this->str_index] != ']')
        ret = JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
    } else if (t == JST_OBJ) {
      if (this->str[this->str_index] != ',' && this->str[this->str_index] != ':' &&
          this->str[this->str_index] != '}')
        ret = JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
    } else
      ret = JST_PARSE_SINGULAR;
  }
  return ret;
}

JRType JNdParser::jst_val_parser_symbol(JNode& node) {
  JType t;
  int index = this->str_index;
  if (str[index] == 't') {
    JST_CONDATION_STATE(
        str.size() < 4 || str[index + 1] != 'r' || str[index + 2] != 'u' || str[index + 3] != 'e',
        node, JST_PARSE_INVALID_VALUE);
    t = JST_TRUE;
    this->str_index += 4;
  } else if (str[index] == 'f') {
    JST_CONDATION_STATE(str.size() < 5 || str[index + 1] != 'a' || str[index + 2] != 'l' ||
                            str[index + 3] != 's' || str[index + 4] != 'e',
                        node, JST_PARSE_INVALID_VALUE);
    t = JST_FALSE;
    this->str_index += 5;
  } else if (str[index] == 'n') {
    JST_CONDATION_STATE(
        str.size() < 4 || str[index + 1] != 'u' || str[index + 2] != 'l' || str[index + 3] != 'l',
        node, JST_PARSE_INVALID_VALUE);
    t = JST_NULL;
    this->str_index += 4;
  }
  node.jst_node_data_set(t);

  return JST_PARSE_OK;
}

JRType JNdParser::jst_val_parser_number(JNode& node) {
  JST_DEBUG(std::isdigit(this->str[this->str_index]) || this->str[this->str_index] == '+' ||
            this->str[this->str_index] == '-');

  int num_count = 0;
  for (int i = this->str_index; i < str.size(); i++) {
    if (std::isdigit(str[i]) || str[i] == '.' || str[i] == 'e' || str[i] == 'E' || str[i] == '+' ||
        str[i] == '-') {
      num_count++;
      continue;
    }
    break;
  }
  JST_FUNCTION_STATE(
      JST_PARSE_OK,
      node.jst_node_data_set(JST_NUM, str.substr(this->str_index, num_count).c_str(), num_count),
      node);
  this->str_index += num_count;

  return JST_PARSE_OK;
}

JRType JNdParser::jst_val_parser_str_utf(unsigned hex, std::vector<char>& sp_vec) {
  if (hex >= 0x0000 && hex <= 0x007F) {
    sp_vec.push_back(hex & 0x00FF);
  } else if (hex >= 0x0080 && hex <= 0x07FF) {
    sp_vec.push_back(0xC0 | ((hex >> 6) & 0xFF));
    sp_vec.push_back(0x80 | (hex & 0x3F));
  } else if (hex >= 0x0800 && hex <= 0xFFFF) {
    sp_vec.push_back(0xE0 | ((hex >> 12) & 0xFF)); /* 0xE0 = 11100000 */
    sp_vec.push_back(0x80 | ((hex >> 6) & 0x3F));  /* 0x80 = 10000000 */
    sp_vec.push_back(0x80 | (hex & 0x3F));         /* 0x3F = 00111111 */
  } else if (hex >= 0x10000 && hex <= 0x10FFFF) {
    sp_vec.push_back(0xF0 | ((hex >> 18) & 0xFF));
    sp_vec.push_back(0x80 | ((hex >> 12) & 0x3F));
    sp_vec.push_back(0x80 | ((hex >> 6) & 0x3F));
    sp_vec.push_back(0x80 | (hex & 0x3F));
  } else
    return JST_PARSE_INVALID_UNICODE_SURROGATE;
  return JST_PARSE_OK;
}

#define CHAR_VECTOR_PUSH(vector, c, ret_type) \
  vector.push_back(c);                        \
  ret = ret_type;                             \
  break

inline JRType JNdParser::jst_val_parser_str_sp(int& index, std::vector<char>& sp_char) {
  JRType ret = JST_PARSE_OK;
  switch (this->str[index]) {
    case 'n':
      CHAR_VECTOR_PUSH(sp_char, '\n', JST_PARSE_OK);
    case '\\':
      CHAR_VECTOR_PUSH(sp_char, '\\', JST_PARSE_OK);
    case 'b':
      CHAR_VECTOR_PUSH(sp_char, '\b', JST_PARSE_OK);
    case 'f':
      CHAR_VECTOR_PUSH(sp_char, '\f', JST_PARSE_OK);
    case 'r':
      CHAR_VECTOR_PUSH(sp_char, '\r', JST_PARSE_OK);
    case 't':
      CHAR_VECTOR_PUSH(sp_char, '\t', JST_PARSE_OK);
    case '\"':
      CHAR_VECTOR_PUSH(sp_char, '\"', JST_PARSE_OK);
    case '/':
      CHAR_VECTOR_PUSH(sp_char, '/', JST_PARSE_OK);
    case 'u': {
      if (index + 4 > this->str.size() || !std::isxdigit(this->str[index + 1]) ||
          !std::isxdigit(this->str[index + 2]) || !std::isxdigit(this->str[index + 3]) ||
          !std::isxdigit(this->str[index + 4])) {
        ret = JST_PARSE_INVALID_UNICODE_HEX;
        break;
      }
      unsigned hex = std::strtol(this->str.substr(index + 1, 4).c_str(), NULL, 16);
      index += 4;
      if (hex >= 0xD800 && hex <= 0xDBFF) {
        if (index + 6 > this->str.size() || this->str[index + 1] != '\\' ||
            this->str[index + 2] != 'u' || !std::isxdigit(this->str[index + 3]) ||
            !std::isxdigit(this->str[index + 4]) || !std::isxdigit(this->str[index + 5]) ||
            !std::isxdigit(this->str[index + 6])) {
          ret = JST_PARSE_INVALID_UNICODE_SURROGATE;
          break;
        }
        unsigned low_hex = std::strtol(this->str.substr(index + 3, 4).c_str(), NULL, 16);
        if (low_hex < 0xDC00 || low_hex > 0xDFFF) {
          ret = JST_PARSE_INVALID_UNICODE_SURROGATE;
          break;
        }
        hex = 0x10000 + (hex - 0xD800) * 0x400 + (low_hex - 0xDC00);
        index += 6;
      }
      if (JST_PARSE_OK != jst_val_parser_str_utf(hex, sp_char)) {
        ret = JST_PARSE_INVALID_UNICODE_SURROGATE;
        break;
      }
      break;
    }
    default:
      ret = JST_PARSE_INVALID_STRING_ESCAPE;
      break;
  }
  return ret;
}

JRType JNdParser::jst_val_parser_string_base(String& s) {
  JST_DEBUG(this->str[this->str_index] == '\"');
  JRType ret = JST_PARSE_OK;

  int index = this->str_index + 1;
  size_t head = this->top;

  const char* cstr = this->str.c_str();
  int cstr_length = this->str.size() + 1;
  while (index < cstr_length) {
    switch (cstr[index]) {
      case '\"': {
        size_t len = this->top - head;
        char* str_head = (char*)stack_pop(len);
        s = std::move(String(str_head, len));
        index++;
        goto RET;
      }
      case '\0':
        this->top = head;
        ret = JST_PARSE_MISS_QUOTATION_MARK;
        goto RET;
      case '\\': {
        if ((++index) >= cstr_length) {
          ret = JST_PARSE_INVALID_VALUE;
          goto RET;
        }
        std::vector<char> sp_char;
        if (JST_PARSE_OK != (ret = jst_val_parser_str_sp(index, sp_char))) {
          this->top = head;
          goto RET;
        }
        for (int i = 0; i < sp_char.size(); i++)
          *(char*)this->stack_push(sizeof(char)) = sp_char[i];
        break;
      }
      default: {
        if (cstr[index] < 32 || cstr[index] == '"') {
          ret = JST_PARSE_INVALID_STRING_CHAR;
          goto RET;
        }
        char& c = *(char*)this->stack_push(sizeof(char));
        c = cstr[index];
        break;
      }
    }
    index++;
  }
RET:
  if (ret == JST_PARSE_OK) this->str_index = index;
  return ret;
}

JRType JNdParser::jst_val_parser_string(JNode& node) {
  std::unique_ptr<String> s = std::make_unique<String>(String());
  JRType ret = jst_val_parser_string_base(*s);
  if (ret != JST_PARSE_OK)
    node.jst_node_type_reset();
  else
    ret = node.jst_node_data_set(JST_STR, std::move(*s));
  return ret;
}

JRType JNdParser::jst_val_parser_array(JNode& node) {
  JST_DEBUG(this->str[this->str_index++] == '[');
  JST_FUNCTION_STATE(JST_PARSE_OK, jst_ws_parser(JST_WS_BEFORE), node);

  JRType ret = JST_PARSE_OK;
  if (this->str[this->str_index] == ']') {
    node.jst_node_data_set(JST_ARR, Array());
    this->str_index++;
    return ret;
  }

  size_t size = 0;
  size_t head = this->top;
  std::unique_ptr<JNode> jn = std::make_unique<JNode>(JNode());
  for (;;) {
    if (this->str_index == this->str.size()) {
      if (size != 0) ret = JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
      break;
    }
    if ((ret = jst_ws_parser(JST_WS_BEFORE)) != JST_PARSE_OK) break;

    ret = jst_val_parser(*jn, true);
    if (ret != JST_PARSE_OK) {
      break;
    }

    if ((ret = jst_ws_parser(JST_WS_AFTER, JST_ARR)) != JST_PARSE_OK) break;

    auto stack_node = (JNode*)this->stack_push(sizeof(JNode));
    *stack_node = std::move(*jn);
    size++;
    jn->jst_node_type_reset();

    if (this->str[this->str_index] == ',') {
      this->str_index++;
    } else if (this->str[this->str_index] == ']') {
      this->str_index++;
      std::unique_ptr<Array> arr(new Array(size));
      JNode* arr_head = (JNode*)this->stack_pop(size * sizeof(JNode));
      for (int i = 0; i < size; i++) (*arr)[i] = std::move(arr_head[i]);
      node.jst_node_data_set(JST_ARR, std::move(*arr));
      size = 0;
      break;
    } else {
      ret = JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
      break;
    }
  }
  if (ret != JST_PARSE_OK) {
    if (size != 0) {
      JNode* arr_head = (JNode*)this->stack_pop(size * sizeof(JNode));
      for (int i = 0; i < size; i++) arr_head[i].jst_node_type_reset();
    }
    this->top = head;
    node.jst_node_type_reset();
  } else
    JST_FUNCTION_STATE(JST_PARSE_OK, jst_ws_parser(JST_WS_AFTER, JST_ARR), node);
  return ret;
}

JRType JNdParser::jst_val_parser_object_member(OjectMember& objm) {
  JRType ret = JST_PARSE_OK;
  std::unique_ptr<String> s = std::make_unique<String>(String());
  std::unique_ptr<JNode> jn = std::make_unique<JNode>(JNode());

  if (this->str[this->str_index] != '\"') return JST_PARSE_MISS_KEY;
  ret = jst_val_parser_string_base(*s);
  if (ret != JST_PARSE_OK) return ret;

  if ((ret = jst_ws_parser(JST_WS_AFTER, JST_OBJ)) != JST_PARSE_OK) return ret;

  if (this->str[this->str_index++] != ':') {
    ret = JST_PARSE_MISS_COLON;
    return ret;
  }

  if ((ret = jst_ws_parser(JST_WS_BEFORE, JST_OBJ)) != JST_PARSE_OK) return ret;

  if ((ret = jst_val_parser(*jn, true)) != JST_PARSE_OK) return ret;

  objm = std::move(OjectMember(std::move(*s), std::move(*jn)));

  return ret;
}

JRType JNdParser::jst_val_parser_object(JNode& node) {
  JST_DEBUG(this->str[this->str_index++] == '{');
  JST_FUNCTION_STATE(JST_PARSE_OK, jst_ws_parser(JST_WS_BEFORE), node);

  JRType ret = JST_PARSE_OK;
  if (this->str[this->str_index] == '}') {
    node.jst_node_data_set(JST_OBJ, Object());
    this->str_index++;
    return ret;
  }

  size_t size = 0;
  int head = this->top;
  std::unique_ptr<OjectMember> objm = std::make_unique<OjectMember>(OjectMember());
  for (;;) {
    if (this->str_index == this->str.size()) {
      if (size != 0) ret = JST_PARSE_MISS_KEY;
      break;
    }
    if ((ret = jst_ws_parser(JST_WS_BEFORE)) != JST_PARSE_OK) break;

    if ((ret = jst_val_parser_object_member(*objm)) != JST_PARSE_OK) break;

    if ((ret = jst_ws_parser(JST_WS_AFTER, JST_OBJ)) != JST_PARSE_OK) break;

    OjectMember* stack_node = (OjectMember*)this->stack_push(sizeof(OjectMember));
    memset(stack_node, 0, sizeof(OjectMember));
    *stack_node = std::move(*objm);
    size++;

    if (this->str[this->str_index] == ',') {
      this->str_index++;
    } else if (this->str[this->str_index] == '}') {
      this->str_index++;
      std::unique_ptr<Object> obj(new Object(size));
      OjectMember* objm_head = (OjectMember*)this->stack_pop(size * sizeof(OjectMember));

      for (int i = 0; i < size; i++) (*obj)[i] = std::move(objm_head[i]);
      node.jst_node_data_set(JST_OBJ, std::move(*obj));
      break;
    } else {
      ret = JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
      break;
    }
  }

  if (ret == JST_PARSE_OK) {
    JST_FUNCTION_STATE(JST_PARSE_OK, jst_ws_parser(JST_WS_AFTER, JST_OBJ), node);
  } else {
    if (size != 0) {
      OjectMember* objm_head = (OjectMember*)this->stack_pop(size * sizeof(OjectMember));
      for (int i = 0; i < size; i++)
        std::unique_ptr<OjectMember> objm =
            std::make_unique<OjectMember>(std::move(objm_head[i]));
    }
    this->top = head;
    node.jst_node_type_reset();
  }
  return ret;
}

JRType JNdParser::jst_val_parser(JNode& node, bool is_local) {
  if (!is_local) JST_FUNCTION_STATE(JST_PARSE_OK, jst_ws_parser(JST_WS_BEFORE), root);

  JRType ret = JST_PARSE_OK;
  switch (str[this->str_index]) {
    case 'n':
      ret = jst_val_parser_symbol(node);
      break;
    case 't':
      ret = jst_val_parser_symbol(node);
      break;
    case 'f':
      ret = jst_val_parser_symbol(node);
      break;
    case '\"':
      ret = jst_val_parser_string(node);
      break;
    case '[':
      ret = jst_val_parser_array(node);
      break;
    case '{':
      ret = jst_val_parser_object(node);
      break;
    case '0' ... '9':
      ret = jst_val_parser_number(node);
      break;
    case '+':
      ret = jst_val_parser_number(node);
      break;
    case '-':
      ret = jst_val_parser_number(node);
      break;
    default:
      ret = JST_PARSE_INVALID_VALUE;
  }

  if (!is_local && ret == JST_PARSE_OK)
    JST_FUNCTION_STATE(JST_PARSE_OK, jst_ws_parser(JST_WS_AFTER, node.jst_node_type_get()), node);

  return ret;
}

#define PUT_STR(json, json_len)                         \
  do {                                                  \
    char* str_head = (char*)this->stack_push(json_len); \
    memcpy(str_head, json, json_len);                   \
  } while (0)

#define PUT_C(json_c)                            \
  do {                                           \
    char* str_head = (char*)this->stack_push(1); \
    *str_head = json_c;                          \
  } while (0)

void JNdParser::jst_stringify_string(const JNode& jn) {
  JST_DEBUG(jn.jst_node_type_get() == JST_STR);
  char* json_str;
  size_t json_str_len;
  jn.jst_node_data_get(&json_str, json_str_len);
  PUT_C('"');
  for (int i = 0; i < json_str_len; i++) {
    unsigned char ch = (unsigned char)json_str[i];
    switch (ch) {
      case '\"':
        PUT_STR("\\\"", 2);
        break;
      case '\n':
        PUT_STR("\\n", 2);
        break;
      case '\\':
        PUT_STR("\\\\", 2);
        break;
      case '\b':
        PUT_STR("\\b", 2);
        break;
      case '\f':
        PUT_STR("\\f", 2);
        break;
      case '\r':
        PUT_STR("\\r", 2);
        break;
      case '\t':
        PUT_STR("\\t", 2);
        break;
      default:
        if (ch < 0x20) {
          char buffer[7] = {0};
          sprintf(buffer, "\\u%04x", ch);
          PUT_STR(buffer, 6);
        } else
          PUT_C(json_str[i]);
        break;
    }
  }
  PUT_C('"');
}

JRType JNdParser::jst_stringify_value(const JNode& jn) {
  JRType ret = JST_STRINGIFY_OK;
  switch (jn.jst_node_type_get()) {
    case JST_NULL:
      PUT_STR("null", 4);
      break;
    case JST_TRUE:
      PUT_STR("true", 4);
      break;
    case JST_FALSE:
      PUT_STR("false", 5);
      break;
    case JST_STR:
      jst_stringify_string(jn);
      break;
    case JST_NUM: {
      double num = 0.0;
      jn.jst_node_data_get(num);
      this->top -= (32 - sprintf((char*)this->stack_push(32), "%.17g", num));
      break;
    }
    case JST_ARR: {
      PUT_C('[');
      Array arr;
      jn.jst_node_data_get(arr);
      size_t arr_len = arr.size();
      for (int i = 0; i < arr_len; i++) {
        if (i > 0) PUT_C(',');
        jst_stringify_value(arr[i]);
      }
      PUT_C(']');
      break;
    }
    case JST_OBJ: {
      PUT_C('{');
      Object obj;
      jn.jst_node_data_get(obj);
      size_t obj_len = obj.size();
      for (int i = 0; i < obj_len; i++) {
        if (i > 0) PUT_C(',');
        String key = obj[i].get_key();
        PUT_C('"');
        PUT_STR(key.c_str(), key.size());
        PUT_C('"');
        PUT_C(':');
        jst_stringify_value(obj[i].get_value());
      }
      PUT_C('}');
      break;
    }
  }
  return ret;
}

JRType JNdParser::jst_stringify(const JNode& jn, char** json_str, size_t& len) {
  JRType ret = JST_STRINGIFY_OK;
  JST_DEBUG(json_str != nullptr);
  JST_DEBUG(this->top == 0);
  if ((ret = jst_stringify_value(jn)) != JST_STRINGIFY_OK) return ret;
  if (this->top) len = this->top;
  *json_str = (char*)this->stack_pop(this->top);
  return ret;
}

}  // namespace jst