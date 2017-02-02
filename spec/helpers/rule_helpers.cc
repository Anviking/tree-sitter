#include "rule_helpers.h"
#include <memory>
#include "compiler/rules/symbol.h"

namespace tree_sitter {
  using std::make_shared;
  using std::set;
  using std::map;
  using std::ostream;
  using std::string;
  using std::to_string;
  using rules::Symbol;

  rule_ptr character(const set<uint32_t> &ranges) {
    return character(ranges, true);
  }

  rule_ptr character(const set<uint32_t> &chars, bool sign) {
    rules::CharacterSet result;
    if (sign) {
      for (uint32_t c : chars)
        result.include(c);
    } else {
      result.include_all();
      for (uint32_t c : chars)
        result.exclude(c);
    }
    return result.copy();
  }

  rule_ptr i_sym(size_t index) {
    return make_shared<Symbol>(index, Symbol::NonTerminal);
  }

  rule_ptr i_token(size_t index) {
    return make_shared<Symbol>(index, Symbol::Terminal);
  }

  rule_ptr metadata(rule_ptr rule, rules::MetadataParams params) {
    return rules::Metadata::build(rule, params);
  }

  rule_ptr active_prec(int precedence, rule_ptr rule) {
    rules::MetadataParams params;
    params.precedence = precedence;
    params.has_precedence = true;
    params.is_active = true;
    return rules::Metadata::build(rule, params);
  }

  bool operator==(const Variable &left, const Variable &right) {
    return left.name == right.name && left.rule->operator==(*right.rule) &&
      left.type == right.type;
  }
}
