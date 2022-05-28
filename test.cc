#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <limits>
#include <cmath>
#include <cstdlib>
#include "json_toy.h"

namespace jst {
static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format)                                                                    \
    do {                                                                                                                    \
        test_count++;                                                                                                       \
        if (equality)                                                                                                       \
            test_pass++;                                                                                                    \
        else {                                                                                                              \
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);          \
            main_ret = 1;                                                                                                   \
        }                                                                                                                   \
    } while (0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define EXPECT_EQ_RET(expect, actual)                                                                                       \
    EXPECT_EQ_BASE((expect) == (actual), jst_ret_type_name[expect], jst_ret_type_name[actual], "%s")
#define EXPECT_EQ_TYPE(expect, actual)                                                                                      \
    EXPECT_EQ_BASE((expect) == (actual), jst_node_type_name[expect], jst_node_type_name[actual], "%s")

#define EXPECT_EQ_DOUBLE(expect, actual)                                                                                    \
    EXPECT_EQ_BASE((std::fabs((expect) - (actual)) < std::numeric_limits<double>::epsilon()), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength)                                                                           \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

#define TEST_ERROR(error, json)                                                                                             \
    do {                                                                                                                    \
        jst_context jc(json);                                                                                               \
        EXPECT_EQ_RET(error, jc.jst_parser());                                                                              \
        EXPECT_EQ_TYPE(JST_NULL, jc.root.jst_node_type_get());                                                              \
    } while (0)

#define TEST_NUMBER(expect, json)                                                                                           \
    do {                                                                                                                    \
        jst_context jc(json);                                                                                               \
        EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());                                                                       \
        EXPECT_EQ_TYPE(JST_NUM, jc.root.jst_node_type_get());                                                               \
        double num = 0.0;                                                                                                   \
        jc.root.jst_node_data_get(num);                                                                                     \
        EXPECT_EQ_DOUBLE(expect, num);                                                                                      \
    } while (0)

#define TEST_STRING(expect, json)                                                                                           \
    do {                                                                                                                    \
        jst_context jc(json);                                                                                               \
        EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());                                                                       \
        EXPECT_EQ_TYPE(JST_STR, jc.root.jst_node_type_get());                                                               \
        char *p = nullptr;                                                                                                  \
        size_t len = 0;                                                                                                     \
        jc.root.jst_node_data_get(&p, len);                                                                                 \
        EXPECT_EQ_STRING(expect, p, len);                                                                                   \
    } while (0)

#define TEST_ARRAY(expect, json)                                                                                            \
    do {                                                                                                                    \
        jst_context c(json);                                                                                                \
        EXPECT_EQ_RET(JST_PARSE_OK, c.jst_parser());                                                                        \
        EXPECT_EQ_TYPE(JST_ARR, c.root.jst_node_type_get());                                                                \
        array arr;                                                                                                          \
        c.root.jst_node_data_get(arr);                                                                                      \
        EXPECT_EQ_SIZE_T(expect, arr.size());                                                                               \
    } while (0)

#define TEST_NODE_STR(str, jn)                                                                                              \
    do {                                                                                                                    \
        char *p = nullptr;                                                                                                  \
        size_t len = 0;                                                                                                     \
        jn.jst_node_data_get(&p, len);                                                                                      \
        EXPECT_EQ_STRING(str, p, len);                                                                                      \
    } while (0)

#define TEST_NODE_NUM(expect, jn)                                                                                           \
    do {                                                                                                                    \
        double num = 0.0;                                                                                                   \
        jn.jst_node_data_get(num);                                                                                          \
        EXPECT_EQ_DOUBLE(expect, num);                                                                                      \
    } while (0)

static void test_parse_null() {
    jst_context jc(" null   ");
    EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());
    EXPECT_EQ_TYPE(JST_NULL, jc.root.jst_node_type_get());
}

static void test_parse_bool_true() {
    jst_context jc(" true   ");
    EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());
    EXPECT_EQ_TYPE(JST_TRUE, jc.root.jst_node_type_get());
}

static void test_parse_bool_false() {
    jst_context jc(" false   ");
    EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());
    EXPECT_EQ_TYPE(JST_FALSE, jc.root.jst_node_type_get());
}

