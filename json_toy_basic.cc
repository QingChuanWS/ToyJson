#include "json_toy_basic.h"
#include "json_toy_node.h"
#include <cctype>
#include <cstring>

namespace jst {
string::string(const char *str, size_t len) : length(len) {
    ASSERT_VECTOR_NO_RET(str, len, s, length);
    if (len == 0)
        this->length = strlen(str);
    this->s = new char[this->length + 1];
    memcpy(this->s, str, this->length * sizeof(char));
    this->s[this->length] = '\0';
}

string::string(const string &str) : length(str.length) {
    ASSERT_VECTOR_NO_RET(str.s, str.length, s, length);
    this->s = new char[this->length + 1];
    memcpy(this->s, str.s, this->length * sizeof(char));
    this->s[this->length] = '\0';
}

string &string::operator=(const string &str) {
    ASSERT_VECTOR_HAS_RET(str.s, str.length, s, length, *this);
    if (this->s != nullptr)
        delete s;
    this->length = str.length;
    this->s = new char[this->length + 1];
    memcpy(this->s, str.s, this->length * sizeof(char));
    this->s[this->length] = '\0';
    return *this;
}

string::string(string &&str) : s(str.s), length(str.length) {
    ASSERT_VECTOR_NO_RET(str.s, str.length, s, length);
    str.s = nullptr;
    str.length = 0;
    return;
}

string &string::operator=(string &&str) {
    ASSERT_VECTOR_HAS_RET(str.s, str.length, s, length, *this);
    if (this->s != nullptr)
        delete this->s;
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

array::array(size_t length) : length(length) {
    this->a = new jst_node[this->length];
    for (int i = 0; i < this->length; i++)
        this->a[i] = jst_node();
}

array::array(const array &arr) : a(arr.a), length(arr.length) {
    ASSERT_VECTOR_NO_RET(arr.a, arr.length, a, length);
    this->length = arr.length;
    this->a = new jst_node[this->length];
    for (int i = 0; i < this->length; i++)
        this->a[i] = arr.a[i];
}

array &array::operator=(const array &arr) {
    ASSERT_VECTOR_HAS_RET(arr.a, arr.length, a, length, *this);
    if (this->a != nullptr)
        delete this->a;
    this->length = arr.length;
    this->a = new jst_node[this->length];
    for (int i = 0; i < this->length; i++)
        this->a[i] = arr.a[i];
    return *this;
}

array::array(array &&arr) : a(arr.a), length(arr.length) {
    ASSERT_VECTOR_NO_RET(arr.a, arr.length, a, length);
    arr.a = nullptr;
    arr.length = 0;
}

array &array::operator=(array &&arr) {
    ASSERT_VECTOR_HAS_RET(arr.a, arr.length, a, length, *this);
    if (this->a != nullptr)
        delete this->a;
    this->a = arr.a;
    this->length = arr.length;
    arr.a = nullptr;
    arr.length = 0;
    return *this;
}

array::~array() {
    if (this->a != nullptr)
        delete[] this->a;
    this->length = 0;
}

jst_node &array::operator[](int index) {
    return this->a[index];
}

} // namespace jst