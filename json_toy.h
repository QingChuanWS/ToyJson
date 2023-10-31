#ifndef __JSON_TOY_H__
#define __JSON_TOY_H__

#include <iostream>
#include <string>
#include <vector>

#include "json_toy_basic.h"
#include "json_toy_enum.h"
#include "json_toy_node.h"

namespace jst {
class JNodeContext {
 public:
  JNodeContext() : str(""), root(), top(0), stack(nullptr), size(0), str_index(0) {}
  JNodeContext(const std::string& j_str)
      : str(j_str), root(), top(0), stack(nullptr), size(0), str_index(0) {}

  JNodeContext(const JNodeContext& context);
  JNodeContext& operator=(const JNodeContext& context);
  JNodeContext(JNodeContext&& context);
  JNodeContext& operator=(JNodeContext&& context);
  ~JNodeContext();

  void reset(const std::string& j_str);

  JRType jst_parser(JNode* node = nullptr);
  JRType jst_stringify(const JNode& jn, char** json_str, size_t& len);

  JNode root;

 private:
  JRType jst_val_parser(JNode& node, bool is_local = false);

  JRType jst_val_parser_symbol(JNode& node);
  JRType jst_val_parser_number(JNode& node);

  // parser spefical char
  JRType jst_val_parser_str_sp(int& char_index, std::vector<char>& sp_char);
  // parser utf code
  JRType jst_val_parser_str_utf(unsigned hex, std::vector<char>& sp_vec);
  JRType jst_val_parser_string_base(String& s);
  JRType jst_val_parser_string(JNode& node);

  JRType jst_val_parser_array(JNode& node);
  JRType jst_val_parser_object_member(OjectMember& objm);
  JRType jst_val_parser_object(JNode& node);

  JRType jst_ws_parser(jst_ws_state state, JType t = JST_NULL);
  JRType jst_stringify_value(const JNode& jn);
  void jst_stringify_string(const JNode& jn);

  void* stack_push(size_t size);
  void* stack_pop(size_t size);

  std::string str;
  int str_index;
  char* stack;
  size_t top, size;

  const size_t init_stack_size = 256;
};

}  // namespace jst

#endif  // !__JSON_TOY_H__
