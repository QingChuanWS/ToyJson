#ifndef __JSON_TOY_NODE_H__
#define __JSON_TOY_NODE_H__

#include <memory>

#include "basic.h"
#include "enum.h"

namespace jst {
using std::shared_ptr;

class JNode {
 public:
  JNode() : type(JST_NULL), data(nullptr){};
  JNode(JType t, const char* str = nullptr, size_t len = 0);
  JNode(JType t, double num);
  JNode(JType t, Array& arr);
  JNode(JType t, Object& obj);

  void jst_node_type_reset();
  void jst_node_data_reset(const bool data);
  void jst_node_data_reset(const String& data);
  void jst_node_data_reset(const double data);
  void jst_node_data_reset(const Array& data);
  void jst_node_data_reset(const Object& data);
  JType jst_node_type_get() const { return type; }

  JNode(const JNode& node);
  JNode(JNode&& node) noexcept;
  JNode& operator=(const JNode& node);
  JNode& operator=(JNode&& node) noexcept;
  ~JNode();

  friend bool operator==(const JNode& jn_1, const JNode& jn_2);
  friend bool operator!=(const JNode& jn_1, const JNode& jn_2);
  friend void swap(JNode& jn_1, JNode& jn_2);

  JRType jst_node_data_set(JType t, const char* str = nullptr, const size_t len = 0);
  JRType jst_node_data_set(JType t, double num);
  JRType jst_node_data_set(JType t, String&& s);
  JRType jst_node_data_set(JType t, Array&& arr);
  JRType jst_node_data_set(JType t, Object&& obj);

  void jst_node_data_get(std::string& node_str) const;
  void jst_node_data_get(char** node_str, size_t& len) const;
  void jst_node_data_get(double& node_num) const;
  void jst_node_data_get(bool& node_bool) const;
  void jst_node_data_get(Array& arr) const;
  void jst_node_data_get(Object& obj) const;

  size_t jst_node_data_length_get() const;

 private:
  JRType jst_node_parser_num(const std::string& str);
  JRType jst_node_parser_str(const char* str, size_t len);

  shared_ptr<JNodeData> data;
  JType type;
};

}  // namespace jst

#endif  //__JSON_TOY_NODE_H__