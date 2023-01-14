#ifndef __JSON_TOY_VECTOR_H__
#define __JSON_TOY_VECTOR_H__

#include <stdlib.h>

#include <algorithm>
#include <cassert>
#include <memory>

#define JST_DEBUG(expr) assert(expr)

namespace jst {
namespace utils {

template <typename T>
class jst_vector {
 private:
  T* begin_;
  size_t len_;
  size_t cap_;

 public:
  jst_vector() : begin_(nullptr), len_(0), cap_(0) {}

  explicit jst_vector(size_t len) {
    this->cap_ = std::max(static_cast<size_t>(2), len);
    this->begin_ = new T[this->cap_];
    this->len_ = len;
  }
  jst_vector(const jst_vector& arr);
  jst_vector(jst_vector&& arr) noexcept;
  jst_vector& operator=(const jst_vector& arr);
  jst_vector& operator=(jst_vector&& arr) noexcept;
  ~jst_vector();

 public:
  T& operator[](size_t index);
  const T& operator[](size_t index) const;

  size_t find(const T& jn) const;
  size_t size() const { return len_; };
  size_t capacity() const { return this->cap_; };
  bool empty() const { return len_ == 0; }

  const T* data() const { return this->begin_; };
  T* data() { return this->begin_; };

 public:
  size_t insert(size_t pos, T& jn);
  size_t erase(size_t pos, size_t count = 1);
  void push_back(const T& jn);
  void pop_back();
  void clear();
  void reserve(size_t new_cap);
  void shrink_to_fit();
};

template <typename T>
jst_vector<T>::jst_vector(const jst_vector<T>& v) {
  this->cap_ = v.cap_;
  this->len_ = v.len_;
  this->begin_ = new T[this->cap_];
  for (int i = 0; i < this->len_; i++) this->begin_[i] = v.begin_[i];
}

template <typename T>
jst_vector<T>::jst_vector(jst_vector<T>&& v) noexcept
    : begin_(v.begin_), len_(v.len_), cap_(v.cap_) {
  v.begin_ = nullptr;
  v.len_ = 0;
  v.cap_ = 0;
}

template <typename T>
jst_vector<T>& jst_vector<T>::operator=(const jst_vector<T>& v) {
  if (this != &v) {
    if (this->begin_ != nullptr) delete[] this->begin_;
    this->cap_ = v.cap_;
    this->len_ = v.len_;
    this->begin_ = new T[this->cap_];
    for (int i = 0; i < this->len_; i++) this->begin_[i] = v.begin_[i];
  }
  return *this;
}

template <typename T>
jst_vector<T>& jst_vector<T>::operator=(jst_vector<T>&& v) noexcept {
  if (this->begin_ != nullptr) delete[] this->begin_;
  this->begin_ = v.begin_;
  this->cap_ = v.cap_;
  this->len_ = v.len_;
  v.begin_ = nullptr;
  v.len_ = 0;
  v.cap_ = 0;
  return *this;
}

template <typename T>
jst_vector<T>::~jst_vector() {
  if (this->begin_ != nullptr) delete[] this->begin_;
  this->begin_ = nullptr;
  this->len_ = 0;
  this->cap_ = 0;
}

template <typename T>
T& jst_vector<T>::operator[](size_t index) {
  JST_DEBUG(index >= 0 && index < len_);
  return this->begin_[index];
}

template <typename T>
const T& jst_vector<T>::operator[](size_t index) const {
  JST_DEBUG(index < size());
  return this->begin_[index];
}

#define JST_NODE_NOT_EXIST ((size_t)-1)

template <typename T>
size_t jst_vector<T>::find(const T& jn) const {
  size_t size = this->size();
  for (size_t i = 0; i < size; i++) {
    if (jn == begin_[i]) return i;
  }
  return JST_NODE_NOT_EXIST;
}

template <typename T>
size_t jst_vector<T>::insert(size_t pos, T& jn) {
  JST_DEBUG(pos <= size());
  if (this->cap_ == 0) {
    this->cap_ = 2;
    this->begin_ = new T[this->cap_];
  }
  if (this->len_ + 1 > this->cap_) {
    this->cap_ += (this->cap_ >> 1);
    T* tmp = this->begin_;
    this->begin_ = new T[this->cap_];
    for (size_t i = 0; i < pos; i++) this->begin_[i] = std::move(tmp[i]);
    this->begin_[pos] = jn;
    for (size_t i = pos + 1; i < this->len_ + 1; i++) this->begin_[i] = tmp[i - 1];
    delete[] tmp;
  } else {
    for (size_t i = this->len_; i > pos; i--) this->begin_[i] = std::move(this->begin_[i - 1]);
    this->begin_[pos] = jn;
  }
  this->len_ += 1;
  return pos;
}

template <typename T>
size_t jst_vector<T>::erase(size_t pos, size_t count) {
  JST_DEBUG(pos < size());
  if (count == 0) return pos;
  count = std::min(count, this->len_ - pos);
  for (size_t i = 0; i < count; i++) this->begin_[pos + i].~T();
  for (size_t i = pos; i + count < this->len_; i++)
    this->begin_[i] = std::move(this->begin_[i + count]);
  this->len_ -= count;
  return pos;
}

template <typename T>
void jst_vector<T>::push_back(const T& jn) {
  if (this->cap_ == 0) {
    this->cap_ = 2;
    this->begin_ = new T[this->cap_];
  }
  if (this->len_ + 1 > this->cap_) {
    this->cap_ = this->cap_ == 1 ? 2 : this->cap_ + (this->cap_ >> 1);
    T* tmp = this->begin_;
    this->begin_ = new T[this->cap_];
    for (size_t i = 0; i < this->len_; i++) this->begin_[i] = std::move(tmp[i]);
    delete[] tmp;
  }
  this->begin_[this->len_++] = jn;
}

template <typename T>
void jst_vector<T>::pop_back() {
  if (this->len_ == 0) return;
  this->begin_[this->len_ - 1].~T();
  this->len_ -= 1;
}

template <typename T>
void jst_vector<T>::clear() {
  for (size_t i = 0; i < this->len_; i++) this->begin_[i].~T();
  this->len_ = 0;
}

template <typename T>
void jst_vector<T>::reserve(size_t new_cap) {
  if (new_cap <= this->cap_) return;
  this->cap_ = std::max(static_cast<size_t>(2), new_cap);
  if (this->len_ > 0) {
    T* tmp = this->begin_;
    this->begin_ = new T[this->cap_];
    for (size_t i = 0; i < this->len_; i++) this->begin_[i] = std::move(tmp[i]);
    delete[] tmp;
  } else
    this->begin_ = new T[this->cap_];
}

template <typename T>
void jst_vector<T>::shrink_to_fit() {
  if (cap_ == len_) return;
  T* tmp = this->begin_;
  this->cap_ = this->len_;
  this->begin_ = new T[this->len_];
  for (size_t i = 0; i < len_; i++) this->begin_[i] = std::move(tmp[i]);
  delete[] tmp;
}

template <typename T>
bool operator==(const jst_vector<T>& v_1, const jst_vector<T>& v_2) {
  if (v_1.size() != v_2.size()) return false;
  size_t size = v_1.size();
  for (int i = 0; i < size; i++) {
    size_t index;
    if ((index = v_1.find(v_2[i])) == JST_NODE_NOT_EXIST) return false;
  }
  return true;
}

}  // namespace utils
}  // namespace jst

#endif  // !__JSON_TOY_VECTOR_H__