#include "enum.h"

namespace jst {
const char* jst_ret_type_name[] = {"JST_PARSE_OK",
                                   "JST_PARSE_EXCEPT_VALUE",
                                   "JST_PARSE_INVALID_VALUE",
                                   "JST_PARSE_ROOT_NOST",
                                   "JST_PARSE_SINGULAR",
                                   "JST_PARSE_NUMBER_TOO_BIG",
                                   "JST_PARSE_MISS_QUOTATION_MARK",
                                   "JST_PARSE_INVALID_STRING_ESCAPE",
                                   "JST_PARSE_INVALID_STRING_CHAR",
                                   "JST_PARSE_INVALID_UNICODE_HEX",
                                   "JST_PARSE_INVALID_UNICODE_SURROGATE",
                                   "JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET",
                                   "JST_PARSE_MISS_KEY",
                                   "JST_PARSE_MISS_COLON",
                                   "JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET",
                                   "JST_STRINGIFY_OK"};

const char* jst_node_type_name[] = {"JST_NULL", "JST_TRUE", "JST_FALSE", "JST_NUM",
                                    "JST_STR",  "JST_ARR",  "JST_OBJ"};
}  // namespace jst
