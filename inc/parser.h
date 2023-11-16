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
  JParser(const std::string& j_str)
      : str(j_str), root(), top(0), stack(nullptr), size(0), str_index(0) {}

  JParser(const JParser& context);
  JParser& operator=(const JParser& context);
  JParser(JParser&& context);
  JParser& operator=(JParser&& context);
  ~JParser();

  void reset(const std::string& j_str);

  JRetType parser(JNode* node = nullptr);
  JRetType stringify(const JNode& jn, char** json_str, size_t& len);

  JNode root;

 private:
  JRetType main_parser(JNode& node, bool is_local = false);

  JRetType parser_symbol(JNode& node);
  JRetType parser_number(JNode& node);

  // parser spefical char
  JRetType parser_specifical_str(int& char_index, std::vector<char>& sp_char);
  // parser utf code
  JRetType parser_utf_str(unsigned hex, std::vector<char>& sp_vec);
  JRetType parser_string_base(JString& s);
  JRetType parser_string(JNode& node);

  JRetType parser_array(JNode& node);
  JRetType parser_object_member(JOjectElement& objm);
  JRetType parser_object(JNode& node);

  JRetType jst_ws_parser(jst_ws_state state, JNType t = JST_NULL);
  JRetType stringify_value(const JNode& jn);
  void stringify_string(const JNode& jn);

  void* stack_push(size_t size);
  void* stack_pop(size_t size);

  std::string str = "";
  int str_index = 0;
  char* stack = nullptr;
  size_t top = 0, size = 0;

  const size_t init_stack_size = 256;
};

}  // namespace jst

#endif  // !__JSON_TOY_H__
