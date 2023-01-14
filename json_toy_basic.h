#ifndef __JSON_TOY_BASIC_H__
#define __JSON_TOY_BASIC_H__

#include <assert.h>

#include <algorithm>
#include <memory>
#include <string>

#include "json_toy_enum.h"
#include "jst_vector.h"

namespace jst {

class jst_node;
using jst::utils::jst_vector;
using std::shared_ptr;

struct jst_num_exp {
  bool is_have;
  int exp_index;
  jst_num_exp() : is_have(false), exp_index(0) {}
};

struct jst_num_point {
  bool is_have;
  int point_index;
  jst_num_point() : is_have(false), point_index(0) {}
};

class jst_node_data {
 public:
  jst_node_data() {}
  virtual ~jst_node_data(){};
};

class string : virtual public jst_node_data {
 private:
  size_t length;
  char* s;

 public:
  explicit string() : s(nullptr), length(0) {}
  explicit string(const char* s, size_t len = 0);

  string(const string& s);
  string& operator=(const string& s);
  string(string&& s) noexcept;
  string& operator=(string&& s) noexcept;
  ~string();

 public:
  size_t size() const;
  char* c_str() const;
  bool empty() const;

 public:
  friend bool operator==(const string& str_1, const string& str_2);
};

class number : public jst_node_data {
 public:
  number() : num(0.0) {}
  friend bool operator==(const number& num_1, const number& num_2);
  explicit number(double n) : num(n){};
  double num;
};

class array : public jst_node_data {
 private:
  jst_node* a_;
  size_t len_;
  size_t cap_;

 public:
  array();
  explicit array(size_t len);
  array(const array& arr);
  array(array&& arr) noexcept;
  array& operator=(const array& arr);
  array& operator=(array&& arr) noexcept;
  ~array();

 public:
  jst_node& operator[](int index);
  const jst_node& operator[](int index) const;

  size_t find(const jst_node& jn) const;
  size_t size() const;
  size_t capacity() const;
  bool empty() const;

  const jst_node* data() const;
  jst_node* data();

 public:
  size_t insert(size_t pos, jst_node& jn);
  size_t erase(size_t pos, size_t count = 1);
  void push_back(const jst_node& jn);
  void pop_back();
  void clear();
  void reserve(size_t new_cap);
  void shrink_to_fit();

 public:
  friend bool operator==(const array& arr_1, const array& arr_2);
};

// object
class object_member {
 private:
  string* key;
  jst_node* value;

 public:
  object_member() : key(), value() {}
  object_member(const string& key, const jst_node& value);
  object_member(string&& key, jst_node&& value);

  object_member(const object_member& om);
  object_member& operator=(const object_member& om);
  object_member(object_member&& om) noexcept;
  object_member& operator=(object_member&& om) noexcept;
  ~object_member();

 public:
  const string& get_key() const { return *key; }
  const jst_node& get_value() const { return *value; }

 public:
  friend bool operator==(const object_member objm_1, const object_member objm_2);
  friend bool operator!=(const object_member objm_1, const object_member objm_2);
};

class object : public jst_node_data {
 private:
  jst_vector<object_member> obj_;

 public:
  object(){};
  explicit object(size_t length) { obj_.reserve(length); }

 public:
  const object_member& operator[](int index) const { return this->obj_[index]; }
  object_member& operator[](int index) { return this->obj_[index]; }

  size_t find_index(const string& key);
  const jst_node* find_value(const string& key);
  bool empty() const { return obj_.empty(); }
  size_t size() const { return obj_.size(); }
  size_t capacity() const { return obj_.capacity(); }

  const object_member* data() const { return this->obj_.data(); }
  object_member* data() { return this->obj_.data(); }

  const string& get_key(size_t index) const { return obj_[index].get_key(); }
  const jst_node& get_value(size_t index) const { return obj_[index].get_value(); }

 public:
  size_t insert(size_t pos, object_member& objm);
  size_t erase(size_t pos, size_t count = 1);
  void push_back(const object_member& objm);
  void pop_back();
  void clear();
  void reserve(size_t new_cap);
  void shrink_to_fit();

 public:
  friend bool operator==(object& obj_1, object& obj_2);
  friend bool operator!=(object& objm_1, object& objm_2);
};

}  // namespace jst

#endif  //__JSON_TOY_BASIC_H__