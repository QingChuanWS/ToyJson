#include "basic.h"
#include "parser.h"
#include "utils.h"

namespace jst {
static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

static void test_parse_expect_value() {
  TEST_ERROR(JST_PARSE_EXCEPT_VALUE, " ");
  TEST_ERROR(JST_PARSE_EXCEPT_VALUE, "");
}

static void test_parse_invalid_value() {
  TEST_ERROR(JST_PARSE_INVALID_VALUE, "nul");
  TEST_ERROR(JST_PARSE_INVALID_VALUE, "?");

  TEST_ERROR(JST_PARSE_INVALID_VALUE, "+0");
  TEST_ERROR(JST_PARSE_INVALID_VALUE, "+1");
  TEST_ERROR(JST_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
  TEST_ERROR(JST_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
  TEST_ERROR(JST_PARSE_INVALID_VALUE, "INF");
  TEST_ERROR(JST_PARSE_INVALID_VALUE, "inf");
  TEST_ERROR(JST_PARSE_INVALID_VALUE, "NAN");
  TEST_ERROR(JST_PARSE_INVALID_VALUE, "nan");

  TEST_ERROR(JST_PARSE_INVALID_VALUE, "-1e1.2");
  TEST_ERROR(JST_PARSE_INVALID_VALUE, "-1eeee.2");
  TEST_ERROR(JST_PARSE_INVALID_VALUE, "-1e.2..");

  TEST_ERROR(JST_PARSE_INVALID_VALUE, "[1,]");
  TEST_ERROR(JST_PARSE_INVALID_VALUE, "[\"a\", nul]");
}

static void test_parse_root_not_singular() {
  TEST_ERROR(JST_PARSE_SINGULAR, " null x");
  TEST_ERROR(JST_PARSE_SINGULAR, " falsetur");

  /* invalid number */
  TEST_ERROR(JST_PARSE_SINGULAR, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
  TEST_ERROR(JST_PARSE_SINGULAR, "0x0");
  TEST_ERROR(JST_PARSE_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
  TEST_ERROR(JST_PARSE_NUMBER_TOO_BIG, "1e309");
  TEST_ERROR(JST_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_missing_quotation_mark() {
  TEST_ERROR(JST_PARSE_MISS_QUOTATION_MARK, "\"");
  TEST_ERROR(JST_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
  TEST_ERROR(JST_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
  TEST_ERROR(JST_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
  TEST_ERROR(JST_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
  TEST_ERROR(JST_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
  TEST_ERROR(JST_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
  TEST_ERROR(JST_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() {
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
}

static void test_parse_invalid_unicode_surrogate() {
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
  TEST_ERROR(JST_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_miss_comma_or_square_bracket() {
  TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
  TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
  TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
  TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
  TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1,");
}

static void test_parse_miss_key() {
  TEST_ERROR(JST_PARSE_MISS_KEY, "{:1,");
  TEST_ERROR(JST_PARSE_MISS_KEY, "{1:1,");
  TEST_ERROR(JST_PARSE_MISS_KEY, "{true:1,");
  TEST_ERROR(JST_PARSE_MISS_KEY, "{false:1,");
  TEST_ERROR(JST_PARSE_MISS_KEY, "{null:1,");
  TEST_ERROR(JST_PARSE_MISS_KEY, "{[]:1,");
  TEST_ERROR(JST_PARSE_MISS_KEY, "{{}:1,");
  TEST_ERROR(JST_PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
  TEST_ERROR(JST_PARSE_MISS_COLON, "{\"a\"}");
  TEST_ERROR(JST_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
  TEST_ERROR(JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
  TEST_ERROR(JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
  TEST_ERROR(JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
  TEST_ERROR(JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

static void test_equal() {
  TEST_EQUAL("true", "true", 1);
  TEST_EQUAL("true", "false", 0);
  TEST_EQUAL("false", "false", 1);
  TEST_EQUAL("null", "null", 1);
  TEST_EQUAL("null", "0", 0);
  TEST_EQUAL("123", "123", 1);
  TEST_EQUAL("123", "456", 0);
  TEST_EQUAL("\"abc\"", "\"abc\"", 1);
  TEST_EQUAL("\"abc\"", "\"abcd\"", 0);
  TEST_EQUAL("[]", "[]", 1);
  TEST_EQUAL("[]", "null", 0);
  TEST_EQUAL("[1,2,3]", "[1,2,3]", 1);
  TEST_EQUAL("[1,2,3]", "[1,2,3,4]", 0);
  TEST_EQUAL("[[]]", "[[]]", 1);
  TEST_EQUAL("{}", "{}", 1);
  TEST_EQUAL("{}", "null", 0);
  TEST_EQUAL("{}", "[]", 0);
  //     TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}", 1);
  //     TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}", 1);
  //     TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":3}", 0);
  //     TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2,\"c\":3}", 0);
  //     TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":{}}}}", 1);
  //     TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":[]}}}", 0);
}

static void test_copy() {
  JParser jc("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
  JNode jn_1, jn_2;
  jc.parser(&jn_1);
  jn_2 = jn_1;
  EXPECT_TRUE(jn_1 == jn_2);
}

static void test_move() {
  JNode jn_1, jn_2, jn_3;
  JParser jc("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
  jc.parser(&jn_1);
  jn_2 = jn_1;
  jn_3 = std::move(jn_2);
  EXPECT_EQ_INT(JST_NULL, jn_2.type());
  EXPECT_TRUE(jn_1 == jn_3);
}

static void test_swap() {
  JNode jn_1(JST_STR, "Hello", strlen("Hello"));
  JNode jn_2(JST_STR, "World!", strlen("World!"));
  swap(jn_1, jn_2);
  auto str = jn_1.data().as<JString>().value();
  EXPECT_EQ_STRING("World!", str.c_str(), str.size());
  str = jn_2.data().as<JString>().value();
  EXPECT_EQ_STRING("Hello", str.c_str(), str.size());
}

static void test_parse() {
  test_parse_expect_value();
  test_parse_invalid_value();
  test_parse_root_not_singular();
  test_parse_number_too_big();
  test_parse_missing_quotation_mark();
  test_parse_invalid_string_escape();
  test_parse_invalid_string_char();
  test_parse_invalid_unicode_hex();
  test_parse_invalid_unicode_surrogate();
  test_parse_miss_comma_or_square_bracket();
  // test_parse_miss_key();
  // test_parse_miss_colon();
  // test_parse_miss_comma_or_curly_bracket();
}

static void test_jst_str_node() {
  const char str[] = "Hello_world";
  do {
    JNode jn_r(JST_STR, str, strlen(str));
    JNode jn = jn_r;
    TEST_NODE_STR(str, jn_r);
    TEST_NODE_STR(str, jn);
  } while (0);
  do {
    JNode jn_r(JST_STR, str, strlen(str));
    JNode jn;
    jn = jn_r;
    TEST_NODE_STR(str, jn);
    TEST_NODE_STR(str, jn_r);
  } while (0);
  char str_2[] = "Nice World";
  do {
    JNode jn_r(JST_STR, str, strlen(str));
    JNode jn(JST_STR, str_2, strlen(str_2));
    jn = jn_r;
    TEST_NODE_STR(str, jn);
    TEST_NODE_STR(str, jn_r);
  } while (0);
  do {
    JNode jn_r(JST_STR, str, strlen(str));
    JNode jn = std::move(jn_r);
    TEST_NODE_STR(str, jn);
    EXPECT_EQ_TYPE(JST_NULL, jn_r.type());
  } while (0);
  do {
    JNode jn_r(JST_STR, str, strlen(str));
    JNode jn(JST_STR, str_2, strlen(str_2));
    jn = std::move(jn_r);
    TEST_NODE_STR(str, jn);
    EXPECT_EQ_TYPE(JST_NULL, jn_r.type());
  } while (0);
}

static void test_jst_num_node() {
  char str[] = "1.23e3";
  do {
    JNode jn_r(JST_NUM, str, strlen(str));
    JNode jn = jn_r;
    TEST_NODE_NUM(1.23e3, jn_r);
    TEST_NODE_NUM(1.23e3, jn);
  } while (0);
  do {
    JNode jn_r(JST_NUM, str, strlen(str));
    JNode jn;
    jn = jn_r;
    TEST_NODE_NUM(1.23e3, jn_r);
    TEST_NODE_NUM(1.23e3, jn);
  } while (0);
  char str_2[] = "5.67e3";
  do {
    JNode jn_r(JST_NUM, str, strlen(str));
    JNode jn(JST_NUM, str_2, strlen(str_2));
    jn = jn_r;
    TEST_NODE_NUM(1.23e3, jn_r);
    TEST_NODE_NUM(1.23e3, jn);
  } while (0);
  do {
    JNode jn_r(JST_NUM, str, strlen(str));
    JNode jn = std::move(jn_r);
    EXPECT_EQ_INT(JST_NULL, jn_r.type());
  } while (0);
  do {
    JNode jn_r(JST_NUM, str, strlen(str));
    JNode jn(JST_NUM, str_2, strlen(str_2));
    jn = std::move(jn_r);
    EXPECT_EQ_INT(JST_NULL, jn_r.type());
  } while (0);
}

}  // namespace jst

int main() {
#ifdef _WINDOWS
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
  jst::test_parse();
  jst::test_jst_str_node();
  jst::test_jst_num_node();
  jst::test_equal();
  // jst::test_copy();
  // jst::test_move();
  jst::test_swap();
  printf("%d/%d (%3.2f%%) passed\n", jst::test_pass, jst::test_count,
         jst::test_pass * 100.0 / jst::test_count);
  return jst::main_ret;
}