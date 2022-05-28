#ifndef __JSON_TOY_BASIC_H__
#define __JSON_TOY_BASIC_H__

#include <string>

namespace jst {

#define ASSERT_VECTOR_NO_RET(vec_head, vec_len, head, length)                                                               \
    if (vec_head == nullptr && vec_len == 0) {                                                                              \
        this->head = nullptr;                                                                                               \
        this->length = 0;                                                                                                   \
        return;                                                                                                             \
    }

#define ASSERT_VECTOR_HAS_RET(vec_head, vec_len, head, length, ret)                                                         \
    if (vec_head == nullptr || vec_len == 0) {                                                                              \
        this->head = nullptr;                                                                                               \
        this->length = 0;                                                                                                   \
        return ret;                                                                                                         \
    }

class jst_node;

class jst_node_data {
public:
    jst_node_data() {
    }
    virtual ~jst_node_data(){};
};

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

class string : public jst_node_data {
public:
    string() : s(nullptr), length(0) {
    }
    string(const char *s, size_t len);

    string(const string &s);
    string &operator=(const string &s);
    string(string &&s) noexcept;
    string &operator=(string &&s) noexcept;
    ~string();

    size_t size() {
        return length;
    };
    void get_c_str(char **str, size_t &len) const {
        *str = s;
        len = length;
    }
    bool is_empty() const {
        return s == nullptr && length == 0;
    }
    size_t size() const {
        return length;
    }

private:
    size_t length;
    char *s;
};

class number : public jst_node_data {
public:
    number() : num(0.0) {
    }
    number(double n) : num(n){};
    ~number() {
        num = 0;
    }
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

    size_t size() {
        return length;
    };
    jst_node *get_array_head();
    void set_array(jst_node *arr, int length);
    void push(jst_node &node);

private:
    jst_node *a;
    size_t length;
};

// object

} // namespace jst

#endif //__JSON_TOY_BASIC_H__