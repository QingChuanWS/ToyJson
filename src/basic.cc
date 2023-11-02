#include "basic.h"

#include <assert.h>

#include <cctype>
#include <cmath>
#include <cstring>
#include <limits>

#include "node.h"

namespace jst {
/*
string class implemention;
*/
JString::JString(const char* str, size_t len) : length(len) {
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

JString::JString(const JString& str) : length(str.length) {
  ASSERT_VECTOR_NO_RET(str, s, length);
  this->s = new char[this->length + 1];
  memcpy(this->s, str.s, this->length * sizeof(char));
  this->s[this->length] = '\0';
}

JString& JString::operator=(const JString& str) {
  if (this->s != nullptr) delete[] this->s;

  ASSERT_VECTOR_HAS_RET(str, s, length, *this);
  this->length = str.length;
  this->s = new char[this->length + 1];
  memcpy(this->s, str.s, this->length * sizeof(char));
  this->s[this->length] = '\0';
  return *this;
}

JString::JString(JString&& str) noexcept : s(str.s), length(str.length) {
  ASSERT_VECTOR_NO_RET(str, s, length);
  str.s = nullptr;
  str.length = 0;
  return;
}

JString& JString::operator=(JString&& str) noexcept {
  if (this->s != nullptr) delete[] this->s;
  ASSERT_VECTOR_HAS_RET(str, s, length, *this);
  this->s = str.s;
  this->length = str.length;
  str.s = nullptr;
  str.length = 0;
  return *this;
}

JString::~JString() {
  if (this->s != nullptr) delete[] this->s;
  this->length = 0;
  this->s = nullptr;
}

const size_t JString::size() const { return length; };

const char* JString::c_str() const { return s; }

const bool JString::empty() const { return s == nullptr || length == 0; }

bool operator==(const JString& str_1, const JString& str_2) {
  return (str_1.length == str_2.length) && memcmp(str_1.c_str(), str_2.c_str(), str_1.size()) == 0;
}

bool operator==(const JNumber& num_1, const JNumber& num_2) {
  return std::fabs((num_1.num) - (num_2.num)) < std::numeric_limits<double>::epsilon();
}

/*
array class implemention;
*/

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

JArray::JArray(size_t len) {
  this->cap_ = tableSizeFor(len);
  this->a_ = new JNode[this->cap_];
  this->len_ = len;
}

JArray::JArray(const JArray& arr) {
  ASSERT_VECTOR_NO_RET(arr, a_, len_);
  this->cap_ = arr.cap_;
  this->len_ = arr.len_;
  this->a_ = new JNode[this->cap_];
  for (int i = 0; i < this->len_; i++) this->a_[i] = arr.a_[i];
}

JArray::JArray(JArray&& arr) noexcept : a_(arr.a_), len_(arr.len_), cap_(arr.cap_) {
  arr.a_ = nullptr;
  arr.len_ = 0;
  arr.cap_ = 0;
}

JArray& JArray::operator=(const JArray& arr) {
  if (this != &arr) {
    if (this->a_ != nullptr) delete[] this->a_;
    ASSERT_VECTOR_HAS_RET(arr, a_, len_, *this);
    this->cap_ = arr.cap_;
    this->len_ = arr.len_;
    this->a_ = new JNode[this->cap_];
    for (int i = 0; i < this->len_; i++) this->a_[i] = arr.a_[i];
  }
  return *this;
}

JArray& JArray::operator=(JArray&& arr) noexcept {
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

JArray::~JArray() {
  if (this->a_ != nullptr) delete[] this->a_;
  this->a_ = nullptr;
  this->len_ = 0;
  this->cap_ = 0;
}

JNode& JArray::operator[](int index) {
  JST_DEBUG(index >= 0 && index < len_);
  return this->a_[index];
}

const JNode& JArray::operator[](int index) const {
  JST_DEBUG(index >= 0 && index < len_);
  return this->a_[index];
}

size_t JArray::size() const { return len_; };
bool JArray::empty() const { return len_ == 0; }

size_t JArray::capacity() const { return this->cap_; }

const JNode* JArray::data() const { return this->a_; };

JNode* JArray::data() { return this->a_; };

#define JST_NODE_NOT_EXIST ((size_t)-1)

bool operator==(const JArray& arr_1, const JArray& arr_2) {
  if (arr_1.len_ != arr_2.len_) return false;
  size_t size = arr_1.size();
  for (int i = 0; i < size; i++) {
    if (arr_1.find(arr_2[i]) == JST_NODE_NOT_EXIST) return false;
  }
  return true;
}

size_t JArray::find(const JNode& jn) const {
  size_t size = this->size();
  for (size_t i = 0; i < size; i++) {
    if (jn == a_[i]) return i;
  }
  return JST_NODE_NOT_EXIST;
}

size_t JArray::insert(size_t pos, JNode& jn) {
  JST_DEBUG(pos <= size());
  if (this->cap_ == 0) {
    this->cap_ = 2;
    this->a_ = new JNode[this->cap_];
  }
  if (this->len_ + 1 > this->cap_) {
    this->cap_ += (this->cap_ >> 1);
    JNode* tmp = this->a_;
    this->a_ = new JNode[this->cap_];
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

size_t JArray::erase(size_t pos, size_t count) {
  JST_DEBUG(pos >= 0 && pos < this->len_);
  if (count == 0) return pos;
  count = std::min(count, this->len_ - pos);
  for (size_t i = pos; i + count < this->len_; i++) this->a_[i] = std::move(this->a_[i + count]);
  this->len_ -= count;
  return pos;
}

void JArray::push_back(const JNode& jn) {
  if (this->cap_ == 0) {
    this->cap_ = 2;
    this->a_ = new JNode[this->cap_];
  }
  if (this->len_ + 1 > this->cap_) {
    this->cap_ = this->cap_ == 1 ? 2 : this->cap_ + (this->cap_ >> 1);
    JNode* tmp = this->a_;
    this->a_ = new JNode[this->cap_];
    for (size_t i = 0; i < this->len_; i++) this->a_[i] = std::move(tmp[i]);
    delete[] tmp;
  }
  this->a_[this->len_++] = jn;
}

void JArray::pop_back() {
  if (this->len_ == 0) return;
  this->len_ -= 1;
}

void JArray::clear() { this->len_ = 0; }

void JArray::reserve(size_t new_cap) {
  if (new_cap <= this->cap_) return;
  this->cap_ = tableSizeFor(new_cap);
  if (this->len_ > 0) {
    JNode* tmp = this->a_;
    this->a_ = new JNode[this->cap_];
    for (size_t i = 0; i < this->len_; i++) this->a_[i] = std::move(tmp[i]);
    delete[] tmp;
  } else
    this->a_ = new JNode[this->cap_];
}

void JArray::shrink_to_fit() {
  if (cap_ == len_) return;
  JNode* tmp = this->a_;
  this->cap_ = this->len_;
  this->a_ = new JNode[this->len_];
  for (size_t i = 0; i < len_; i++) this->a_[i] = std::move(tmp[i]);
  delete[] tmp;
}

/*
object member implementation.
*/
JOjectMem::JOjectMem(const JString& key, const JNode& value) {
  this->key = new JString(key);
  this->value = new JNode(value);
}

JOjectMem::JOjectMem(JString&& key, JNode&& value) {
  this->key = new JString(std::move(key));
  this->value = new JNode(std::move(value));
}

JOjectMem::JOjectMem(const JOjectMem& om) {
  this->key = new JString(*om.key);
  this->value = new JNode(*om.value);
}

JOjectMem& JOjectMem::operator=(const JOjectMem& om) {
  if (this != &om) {
    if (this->key != nullptr) delete this->key;
    if (this->value != nullptr) delete this->value;
    this->key = new JString(*om.key);
    this->value = new JNode(*om.value);
  }
  return *this;
}

JOjectMem::JOjectMem(JOjectMem&& om) noexcept : key(om.key), value(om.value) {
  om.value = nullptr;
  om.key = nullptr;
}

JOjectMem& JOjectMem::operator=(JOjectMem&& om) noexcept {
  if (this->key != nullptr) delete this->key;
  if (this->value != nullptr) delete this->value;
  this->key = om.key;
  this->value = om.value;
  om.value = nullptr;
  om.key = nullptr;
  return *this;
}
JOjectMem::~JOjectMem() {
  if (this->value != nullptr) delete this->value;
  this->value = nullptr;
  if (this->key != nullptr) delete this->key;
  this->key = nullptr;
}

bool operator==(JOjectMem objm_1, JOjectMem objm_2) {
  return (objm_1.get_key() == objm_2.get_key()) && (objm_1.get_value() == objm_2.get_value());
}

bool operator!=(const JOjectMem objm_1, const JOjectMem objm_2) { return !(objm_1 == objm_2); }

/*
object class implemention;
*/

#define JST_KEY_NOT_EXIST ((size_t)-1)

size_t JObject::find_index(const JString& ky) {
  size_t size = this->size();
  for (size_t i = 0; i < size; i++) {
    if (ky == obj_[i].get_key()) return i;
  }
  return JST_KEY_NOT_EXIST;
}

const JNode* JObject::find_value(const JString& ky) {
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

bool operator==(JObject& obj_1, JObject& obj_2) {
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

bool operator!=(JObject& objm_1, JObject& objm_2) { return !(objm_1 == objm_2); }

}  // namespace jst