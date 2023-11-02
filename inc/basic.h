#ifndef __JSON_TOY_BASIC_H__
#define __JSON_TOY_BASIC_H__

#include <assert.h>

#include <algorithm>
#include <memory>
#include <string>

#include "enum.h"
#include "utils.h"

namespace jst {

class JNode;
using jst::utils::JVector;

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

class JData {
 public:
  JData() = default;
  virtual ~JData(){};
};

class JString : virtual public JData {
 private:
  size_t length = 0;
  char* s = nullptr;

 public:
  explicit JString() = default;
  explicit JString(const char* s, size_t len = 0);

  JString(const JString& s);
  JString& operator=(const JString& s);
  JString(JString&& s) noexcept;
  JString& operator=(JString&& s) noexcept;
  ~JString();

 public:
  const size_t size() const;
  const char* c_str() const;
  const bool empty() const;

 public:
  friend bool operator==(const JString& str_1, const JString& str_2);
};

class JNumber : public JData {
 private:
  double num = 0.0;

 public:
  JNumber() = default;
  friend bool operator==(const JNumber& num_1, const JNumber& num_2);
  explicit JNumber(double n) : num(n){};
  const double get() const { return num; };
};

class JArray : public JData {
 private:
  JNode* a_ = nullptr;
  size_t len_ = 0;
  size_t cap_ = 0;

 public:
  JArray() = default;
  explicit JArray(size_t len);
  JArray(const JArray& arr);
  JArray(JArray&& arr) noexcept;
  JArray& operator=(const JArray& arr);
  JArray& operator=(JArray&& arr) noexcept;
  ~JArray();

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
  friend bool operator==(const JArray& left, const JArray& right);
};

// object
class JOjectMem {
 private:
  JString* key = nullptr;
  JNode* value = nullptr;

 public:
  JOjectMem() = default;
  JOjectMem(const JString& key, const JNode& value);
  JOjectMem(JString&& key, JNode&& value);

  JOjectMem(const JOjectMem& om);
  JOjectMem& operator=(const JOjectMem& om);
  JOjectMem(JOjectMem&& om) noexcept;
  JOjectMem& operator=(JOjectMem&& om) noexcept;
  ~JOjectMem();

 public:
  const JString& get_key() const { return *key; }
  const JNode& get_value() const { return *value; }

 public:
  friend bool operator==(const JOjectMem objm_1, const JOjectMem objm_2);
  friend bool operator!=(const JOjectMem objm_1, const JOjectMem objm_2);
};

class JObject : public JData {
 private:
  JVector<JOjectMem> obj_;

 public:
  JObject(){};
  explicit JObject(size_t length) { obj_.reserve(length); }

 public:
  const JOjectMem& operator[](int index) const { return this->obj_[index]; }
  JOjectMem& operator[](int index) { return this->obj_[index]; }

  size_t find_index(const JString& key);
  const JNode* find_value(const JString& key);
  bool empty() const { return obj_.empty(); }
  size_t size() const { return obj_.size(); }
  size_t capacity() const { return obj_.capacity(); }

  const JOjectMem* data() const { return this->obj_.data(); }
  JOjectMem* data() { return this->obj_.data(); }

  const JString& get_key(size_t index) const { return obj_[index].get_key(); }
  const JNode& get_value(size_t index) const { return obj_[index].get_value(); }

 public:
  size_t insert(size_t pos, JOjectMem& objm);
  size_t erase(size_t pos, size_t count = 1);
  void push_back(const JOjectMem& objm);
  void pop_back();
  void clear();
  void reserve(size_t new_cap);
  void shrink_to_fit();

 public:
  friend bool operator==(JObject& obj_1, JObject& obj_2);
  friend bool operator!=(JObject& objm_1, JObject& objm_2);
};

}  // namespace jst

#endif  //__JSON_TOY_BASIC_H__