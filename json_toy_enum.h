#ifndef __JSON_TOY_ENUM_H__
#define __JSON_TOY_ENUM_H__

#include <string>

namespace jst {

#define JST_CONDATION_STATE(condition, node, ret) \
  if (condition) {                                \
    jst_node& p = node;                           \
    node.jst_node_type_reset();                   \
    return ret;                                   \
  }

#define JST_FUNCTION_STATE(expect, func, node) \
  do {                                         \
    jst_ret_type ret = func;                   \
    if (ret != expect) {                       \
      node.jst_node_type_reset();              \
      return ret;                              \
    }                                          \
  } while (0);

#define JST_DEBUG(expr) assert(expr)

#define ASSERT_VECTOR_NO_RET(vec, head, length) \
  if (vec.empty()) {                            \
    this->head = nullptr;                       \
    this->length = 0;                           \
    return;                                     \
  }

#define ASSERT_VECTOR_HAS_RET(vec, head, length, ret) \
  if (vec.empty()) {                                  \
    this->head = nullptr;                             \
    this->length = 0;                                 \
    return ret;                                       \
  }

typedef enum { JST_NULL = 0, JST_TRUE, JST_FALSE, JST_NUM, JST_STR, JST_ARR, JST_OBJ } jst_type;

extern const char* jst_node_type_name[7];

typedef enum {
  JST_PARSE_OK = 0,
  JST_PARSE_EXCEPT_VALUE,
  JST_PARSE_INVALID_VALUE,
  JST_PARSE_ROOT_NOST,
  JST_PARSE_SINGULAR,
  JST_PARSE_NUMBER_TOO_BIG,
  JST_PARSE_MISS_QUOTATION_MARK,
  JST_PARSE_INVALID_STRING_ESCAPE,
  JST_PARSE_INVALID_STRING_CHAR,
  JST_PARSE_INVALID_UNICODE_HEX,
  JST_PARSE_INVALID_UNICODE_SURROGATE,
  JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
  JST_PARSE_MISS_KEY,
  JST_PARSE_MISS_COLON,
  JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET,
  JST_STRINGIFY_OK,
} jst_ret_type;

extern const char* jst_ret_type_name[16];

typedef enum { JST_WS_BEFORE, JST_WS_AFTER } jst_ws_state;
}  // namespace jst

#endif  // __JSON_TOY_ENUM_H__