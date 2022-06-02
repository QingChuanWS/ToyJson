#ifndef __JSON_TOY_H__
#define __JSON_TOY_H__

#include "json_toy_enum.h"
#include "json_toy_node.h"
#include "json_toy_basic.h"

#include <string>
#include <vector>
#include <iostream>

namespace jst {

class jst_context {
public:
    jst_context() : str(""), root(), top(0), stack(nullptr), size(0), str_index(0) {
    }
    jst_context(const std::string &j_str) : str(j_str), str_index(0), root(), top(0), stack(nullptr), size(0) {
    }
    jst_context(const jst_context &context);
    jst_context &operator=(const jst_context &context);
    jst_context(jst_context &&context);
    jst_context &operator=(jst_context &&context);
    ~jst_context();

    jst_ret_type jst_parser();

    jst_node root;

private:
    jst_ret_type jst_val_parser(jst_node &node, bool is_local = false);

    jst_ret_type jst_val_parser_symbol(jst_node &node);
    jst_ret_type jst_val_parser_number(jst_node &node);
    jst_ret_type jst_val_parser_string_base(string &s);
    jst_ret_type jst_val_parser_string(jst_node &node);
    jst_ret_type jst_val_parser_array(jst_node &node);
    jst_ret_type jst_val_parser_object_member(object_member & objm);
    jst_ret_type jst_val_parser_object(jst_node &node);
    jst_ret_type jst_ws_parser(jst_ws_state state, jst_type t = JST_NULL);
    // parser spefical char
    inline jst_ret_type jst_val_parser_str_sp(int &char_index, std::vector<char> &sp_char);
    // parser utf code
    jst_ret_type jst_val_parser_str_utf(unsigned hex, std::vector<char> &sp_vec);

    void *stack_push(size_t size);
    void *stack_pop(size_t size);

    std::string str;
    int str_index;
    char *stack;
    size_t top, size;

    const size_t init_stack_size = 256;
};

} // namespace jst

#endif // !__JSON_TOY_H__
