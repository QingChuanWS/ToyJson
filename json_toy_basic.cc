#include "json_toy_basic.h"
#include "json_toy_node.h"
#include <cctype>
#include <cstring>
#include <assert.h>

namespace jst {

/*
string class implemention;
*/
string::string(const char *str, size_t len) : length(len) {
    if (str == nullptr) {
        this->s = nullptr;
        this->length = 0;
        return;
    }
    if (len == 0)
        this->length = strlen(str);
    this->s = new char[this->length + 1];
    memcpy(this->s, str, this->length * sizeof(char));
    this->s[this->length] = '\0';
}

string::string(const string &str) : length(str.length) {
    ASSERT_VECTOR_NO_RET(str, s, length);
    this->s = new char[this->length + 1];
    memcpy(this->s, str.s, this->length * sizeof(char));
    this->s[this->length] = '\0';
}

string &string::operator=(const string &str) {
    if (this->s != nullptr)
        delete[] this->s;

    ASSERT_VECTOR_HAS_RET(str, s, length, *this);
    this->length = str.length;
    this->s = new char[this->length + 1];
    memcpy(this->s, str.s, this->length * sizeof(char));
    this->s[this->length] = '\0';
    return *this;
}

string::string(string &&str) noexcept : s(str.s), length(str.length) {
    ASSERT_VECTOR_NO_RET(str, s, length);
    str.s = nullptr;
    str.length = 0;
    return;
}

string &string::operator=(string &&str) noexcept {
    if (this->s != nullptr)
        delete[] this->s;
    ASSERT_VECTOR_HAS_RET(str, s, length, *this);
    this->s = str.s;
    this->length = str.length;
    str.s = nullptr;
    str.length = 0;
    return *this;
}

string::~string() {
    if (this->s != nullptr)
        delete[] this->s;
    this->length = 0;
}

/*
array class implemention;
*/
array::array(size_t length) : length(length) {
    this->a = new jst_node[this->length];
    for (int i = 0; i < this->length; i++)
        this->a[i] = jst_node();
}

array::array(const array &arr) : a(arr.a), length(arr.length) {
    ASSERT_VECTOR_NO_RET(arr, a, length);
    this->length = arr.length;
    this->a = new jst_node[this->length];
    for (int i = 0; i < this->length; i++)
        this->a[i] = arr.a[i];
}

array &array::operator=(const array &arr) {
    if (this->a != nullptr)
        delete[] this->a;
    ASSERT_VECTOR_HAS_RET(arr, a, length, *this);
    this->length = arr.length;
    this->a = new jst_node[this->length];
    for (int i = 0; i < this->length; i++)
        this->a[i] = arr.a[i];
    return *this;
}

array::array(array &&arr) noexcept : a(arr.a), length(arr.length) {
    arr.a = nullptr;
    arr.length = 0;
}

array &array::operator=(array &&arr) noexcept {
    if (this->a != nullptr)
        delete[] this->a;
    ASSERT_VECTOR_HAS_RET(arr, a, length, *this);
    this->a = arr.a;
    this->length = arr.length;
    arr.a = nullptr;
    arr.length = 0;
    return *this;
}

array::~array() {
    if (this->a != nullptr)
        delete[] this->a;
    this->a = nullptr;
    this->length = 0;
}

jst_node &array::operator[](int index) {
    assert(index >= 0 && index < length);
    return this->a[index];
}

/*
object class implemention;
*/
object::object(size_t len) : obj_m(nullptr), length(0) {
    if (len == 0)
        return;
    this->length = len;
    this->obj_m = new object_member[this->length];
    for (int i = 0; i < this->length; i++)
        this->obj_m[i] = object_member();
}

object::object(const object &o) : length(o.length) {
    if (o.is_empty()) {
        this->obj_m = nullptr;
        this->length = 0;
        return;
    }
    this->length = o.length;
    this->obj_m = new object_member[this->length];
    for (int i = 0; i < this->length; i++)
        this->obj_m[i] = o.obj_m[i];
}

object &object::operator=(const object &o) {
    if (this->obj_m == nullptr)
        delete[] this->obj_m;
    if (o.is_empty()) {
        this->obj_m = nullptr;
        this->length = 0;
        return *this;
    }
    this->length = o.length;
    this->obj_m = new object_member[this->length];
    for (int i = 0; i < this->length; i++)
        this->obj_m[i] = o.obj_m[i];
    return *this;
}

object::object(object &&o) noexcept : obj_m(o.obj_m), length(o.length) {
    o.obj_m = nullptr;
    o.length = 0;
}

object &object::operator=(object &&o) noexcept {
    if (this->obj_m != nullptr)
        delete[] this->obj_m;
    if (o.is_empty()) {
        this->obj_m = nullptr;
        this->length = 0;
        return *this;
    }
    this->obj_m = o.obj_m;
    this->length = o.length;
    o.obj_m = nullptr;
    o.length = 0;
    return *this;
}

object::~object() {
    if (this->obj_m != nullptr)
        delete[] this->obj_m;
    this->obj_m = nullptr;
    this->length = 0;
}

object_member &object::operator[](int index) {
    assert(index >= 0 && index < length);
    return this->obj_m[index];
}

} // namespace jst