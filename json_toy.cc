#include "json_toy.h"
#include <string>
#include <assert.h>
#include <cctype>
#include <cmath>
#include <limits>
#include <cctype>
#include <cstring>
#include <stack>
#include <memory>

namespace jst {

#define ASSERT_JST_RET(expect, func, node)                                                                                  \
    do {                                                                                                                    \
        jst_ret_type ret = func;                                                                                            \
        if (ret != expect) {                                                                                                \
            node.jst_node_type_reset();                                                                                     \
            return ret;                                                                                                     \
        }                                                                                                                   \
    } while (0);

jst_context::jst_context(const jst_context &context) : str(context.str), str_index(context.str_index), root(context.root) {
    if (context.stack == nullptr) {
        stack = nullptr, size = 0, top = 0;
        return;
    }
    stack = new char[context.size];
    size = context.size;
    top = context.top;
    std::memcpy(stack, context.stack, size);
}

jst_context &jst_context::operator=(const jst_context &context) {
    this->str = context.str;
    this->root = context.root;
    this->str_index = context.str_index;

    if (this->stack != nullptr)
        delete this->stack;
    this->size = context.size;
    this->stack = new char[context.size];
    this->top = context.top;
    std::memcpy(stack, context.stack, size);
    return *this;
}

jst_context::jst_context(jst_context &&context) : str(std::move(context.str)), root(std::move(context.root)) {
    this->stack = context.stack;
    this->size = context.size;
    this->top = context.top;
    this->str_index = context.str_index;

    context.stack = nullptr;
    context.size = 0;
    context.top = 0;
    context.str_index = 0;
}

jst_context &jst_context::operator=(jst_context &&context) {
    str = std::move(context.str);
    root = std::move(context.root);
    if (this->stack != nullptr)
        delete this->stack;

    this->stack = context.stack;
    this->size = context.size;
    this->top = context.top;
    this->str_index = context.str_index;

    context.stack = nullptr;
    context.size = 0;
    context.top = 0;
    context.str_index = 0;
    return *this;
}

jst_context::~jst_context() {
    assert(top == 0);
    if (stack != nullptr)
        delete[] stack;
    size = 0;
    top = 0;
}

void *jst_context::stack_push(size_t p_size) {
    assert(p_size > 0);
    if (this->top + p_size > this->size) {
        if (size == 0)
            this->size = init_stack_size;
        while (top + p_size >= size)
            this->size += this->size >> 1;
        this->stack = (char *)realloc(this->stack, this->size);
    }
    void *ret = this->stack + this->top;
    this->top += p_size;
    return ret;
}

void *jst_context::stack_pop(size_t p_size) {
    assert(this->top >= p_size);
    top -= p_size;
    return this->stack + top;
}

jst_ret_type jst_context::jst_parser() {
    return jst_val_parser(root);
}

jst_ret_type jst_context::jst_ws_parser(jst_ws_state state, jst_type t) {
    int ws_count = 0;
    int index = this->str_index;
    for (int i = index; i < this->str.size(); i++) {
        if (this->str[i] == ' ' || this->str[i] == '\n' || this->str[i] == '\t' || this->str[i] == '\r') {
            ws_count++;
            continue;
        }
        break;
    }
    this->str_index += ws_count;

    if (state == JST_WS_BEFORE && this->str_index == this->str.size()) {
        return JST_PARSE_EXCEPT_VALUE;
    } else if (state == JST_WS_AFTER && this->str_index != this->str.size()) {
        if (this->str[this->str_index] != ',' && this->str[this->str_index] != ']')
            return JST_PARSE_SINGULAR;
    }
    return JST_PARSE_OK;
}

jst_ret_type jst_context::jst_val_parser_symbol(jst_node &node) {
    jst_type t;
    int index = this->str_index;
    if (str[index] == 't') {
        ASSERT_JST_CONDATION(str.size() < 4 || str[index + 1] != 'r' || str[index + 2] != 'u' || str[index + 3] != 'e', node,
                             JST_PARSE_INVALID_VALUE);
        t = JST_TRUE;
        this->str_index += 4;
    } else if (str[index] == 'f') {
        ASSERT_JST_CONDATION(str.size() < 5 || str[index + 1] != 'a' || str[index + 2] != 'l' || str[index + 3] != 's'
                                 || str[index + 4] != 'e',
                             node, JST_PARSE_INVALID_VALUE);
        t = JST_FALSE;
        this->str_index += 5;
    } else if (str[index] == 'n') {
        ASSERT_JST_CONDATION(str.size() < 4 || str[index + 1] != 'u' || str[index + 2] != 'l' || str[index + 3] != 'l', node,
                             JST_PARSE_INVALID_VALUE);
        t = JST_NULL;
        this->str_index += 4;
    }
    ASSERT_JST_RET(JST_PARSE_OK, jst_ws_parser(JST_WS_AFTER), node);
    node.jst_node_data_set(t);

    return JST_PARSE_OK;
}

jst_ret_type jst_context::jst_val_parser_number(jst_node &node) {
    int num_count = 0;
    for (int i = this->str_index; i < str.size(); i++) {
        if (std::isdigit(str[i]) || str[i] == '.' || str[i] == 'e' || str[i] == 'E' || str[i] == '+' || str[i] == '-') {
            num_count++;
            continue;
        }
        break;
    }
    ASSERT_JST_RET(JST_PARSE_OK, node.jst_node_data_set(JST_NUM, str.substr(this->str_index, num_count).c_str(), num_count),
                   node);

    this->str_index += num_count;
    ASSERT_JST_RET(JST_PARSE_OK, jst_ws_parser(JST_WS_AFTER), node);

    return JST_PARSE_OK;
}

jst_ret_type jst_context::jst_val_parser_str_utf(unsigned hex, std::vector<char> &sp_vec) {
    if (hex >= 0x0000 && hex <= 0x007F) {
        sp_vec.push_back(hex & 0x00FF);
    } else if (hex >= 0x0080 && hex <= 0x07FF) {
        sp_vec.push_back(0xC0 | ((hex >> 6) & 0xFF));
        sp_vec.push_back(0x80 | (hex & 0x3F));
    } else if (hex >= 0x0800 && hex <= 0xFFFF) {
        sp_vec.push_back(0xE0 | ((hex >> 12) & 0xFF)); /* 0xE0 = 11100000 */
        sp_vec.push_back(0x80 | ((hex >> 6) & 0x3F));  /* 0x80 = 10000000 */
        sp_vec.push_back(0x80 | (hex & 0x3F));         /* 0x3F = 00111111 */
    } else if (hex >= 0x10000 && hex <= 0x10FFFF) {
        sp_vec.push_back(0xF0 | ((hex >> 18) & 0xFF));
        sp_vec.push_back(0x80 | ((hex >> 12) & 0x3F));
        sp_vec.push_back(0x80 | ((hex >> 6) & 0x3F));
        sp_vec.push_back(0x80 | (hex & 0x3F));
    } else
        return JST_PARSE_INVALID_UNICODE_SURROGATE;
    return JST_PARSE_OK;
}

#define CHAR_VECTOR_PUSH(vector, c, ret_type)                                                                               \
    vector.push_back(c);                                                                                                    \
    ret = ret_type;                                                                                                         \
    break

inline jst_ret_type jst_context::jst_val_parser_str_sp(int &index, std::vector<char> &sp_char) {
    jst_ret_type ret = JST_PARSE_OK;
    switch (this->str[index]) {
    case 'n': CHAR_VECTOR_PUSH(sp_char, '\n', JST_PARSE_OK);
    case '\\': CHAR_VECTOR_PUSH(sp_char, '\\', JST_PARSE_OK);
    case 'b': CHAR_VECTOR_PUSH(sp_char, '\b', JST_PARSE_OK);
    case 'f': CHAR_VECTOR_PUSH(sp_char, '\f', JST_PARSE_OK);
    case 'r': CHAR_VECTOR_PUSH(sp_char, '\r', JST_PARSE_OK);
    case 't': CHAR_VECTOR_PUSH(sp_char, '\t', JST_PARSE_OK);
    case '\"': CHAR_VECTOR_PUSH(sp_char, '\"', JST_PARSE_OK);
    case 'u': {
        if (index + 4 > this->str.size() || !std::isxdigit(this->str[index + 1]) || !std::isxdigit(this->str[index + 2])
            || !std::isxdigit(this->str[index + 3]) || !std::isxdigit(this->str[index + 4])) {
            ret = JST_PARSE_INVALID_UNICODE_HEX;
            break;
        }
        unsigned hex = std::strtol(this->str.substr(index + 1, 4).c_str(), NULL, 16);
        index += 4;
        if (hex >= 0xD800 && hex <= 0xDBFF) {
            if (index + 6 > this->str.size() || this->str[index + 1] != '\\' || this->str[index + 2] != 'u'
                || !std::isxdigit(this->str[index + 3]) || !std::isxdigit(this->str[index + 4])
                || !std::isxdigit(this->str[index + 5]) || !std::isxdigit(this->str[index + 6])) {
                ret = JST_PARSE_INVALID_UNICODE_SURROGATE;
                break;
            }
            unsigned low_hex = std::strtol(this->str.substr(index + 3, 4).c_str(), NULL, 16);
            if (low_hex < 0xDC00 || low_hex > 0xDFFF) {
                ret = JST_PARSE_INVALID_UNICODE_SURROGATE;
                break;
            }
            hex = 0x10000 + (hex - 0xD800) * 0x400 + (low_hex - 0xDC00);
            index += 6;
        }
        if (JST_PARSE_OK != jst_val_parser_str_utf(hex, sp_char)) {
            ret = JST_PARSE_INVALID_UNICODE_SURROGATE;
            break;
        }
        break;
    }
    case '/': CHAR_VECTOR_PUSH(sp_char, '/', JST_PARSE_OK);
    default: ret = JST_PARSE_INVALID_STRING_ESCAPE; break;
    }
    return ret;
}

jst_ret_type jst_context::jst_val_parser_string(jst_node &node) {
    assert(this->str[this->str_index] == '\"');
    jst_ret_type ret = JST_PARSE_OK;

    int index = this->str_index + 1;
    size_t head = this->top;

    const char *cstr = this->str.c_str();
    int cstr_length = this->str.size() + 1;
    while (index < cstr_length) {
        switch (cstr[index]) {
        case '\"': {
            size_t len = this->top - head;
            char *str_head = (char *)stack_pop(len);
            ret = node.jst_node_data_set(JST_STR, str_head, len);
            index++;
            goto RET;
        }
        case '\0':
            this->top = head;
            node.jst_node_type_reset();
            ret = JST_PARSE_MISS_QUOTATION_MARK;
            goto RET;
        case '\\': {
            ASSERT_JST_CONDATION(index + 1 >= cstr_length, node, JST_PARSE_INVALID_VALUE);
            std::vector<char> sp_char;
            index += 1;
            if (JST_PARSE_OK != (ret = jst_val_parser_str_sp(index, sp_char))) {
                this->top = head;
                node.jst_node_type_reset();
                goto RET;
            }
            for (int i = 0; i < sp_char.size(); i++)
                *(char *)this->stack_push(sizeof(char)) = sp_char[i];
            break;
        }
        default: {
            ASSERT_JST_CONDATION(cstr[index] < 32 || cstr[index] == '"', node, JST_PARSE_INVALID_STRING_CHAR);
            char &c = *(char *)this->stack_push(sizeof(char));
            c = cstr[index];
            break;
        }
        }
        index++;
    }
RET:
    if (ret == JST_PARSE_OK) {
        this->str_index = index;
        ASSERT_JST_RET(JST_PARSE_OK, jst_ws_parser(JST_WS_AFTER), node);
    }
    return ret;
}

jst_ret_type jst_context::jst_val_parser_array(jst_node &node) {
    assert(this->str[this->str_index++] == '[');
    ASSERT_JST_RET(JST_PARSE_OK, jst_ws_parser(JST_WS_BEFORE), node);

    jst_ret_type ret = JST_PARSE_OK;
    if (this->str[this->str_index] == ']') {
        node.jst_node_data_set(JST_ARR, array());
        this->str_index++;
        ASSERT_JST_RET(JST_PARSE_OK, jst_ws_parser(JST_WS_AFTER, JST_ARR), node);
        return ret;
    }

    size_t size = 0;
    size_t head = this->top;
    std::unique_ptr<jst_node> jn(new jst_node);
    while (this->str_index < this->str.size()) {
        if ((ret = jst_val_parser(*jn)) != JST_PARSE_OK) {
            if (ret == JST_PARSE_SINGULAR)
                ret = JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            jn->jst_node_type_reset();
            break;
        }
        auto stack_node = (jst_node *)this->stack_push(sizeof(jst_node));
        memset(stack_node, 0, sizeof(jst_node));
        *stack_node = *jn;
        size++;
        jn->jst_node_type_reset();

        if (this->str[this->str_index] == ',') {
            this->str_index++;
        } else if (this->str[this->str_index] == ']') {
            this->str_index++;
            std::unique_ptr<array> arr(new array(size));
            jst_node *arr_head = (jst_node *)this->stack_pop(size * sizeof(jst_node));
            for (int i = 0; i < size; i++)
                (*arr)[i] = std::move(arr_head[i]);
            node.jst_node_data_set(JST_ARR, std::move(*arr));
            break;
        } else {
            ret = JST_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
    if (ret == JST_PARSE_OK) {
        ASSERT_JST_RET(JST_PARSE_OK, jst_ws_parser(JST_WS_AFTER, JST_ARR), node);
    } else {
        if (size != 0) {
            jst_node *arr_head = (jst_node *)this->stack_pop(size * sizeof(jst_node));
            for (int i = 0; i < size; i++)
                arr_head[i].jst_node_type_reset();
        }
        this->top = head;
        node.jst_node_type_reset();
    }
    return ret;
}

jst_ret_type jst_context::jst_val_parser(jst_node &node) {
    ASSERT_JST_RET(JST_PARSE_OK, jst_ws_parser(JST_WS_BEFORE), root);
    switch (str[this->str_index]) {
    case 'n': return jst_val_parser_symbol(node);
    case 't': return jst_val_parser_symbol(node);
    case 'f': return jst_val_parser_symbol(node);
    case '\"': return jst_val_parser_string(node);
    case '[': return jst_val_parser_array(node);
    case '{': return JST_PARSE_OK;
    case '0' ... '9': return jst_val_parser_number(node);
    case '+': return jst_val_parser_number(node);
    case '-': return jst_val_parser_number(node);
    default: return JST_PARSE_INVALID_VALUE;
    }
    return JST_PARSE_OK;
}

} // namespace jst