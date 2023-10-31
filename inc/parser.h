#ifndef __JSON_TOY_H__
#define __JSON_TOY_H__

#include <iostream>
#include <string>
#include <vector>

#include "basic.h"
#include "enum.h"
#include "node.h"

namespace jst {
class JParser {
 public:
  JParser() : str(""), root(), top(0), stack(nullptr), size(0), str_index(0) {}
  JParser(const std::string& j_str)
      : str(j_str), root(), top(0), stack(nullptr), size(0), str_index(0) {}

  JParser(const JParser& context);
  JParser& operator=(const JParser& context);
  JParser(JParser&& context);
  JParser& operator=(JParser&& context);
  ~JParser();

  void reset(const std::string& j_str);

  JRetType jst_parser(JNode* node = nullptr);
  JRetType jst_stringify(const JNode& jn, char** json_str, size_t& len);

  JNode root;

 private:
  JRetType jst_val_parser(JNode& node, bool is_local = false);

  JRetType jst_val_parser_symbol(JNode& node);
  JRetType jst_val_parser_number(JNode& node);

  // parser spefical char
  JRetType jst_val_parser_str_sp(int& char_index, std::vector<char>& sp_char);
  // parser utf code
  JRetType jst_val_parser_str_utf(unsigned hex, std::vector<char>& sp_vec);
  JRetType jst_val_parser_string_base(JString& s);
  JRetType jst_val_parser_string(JNode& node);

  JRetType jst_val_parser_array(JNode& node);
  JRetType jst_val_parser_object_member(JOjectMem& objm);
  JRetType jst_val_parser_object(JNode& node);

  JRetType jst_ws_parser(jst_ws_state state, JType t = JST_NULL);
  JRetType jst_stringify_value(const JNode& jn);
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
