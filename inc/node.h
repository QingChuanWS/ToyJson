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
  JNode(JNType t, const char* str, size_t len = 0);
  explicit JNode(JNType t) : _type(t), _data(nullptr) {}
  JNode(const JString& s) : _type(JST_STR), _data(std::make_shared<JString>(s)) {}
  JNode(double num) : _type(JST_NUM), _data(std::make_shared<JNumber>(num)) {}
  JNode(const JArray& arr) : _type(JST_ARR), _data(std::make_shared<JArray>(arr)) {}
  JNode(const JObject& obj) : _type(JST_OBJ), _data(std::make_shared<JObject>(obj)) {}

  JNode(const JNode& node);
  JNode(JNode&& node) noexcept;
  JNode& operator=(const JNode& node);
  JNode& operator=(JNode&& node) noexcept;
  ~JNode();

  friend bool operator==(const JNode& jn_1, const JNode& jn_2);
  friend bool operator!=(const JNode& jn_1, const JNode& jn_2);
  friend void swap(JNode& jn_1, JNode& jn_2);

  JRetType data_set(JNType t, const char* str = nullptr, const size_t len = 0);

  void jst_node_data_get(std::string& node_str) const;
  void jst_node_data_get(const char** node_str, size_t& len) const;
  void jst_node_data_get(double& node_num) const;
  void jst_node_data_get(bool& node_bool) const;
  void jst_node_data_get(JArray& arr) const;
  void jst_node_data_get(JObject& obj) const;

  JNType type() const { return _type; }
  JData data() const { return *_data; }

 private:
  JRetType jst_node_parser_num(const std::string& str);

  shared_ptr<JData> _data = nullptr;
  JNType _type = JST_NULL;
};

}  // namespace jst

#endif  //__JSON_TOY_NODE_H__