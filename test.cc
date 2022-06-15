#include "json_toy.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdlib.h>
#include <string.h>
#include "jst_vector.h"

namespace jst {
static int main_ret   = 0;
static int test_count = 0;
static int test_pass  = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format)              \
    do {                                                              \
        test_count++;                                                 \
        if (equality)                                                 \
            test_pass++;                                              \
        else {                                                        \
            fprintf(stderr,                                           \
                    "%s:%d: expect: " format " actual: " format "\n", \
                    __FILE__,                                         \
                    __LINE__,                                         \
                    expect,                                           \
                    actual);                                          \
            main_ret = 1;                                             \
        }                                                             \
    } while (0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define EXPECT_EQ_RET(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), jst_ret_type_name[expect], jst_ret_type_name[actual], "%s")
#define EXPECT_EQ_TYPE(expect, actual) \
    EXPECT_EQ_BASE(                    \
        (expect) == (actual), jst_node_type_name[expect], jst_node_type_name[actual], "%s")

#define EXPECT_EQ_DOUBLE(expect, actual)                                                      \
    EXPECT_EQ_BASE((std::fabs((expect) - (actual)) < std::numeric_limits<double>::epsilon()), \
                   expect,                                                                    \
                   actual,                                                                    \
                   "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength)                                           \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, \
                   expect,                                                                  \
                   actual,                                                                  \
                   "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#if defined(_MSC_VER)
#    define EXPECT_EQ_SIZE_T(expect, actual) \
        EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#    define EXPECT_EQ_SIZE_T(expect, actual) \
        EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

#define TEST_ERROR(error, json)                                \
    do {                                                       \
        jst_context jc(json);                                  \
        EXPECT_EQ_RET(error, jc.jst_parser());                 \
        EXPECT_EQ_TYPE(JST_NULL, jc.root.jst_node_type_get()); \
    } while (0)

#define TEST_NUMBER(expect, json)                             \
    do {                                                      \
        jst_context jc(json);                                 \
        EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());         \
        EXPECT_EQ_TYPE(JST_NUM, jc.root.jst_node_type_get()); \
        double num = 0.0;                                     \
        jc.root.jst_node_data_get(num);                       \
        EXPECT_EQ_DOUBLE(expect, num);                        \
    } while (0)

#define TEST_STRING(expect, json)                             \
    do {                                                      \
        jst_context jc(json);                                 \
        EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());         \
        EXPECT_EQ_TYPE(JST_STR, jc.root.jst_node_type_get()); \
        char*  p   = nullptr;                                 \
        size_t len = 0;                                       \
        jc.root.jst_node_data_get(&p, len);                   \
        EXPECT_EQ_STRING(expect, p, len);                     \
    } while (0)

#define TEST_ARRAY(expect, json)                             \
    do {                                                     \
        jst_context c(json);                                 \
        EXPECT_EQ_RET(JST_PARSE_OK, c.jst_parser());         \
        EXPECT_EQ_TYPE(JST_ARR, c.root.jst_node_type_get()); \
        array arr;                                           \
        c.root.jst_node_data_get(arr);                       \
        EXPECT_EQ_SIZE_T(expect, arr.size());                \
    } while (0)

#define TEST_NODE_STR(str, jn)                           \
    do {                                                 \
        EXPECT_EQ_TYPE(JST_STR, jn.jst_node_type_get()); \
        char*  p   = nullptr;                            \
        size_t len = 0;                                  \
        jn.jst_node_data_get(&p, len);                   \
        EXPECT_EQ_STRING(str, p, len);                   \
    } while (0)

#define TEST_NODE_NUM(expect, jn)                        \
    do {                                                 \
        EXPECT_EQ_TYPE(JST_NUM, jn.jst_node_type_get()); \
        double num = 0.0;                                \
        jn.jst_node_data_get(num);                       \
        EXPECT_EQ_DOUBLE(expect, num);                   \
    } while (0)

static void test_parse_null()
{
    jst_context jc(" null   ");
    EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());
    EXPECT_EQ_TYPE(JST_NULL, jc.root.jst_node_type_get());
}

static void test_parse_bool_true()
{
    jst_context jc(" true   ");
    EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());
    EXPECT_EQ_TYPE(JST_TRUE, jc.root.jst_node_type_get());
}

