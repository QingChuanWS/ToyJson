#include "json_toy_node.h"
#include "json_toy_basic.h"
#include <assert.h>
#include <cctype>
#include <cmath>
#include <limits>
#include <cstring>
#include <stack>

namespace jst {

template <typename jnd_type>
jnd_type *jst_node::jst_node_data_p_get(jst_node_data *data) const {
    return dynamic_cast<jnd_type *>(data);
}

template <typename jnd_type>
jnd_type &jst_node::jst_node_data_mem_get(jst_node_data &data) const {
    return dynamic_cast<jnd_type &>(data);
}

shared_ptr<jst_node_data> jst_node::jst_node_data_copy(shared_ptr<jst_node_data> jnd) {
    if (type == JST_STR) {
        auto &node_data = jst_node_data_mem_get<string>(*jnd);
        return std::make_shared<string>(string(node_data));
    } else if (type == JST_NUM) {
        auto &node_data = jst_node_data_mem_get<number>(*jnd);
        return std::make_shared<number>(number(node_data));
    } else if (type == JST_ARR) {
        auto &node_data = jst_node_data_mem_get<array>(*jnd);
        return std::make_shared<array>(array(node_data));
    } else
        return nullptr;
}

jst_node::jst_node(jst_type t, const char *str, size_t len) : type(JST_NULL), data(nullptr) {
    assert(t == JST_NUM || t == JST_STR);
    type = t;
    if (type == JST_NUM) {
        if (len == 0)
            jst_node_parser_num(std::string(str));
        else
            jst_node_parser_num(std::string(str, len));
    } else if (type == JST_STR)
        jst_node_parser_str(str, len);
}

jst_node::jst_node(jst_type t, double num) : type(JST_NULL), data(nullptr) {
    assert(t == JST_NUM);
    type = t;
    this->data = std::make_shared<number>(number(num));
}

jst_node::jst_node(jst_type t, array &arr) : type(JST_ARR), data(nullptr) {
    assert(t == JST_ARR);
    this->data = std::make_shared<array>(array(arr));
}

// copy construct
jst_node::jst_node(const jst_node &node) : type(node.type) {
    if (node.data == nullptr) {
        this->data = nullptr;
        return;
    }
    this->data = jst_node_data_copy(node.data);
}

// assigment construct
jst_node &jst_node::operator=(const jst_node &node) {
    type = node.type;
    this->data = jst_node_data_copy(node.data);
    return *this;
}

// move copy construct
jst_node::jst_node(jst_node &&node) : type(node.type), data(std::move(node.data)) {
    node.data = nullptr;
    node.type = JST_NULL;
}

// move assigment construct
jst_node &jst_node::operator=(jst_node &&node) {
    this->type = node.type;
    this->data = std::move(node.data);
    node.data = nullptr;
    node.type = JST_NULL;

    return *this;
}

// deconstruct
jst_node::~jst_node() {
    type = JST_NULL;
    this->data = nullptr;
    ;
}

// reset the node type as JST_NULL
void jst_node::jst_node_type_reset() {
    this->data = nullptr;
    type = JST_NULL;
}

// get the data of number node;
void jst_node::jst_node_data_get(double &num) const {
    assert(type == JST_NUM);
    auto &this_data = jst_node_data_mem_get<number>(*(this->data));
    num = this_data.num;
}

// get the data of string node;
void jst_node::jst_node_data_get(std::string &str) const {
    assert(type == JST_STR);
    auto &this_data = jst_node_data_mem_get<string>(*(this->data));
    char *str_head = nullptr;
    size_t len = 0;
    this_data.get_c_str(&str_head, len);
    str = std::string(str_head);
}

void jst_node::jst_node_data_get(char **node_str, size_t &len) const {
    assert(type == JST_STR);
    auto &this_data = jst_node_data_mem_get<string>(*(this->data));
    this_data.get_c_str(node_str, len);
}

void jst_node::jst_node_data_get(bool &b) const {
    assert(type == JST_FALSE || type == JST_TRUE);
    b = type == JST_TRUE ? true : false;
}

void jst_node::jst_node_data_get(array &arr) const {
    assert(type == JST_ARR);
    auto &this_data = jst_node_data_mem_get<array>(*(this->data));
    arr = this_data;
}

size_t jst_node::jst_node_data_length_get() const {
    assert(type == JST_STR);
    auto &this_data = jst_node_data_mem_get<string>(*(this->data));
    return this_data.size();
}

jst_ret_type jst_node::jst_node_parser_num(const std::string &str) {
    ASSERT_JST_CONDATION(str[0] == '0' && str.size() > 1, (*this), JST_PARSE_SINGULAR);

    jst_ret_type ret = JST_PARSE_OK;
    jst_num_exp exp_state;
    jst_num_point point_state;

    int index = 0;
    if (str[0] == '-')
        index += 1;

    while (index < str.size()) {
        if (str[index] == ' ')
            break;
        if (std::isdigit(str[index])) {
            ASSERT_JST_CONDATION(exp_state.is_have && point_state.is_have && exp_state.exp_index < point_state.point_index,
                                 (*this), JST_PARSE_INVALID_VALUE);
        } else if (str[index] == 'E' || str[index] == 'e') {
            ASSERT_JST_CONDATION(exp_state.is_have || index + 1 >= str.size(), (*this), JST_PARSE_INVALID_VALUE);
            exp_state.is_have = true;
            if (str[index + 1] == '+' || str[index + 1] == '-') {
                exp_state.exp_index = index + 2;
                index++;
            } else if (std::isdigit(str[index + 1]))
                exp_state.exp_index = index + 1;
            else {
                ret = JST_PARSE_INVALID_VALUE;
                break;
            }
        } else if (str[index] == '.') {
            ASSERT_JST_CONDATION(point_state.is_have || index + 1 >= str.size(), (*this), JST_PARSE_INVALID_VALUE);
            point_state.is_have = true;
            point_state.point_index = index;
        } else {
            ret = JST_PARSE_INVALID_VALUE;
            break;
        }
        index++;
    }

    ASSERT_JST_CONDATION(ret != JST_PARSE_OK, (*this), ret);
    std::string n_str = str.substr(0, index);
    double n = std::strtod(n_str.c_str(), NULL);
    ASSERT_JST_CONDATION(n == HUGE_VAL || n == -HUGE_VAL, (*this), JST_PARSE_NUMBER_TOO_BIG)
    this->data = std::make_shared<number>(number(n));

    return ret;
}

jst_ret_type jst_node::jst_node_parser_str(const char *str, size_t len) {
    this->data = std::make_shared<string>(string(str, len));
    return JST_PARSE_OK;
}

jst_ret_type jst_node::jst_node_data_set(jst_type t, const char *str, size_t len) {
    assert(t != JST_ARR);
    jst_ret_type ret = JST_PARSE_OK;
    type = t;
    if (t == JST_NULL || t == JST_TRUE || t == JST_FALSE)
        ret = JST_PARSE_OK;
    else if (type == JST_NUM)
        ret = jst_node_parser_num(std::string(str, len));
    else if (type == JST_STR)
        ret = jst_node_parser_str(str, len);
    if (ret != JST_PARSE_OK)
        this->data = nullptr;
    return ret;
}

jst_ret_type jst_node::jst_node_data_set(jst_type t, array &&arr) {
    assert(t == JST_ARR);
    type = t;
    this->data = std::make_shared<array>(array(arr));
    return JST_PARSE_OK;
}

} // namespace jst