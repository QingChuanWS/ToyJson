#include "parser.h"

#include <assert.h>

#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <stack>
#include <string>

#include "basic.h"
#include "enum.h"

namespace jst {

JParser::JParser(const JParser& parser)
    : str(parser.str), str_index(parser.str_index), root(parser.root) {
  if (parser.stack == nullptr) {
    stack = nullptr, size = 0, top = 0;
    return;
  }
  stack = (char*)malloc(parser.size * sizeof(char));
  size = parser.size;
  memset(this->stack, 0, this->size);
  top = parser.top;
  std::memcpy(stack, parser.stack, size);
}

JParser& JParser::operator=(const JParser& parser) {
  this->str = parser.str;
  this->root = parser.root;
  this->str_index = parser.str_index;

  if (this->stack != nullptr) {
    free(this->stack);
  }
  this->stack = (char*)malloc(parser.size * sizeof(char));
  this->size = parser.size;
  memset(this->stack, 0, this->size);
  this->top = parser.top;
  std::memcpy(stack, parser.stack, size);
  return *this;
}

JParser::JParser(JParser&& parser) : str(std::move(parser.str)), root(std::move(parser.root)) {
  this->stack = parser.stack;
  this->size = parser.size;
  this->top = parser.top;
  this->str_index = parser.str_index;

  parser.stack = nullptr;
  parser.size = 0;
  parser.top = 0;
  parser.str_index = 0;
}

JParser& JParser::operator=(JParser&& parser) {
  str = std::move(parser.str);
  root = std::move(parser.root);

  if (this->stack != nullptr) {
    free(this->stack);
  }
  this->stack = parser.stack;
  this->size = parser.size;
  this->top = parser.top;
  this->str_index = parser.str_index;

  parser.stack = nullptr;
  parser.size = 0;
  parser.top = 0;
  parser.str_index = 0;
  return *this;
}

JParser::~JParser() {
  JST_DEBUG(top == 0);
  if (this->stack != nullptr) {
    free(stack);
  }
  this->stack = 0;
  this->str_index = 0;
  this->size = 0;
  this->top = 0;
}

void* JParser::stack_push(size_t p_size) {
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

void* JParser::stack_pop(size_t p_size) {
  JST_DEBUG(this->top >= p_size);
  top -= p_size;
  return this->stack + top;
}

void JParser::reset(const std::string& j_str) {
  JST_DEBUG(top == 0);
  this->str = j_str;
  if (this->stack != nullptr) free(stack);
  this->stack = 0;
  this->str_index = 0;
  this->size = 0;
  this->top = 0;
}

JRetType JParser::parser(JNode* node) {
  return node == nullptr ? main_parser(root) : main_parser(*node);
}

JRetType JParser::jst_ws_parser(jst_ws_state state, JNType t) {
  int ws_count = 0;
  int index = this->str_index;
  for (int i = index; i < this->str.size(); i++) {
    if (this->str[i] == ' ' || this->str[i] == '\n' || this->str[i] == '\t' ||
        this->str[i] == '\r') {
      ws_count++;
      continue;
    }
    break;
  }
  this->str_index += ws_count;

  auto ret = JST_PARSE_OK;
  if (state == JST_WS_BEFORE && this->str_index == this->str.size()) {
    ret = JST_PARSE_EXCEPT_VALUE;
  }
  if (state == JST_WS_AFTER && this->str_index != this->str.size()) {
    if (t == JST_ARR) {
      if (this->str[this->str_index] != ',' && this->str[this->str_index] != ']')
        ret = JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
    } else if (t == JST_OBJ) {
      if (this->str[this->str_index] != ',' && this->str[this->str_index] != ':' &&
          this->str[this->str_index] != '}')
        ret = JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
    } else {
      ret = JST_PARSE_SINGULAR;
    }
  }
  return ret;
}

JRetType JParser::parser_symbol(JNode& node) {
  JNType t;
  auto index = this->str_index;
  auto ret = JST_PARSE_OK;

  if (str[index] == 't') {
    if (str.size() < 4 || str[index + 1] != 'r' || str[index + 2] != 'u' || str[index + 3] != 'e') {
      t = JST_NULL;
      ret = JST_PARSE_INVALID_VALUE;
      goto RETURN;
    }
    t = JST_TRUE;
    this->str_index += 4;
  } else if (str[index] == 'f') {
    if (str.size() < 5 || str[index + 1] != 'a' || str[index + 2] != 'l' || str[index + 3] != 's' ||
        str[index + 4] != 'e') {
      t = JST_NULL;
      ret = JST_PARSE_INVALID_VALUE;
      goto RETURN;
    }
    t = JST_FALSE;
    this->str_index += 5;
  } else if (str[index] == 'n') {
    if (str.size() < 4 || str[index + 1] != 'u' || str[index + 2] != 'l' || str[index + 3] != 'l') {
      t = JST_NULL;
      ret = JST_PARSE_INVALID_VALUE;
      goto RETURN;
    }
    t = JST_NULL;
    this->str_index += 4;
  }
RETURN:
  node.jst_node_data_set(t);
  return ret;
}

JRetType JParser::parser_number(JNode& node) {
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

JRetType JParser::parser_utf_str(unsigned hex, std::vector<char>& sp_vec) {
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

inline JRetType JParser::parser_specifical_str(int& index, std::vector<char>& sp_char) {
  JRetType ret = JST_PARSE_OK;
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
      if (JST_PARSE_OK != parser_utf_str(hex, sp_char)) {
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

JRetType JParser::parser_string_base(JString& s) {
  JST_DEBUG(this->str[this->str_index] == '\"');
  JRetType ret = JST_PARSE_OK;

  int index = this->str_index + 1;
  size_t head = this->top;

  const char* cstr = this->str.c_str();
  int cstr_length = this->str.size() + 1;
  while (index < cstr_length) {
    switch (cstr[index]) {
      case '\"': {
        size_t len = this->top - head;
        char* str_head = (char*)stack_pop(len);
        s = std::move(JString(str_head, len));
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
        if (JST_PARSE_OK != (ret = parser_specifical_str(index, sp_char))) {
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

JRetType JParser::parser_string(JNode& node) {
  std::unique_ptr<JString> s = std::make_unique<JString>(JString());
  auto ret = parser_string_base(*s);
  if (ret == JST_PARSE_OK) {
    ret = node.jst_node_data_set(JST_STR, std::move(*s));
  }
  return ret;
}

JRetType JParser::parser_array(JNode& node) {
  JST_DEBUG(this->str[this->str_index++] == '[');
  JST_FUNCTION_STATE(JST_PARSE_OK, jst_ws_parser(JST_WS_BEFORE), node);

  auto ret = JST_PARSE_OK;
  if (this->str[this->str_index] == ']') {
    node.jst_node_data_set(JST_ARR, JArray());
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
    if ((ret = jst_ws_parser(JST_WS_BEFORE)) != JST_PARSE_OK) {
      break;
    }
    ret = main_parser(*jn, true);
    if (ret != JST_PARSE_OK) {
      break;
    }
    if ((ret = jst_ws_parser(JST_WS_AFTER, JST_ARR)) != JST_PARSE_OK) {
      break;
    }

    auto stack_node = (JNode*)this->stack_push(sizeof(JNode));
    *stack_node = std::move(*jn);
    size++;

    if (this->str[this->str_index] == ',') {
      this->str_index++;
    } else if (this->str[this->str_index] == ']') {
      this->str_index++;
      std::unique_ptr<JArray> arr(new JArray(size));
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
    node.jst_node_data_set(JST_NULL);
    this->top = head;
  } else {
    JST_FUNCTION_STATE(JST_PARSE_OK, jst_ws_parser(JST_WS_AFTER, JST_ARR), node);
  }
  return ret;
}

JRetType JParser::parser_object_member(JOjectElement& objm) {
  JRetType ret = JST_PARSE_OK;
  std::unique_ptr<JString> s = std::make_unique<JString>(JString());
  std::unique_ptr<JNode> jn = std::make_unique<JNode>(JNode());

  if (this->str[this->str_index] != '\"') {
    return JST_PARSE_MISS_KEY;
  }
  ret = parser_string_base(*s);
  if (ret != JST_PARSE_OK) {
    return ret;
  }

  if ((ret = jst_ws_parser(JST_WS_AFTER, JST_OBJ)) != JST_PARSE_OK) {
    return ret;
  }

  if (this->str[this->str_index++] != ':') {
    ret = JST_PARSE_MISS_COLON;
    return ret;
  }
  if ((ret = jst_ws_parser(JST_WS_BEFORE, JST_OBJ)) != JST_PARSE_OK) {
    return ret;
  }
  if ((ret = main_parser(*jn, true)) != JST_PARSE_OK) {
    return ret;
  }

  objm = std::move(JOjectElement(std::move(*s), std::move(*jn)));
  return ret;
}

JRetType JParser::parser_object(JNode& node) {
  JST_DEBUG(this->str[this->str_index++] == '{');
  JST_FUNCTION_STATE(JST_PARSE_OK, jst_ws_parser(JST_WS_BEFORE), node);

  JRetType ret = JST_PARSE_OK;
  if (this->str[this->str_index] == '}') {
    node.jst_node_data_set(JST_OBJ, JObject());
    this->str_index++;
    return ret;
  }

  size_t size = 0;
  int head = this->top;
  std::unique_ptr<JOjectElement> objm = std::make_unique<JOjectElement>(JOjectElement());
  for (;;) {
    if (this->str_index == this->str.size()) {
      if (size != 0) ret = JST_PARSE_MISS_KEY;
      break;
    }
    if ((ret = jst_ws_parser(JST_WS_BEFORE)) != JST_PARSE_OK) {
      break;
    }
    if ((ret = parser_object_member(*objm)) != JST_PARSE_OK) {
      break;
    }
    if ((ret = jst_ws_parser(JST_WS_AFTER, JST_OBJ)) != JST_PARSE_OK) {
      break;
    }

    JOjectElement* stack_node = (JOjectElement*)this->stack_push(sizeof(JOjectElement));
    memset(stack_node, 0, sizeof(JOjectElement));
    *stack_node = std::move(*objm);
    size++;

    if (this->str[this->str_index] == ',') {
      this->str_index++;
    } else if (this->str[this->str_index] == '}') {
      this->str_index++;
      std::unique_ptr<JObject> obj(new JObject(size));
      JOjectElement* objm_head = (JOjectElement*)this->stack_pop(size * sizeof(JOjectElement));

      for (int i = 0; i < size; i++) (*obj)[i] = std::move(objm_head[i]);
      node.jst_node_data_set(JST_OBJ, std::move(*obj));
      break;
    } else {
      ret = JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
      break;
    }
  }

  if (ret == JST_PARSE_OK) {
    auto ret = jst_ws_parser(JST_WS_AFTER, JST_OBJ);
    if (ret != JST_PARSE_OK) {
      node.jst_node_data_set(JST_NULL);
      return ret;
    }
  } else {
    if (size != 0) {
      JOjectElement* objm_head = (JOjectElement*)this->stack_pop(size * sizeof(JOjectElement));
      for (int i = 0; i < size; i++)
        std::unique_ptr<JOjectElement> objm = std::make_unique<JOjectElement>(std::move(objm_head[i]));
    }
    this->top = head;
  }
  return ret;
}

JRetType JParser::main_parser(JNode& node, bool is_local) {
  if (!is_local) {
    auto ret = jst_ws_parser(JST_WS_BEFORE);
    if (ret != JST_PARSE_OK) {
      node.jst_node_data_set(JST_NULL);
      return ret;
    }
  }

  JRetType ret = JST_PARSE_OK;
  switch (str[this->str_index]) {
    case 'n':
      ret = parser_symbol(node);
      break;
    case 't':
      ret = parser_symbol(node);
      break;
    case 'f':
      ret = parser_symbol(node);
      break;
    case '\"':
      ret = parser_string(node);
      break;
    case '[':
      ret = parser_array(node);
      break;
    case '{':
      ret = parser_object(node);
      break;
    case '0' ... '9':
      ret = parser_number(node);
      break;
    case '+':
      ret = parser_number(node);
      break;
    case '-':
      ret = parser_number(node);
      break;
    default:
      ret = JST_PARSE_INVALID_VALUE;
  }

  if (!is_local && ret == JST_PARSE_OK) {
    auto ret = jst_ws_parser(JST_WS_AFTER, node.type());
    if (ret != JST_PARSE_OK) {
      node.jst_node_data_set(JST_NULL);
      return ret;
    }
  }
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

void JParser::stringify_string(const JNode& jn) {
  JST_DEBUG(jn.type() == JST_STR);
  const char* json_str;
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

JRetType JParser::stringify_value(const JNode& jn) {
  JRetType ret = JST_STRINGIFY_OK;
  switch (jn.type()) {
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
      stringify_string(jn);
      break;
    case JST_NUM: {
      double num = 0.0;
      jn.jst_node_data_get(num);
      this->top -= (32 - sprintf((char*)this->stack_push(32), "%.17g", num));
      break;
    }
    case JST_ARR: {
      PUT_C('[');
      JArray arr;
      jn.jst_node_data_get(arr);
      size_t arr_len = arr.size();
      for (int i = 0; i < arr_len; i++) {
        if (i > 0) PUT_C(',');
        stringify_value(arr[i]);
      }
      PUT_C(']');
      break;
    }
    case JST_OBJ: {
      PUT_C('{');
      JObject obj;
      jn.jst_node_data_get(obj);
      size_t obj_len = obj.size();
      for (int i = 0; i < obj_len; i++) {
        if (i > 0) PUT_C(',');
        JString key = obj[i].get_key();
        PUT_C('"');
        PUT_STR(key.c_str(), key.size());
        PUT_C('"');
        PUT_C(':');
        stringify_value(obj[i].get_value());
      }
      PUT_C('}');
      break;
    }
  }
  return ret;
}

JRetType JParser::stringify(const JNode& jn, char** json_str, size_t& len) {
  JRetType ret = JST_STRINGIFY_OK;
  JST_DEBUG(json_str != nullptr);
  JST_DEBUG(this->top == 0);
  if ((ret = stringify_value(jn)) != JST_STRINGIFY_OK) {
    return ret;
  }
  if (this->top) {
    len = this->top;
  }
  *json_str = (char*)this->stack_pop(this->top);
  return ret;
}

}  // namespace jst