static void test_parse_bool_false()
{
    jst_context jc(" false   ");
    EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());
    EXPECT_EQ_TYPE(JST_FALSE, jc.root.jst_node_type_get());
}

static void test_parse_number()
{
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002");           /* the smallest number > 1 */
    TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308"); /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER(2.2250738585072014e-308,
                "2.2250738585072014e-308"); /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER(1.7976931348623157, "1.7976931348623157"); /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_jst_str_node()
{
    char str[] = "Hello_world";
    do {
        jst_node jn_r(JST_STR, str, strlen(str));
        jst_node jn = jn_r;
        TEST_NODE_STR(str, jn_r);
        TEST_NODE_STR(str, jn);
    } while (0);
    do {
        jst_node jn_r(JST_STR, str, strlen(str));
        jst_node jn;
        jn = jn_r;
        TEST_NODE_STR(str, jn);
        TEST_NODE_STR(str, jn_r);
    } while (0);
    char str_2[] = "Nice World";
    do {
        jst_node jn_r(JST_STR, str, strlen(str));
        jst_node jn(JST_STR, str_2, strlen(str_2));
        jn = jn_r;
        TEST_NODE_STR(str, jn);
        TEST_NODE_STR(str, jn_r);
    } while (0);
    do {
        jst_node jn_r(JST_STR, str, strlen(str));
        jst_node jn = std::move(jn_r);
        TEST_NODE_STR(str, jn);
        EXPECT_EQ_TYPE(JST_NULL, jn_r.jst_node_type_get());
    } while (0);
    do {
        jst_node jn_r(JST_STR, str, strlen(str));
        jst_node jn(JST_STR, str_2, strlen(str_2));
        jn = std::move(jn_r);
        TEST_NODE_STR(str, jn);
        EXPECT_EQ_TYPE(JST_NULL, jn_r.jst_node_type_get());
    } while (0);
}

static void test_jst_num_node()
{
    char str[] = "1.23e3";
    do {
        jst_node jn_r(JST_NUM, str, strlen(str));
        jst_node jn = jn_r;
        TEST_NODE_NUM(1.23e3, jn_r);
        TEST_NODE_NUM(1.23e3, jn);
    } while (0);
    do {
        jst_node jn_r(JST_STR, str, strlen(str));
        jst_node jn;
        jn = jn_r;
        TEST_NODE_NUM(1.23e3, jn_r);
        TEST_NODE_NUM(1.23e3, jn);
    } while (0);
    char str_2[] = "5.67e3";
    do {
        jst_node jn_r(JST_STR, str, strlen(str));
        jst_node jn(JST_STR, str_2, strlen(str_2));
        jn = jn_r;
        TEST_NODE_NUM(1.23e3, jn_r);
        TEST_NODE_NUM(1.23e3, jn);
    } while (0);
    do {
        jst_node jn_r(JST_STR, str, strlen(str));
        jst_node jn = std::move(jn_r);
        TEST_NODE_NUM(1.23e3, jn_r);
        EXPECT_EQ_INT(JST_NULL, jn_r.jst_node_type_get());
    } while (0);
    do {
        jst_node jn_r(JST_STR, str, strlen(str));
        jst_node jn(JST_STR, str_2, strlen(str_2));
        jn = std::move(jn_r);
        TEST_NODE_NUM(1.23e3, jn_r);
        EXPECT_EQ_INT(JST_NULL, jn_r.jst_node_type_get());
    } while (0);
}

static void test_parse_string()
{
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");                    /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");                /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\"");            /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\""); /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\""); /* G clef sign U+1D11E */
}

static void test_parse_expect_value()
{
    TEST_ERROR(JST_PARSE_EXCEPT_VALUE, " ");
    TEST_ERROR(JST_PARSE_EXCEPT_VALUE, "");
}

