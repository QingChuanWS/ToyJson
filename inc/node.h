#ifndef __JSON_TOY_NODE_H__
#define __JSON_TOY_NODE_H__

#include <memory>

#include "basic.h"
#include "enum.h"

namespace jst {
using std::shared_ptr;

class JNode {
 public:
  JNode() = default;
  JNode(JNType t, const char* str = nullptr, size_t len = 0);
  JNode(JNType t, double num);
  JNode(JNType t, JArray& arr);
  JNode(JNType t, JObject& obj);

  void jst_node_data_reset(const bool data);
  void jst_node_data_reset(const JString& data);
  void jst_node_data_reset(const double data);
  void jst_node_data_reset(const JArray& data);
  void jst_node_data_reset(const JObject& data);
  JNType type() const { return ty; }

  JNode(const JNode& node);
  JNode(JNode&& node) noexcept;
  JNode& operator=(const JNode& node);
  JNode& operator=(JNode&& node) noexcept;
  ~JNode();

  friend bool operator==(const JNode& jn_1, const JNode& jn_2);
  friend bool operator!=(const JNode& jn_1, const JNode& jn_2);
  friend void swap(JNode& jn_1, JNode& jn_2);

  JRetType jst_node_data_set(JNType t, const char* str = nullptr, const size_t len = 0);
  JRetType jst_node_data_set(JNType t, double num);
  JRetType jst_node_data_set(JNType t, JString&& s);
  JRetType jst_node_data_set(JNType t, JArray&& arr);
  JRetType jst_node_data_set(JNType t, JObject&& obj);

  void jst_node_data_get(std::string& node_str) const;
  void jst_node_data_get(const char** node_str, size_t& len) const;
  void jst_node_data_get(double& node_num) const;
  void jst_node_data_get(bool& node_bool) const;
  void jst_node_data_get(JArray& arr) const;
  void jst_node_data_get(JObject& obj) const;

  size_t jst_node_data_length_get() const;

 private:
  JRetType jst_node_parser_num(const std::string& str);
  JRetType jst_node_parser_str(const char* str, size_t len);

  shared_ptr<JData> data = nullptr;
  JNType ty = JST_NULL;
};

}  // namespace jst

#endif  //__JSON_TOY_NODE_H__