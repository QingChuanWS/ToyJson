#include "json_toy_node.h"
#include "json_toy_basic.h"
#include <assert.h>
#include <cctype>
#include <cmath>
#include <limits>
#include <cstring>
#include <stack>

namespace jst
{
template <typename jnd_type>
static jnd_type &jst_node_data_mem_get(jst_node_data &data)
{
    return dynamic_cast<jnd_type &>(data);
}

static shared_ptr<jst_node_data> jst_node_data_copy(jst_type type, shared_ptr<jst_node_data> jnd)
{
    switch (type) {
    case JST_STR: return std::make_shared<string>(string(jst_node_data_mem_get<string>(*jnd)));
    case JST_NUM: return std::make_shared<number>(number(jst_node_data_mem_get<number>(*jnd)));
    case JST_ARR: return std::make_shared<array>(array(jst_node_data_mem_get<array>(*jnd)));
    case JST_OBJ: return std::make_shared<object>(object(jst_node_data_mem_get<object>(*jnd)));
    default: return nullptr;
    }
}

jst_node::jst_node(jst_type t, const char *str, size_t len) :
    type(JST_NULL), data(nullptr)
{
    JST_DEBUG(t == JST_NUM || t == JST_STR);
    type = t;
    if (type == JST_NUM) {
        if (len == 0)
            jst_node_parser_num(std::string(str));
        else
            jst_node_parser_num(std::string(str, len));
    } else if (type == JST_STR)
        jst_node_parser_str(str, len);
}

jst_node::jst_node(jst_type t, double num) :
    type(JST_NULL), data(nullptr)
{
    JST_DEBUG(t == JST_NUM);
    type = t;
    this->data = std::make_shared<number>(number(num));
}

jst_node::jst_node(jst_type t, array &arr) :
    type(JST_ARR), data(nullptr)
{
    JST_DEBUG(t == JST_ARR);
    this->data = std::make_shared<array>(array(arr));
}

jst_node::jst_node(jst_type t, object &obj) :
    type(JST_OBJ), data(nullptr)
{
    JST_DEBUG(t == JST_ARR);
    this->data = std::make_shared<object>(object(obj));
}

// copy construct
jst_node::jst_node(const jst_node &node) :
    type(node.type)
{
    if (node.data == nullptr) {
        this->data = nullptr;
        return;
    }
    this->data = jst_node_data_copy(node.type, node.data);
}

// assigment construct
jst_node &jst_node::operator=(const jst_node &node)
{
    type = node.type;
    this->data = jst_node_data_copy(node.type, node.data);
    return *this;
}

// move copy construct
jst_node::jst_node(jst_node &&node) noexcept :
    type(node.type), data(std::move(node.data))
{
    node.data = nullptr;
    node.type = JST_NULL;
}

// move assigment construct
jst_node &jst_node::operator=(jst_node &&node) noexcept
{
    this->type = node.type;
    this->data = std::move(node.data);
    node.data = nullptr;
    node.type = JST_NULL;

    return *this;
}

// deconstruct
jst_node::~jst_node()
{
    type = JST_NULL;
    this->data = nullptr;
}

// reset the node type as JST_NULL
void jst_node::jst_node_type_reset()
{
    this->data = nullptr;
    type = JST_NULL;
}

void jst_node::jst_node_data_reset(const bool data)
{
    this->type = data ? JST_TRUE : JST_FALSE;
    this->data = nullptr;
}

void jst_node::jst_node_data_reset(const string &data)
{
    this->type = JST_STR;
    this->data = std::make_shared<string>(data);
}
void jst_node::jst_node_data_reset(const double data)
{
    this->type = JST_NUM;
    this->data = std::make_shared<number>(data);
}
void jst_node::jst_node_data_reset(const array &data)
{
    this->type = JST_ARR;
    this->data = std::make_shared<array>(data);
}
void jst_node::jst_node_data_reset(const object &data)
{
    this->type = JST_OBJ;
    this->data = std::make_shared<object>(data);
}

// get the data of number node;
void jst_node::jst_node_data_get(double &num) const
{
    JST_DEBUG(type == JST_NUM);
    auto &this_data = jst_node_data_mem_get<number>(*(this->data));
    num = this_data.num;
}

// get the data of string node;
void jst_node::jst_node_data_get(std::string &str) const
{
    JST_DEBUG(type == JST_STR);
    auto &this_data = jst_node_data_mem_get<string>(*(this->data));
    str = std::string(this_data.c_str(), this_data.size());
}

void jst_node::jst_node_data_get(char **node_str, size_t &len) const
{
    JST_DEBUG(type == JST_STR);
    auto &this_data = jst_node_data_mem_get<string>(*(this->data));
    *node_str = this_data.c_str();
    len = this_data.size();
}

void jst_node::jst_node_data_get(bool &b) const
{
    JST_DEBUG(type == JST_FALSE || type == JST_TRUE);
    b = type == JST_TRUE ? true : false;
}

void jst_node::jst_node_data_get(array &arr) const
{
    JST_DEBUG(type == JST_ARR);
    auto &this_data = jst_node_data_mem_get<array>(*(this->data));
    arr = this_data;
}

void jst_node::jst_node_data_get(object &obj) const
{
    JST_DEBUG(type == JST_OBJ);
    auto &this_data = jst_node_data_mem_get<object>(*(this->data));
    obj = this_data;
}

size_t jst_node::jst_node_data_length_get() const
{
    JST_DEBUG(type == JST_STR);
    auto &this_data = jst_node_data_mem_get<string>(*(this->data));
    return this_data.size();
}

jst_ret_type jst_node::jst_node_parser_num(const std::string &str)
{
    JST_CONDATION_STATE(str[0] == '0' && str.size() > 1, (*this), JST_PARSE_SINGULAR);

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
            JST_CONDATION_STATE(exp_state.is_have && point_state.is_have && exp_state.exp_index < point_state.point_index,
                                 (*this), JST_PARSE_INVALID_VALUE);
        } else if (str[index] == 'E' || str[index] == 'e') {
            JST_CONDATION_STATE(exp_state.is_have || index + 1 >= str.size(), (*this), JST_PARSE_INVALID_VALUE);
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
            JST_CONDATION_STATE(point_state.is_have || index + 1 >= str.size(), (*this), JST_PARSE_INVALID_VALUE);
            point_state.is_have = true;
            point_state.point_index = index;
        } else {
            ret = JST_PARSE_INVALID_VALUE;
            break;
        }
        index++;
    }

    JST_CONDATION_STATE(ret != JST_PARSE_OK, (*this), ret);
    std::string n_str = str.substr(0, index);
    double n = std::strtod(n_str.c_str(), NULL);
    JST_CONDATION_STATE(n == HUGE_VAL || n == -HUGE_VAL, (*this), JST_PARSE_NUMBER_TOO_BIG)
    this->data = std::make_shared<number>(number(n));

    return ret;
}

