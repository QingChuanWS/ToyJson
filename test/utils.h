
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

#define TEST_ERROR(error, json)                            \
  do {                                                     \
    JParser jc(json);                                      \
    EXPECT_EQ_RET(error, jc.jst_parser());                 \
    EXPECT_EQ_TYPE(JST_NULL, jc.root.get_type()); \
  } while (0)

#define TEST_NUMBER(expect, json)                         \
  do {                                                    \
    JParser jc(json);                                     \
    EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());         \
    EXPECT_EQ_TYPE(JST_NUM, jc.root.get_type()); \
    double num = 0.0;                                     \
    jc.root.jst_node_data_get(num);                       \
    EXPECT_EQ_DOUBLE(expect, num);                        \
  } while (0)

#define TEST_STRING(expect, json)                         \
  do {                                                    \
    JParser jc(json);                                     \
    EXPECT_EQ_RET(JST_PARSE_OK, jc.jst_parser());         \
    EXPECT_EQ_TYPE(JST_STR, jc.root.get_type()); \
    char* p = nullptr;                                    \
    size_t len = 0;                                       \
    jc.root.jst_node_data_get(&p, len);                   \
    EXPECT_EQ_STRING(expect, p, len);                     \
  } while (0)

#define TEST_ARRAY(expect, json)                         \
  do {                                                   \
    JParser c(json);                                     \
    EXPECT_EQ_RET(JST_PARSE_OK, c.jst_parser());         \
    EXPECT_EQ_TYPE(JST_ARR, c.root.get_type()); \
    JArray arr;                                          \
    c.root.jst_node_data_get(arr);                       \
    EXPECT_EQ_SIZE_T(expect, arr.size());                \
  } while (0)

#define TEST_NODE_STR(str, jn)                       \
  do {                                               \
    EXPECT_EQ_TYPE(JST_STR, jn.get_type()); \
    char* p = nullptr;                               \
    size_t len = 0;                                  \
    jn.jst_node_data_get(&p, len);                   \
    EXPECT_EQ_STRING(str, p, len);                   \
  } while (0)

#define TEST_NODE_NUM(expect, jn)                    \
  do {                                               \
    EXPECT_EQ_TYPE(JST_NUM, jn.get_type()); \
    double num = 0.0;                                \
    jn.jst_node_data_get(num);                       \
    EXPECT_EQ_DOUBLE(expect, num);                   \
  } while (0)

  
#define TEST_OBJ_KEY(str, key) EXPECT_EQ_STRING(str, key.c_str(), key.size());

}  // namespace jst