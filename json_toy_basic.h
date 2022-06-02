#ifndef __JSON_TOY_BASIC_H__
#define __JSON_TOY_BASIC_H__

#include <string>
#include <memory>
#include <assert.h>

namespace jst {

#define ASSERT_VECTOR_NO_RET(vec, head, length)                                                                             \
    if (vec.is_empty()) {                                                                                                   \
        this->head = nullptr;                                                                                               \
        this->length = 0;                                                                                                   \
        return;                                                                                                             \
    }

#define ASSERT_VECTOR_HAS_RET(vec, head, length, ret)                                                                       \
    if (vec.is_empty()) {                                                                                                   \
        this->head = nullptr;                                                                                               \
        this->length = 0;                                                                                                   \
        return ret;                                                                                                         \
    }

class jst_node;
using std::shared_ptr;

struct jst_num_exp {
    bool is_have;
    int exp_index;
    jst_num_exp() : is_have(false), exp_index(0) {
    }
};

struct jst_num_point {
    bool is_have;
    int point_index;
    jst_num_point() : is_have(false), point_index(0) {
    }
};

class jst_node_data {
public:
    jst_node_data() {
    }
    virtual ~jst_node_data(){};
};

class string : public jst_node_data {
public:
    string() : s(nullptr), length(0) {
    }
    string(const char *s, size_t len = 0);

    string(const string &s);
    string &operator=(const string &s);
    string(string &&s) noexcept;
    string &operator=(string &&s) noexcept;
    ~string();

    size_t size() const {
        return length;
    };
    void get_c_str(char **str, size_t &len) const {
        *str = s;
        len = length;
    }
    bool is_empty() const {
        return s == nullptr || length == 0;
    }

private:
    size_t length;
    char *s;
};

class number : public jst_node_data {
public:
    number() : num(0.0) {
    }
    explicit number(double n) : num(n){};
    double num;
};

class array : public jst_node_data {
public:
    array() : a(nullptr), length(0) {
    }
    array(size_t length);
    array(const array &arr);
    array &operator=(const array &arr);
    array(array &&arr) noexcept;
    array &operator=(array &&arr) noexcept;
    jst_node &operator[](int index);
    ~array();

    size_t size() const {
        return length;
    };
    bool is_empty() const {
        return a == nullptr || length == 0;
    }
    const jst_node *get_array_head() const {
        return this->a;
    };
    void set_array(jst_node *arr, int length);
    void push(jst_node &node);

private:
    jst_node *a;
    size_t length;
};

// object
class object_member {
public:
    object_member() : key(), value() {
    }
    object_member(string &key, shared_ptr<jst_node> value) : key(key), value(value) {
    }
    object_member(string &&key, shared_ptr<jst_node> &&value) noexcept : key(std::move(key)), value(std::move(value)) {
    }
    object_member(const object_member &om) : key(om.key), value(om.value) {
    }
    object_member &operator=(const object_member &om) {
        key = om.key;
        value = om.value;
        return *this;
    };
    object_member(object_member &&om) noexcept : key(std::move(om.key)), value(std::move(om.value)) {
    }
    object_member &operator=(const object_member &&om) noexcept {
        key = std::move(om.key);
        value = std::move(om.value);
        return *this;
    }
    ~object_member() {
        value = nullptr;
    }

    void set_key(const string &s) {
        key = s;
    }
    void set_key(string &&s) {
        key = std::move(s);
    }
    void set_value(const jst_node &v) {
        value = std::make_unique<jst_node>(v);
    }
    void set_value(jst_node &&v) {
        value = std::make_unique<jst_node>(v);
    }

    const string &get_key() const {
        return key;
    }
    const jst_node &get_value() const {
        return *value;
    }

private:
    string key;
    shared_ptr<jst_node> value;
};

class object : public jst_node_data {
public:
    object() : obj_m(nullptr), length(0) {
    }
    object(size_t length);

    object(const object &o);
    object &operator=(const object &o);
    object(object &&o) noexcept;
    object &operator=(object &&o) noexcept;
    ~object();

    object_member &operator[](int index);

    bool is_empty() const {
        return obj_m == nullptr && length == 0;
    }

    size_t get_object_size() const {
        return length;
    };
    const string &get_object_key(size_t index) const {
        assert(index >= 0 && index < this->length);
        return obj_m[index].get_key();
    };
    const jst_node &get_object_value(size_t index) const {
        assert(index >= 0 && index < this->length);
        return obj_m[index].get_value();
    };

private:
    object_member *obj_m;
    size_t length;
};

} // namespace jst

#endif //__JSON_TOY_BASIC_H__