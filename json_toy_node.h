#ifndef __JSON_TOY_NODE_H__
#define __JSON_TOY_NODE_H__

#include <memory>

#include "json_toy_basic.h"
#include "json_toy_enum.h"

namespace jst {
using std::shared_ptr;

class jst_node {
 public:
  jst_node() : type(JST_NULL), data(nullptr){};
  jst_node(jst_type t, const char* str = nullptr, size_t len = 0);
  jst_node(jst_type t, double num);
  jst_node(jst_type t, array& arr);
  jst_node(jst_type t, object& obj);

  void jst_node_type_reset();
  void jst_node_data_reset(const bool data);
  void jst_node_data_reset(const string& data);
  void jst_node_data_reset(const double data);
  void jst_node_data_reset(const array& data);
  void jst_node_data_reset(const object& data);
  jst_type jst_node_type_get() const { return type; }

  jst_node(const jst_node& node);
  jst_node(jst_node&& node) noexcept;
  jst_node& operator=(const jst_node& node);
  jst_node& operator=(jst_node&& node) noexcept;
  ~jst_node();

  friend bool operator==(const jst_node& jn_1, const jst_node& jn_2);
  friend bool operator!=(const jst_node& jn_1, const jst_node& jn_2);
  friend void swap(jst_node& jn_1, jst_node& jn_2);

  jst_ret_type jst_node_data_set(jst_type t, const char* str = nullptr, const size_t len = 0);
  jst_ret_type jst_node_data_set(jst_type t, double num);
  jst_ret_type jst_node_data_set(jst_type t, string&& s);
  jst_ret_type jst_node_data_set(jst_type t, array&& arr);
  jst_ret_type jst_node_data_set(jst_type t, object&& obj);

  void jst_node_data_get(std::string& node_str) const;
  void jst_node_data_get(char** node_str, size_t& len) const;
  void jst_node_data_get(double& node_num) const;
  void jst_node_data_get(bool& node_bool) const;
  void jst_node_data_get(array& arr) const;
  void jst_node_data_get(object& obj) const;

  size_t jst_node_data_length_get() const;

 private:
  jst_ret_type jst_node_parser_num(const std::string& str);
  jst_ret_type jst_node_parser_str(const char* str, size_t len);

  shared_ptr<jst_node_data> data;
  jst_type type;
};

}  // namespace jst

#endif  //__JSON_TOY_NODE_H__