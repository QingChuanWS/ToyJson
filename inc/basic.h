#ifndef __JSON_TOY_BASIC_H__
#define __JSON_TOY_BASIC_H__

#include <assert.h>

#include <algorithm>
#include <memory>
#include <string>

#include "enum.h"
#include "jst_vector.h"

namespace jst {

class JNode;
using jst::utils::jst_vector;

struct NumberExp {
  bool is_have;
  int exp_index;
  NumberExp() : is_have(false), exp_index(0) {}
};

struct NumberPoint {
  bool is_have;
  int point_index;
  NumberPoint() : is_have(false), point_index(0) {}
};

class JNodeData {
 public:
  JNodeData() {}
  virtual ~JNodeData(){};
};

class String : virtual public JNodeData {
 private:
  size_t length;
  char* s;

 public:
  explicit String() : s(nullptr), length(0) {}
  explicit String(const char* s, size_t len = 0);

  String(const String& s);
  String& operator=(const String& s);
  String(String&& s) noexcept;
  String& operator=(String&& s) noexcept;
  ~String();

 public:
  size_t size() const;
  char* c_str() const;
  bool empty() const;

 public:
  friend bool operator==(const String& str_1, const String& str_2);
};

class Number : public JNodeData {
 public:
  Number() : num(0.0) {}
  friend bool operator==(const Number& num_1, const Number& num_2);
  explicit Number(double n) : num(n){};
  double num;
};

class Array : public JNodeData {
 private:
  JNode* a_;
  size_t len_;
  size_t cap_;

 public:
  Array();
  explicit Array(size_t len);
  Array(const Array& arr);
  Array(Array&& arr) noexcept;
  Array& operator=(const Array& arr);
  Array& operator=(Array&& arr) noexcept;
  ~Array();

 public:
  JNode& operator[](int index);
  const JNode& operator[](int index) const;

  size_t find(const JNode& jn) const;
  size_t size() const;
  size_t capacity() const;
  bool empty() const;

  const JNode* data() const;
  JNode* data();

 public:
  size_t insert(size_t pos, JNode& jn);
  size_t erase(size_t pos, size_t count = 1);
  void push_back(const JNode& jn);
  void pop_back();
  void clear();
  void reserve(size_t new_cap);
  void shrink_to_fit();

 public:
  friend bool operator==(const Array& arr_1, const Array& arr_2);
};

// object
class OjectMember {
 private:
  String* key;
  JNode* value;

 public:
  OjectMember() : key(), value() {}
  OjectMember(const String& key, const JNode& value);
  OjectMember(String&& key, JNode&& value);

  OjectMember(const OjectMember& om);
  OjectMember& operator=(const OjectMember& om);
  OjectMember(OjectMember&& om) noexcept;
  OjectMember& operator=(OjectMember&& om) noexcept;
  ~OjectMember();

 public:
  const String& get_key() const { return *key; }
  const JNode& get_value() const { return *value; }

 public:
  friend bool operator==(const OjectMember objm_1, const OjectMember objm_2);
  friend bool operator!=(const OjectMember objm_1, const OjectMember objm_2);
};

class Object : public JNodeData {
 private:
  jst_vector<OjectMember> obj_;

 public:
  Object(){};
  explicit Object(size_t length) { obj_.reserve(length); }

 public:
  const OjectMember& operator[](int index) const { return this->obj_[index]; }
  OjectMember& operator[](int index) { return this->obj_[index]; }

  size_t find_index(const String& key);
  const JNode* find_value(const String& key);
  bool empty() const { return obj_.empty(); }
  size_t size() const { return obj_.size(); }
  size_t capacity() const { return obj_.capacity(); }

  const OjectMember* data() const { return this->obj_.data(); }
  OjectMember* data() { return this->obj_.data(); }

  const String& get_key(size_t index) const { return obj_[index].get_key(); }
  const JNode& get_value(size_t index) const { return obj_[index].get_value(); }

 public:
  size_t insert(size_t pos, OjectMember& objm);
  size_t erase(size_t pos, size_t count = 1);
  void push_back(const OjectMember& objm);
  void pop_back();
  void clear();
  void reserve(size_t new_cap);
  void shrink_to_fit();

 public:
  friend bool operator==(Object& obj_1, Object& obj_2);
  friend bool operator!=(Object& objm_1, Object& objm_2);
};

}  // namespace jst

#endif  //__JSON_TOY_BASIC_H__