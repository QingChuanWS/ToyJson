#include "parser.h"
#include "utils.h"

namespace jst {
static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

static void test_parse_null() {
  JParser jc(" null   ");
  EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());
  EXPECT_EQ_TYPE(JST_NULL, jc.root.get_type());
}

static void test_parse_bool_true() {
  JParser jc(" true   ");
  EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());
  EXPECT_EQ_TYPE(JST_TRUE, jc.root.get_type());
}

static void test_parse_bool_false() {
  JParser jc(" false   ");
  EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());
  EXPECT_EQ_TYPE(JST_FALSE, jc.root.get_type());
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

static void test_parser_array_1() {
  JParser c("[ null , false , true , 123 , \"abc\" ]");
  EXPECT_EQ_RET(JST_PARSE_OK, c.jst_parser());
  EXPECT_EQ_TYPE(JST_ARR, c.root.get_type());
  JArray arr;
  c.root.jst_node_data_get(arr);
  EXPECT_EQ_TYPE(JST_NULL, arr[0].get_type());
  EXPECT_EQ_TYPE(JST_FALSE, arr[1].get_type());
  EXPECT_EQ_TYPE(JST_TRUE, arr[2].get_type());
  TEST_NODE_NUM(123.0, arr[3]);
  TEST_NODE_STR("abc", arr[4]);
}

static void test_parser_array_2() {
  JParser c("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]");
  EXPECT_EQ_INT(JST_PARSE_OK, c.jst_parser());
  EXPECT_EQ_INT(JST_ARR, c.root.get_type());
  JArray arr;
  c.root.jst_node_data_get(arr);

  do {
    EXPECT_EQ_SIZE_T(JST_ARR, arr[0].get_type());
    JArray arr_p;
    arr[0].jst_node_data_get(arr_p);
    EXPECT_EQ_SIZE_T(0, arr_p.size());
  } while (0);

  do {
    EXPECT_EQ_SIZE_T(JST_ARR, arr[1].get_type());
    JArray arr_p;
    arr[1].jst_node_data_get(arr_p);
    EXPECT_EQ_SIZE_T(1, arr_p.size());
    TEST_NODE_NUM(0.0, arr_p[0]);
  } while (0);

  do {
    EXPECT_EQ_SIZE_T(JST_ARR, arr[2].get_type());
    JArray arr_p;
    arr[2].jst_node_data_get(arr_p);
    EXPECT_EQ_SIZE_T(2, arr_p.size());
    TEST_NODE_NUM(0.0, arr_p[0]);
    TEST_NODE_NUM(1.0, arr_p[1]);
  } while (0);

  do {
    EXPECT_EQ_SIZE_T(JST_ARR, arr[3].get_type());
    JArray arr_p;
    arr[3].jst_node_data_get(arr_p);
    EXPECT_EQ_SIZE_T(3, arr_p.size());
    TEST_NODE_NUM(0.0, arr_p[0]);
    TEST_NODE_NUM(1.0, arr_p[1]);
    TEST_NODE_NUM(2.0, arr_p[2]);
  } while (0);
}

static void test_parse_array() {
  TEST_ARRAY(0, "[ ]");
  test_parser_array_1();
  test_parser_array_2();
}

static void test_parse_object() {
  JParser c(
      " { "
      "\"n\" : null , "
      "\"f\" : false , "
      "\"t\" : true , "
      "\"i\" : 123 , "
      "\"s\" : \"abc\", "
      "\"a\" : [ 1, 2, 3 ],"
      "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
      " } ");
  EXPECT_EQ_RET(JST_PARSE_OK, c.jst_parser());
  EXPECT_EQ_TYPE(JST_OBJ, c.root.get_type());
  JObject obj;
  c.root.jst_node_data_get(obj);
  EXPECT_EQ_SIZE_T(7, obj.size());

  TEST_OBJ_KEY("n", obj[0].get_key());
  EXPECT_EQ_TYPE(JST_NULL, obj[0].get_value().get_type());

  TEST_OBJ_KEY("f", obj[1].get_key());
  EXPECT_EQ_TYPE(JST_FALSE, obj[1].get_value().get_type());

  TEST_OBJ_KEY("t", obj[2].get_key());
  EXPECT_EQ_TYPE(JST_TRUE, obj[2].get_value().get_type());

  TEST_OBJ_KEY("i", obj[3].get_key());
  TEST_NODE_NUM(123.0, obj[3].get_value());

  TEST_OBJ_KEY("s", obj[4].get_key());
  TEST_NODE_STR("abc", obj[4].get_value());

  TEST_OBJ_KEY("a", obj[5].get_key());
  do {
    EXPECT_EQ_SIZE_T(JST_ARR, obj[5].get_value().get_type());
    JArray arr_p;
    obj[5].get_value().jst_node_data_get(arr_p);
    EXPECT_EQ_SIZE_T(3, arr_p.size());
    TEST_NODE_NUM(1.0, arr_p[0]);
    TEST_NODE_NUM(2.0, arr_p[1]);
    TEST_NODE_NUM(3.0, arr_p[2]);
  } while (0);

  TEST_OBJ_KEY("o", obj[6].get_key());
  EXPECT_EQ_TYPE(JST_OBJ, obj[6].get_value().get_type());
  do {
    EXPECT_EQ_SIZE_T(JST_OBJ, obj[6].get_value().get_type());
    JObject obj_p;
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

static void test_parse() {
  test_parse_null();
  test_parse_bool_true();
  test_parse_bool_false();
  test_parse_number();
  test_parse_string();
  test_parse_array();
  // test_parse_object();
}
}  // namespace jst

int main() {
#ifdef _WINDOWS
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
  jst::test_parse();
  printf("%d/%d (%3.2f%%) passed\n", jst::test_pass, jst::test_count,
         jst::test_pass * 100.0 / jst::test_count);
  return jst::main_ret;
}