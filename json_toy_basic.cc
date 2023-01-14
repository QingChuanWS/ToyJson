#include "json_toy_basic.h"

#include <assert.h>

#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>

#include "json_toy_node.h"

namespace jst {
/*
string class implemention;
*/
string::string(const char* str, size_t len) : length(len) {
  if (str == nullptr) {
    this->s = nullptr;
    this->length = 0;
    return;
  }
  if (len == 0) this->length = strlen(str);
  this->s = new char[this->length + 1];
  memcpy(this->s, str, this->length * sizeof(char));
  this->s[this->length] = '\0';
}

string::string(const string& str) : length(str.length) {
  ASSERT_VECTOR_NO_RET(str, s, length);
  this->s = new char[this->length + 1];
  memcpy(this->s, str.s, this->length * sizeof(char));
  this->s[this->length] = '\0';
}

string& string::operator=(const string& str) {
  if (this->s != nullptr) delete[] this->s;

  ASSERT_VECTOR_HAS_RET(str, s, length, *this);
  this->length = str.length;
  this->s = new char[this->length + 1];
  memcpy(this->s, str.s, this->length * sizeof(char));
  this->s[this->length] = '\0';
  return *this;
}

string::string(string&& str) noexcept : s(str.s), length(str.length) {
  ASSERT_VECTOR_NO_RET(str, s, length);
  str.s = nullptr;
  str.length = 0;
  return;
}

string& string::operator=(string&& str) noexcept {
  if (this->s != nullptr) delete[] this->s;
  ASSERT_VECTOR_HAS_RET(str, s, length, *this);
  this->s = str.s;
  this->length = str.length;
  str.s = nullptr;
  str.length = 0;
  return *this;
}

string::~string() {
  if (this->s != nullptr) delete[] this->s;
  this->length = 0;
  this->s = nullptr;
}

size_t string::size() const { return length; };

char* string::c_str() const { return s; }

bool string::empty() const { return s == nullptr || length == 0; }

bool operator==(const string& str_1, const string& str_2) {
  return (str_1.length == str_2.length) && memcmp(str_1.c_str(), str_2.c_str(), str_1.size()) == 0;
}

bool operator==(const number& num_1, const number& num_2) {
  return std::fabs((num_1.num) - (num_2.num)) < std::numeric_limits<double>::epsilon();
}

/*
array class implemention;
*/

array::array() : a_(nullptr), len_(0), cap_(0) {}

// The expansion threshold is the nearest 2 to the NTH power of the input
// capacity.
inline int tableSizeFor(int cap) {
  // In case it's a power of two
  int n = cap - 1;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  return (n < 0) ? 1 : n + 1;
}

array::array(size_t len) {
  this->cap_ = tableSizeFor(len);
  this->a_ = new jst_node[this->cap_];
  this->len_ = len;
}

array::array(const array& arr) {
  ASSERT_VECTOR_NO_RET(arr, a_, len_);
  this->cap_ = arr.cap_;
  this->len_ = arr.len_;
  this->a_ = new jst_node[this->cap_];
  for (int i = 0; i < this->len_; i++) this->a_[i] = arr.a_[i];
}

array::array(array&& arr) noexcept : a_(arr.a_), len_(arr.len_), cap_(arr.cap_) {
  arr.a_ = nullptr;
  arr.len_ = 0;
  arr.cap_ = 0;
}

array& array::operator=(const array& arr) {
  if (this != &arr) {
    if (this->a_ != nullptr) delete[] this->a_;
    ASSERT_VECTOR_HAS_RET(arr, a_, len_, *this);
    this->cap_ = arr.cap_;
    this->len_ = arr.len_;
    this->a_ = new jst_node[this->cap_];
    for (int i = 0; i < this->len_; i++) this->a_[i] = arr.a_[i];
  }
  return *this;
}

array& array::operator=(array&& arr) noexcept {
  if (this->a_ != nullptr) delete[] this->a_;
  ASSERT_VECTOR_HAS_RET(arr, a_, len_, *this);
  this->a_ = arr.a_;
  this->cap_ = arr.cap_;
  this->len_ = arr.len_;
  arr.a_ = nullptr;
  arr.len_ = 0;
  arr.cap_ = 0;
  return *this;
}

array::~array() {
  if (this->a_ != nullptr) delete[] this->a_;
  this->a_ = nullptr;
  this->len_ = 0;
  this->cap_ = 0;
}

jst_node& array::operator[](int index) {
  JST_DEBUG(index >= 0 && index < len_);
  return this->a_[index];
}

const jst_node& array::operator[](int index) const {
  JST_DEBUG(index >= 0 && index < len_);
  return this->a_[index];
}

size_t array::size() const { return len_; };
bool array::empty() const { return len_ == 0; }

size_t array::capacity() const { return this->cap_; }

const jst_node* array::data() const { return this->a_; };

jst_node* array::data() { return this->a_; };

#define JST_NODE_NOT_EXIST ((size_t)-1)

bool operator==(const array& arr_1, const array& arr_2) {
  if (arr_1.len_ != arr_2.len_) return false;
  size_t size = arr_1.size();
  for (int i = 0; i < size; i++) {
    if (arr_1.find(arr_2[i]) == JST_NODE_NOT_EXIST) return false;
  }
  return true;
}

size_t array::find(const jst_node& jn) const {
  size_t size = this->size();
  for (size_t i = 0; i < size; i++) {
    if (jn == a_[i]) return i;
  }
  return JST_NODE_NOT_EXIST;
}

size_t array::insert(size_t pos, jst_node& jn) {
  JST_DEBUG(pos <= size());
  if (this->cap_ == 0) {
    this->cap_ = 2;
    this->a_ = new jst_node[this->cap_];
  }
  if (this->len_ + 1 > this->cap_) {
    this->cap_ += (this->cap_ >> 1);
    jst_node* tmp = this->a_;
    this->a_ = new jst_node[this->cap_];
    for (size_t i = 0; i < pos; i++) this->a_[i] = std::move(tmp[i]);
    this->a_[pos] = jn;
    for (size_t i = pos + 1; i < this->len_ + 1; i++) this->a_[i] = tmp[i - 1];
    delete[] tmp;
  } else {
    for (size_t i = this->len_; i > pos; i--) this->a_[i] = std::move(this->a_[i - 1]);
    this->a_[pos] = jn;
  }
  this->len_ += 1;
  return pos;
}

size_t array::erase(size_t pos, size_t count) {
  JST_DEBUG(pos >= 0 && pos < this->len_);
  if (count == 0) return pos;
  count = std::min(count, this->len_ - pos);
  for (size_t i = 0; i < count; i++) this->a_[pos + i].jst_node_type_reset();
  for (size_t i = pos; i + count < this->len_; i++) this->a_[i] = std::move(this->a_[i + count]);
  this->len_ -= count;
  return pos;
}

void array::push_back(const jst_node& jn) {
  if (this->cap_ == 0) {
    this->cap_ = 2;
    this->a_ = new jst_node[this->cap_];
  }
  if (this->len_ + 1 > this->cap_) {
    this->cap_ = this->cap_ == 1 ? 2 : this->cap_ + (this->cap_ >> 1);
    jst_node* tmp = this->a_;
    this->a_ = new jst_node[this->cap_];
    for (size_t i = 0; i < this->len_; i++) this->a_[i] = std::move(tmp[i]);
    delete[] tmp;
  }
  this->a_[this->len_++] = jn;
}

void array::pop_back() {
  if (this->len_ == 0) return;
  this->a_[this->len_ - 1].jst_node_type_reset();
  this->len_ -= 1;
}

void array::clear() {
  for (size_t i = 0; i < this->len_; i++) this->a_[i].jst_node_type_reset();
  this->len_ = 0;
}

void array::reserve(size_t new_cap) {
  if (new_cap <= this->cap_) return;
  this->cap_ = tableSizeFor(new_cap);
  if (this->len_ > 0) {
    jst_node* tmp = this->a_;
    this->a_ = new jst_node[this->cap_];
    for (size_t i = 0; i < this->len_; i++) this->a_[i] = std::move(tmp[i]);
    delete[] tmp;
  } else
    this->a_ = new jst_node[this->cap_];
}

void array::shrink_to_fit() {
  if (cap_ == len_) return;
  jst_node* tmp = this->a_;
  this->cap_ = this->len_;
  this->a_ = new jst_node[this->len_];
  for (size_t i = 0; i < len_; i++) this->a_[i] = std::move(tmp[i]);
  delete[] tmp;
}

/*
object member implementation.
*/
object_member::object_member(const string& key, const jst_node& value) {
  this->key = new string(key);
  this->value = new jst_node(value);
}

object_member::object_member(string&& key, jst_node&& value) {
  this->key = new string(std::move(key));
  this->value = new jst_node(std::move(value));
}

object_member::object_member(const object_member& om) {
  this->key = new string(*om.key);
  this->value = new jst_node(*om.value);
}

object_member& object_member::operator=(const object_member& om) {
  if (this != &om) {
    if (this->key != nullptr) delete this->key;
    if (this->value != nullptr) delete this->value;
    this->key = new string(*om.key);
    this->value = new jst_node(*om.value);
  }
  return *this;
}

object_member::object_member(object_member&& om) noexcept : key(om.key), value(om.value) {
  om.value = nullptr;
  om.key = nullptr;
}

object_member& object_member::operator=(object_member&& om) noexcept {
  if (this->key != nullptr) delete this->key;
  if (this->value != nullptr) delete this->value;
  this->key = om.key;
  this->value = om.value;
  om.value = nullptr;
  om.key = nullptr;
  return *this;
}
object_member::~object_member() {
  if (this->value != nullptr) delete this->value;
  this->value = nullptr;
  if (this->key != nullptr) delete this->key;
  this->key = nullptr;
}

bool operator==(object_member objm_1, object_member objm_2) {
  return (objm_1.get_key() == objm_2.get_key()) && (objm_1.get_value() == objm_2.get_value());
}

bool operator!=(const object_member objm_1, const object_member objm_2) {
  return !(objm_1 == objm_2);
}

/*
object class implemention;
*/

#define JST_KEY_NOT_EXIST ((size_t)-1)

size_t object::find_index(const string& ky) {
  size_t size = this->size();
  for (size_t i = 0; i < size; i++) {
    if (ky == obj_[i].get_key()) return i;
  }
  return JST_KEY_NOT_EXIST;
}

const jst_node* object::find_value(const string& ky) {
  size_t size = this->size();
  size_t i = JST_KEY_NOT_EXIST;
  for (size_t t = 0; t < size; t++) {
    if (ky == obj_[i].get_key()) {
      i = t;
      break;
    }
  }
  return i != JST_KEY_NOT_EXIST ? &obj_[i].get_value() : nullptr;
}

bool operator==(object& obj_1, object& obj_2) {
  if (obj_1.size() != obj_2.size()) return false;
  size_t size = obj_1.size();
  for (int i = 0; i < size; i++) {
    size_t index;
    if ((index = obj_1.find_index(obj_2[i].get_key())) == JST_KEY_NOT_EXIST ||
        obj_1[index] != obj_2[i])
      return false;
  }
  return true;
}

bool operator!=(object& objm_1, object& objm_2) { return !(objm_1 == objm_2); }

}  // namespace jst