jst_ret_type jst_node::jst_node_parser_str(const char *str, size_t len)
{
    this->data = std::make_shared<string>(string(str, len));
    return JST_PARSE_OK;
}

jst_ret_type jst_node::jst_node_data_set(jst_type t, const char *str, const size_t len)
{
    JST_DEBUG(t != JST_ARR);
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

jst_ret_type jst_node::jst_node_data_set(jst_type t, string &&s)
{
    JST_DEBUG(t == JST_STR);
    jst_ret_type ret = JST_PARSE_OK;
    type = t;
    this->data = std::make_shared<string>(string(s));
    return ret;
}

jst_ret_type jst_node::jst_node_data_set(jst_type t, array &&arr)
{
    JST_DEBUG(t == JST_ARR);
    type = t;
    this->data = std::make_shared<array>(array(arr));
    return JST_PARSE_OK;
}

jst_ret_type jst_node::jst_node_data_set(jst_type t, object &&obj)
{
    JST_DEBUG(t == JST_OBJ);
    type = t;
    this->data = std::make_shared<object>(object(obj));
    return JST_PARSE_OK;
}

jst_ret_type jst_node::jst_node_data_set(jst_type t, double num)
{
    JST_DEBUG(t == JST_NUM);
    type = t;
    this->data = std::make_shared<number>(number(num));
    return JST_PARSE_OK;
}

bool operator==(const jst_node &jn_1, const jst_node &jn_2)
{
    if (jn_1.jst_node_type_get() != jn_2.jst_node_type_get())
        return false;
    switch (jn_1.jst_node_type_get()) {
    case JST_STR: return jst_node_data_mem_get<string>(*jn_1.data) == jst_node_data_mem_get<string>(*jn_2.data);
    case JST_NUM: return jst_node_data_mem_get<number>(*jn_1.data) == jst_node_data_mem_get<number>(*jn_2.data);
    case JST_ARR: return jst_node_data_mem_get<array>(*jn_1.data) == jst_node_data_mem_get<array>(*jn_2.data);
    case JST_OBJ: return jst_node_data_mem_get<object>(*jn_1.data) == jst_node_data_mem_get<object>(*jn_2.data);
    default: return true;
    }
}

bool operator!=(jst_node &jn_1, jst_node &jn_2)
{
    return !(jn_1 == jn_2);
}

void swap(jst_node &jn_1, jst_node &jn_2)
{
    jst_node tmp = std::move(jn_1);
    jn_1 = std::move(jn_2);
    jn_2 = std::move(tmp);
}

} // namespace jst