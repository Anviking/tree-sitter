#include "compiler/prepare_grammar/parse_regex.h"
#include <string>
#include <utility>
#include <vector>
#include "compiler/rules/choice.h"
#include "compiler/rules/seq.h"
#include "compiler/rules/repeat.h"
#include "compiler/rules/character_set.h"
#include "compiler/rules/blank.h"
#include "compiler/util/string_helpers.h"
#include "utf8proc.h"

namespace tree_sitter {
namespace prepare_grammar {

using std::string;
using std::vector;
using std::pair;
using std::make_shared;
using rules::CharacterSet;
using rules::Seq;
using rules::Blank;
using rules::Choice;
using rules::Repeat;

class PatternParser {
 public:
  explicit PatternParser(const string &input)
      : input(input),
        iter((const uint8_t *)input.data()),
        end(iter + input.size()) {
    next();
  }

  pair<rule_ptr, CompileError> rule(bool nested) {
    vector<rule_ptr> choices = {};
    do {
      if (!choices.empty()) {
        if (peek() == '|')
          next();
        else
          break;
      }
      auto pair = term(nested);
      if (pair.second.type)
        return { Blank::build(), pair.second };
      choices.push_back(pair.first);
    } while (has_more_input());
    auto rule =
      (choices.size() > 1) ? make_shared<Choice>(choices) : choices.front();
    return { rule, CompileError::none() };
  }

 private:
  pair<rule_ptr, CompileError> term(bool nested) {
    rule_ptr result = Blank::build();
    do {
      if (peek() == '|')
        break;
      if (nested && peek() == ')')
        break;
      auto pair = factor();
      if (pair.second.type)
        return { Blank::build(), pair.second };
      result = Seq::build({ result, pair.first });
    } while (has_more_input());
    return { result, CompileError::none() };
  }

  pair<rule_ptr, CompileError> factor() {
    auto pair = atom();
    if (pair.second.type)
      return { Blank::build(), pair.second };
    rule_ptr result = pair.first;
    if (has_more_input()) {
      switch (peek()) {
        case '*':
          next();
          result = Choice::build({ Repeat::build(result), Blank::build() });
          break;
        case '+':
          next();
          result = Repeat::build(result);
          break;
        case '?':
          next();
          result = Choice::build({ result, Blank::build() });
          break;
      }
    }
    return { result, CompileError::none() };
  }

  pair<rule_ptr, CompileError> atom() {
    switch (peek()) {
      case '(': {
        next();
        auto pair = rule(true);
        if (pair.second.type)
          return { Blank::build(), pair.second };
        if (peek() != ')')
          return error("unmatched open paren");
        next();
        return { pair.first, CompileError::none() };
      }
      case '[': {
        next();
        auto pair = char_set();
        if (pair.second.type)
          return { Blank::build(), pair.second };
        if (peek() != ']')
          return error("unmatched open square bracket");
        next();
        return { pair.first.copy(), CompileError::none() };
      }
      case ')': {
        return error("unmatched close paren");
      }
      case ']': {
        return error("unmatched close square bracket");
      }
      case '.': {
        next();
        return { CharacterSet().include_all().exclude('\n').copy(),
                 CompileError::none() };
      }
      default: {
        auto pair = single_char();
        if (pair.second.type)
          return { Blank::build(), pair.second };
        return { pair.first.copy(), CompileError::none() };
      }
    }
  }

  pair<CharacterSet, CompileError> char_set() {
    CharacterSet result;
    bool is_affirmative = true;
    if (peek() == '^') {
      next();
      is_affirmative = false;
      result.include_all();
    }

    while (has_more_input() && (peek() != ']')) {
      auto pair = single_char();
      if (pair.second.type)
        return { CharacterSet(), pair.second };
      if (is_affirmative)
        result.add_set(pair.first);
      else
        result.remove_set(pair.first);
    }

    return { result, CompileError::none() };
  }

  pair<CharacterSet, CompileError> single_char() {
    CharacterSet value;
    switch (peek()) {
      case '\\':
        next();
        value = escaped_char(peek());
        next();
        break;
      default:
        uint32_t first_char = peek();
        next();
        if (peek() == '-') {
          next();
          value = CharacterSet().include(first_char, peek());
          next();
        } else {
          value = CharacterSet().include(first_char);
        }
    }
    return { value, CompileError::none() };
  }

  CharacterSet escaped_char(uint32_t value) {
    switch (value) {
      case 'a':
        return CharacterSet().include('a', 'z').include('A', 'Z');
      case 'w':
        return CharacterSet()
          .include('a', 'z')
          .include('A', 'Z')
          .include('0', '9')
          .include('_');
      case 'W':
        return CharacterSet()
          .include_all()
          .exclude('a', 'z')
          .exclude('A', 'Z')
          .exclude('0', '9')
          .exclude('_');
      case 'd':
        return CharacterSet().include('0', '9');
      case 'D':
        return CharacterSet().include_all().exclude('0', '9');
      case 's':
        return CharacterSet().include(' ').include('\t').include('\n').include(
          '\r');
      case 'S':
        return CharacterSet()
          .include_all()
          .exclude(' ')
          .exclude('\t')
          .exclude('\n')
          .exclude('\r');
      case 't':
        return CharacterSet().include('\t');
      case 'n':
        return CharacterSet().include('\n');
      case 'r':
        return CharacterSet().include('\r');
      default:
        return CharacterSet().include(value);
    }
  }

  void next() {
    size_t lookahead_size = utf8proc_iterate(iter, end - iter, &lookahead);
    if (!lookahead_size)
      lookahead = 0;
    iter += lookahead_size;
  }

  uint32_t peek() {
    return lookahead;
  }

  bool has_more_input() {
    return lookahead && iter <= end;
  }

  pair<rule_ptr, CompileError> error(string msg) {
    return { Blank::build(), CompileError(TSCompileErrorTypeInvalidRegex, msg) };
  }

  string input;
  const uint8_t *iter;
  const uint8_t *end;
  int32_t lookahead;
};

pair<rule_ptr, CompileError> parse_regex(const std::string &input) {
  return PatternParser(input.c_str()).rule(false);
}

}  // namespace prepare_grammar
}  // namespace tree_sitter
