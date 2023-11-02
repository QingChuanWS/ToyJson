/*
 * @Author: QingChuanWS bingshan45@163.com
 * @Date: 2023-01-15 01:41:20
 * @LastEditors: QingChuanWS bingshan45@163.com
 * @LastEditTime: 2023-11-02 23:21:58
 * @FilePath: /json_toy/inc/enum.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置:
 * https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __JSON_TOY_ENUM_H__
#define __JSON_TOY_ENUM_H__

#include <string>

namespace jst {

#define JST_FUNCTION_STATE(expect, func, node) \
  do {                                         \
    JRetType ret = func;                       \
    if (ret != expect) {                       \
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

typedef enum { JST_NULL = 0, JST_TRUE, JST_FALSE, JST_NUM, JST_STR, JST_ARR, JST_OBJ } JNType;

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
} JRetType;

extern const char* jst_ret_type_name[16];

typedef enum { JST_WS_BEFORE, JST_WS_AFTER } jst_ws_state;
}  // namespace jst

#endif  // __JSON_TOY_ENUM_H__