#ifndef __JSON_TOY_NODE_H__
#define __JSON_TOY_NODE_H__

#include "json_toy_enum.h"
#include "json_toy_basic.h"
#include <memory>

namespace jst {
using std::shared_ptr;

class jst_node {
public:
    jst_node() : type(JST_NULL), data(){};
    jst_node(jst_type t, const char *str = nullptr, size_t len = 0);
    jst_node(jst_type t, double num);
    jst_node(jst_type t, array &arr);

    void jst_node_type_reset();
    jst_type jst_node_type_get() {
        return type;
    }

    jst_node(const jst_node &node);
    jst_node(jst_node &&node);
    jst_node &operator=(const jst_node &node);
    jst_node &operator=(jst_node &&node);
    ~jst_node();

    jst_ret_type jst_node_data_set(jst_type t, const char *str = nullptr, size_t len = 0);
    jst_ret_type jst_node_data_set(jst_type t, array &&arr);
    void jst_node_data_get(std::string &node_str) const;
    void jst_node_data_get(char **node_str, size_t &len) const;
    void jst_node_data_get(double &node_num) const;
    void jst_node_data_get(bool &node_bool) const;
    void jst_node_data_get(array &arr) const;
    size_t jst_node_data_length_get() const;

private:
    jst_ret_type jst_node_parser_num(const std::string &str);
    jst_ret_type jst_node_parser_str(const char *str, size_t len);

    shared_ptr<jst_node_data> jst_node_data_copy(shared_ptr<jst_node_data> jnd);

    template <typename jnd_type>
    jnd_type *jst_node_data_p_get(jst_node_data *data) const;

    template <typename jnd_type>
    jnd_type &jst_node_data_mem_get(jst_node_data &data) const;

    shared_ptr<jst_node_data> data;
    jst_type type;
};

} // namespace jst

#endif //__JSON_TOY_NODE_H__