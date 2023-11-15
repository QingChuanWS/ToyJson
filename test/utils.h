
#include <stdlib.h>
#include <string.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

#include "../inc/utils.h"

namespace jst {

#define EXPECT_EQ_BASE(equality, expect, actual, format)                                    \
  do {                                                                                      \
    test_count++;                                                                           \
    if (equality)                                                                           \
      test_pass++;                                                                          \
    else {                                                                                  \
      fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, \
              expect, actual);                                                              \
      main_ret = 1;                                                                         \
    }                                                                                       \
  } while (0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define EXPECT_EQ_RET(expect, actual) \
  EXPECT_EQ_BASE((expect) == (actual), jst_ret_type_name[expect], jst_ret_type_name[actual], "%s")
#define EXPECT_EQ_TYPE(expect, actual) \
  EXPECT_EQ_BASE((expect) == (actual), jst_node_type_name[expect], jst_node_type_name[actual], "%s")

#define EXPECT_EQ_DOUBLE(expect, actual)                                                    \
  EXPECT_EQ_BASE((std::fabs((expect) - (actual)) < std::numeric_limits<double>::epsilon()), \
                 expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength)                                                 \
  EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, \
                 actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) \
  EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) \
  EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

#define TEST_ERROR(error, json)               \
  do {                                        \
    JParser jc(json);                         \
    EXPECT_EQ_RET(error, jc.parser());        \
    EXPECT_EQ_TYPE(JST_NULL, jc.root.type()); \
  } while (0)

#define TEST_NUMBER(expect, json)                      \
  do {                                                 \
    JParser jc(json);                                  \
    EXPECT_EQ_RET(JST_PARSE_OK, jc.parser());          \
    EXPECT_EQ_TYPE(JST_NUM, jc.root.type());           \
    double num = jc.root.data().as<JNumber>().value(); \
    EXPECT_EQ_DOUBLE(expect, num);                     \
  } while (0)

#define TEST_STRING(expect, json)                      \
  do {                                                 \
    JParser jc(json);                                  \
    EXPECT_EQ_RET(JST_PARSE_OK, jc.parser());          \
    EXPECT_EQ_TYPE(JST_STR, jc.root.type());           \
    auto cur = jc.root.data().as<JString>().value();   \
    EXPECT_EQ_STRING(expect, cur.c_str(), cur.size()); \
  } while (0)

#define TEST_ARRAY(expect, json)                      \
  do {                                                \
    JParser c(json);                                  \
    EXPECT_EQ_RET(JST_PARSE_OK, c.parser());          \
    EXPECT_EQ_TYPE(JST_ARR, c.root.type());           \
    JArray arr = c.root.data().as<JArray>().value(); \
    EXPECT_EQ_SIZE_T(expect, arr.size());             \
  } while (0)

#define TEST_NODE_STR(str, jn)                      \
  do {                                              \
    EXPECT_EQ_TYPE(JST_STR, jn.type());             \
    auto cur = jn.data().as<JString>().value();     \
    EXPECT_EQ_STRING(str, cur.c_str(), cur.size()); \
  } while (0)

#define TEST_NODE_NUM(expect, jn)                 \
  do {                                            \
    EXPECT_EQ_TYPE(JST_NUM, jn.type());           \
    double num = jn.data().as<JNumber>().value(); \
    EXPECT_EQ_DOUBLE(expect, num);                \
  } while (0)

#define TEST_OBJ_KEY(str, key) EXPECT_EQ_STRING(str, key.c_str(), key.size());

#define TEST_ROUNDTRIP(json)                                                \
  do {                                                                      \
    JParser jc(json);                                                       \
    EXPECT_EQ_INT(JST_PARSE_OK, jc.parser());                               \
    char* json2;                                                            \
    size_t length;                                                          \
    EXPECT_EQ_INT(JST_STRINGIFY_OK, jc.stringify(jc.root, &json2, length)); \
    EXPECT_EQ_STRING(json, json2, length);                                  \
  } while (0)

#define TEST_EQUAL(json1, json2, equality)         \
  do {                                             \
    JParser jc(json1);                             \
    JNode jn_1;                                    \
    EXPECT_EQ_INT(JST_PARSE_OK, jc.parser(&jn_1)); \
    jc.reset(json2);                               \
    JNode jn_2;                                    \
    EXPECT_EQ_INT(JST_PARSE_OK, jc.parser(&jn_2)); \
    EXPECT_EQ_INT(equality, jn_1 == jn_2);         \
  } while (0)

}  // namespace jst