static void test_parse_number() {
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
    TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308"); /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER(1.7976931348623157, "1.7976931348623157"); /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_jst_str_node() {
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

static void test_jst_num_node() {
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

static void test_parse_string() {
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

static void test_parse_array() {
    TEST_ARRAY(0, "[ ]");
    do {
        jst_context c("[ null , false , true , 123 , \"abc\" ]");
        EXPECT_EQ_RET(JST_PARSE_OK, c.jst_parser());
        EXPECT_EQ_TYPE(JST_ARR, c.root.jst_node_type_get());
        array arr;
        c.root.jst_node_data_get(arr);
        EXPECT_EQ_TYPE(JST_NULL, arr[0].jst_node_type_get());
        EXPECT_EQ_TYPE(JST_FALSE, arr[1].jst_node_type_get());
        EXPECT_EQ_TYPE(JST_TRUE, arr[2].jst_node_type_get());
        EXPECT_EQ_TYPE(JST_NUM, arr[3].jst_node_type_get());
        TEST_NODE_NUM(123.0, arr[3]);
        EXPECT_EQ_TYPE(JST_STR, arr[4].jst_node_type_get());
        TEST_NODE_STR("abc", arr[4]);
    } while (0);
    do {
        jst_context c("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]");
        EXPECT_EQ_INT(JST_PARSE_OK, c.jst_parser());
        EXPECT_EQ_INT(JST_ARR, c.root.jst_node_type_get());
        array arr;
        c.root.jst_node_data_get(arr);
        EXPECT_EQ_SIZE_T(JST_ARR, arr[0].jst_node_type_get());
        do {
            array arr_p;
            arr[0].jst_node_data_get(arr_p);
            EXPECT_EQ_SIZE_T(0, arr_p.size());
        } while (0);
        EXPECT_EQ_SIZE_T(JST_ARR, arr[1].jst_node_type_get());
        do {
            array arr_p;
            arr[1].jst_node_data_get(arr_p);
            EXPECT_EQ_SIZE_T(1, arr_p.size());
            EXPECT_EQ_INT(JST_NUM, arr_p[0].jst_node_type_get());
            TEST_NODE_NUM(0.0, arr_p[0]);
        } while (0);
        EXPECT_EQ_SIZE_T(JST_ARR, arr[2].jst_node_type_get());
        do {
            array arr_p;
            arr[2].jst_node_data_get(arr_p);
            EXPECT_EQ_SIZE_T(2, arr_p.size());
            do {
                EXPECT_EQ_INT(JST_NUM, arr_p[0].jst_node_type_get());
                TEST_NODE_NUM(0.0, arr_p[0]);
            } while (0);
            do {
                EXPECT_EQ_INT(JST_NUM, arr_p[1].jst_node_type_get());
                TEST_NODE_NUM(1.0, arr_p[1]);
            } while (0);
        } while (0);
        EXPECT_EQ_SIZE_T(JST_ARR, arr[3].jst_node_type_get());
        do {
            array arr_p;
            arr[3].jst_node_data_get(arr_p);
            EXPECT_EQ_SIZE_T(3, arr_p.size());
            do {
                EXPECT_EQ_INT(JST_NUM, arr_p[0].jst_node_type_get());
                TEST_NODE_NUM(0.0, arr_p[0]);
            } while (0);
            do {
                EXPECT_EQ_INT(JST_NUM, arr_p[1].jst_node_type_get());
                TEST_NODE_NUM(1.0, arr_p[1]);
            } while (0);
            do {
                EXPECT_EQ_INT(JST_NUM, arr_p[2].jst_node_type_get());
                TEST_NODE_NUM(2.0, arr_p[2]);
            } while (0);
        } while (0);
    } while (0);
}

static void test_parse_miss_comma_or_square_bracket() {
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse() {
    test_parse_null();
    test_parse_bool_true();
    test_parse_bool_false();

    test_parse_number();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();

    test_parse_string();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();

    test_parse_array();

    test_parse_miss_comma_or_square_bracket();

    test_jst_str_node();
}

} // namespace jst

int main() {
    jst::test_parse();
    printf("%d/%d (%3.2f%%) passed\n", jst::test_pass, jst::test_count, jst::test_pass * 100.0 / jst::test_count);
    return jst::main_ret;
}