static void test_parse_invalid_value()
{
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

static void test_parse_root_not_singular()
{
    TEST_ERROR(JST_PARSE_SINGULAR, " null x");
    TEST_ERROR(JST_PARSE_SINGULAR, " falsetur");

    /* invalid number */
    TEST_ERROR(JST_PARSE_SINGULAR, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST_ERROR(JST_PARSE_SINGULAR, "0x0");
    TEST_ERROR(JST_PARSE_SINGULAR, "0x123");
}

static void test_parse_number_too_big()
{
    TEST_ERROR(JST_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(JST_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_missing_quotation_mark()
{
    TEST_ERROR(JST_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(JST_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape()
{
    TEST_ERROR(JST_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(JST_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(JST_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(JST_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char()
{
    TEST_ERROR(JST_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(JST_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex()
{
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

static void test_parse_invalid_unicode_surrogate()
{
    TEST_ERROR(JST_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(JST_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(JST_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(JST_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(JST_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parser_array_1()
{
    jst_context c("[ null , false , true , 123 , \"abc\" ]");
    EXPECT_EQ_RET(JST_PARSE_OK, c.jst_parser());
    EXPECT_EQ_TYPE(JST_ARR, c.root.jst_node_type_get());
    array arr;
    c.root.jst_node_data_get(arr);
    EXPECT_EQ_TYPE(JST_NULL, arr[0].jst_node_type_get());
    EXPECT_EQ_TYPE(JST_FALSE, arr[1].jst_node_type_get());
    EXPECT_EQ_TYPE(JST_TRUE, arr[2].jst_node_type_get());
    TEST_NODE_NUM(123.0, arr[3]);
    TEST_NODE_STR("abc", arr[4]);
}

static void test_parser_array_2()
{
    jst_context c("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]");
    EXPECT_EQ_INT(JST_PARSE_OK, c.jst_parser());
    EXPECT_EQ_INT(JST_ARR, c.root.jst_node_type_get());
    array arr;
    c.root.jst_node_data_get(arr);

    do {
        EXPECT_EQ_SIZE_T(JST_ARR, arr[0].jst_node_type_get());
        array arr_p;
        arr[0].jst_node_data_get(arr_p);
        EXPECT_EQ_SIZE_T(0, arr_p.size());
    } while (0);

    do {
        EXPECT_EQ_SIZE_T(JST_ARR, arr[1].jst_node_type_get());
        array arr_p;
        arr[1].jst_node_data_get(arr_p);
        EXPECT_EQ_SIZE_T(1, arr_p.size());
        TEST_NODE_NUM(0.0, arr_p[0]);
    } while (0);

    do {
        EXPECT_EQ_SIZE_T(JST_ARR, arr[2].jst_node_type_get());
        array arr_p;
        arr[2].jst_node_data_get(arr_p);
        EXPECT_EQ_SIZE_T(2, arr_p.size());
        TEST_NODE_NUM(0.0, arr_p[0]);
        TEST_NODE_NUM(1.0, arr_p[1]);
    } while (0);

    do {
        EXPECT_EQ_SIZE_T(JST_ARR, arr[3].jst_node_type_get());
        array arr_p;
        arr[3].jst_node_data_get(arr_p);
        EXPECT_EQ_SIZE_T(3, arr_p.size());
        TEST_NODE_NUM(0.0, arr_p[0]);
        TEST_NODE_NUM(1.0, arr_p[1]);
        TEST_NODE_NUM(2.0, arr_p[2]);
    } while (0);
}

static void test_parse_array()
{
    TEST_ARRAY(0, "[ ]");
    test_parser_array_1();
    test_parser_array_2();
}

#define TEST_OBJ_KEY(str, key) EXPECT_EQ_STRING(str, key.c_str(), key.size());

static void test_parse_object()
{
    jst_context c(" { "
                  "\"n\" : null , "
                  "\"f\" : false , "
                  "\"t\" : true , "
                  "\"i\" : 123 , "
                  "\"s\" : \"abc\", "
                  "\"a\" : [ 1, 2, 3 ],"
                  "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                  " } ");
    EXPECT_EQ_RET(JST_PARSE_OK, c.jst_parser());
    EXPECT_EQ_TYPE(JST_OBJ, c.root.jst_node_type_get());
    object obj;
    c.root.jst_node_data_get(obj);
    EXPECT_EQ_SIZE_T(7, obj.size());

    TEST_OBJ_KEY("n", obj[0].get_key());
    EXPECT_EQ_TYPE(JST_NULL, obj[0].get_value().jst_node_type_get());

    TEST_OBJ_KEY("f", obj[1].get_key());
    EXPECT_EQ_TYPE(JST_FALSE, obj[1].get_value().jst_node_type_get());

    TEST_OBJ_KEY("t", obj[2].get_key());
    EXPECT_EQ_TYPE(JST_TRUE, obj[2].get_value().jst_node_type_get());

    TEST_OBJ_KEY("i", obj[3].get_key());
    TEST_NODE_NUM(123.0, obj[3].get_value());

    TEST_OBJ_KEY("s", obj[4].get_key());
    TEST_NODE_STR("abc", obj[4].get_value());

    TEST_OBJ_KEY("a", obj[5].get_key());
    do {
        EXPECT_EQ_SIZE_T(JST_ARR, obj[5].get_value().jst_node_type_get());
        array arr_p;
        obj[5].get_value().jst_node_data_get(arr_p);
        EXPECT_EQ_SIZE_T(3, arr_p.size());
        TEST_NODE_NUM(1.0, arr_p[0]);
        TEST_NODE_NUM(2.0, arr_p[1]);
        TEST_NODE_NUM(3.0, arr_p[2]);
    } while (0);

    TEST_OBJ_KEY("o", obj[6].get_key());
    EXPECT_EQ_TYPE(JST_OBJ, obj[6].get_value().jst_node_type_get());
    do {
        EXPECT_EQ_SIZE_T(JST_OBJ, obj[6].get_value().jst_node_type_get());
        object obj_p;
        obj[6].get_value().jst_node_data_get(obj_p);
        EXPECT_EQ_SIZE_T(3, obj_p.size());

        TEST_OBJ_KEY("1", obj_p[0].get_key());
        TEST_NODE_NUM(1.0, obj_p[0].get_value());

        TEST_OBJ_KEY("2", obj_p[1].get_key());
        TEST_NODE_NUM(2.0, obj_p[1].get_value());

        TEST_OBJ_KEY("3", obj_p[2].get_key());
        TEST_NODE_NUM(3.0, obj_p[2].get_value());
    } while (0);
}

static void test_parse_miss_comma_or_square_bracket()
{
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1,");
}

static void test_parse_miss_key()
{
    TEST_ERROR(JST_PARSE_MISS_KEY, "{:1,");
    TEST_ERROR(JST_PARSE_MISS_KEY, "{1:1,");
    TEST_ERROR(JST_PARSE_MISS_KEY, "{true:1,");
    TEST_ERROR(JST_PARSE_MISS_KEY, "{false:1,");
    TEST_ERROR(JST_PARSE_MISS_KEY, "{null:1,");
    TEST_ERROR(JST_PARSE_MISS_KEY, "{[]:1,");
    TEST_ERROR(JST_PARSE_MISS_KEY, "{{}:1,");
    TEST_ERROR(JST_PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon()
{
    TEST_ERROR(JST_PARSE_MISS_COLON, "{\"a\"}");
    TEST_ERROR(JST_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket()
{
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

#define TEST_ROUNDTRIP(json)                                                        \
    do {                                                                            \
        jst_context jc(json);                                                       \
        EXPECT_EQ_INT(JST_PARSE_OK, jc.jst_parser());                               \
        char*  json2;                                                               \
        size_t length;                                                              \
        EXPECT_EQ_INT(JST_STRINGIFY_OK, jc.jst_stringify(jc.root, &json2, length)); \
        EXPECT_EQ_STRING(json, json2, length);                                      \
    } while (0)

static void test_stringify_number()
{
    TEST_ROUNDTRIP("0");
    TEST_ROUNDTRIP("-0");
    TEST_ROUNDTRIP("1");
    TEST_ROUNDTRIP("-1");
    TEST_ROUNDTRIP("1.5");
    TEST_ROUNDTRIP("-1.5");
    TEST_ROUNDTRIP("3.25");
    TEST_ROUNDTRIP("1e+20");
    TEST_ROUNDTRIP("1.234e+20");
    TEST_ROUNDTRIP("1.234e-20");

    TEST_ROUNDTRIP("1.0000000000000002");      /* the smallest number > 1 */
    TEST_ROUNDTRIP("4.9406564584124654e-324"); /* minimum denormal */
    TEST_ROUNDTRIP("-4.9406564584124654e-324");
    TEST_ROUNDTRIP("2.2250738585072009e-308"); /* Max subnormal double */
    TEST_ROUNDTRIP("-2.2250738585072009e-308");
    TEST_ROUNDTRIP("2.2250738585072014e-308"); /* Min normal positive double */
    TEST_ROUNDTRIP("-2.2250738585072014e-308");
    TEST_ROUNDTRIP("1.7976931348623157e+308"); /* Max double */
    TEST_ROUNDTRIP("-1.7976931348623157e+308");
}

static void test_stringify_string()
{
    TEST_ROUNDTRIP("\"\"");
    TEST_ROUNDTRIP("\"Hello\"");
    TEST_ROUNDTRIP("\"Hello\\nWorld\"");
    TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
    TEST_ROUNDTRIP("\"Hello\\u0000World\"");
}

static void test_stringify_array()
{
    TEST_ROUNDTRIP("[]");
    TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringify_object()
{
    TEST_ROUNDTRIP("{}");
    TEST_ROUNDTRIP("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":"
                   "{\"1\":1,\"2\":2,\"3\":3}}");
}

static void test_stringify()
{
    TEST_ROUNDTRIP("null");
    TEST_ROUNDTRIP("false");
    TEST_ROUNDTRIP("true");
    test_stringify_number();
    test_stringify_string();
    test_stringify_array();
    test_stringify_object();
}

#define TEST_EQUAL(json1, json2, equality)                 \
    do {                                                   \
        jst_context jc(json1);                             \
        jst_node    jn_1;                                  \
        EXPECT_EQ_INT(JST_PARSE_OK, jc.jst_parser(&jn_1)); \
        jc.reset(json2);                                   \
        jst_node jn_2;                                     \
        EXPECT_EQ_INT(JST_PARSE_OK, jc.jst_parser(&jn_2)); \
        EXPECT_EQ_INT(equality, jn_1 == jn_2);             \
    } while (0)

static void test_equal()
{
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
    TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}", 1);
    TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}", 1);
    TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":3}", 0);
    TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2,\"c\":3}", 0);
    TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":{}}}}", 1);
    TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":[]}}}", 0);
}

static void test_copy()
{
    jst_context jc("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
    jst_node    jn_1, jn_2;
    jc.jst_parser(&jn_1);
    jn_2 = jn_1;
    EXPECT_TRUE(jn_1 == jn_2);
}

static void test_move()
{
    jst_node    jn_1, jn_2, jn_3;
    jst_context jc("{\"t\":true,\"f\":false,\"n\":null,\"d\":1.5,\"a\":[1,2,3]}");
    jc.jst_parser(&jn_1);
    jn_2 = jn_1;
    jn_3 = std::move(jn_2);
    EXPECT_EQ_INT(JST_NULL, jn_2.jst_node_type_get());
    EXPECT_TRUE(jn_1 == jn_3);
}

static void test_swap()
{
    jst_node jn_1(JST_STR, "Hello", strlen("Hello"));
    jst_node jn_2(JST_STR, "World!", strlen("World!"));
    swap(jn_1, jn_2);
    char*  str_head;
    size_t str_len;
    jn_1.jst_node_data_get(&str_head, str_len);
    EXPECT_EQ_STRING("World!", str_head, str_len);
    jn_2.jst_node_data_get(&str_head, str_len);
    EXPECT_EQ_STRING("Hello", str_head, str_len);
}

static void test_access_null()
{
    jst_node jn;
    jn.jst_node_data_set(JST_STR, "a", 1);
    jn.jst_node_type_reset();
    EXPECT_EQ_INT(JST_NULL, jn.jst_node_type_get());
}

static void test_access_boolean()
{
    jst_node jn;
    jn.jst_node_data_set(JST_STR, "a", 1);
    jn.jst_node_data_reset(true);
    bool data;
    jn.jst_node_data_get(data);
    EXPECT_TRUE(data);
    jn.jst_node_data_set(JST_FALSE);
    jn.jst_node_data_get(data);
    EXPECT_FALSE(data);
}

static void test_access_number()
{
    jst_node jn;
    jn.jst_node_data_set(JST_STR, "a", 1);
    jn.jst_node_data_reset(1234.5);
    double num;
    jn.jst_node_data_get(num);
    EXPECT_EQ_DOUBLE(1234.5, num);
}

static void test_access_string()
{
    jst_node jn;
    jn.jst_node_data_set(JST_STR, "", 0);
    char*  str_head;
    size_t str_len;
    jn.jst_node_data_get(&str_head, str_len);
    EXPECT_EQ_STRING("", str_head, str_len);
    jn.jst_node_data_reset(string("Hello", 5));
    jn.jst_node_data_get(&str_head, str_len);
    EXPECT_EQ_STRING("Hello", str_head, str_len);
}

static void test_access_array()
{
    array    a;
    jst_node e;
    size_t   i, j;

    a.reserve(1);
    EXPECT_EQ_SIZE_T(0, a.size());
    EXPECT_EQ_SIZE_T(1, a.capacity());
    for (i = 0; i < 10; i++) {
        e.jst_node_data_reset((double)i);
        a.push_back(e);
    }

    EXPECT_EQ_SIZE_T(10, a.size());
    for (i = 0; i < 10; i++) {
        double num;
        a[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i, num);
    }

    a.pop_back();
    EXPECT_EQ_SIZE_T(9, a.size());
    for (i = 0; i < 9; i++) {
        double num;
        a[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i, num);
    }

    a.erase(4, 0);
    EXPECT_EQ_SIZE_T(9, a.size());
    for (i = 0; i < 9; i++) {
        double num;
        a[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i, num);
    }

    a.erase(8, 1);
    EXPECT_EQ_SIZE_T(8, a.size());
    for (i = 0; i < 8; i++) {
        double num;
        a[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i, num);
    }

    a.erase(0, 2);
    EXPECT_EQ_SIZE_T(6, a.size());
    for (i = 0; i < 6; i++) {
        double num;
        a[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i + 2, num);
    }

    for (i = 0; i < 2; i++) {
        e.jst_node_data_reset((double)i);
        a.insert(i, e);
    }

    EXPECT_EQ_SIZE_T(8, a.size());
    for (i = 0; i < 8; i++) {
        double num;
        a[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i, num);
    }

    EXPECT_TRUE(a.capacity() > 8);
    a.shrink_to_fit();
    EXPECT_EQ_SIZE_T(8, a.capacity());
    EXPECT_EQ_SIZE_T(8, a.size());
    for (i = 0; i < 8; i++) {
        double num;
        a[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i, num);
    }

    e.jst_node_data_reset(string("Hello", 5));
    a.push_back(e);

    i = a.capacity();
    a.clear();
    EXPECT_EQ_SIZE_T(0, a.size());
    EXPECT_EQ_SIZE_T(i, a.capacity()); /* capacity remains unchanged */
}

static void test_access_vector()
{
    using jst::utils::jst_vector;
    
    jst_vector<jst_node>    v;
    jst_node e;
    size_t   i, j;

    v.reserve(3);
    EXPECT_EQ_SIZE_T(0, v.size());
    EXPECT_EQ_SIZE_T(3, v.capacity());
    for (i = 0; i < 10; i++) {
        e.jst_node_data_reset((double)i);
        v.push_back(e);
    }

    EXPECT_EQ_SIZE_T(10, v.size());
    for (i = 0; i < 10; i++) {
        double num;
        v[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i, num);
    }

    v.pop_back();
    EXPECT_EQ_SIZE_T(9, v.size());
    for (i = 0; i < 9; i++) {
        double num;
        v[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i, num);
    }

    v.erase(4, 0);
    EXPECT_EQ_SIZE_T(9, v.size());
    for (i = 0; i < 9; i++) {
        double num;
        v[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i, num);
    }

    v.erase(8, 1);
    EXPECT_EQ_SIZE_T(8, v.size());
    for (i = 0; i < 8; i++) {
        double num;
        v[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i, num);
    }

    v.erase(0, 2);
    EXPECT_EQ_SIZE_T(6, v.size());
    for (i = 0; i < 6; i++) {
        double num;
        v[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i + 2, num);
    }

    for (i = 0; i < 2; i++) {
        e.jst_node_data_reset((double)i);
        v.insert(i, e);
    }

    EXPECT_EQ_SIZE_T(8, v.size());
    for (i = 0; i < 8; i++) {
        double num;
        v[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i, num);
    }

    EXPECT_TRUE(v.capacity() > 8);
    v.shrink_to_fit();
    EXPECT_EQ_SIZE_T(8, v.capacity());
    EXPECT_EQ_SIZE_T(8, v.size());
    for (i = 0; i < 8; i++) {
        double num;
        v[i].jst_node_data_get(num);
        EXPECT_EQ_DOUBLE((double)i, num);
    }

    e.jst_node_data_reset(string("Hello", 5));
    v.push_back(e);

    i = v.capacity();
    v.clear();
    EXPECT_EQ_SIZE_T(0, v.size());
    EXPECT_EQ_SIZE_T(i, v.capacity()); /* capacity remains unchanged */
}

static void test_access_object()
{
    // lept_value o, v, *pv;
    // size_t i, j, index;

    // lept_init(&o);

    // for (j = 0; j <= 5; j += 5) {
    //     lept_set_object(&o, j);
    //     EXPECT_EQ_SIZE_T(0, lept_get_object_size(&o));
    //     EXPECT_EQ_SIZE_T(j, lept_get_object_capacity(&o));
    //     for (i = 0; i < 10; i++) {
    //         char key[2] = "a";
    //         key[0] += i;
    //         lept_init(&v);
    //         lept_set_number(&v, i);
    //         lept_move(lept_set_object_value(&o, key, 1), &v);
    //         lept_free(&v);
    //     }
    //     EXPECT_EQ_SIZE_T(10, lept_get_object_size(&o));
    //     for (i = 0; i < 10; i++) {
    //         char key[] = "a";
    //         key[0] += i;
    //         index = lept_find_object_index(&o, key, 1);
    //         EXPECT_TRUE(index != LEPT_KEY_NOT_EXIST);
    //         pv = lept_get_object_value(&o, index);
    //         EXPECT_EQ_DOUBLE((double)i, lept_get_number(pv));
    //     }
    // }

    // index = lept_find_object_index(&o, "j", 1);
    // EXPECT_TRUE(index != LEPT_KEY_NOT_EXIST);
    // lept_remove_object_value(&o, index);
    // index = lept_find_object_index(&o, "j", 1);
    // EXPECT_TRUE(index == LEPT_KEY_NOT_EXIST);
    // EXPECT_EQ_SIZE_T(9, lept_get_object_size(&o));

    // index = lept_find_object_index(&o, "a", 1);
    // EXPECT_TRUE(index != LEPT_KEY_NOT_EXIST);
    // lept_remove_object_value(&o, index);
    // index = lept_find_object_index(&o, "a", 1);
    // EXPECT_TRUE(index == LEPT_KEY_NOT_EXIST);
    // EXPECT_EQ_SIZE_T(8, lept_get_object_size(&o));

    // EXPECT_TRUE(lept_get_object_capacity(&o) > 8);
    // lept_shrink_object(&o);
    // EXPECT_EQ_SIZE_T(8, lept_get_object_capacity(&o));
    // EXPECT_EQ_SIZE_T(8, lept_get_object_size(&o));
    // for (i = 0; i < 8; i++) {
    //     char key[] = "a";
    //     key[0] += i + 1;
    //     EXPECT_EQ_DOUBLE((double)i + 1, lept_get_number(lept_get_object_value(&o,
    //     lept_find_object_index(&o, key, 1))));
    // }

    // lept_set_string(&v, "Hello", 5);
    // lept_move(lept_set_object_value(&o, "World", 5), &v); /* Test if element is freed */
    // lept_free(&v);

    // pv = lept_find_object_value(&o, "World", 5);
    // EXPECT_TRUE(pv != NULL);
    // EXPECT_EQ_STRING("Hello", lept_get_string(pv), lept_get_string_length(pv));

    // i = lept_get_object_capacity(&o);
    // lept_clear_object(&o);
    // EXPECT_EQ_SIZE_T(0, lept_get_object_size(&o));
    // EXPECT_EQ_SIZE_T(i, lept_get_object_capacity(&o)); /* capacity remains unchanged */
    // lept_shrink_object(&o);
    // EXPECT_EQ_SIZE_T(0, lept_get_object_capacity(&o));

    // lept_free(&o);
}

static void test_access()
{
    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
    test_access_array();
    test_access_vector();
    // test_access_object();
}

static void test_parse()
{
    test_parse_null();
    test_parse_bool_true();
    test_parse_bool_false();
    test_parse_number();
    test_parse_string();
    test_parse_array();
    test_parse_object();

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
    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
}

}   // namespace jst

int main()
{
#ifdef _WINDOWS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    jst::test_parse();
    jst::test_stringify();
    jst::test_jst_str_node();
    jst::test_equal();
    jst::test_copy();
    jst::test_move();
    jst::test_swap();
    jst::test_access();
    printf("%d/%d (%3.2f%%) passed\n",
           jst::test_pass,
           jst::test_count,
           jst::test_pass * 100.0 / jst::test_count);
    return jst::main_ret;
}