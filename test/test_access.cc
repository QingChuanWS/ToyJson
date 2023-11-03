#include "parser.h"
#include "utils.h"

namespace jst {
static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

static void test_access_null() {
  JNode jn;
  EXPECT_EQ_INT(JST_NULL, jn.type());
}

static void test_access_boolean() {
  JNode jn;
  jn.jst_node_data_set(JST_STR, "a", 1);
  jn.jst_node_data_reset(true);
  bool data;
  jn.jst_node_data_get(data);
  EXPECT_TRUE(data);
  jn.jst_node_data_set(JST_FALSE);
  jn.jst_node_data_get(data);
  EXPECT_FALSE(data);
}

static void test_access_number() {
  JNode jn;
  jn.jst_node_data_set(JST_STR, "a", 1);
  jn.jst_node_data_reset(1234.5);
  double num;
  jn.jst_node_data_get(num);
  EXPECT_EQ_DOUBLE(1234.5, num);
}

static void test_access_string() {
  JNode jn;
  jn.jst_node_data_set(JST_STR, "", 0);
  const char* str_head;
  size_t str_len;
  jn.jst_node_data_get(&str_head, str_len);
  EXPECT_EQ_STRING("", str_head, str_len);
  jn.jst_node_data_reset(JString("Hello", 5));
  jn.jst_node_data_get(&str_head, str_len);
  EXPECT_EQ_STRING("Hello", str_head, str_len);
}

static void test_access_array() {
  JArray a;
  JNode e;
  size_t i, j;

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

  e.jst_node_data_reset(JString("Hello", 5));
  a.push_back(e);

  i = a.capacity();
  a.clear();
  EXPECT_EQ_SIZE_T(0, a.size());
  EXPECT_EQ_SIZE_T(i, a.capacity()); /* capacity remains unchanged */
}

static void test_access_vector() {
  using jst::utils::JVector;

  JVector<JNode> v;
  JNode e;
  size_t i, j;

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

  e.jst_node_data_reset(JString("Hello", 5));
  v.push_back(e);

  i = v.capacity();
  v.clear();
  EXPECT_EQ_SIZE_T(0, v.size());
  EXPECT_EQ_SIZE_T(i, v.capacity()); /* capacity remains unchanged */
}

static void test_access_object() {
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

static void test_access() {
  test_access_null();
  test_access_boolean();
  test_access_number();
  test_access_string();
  test_access_array();
  test_access_vector();
  // test_access_object();
}
}  // namespace jst



int main() {
#ifdef _WINDOWS
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
  jst::test_access();
  printf("%d/%d (%3.2f%%) passed\n", jst::test_pass, jst::test_count,
         jst::test_pass * 100.0 / jst::test_count);
  return jst::main_ret;
}