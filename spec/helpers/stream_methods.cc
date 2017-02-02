#include "helpers/stream_methods.h"
#include "spec_helper.h"
#include "tree_sitter/compiler.h"
#include "compiler/parse_table.h"
#include "compiler/syntax_grammar.h"
#include "compiler/build_tables/parse_item.h"
#include "compiler/build_tables/lex_item.h"

namespace tree_sitter {

ostream &operator<<(ostream &stream, const Grammar &grammar) {
  stream << string("#<grammar");
  stream << " rules: " << grammar.rules;
  return stream << string("}>");
}

ostream &operator<<(ostream &stream, const CompileError &error) {
  if (error.type)
    return stream << (string("#<compile-error '") + error.message + "'>");
  else
    return stream << string("#<no-compile-error>");
}

ostream &operator<<(ostream &stream, const Rule &rule) {
  return stream << rule.to_string();
}

ostream &operator<<(ostream &stream, const rule_ptr &rule) {
  if (rule.get())
    stream << *rule;
  else
    stream << string("(null-rule)");
  return stream;
}

ostream &operator<<(ostream &stream, const Variable &variable) {
  return stream << string("{") << variable.name << string(", ") << variable.rule << string(", ") << to_string(variable.type) << string("}");
}

ostream &operator<<(ostream &stream, const SyntaxVariable &variable) {
  return stream << string("{") << variable.name << string(", ") << variable.productions << string(", ") << to_string(variable.type) << string("}");
}

std::ostream &operator<<(std::ostream &stream, const AdvanceAction &action) {
  return stream << string("#<advance ") + to_string(action.state_index) + ">";
}

std::ostream &operator<<(std::ostream &stream, const AcceptTokenAction &action) {
  return stream << string("#<accept ") + to_string(action.symbol.index) + ">";
}

ostream &operator<<(ostream &stream, const ParseAction &action) {
  switch (action.type) {
    case ParseActionTypeError:
      return stream << string("#<error>");
    case ParseActionTypeAccept:
      return stream << string("#<accept>");
    case ParseActionTypeShift:
      return stream << string("#<shift state:") << to_string(action.state_index) << ">";
    case ParseActionTypeReduce:
      return stream << ("#<reduce sym" + to_string(action.symbol.index) + " " +
                        to_string(action.consumed_symbol_count) + ">");
    default:
      return stream;
  }
}

ostream &operator<<(ostream &stream, const ParseTableEntry &entry) {
  return stream << entry.actions;
}

ostream &operator<<(ostream &stream, const ParseState &state) {
  stream << string("#<parse_state terminal_entries:");
  stream << state.terminal_entries;
  stream << " nonterminal_entries: " << state.nonterminal_entries;
  return stream << string(">");
}

ostream &operator<<(ostream &stream, const ExternalToken &external_token) {
  return stream << "{" << external_token.name << ", " << external_token.type <<
    "," << external_token.corresponding_internal_token << "}";
}

ostream &operator<<(ostream &stream, const ProductionStep &step) {
  stream << "(symbol: " << step.symbol << ", precedence:" << to_string(step.precedence);
  stream << ", associativity: ";
  switch (step.associativity) {
    case rules::AssociativityLeft:
      return stream << "left)";
    case rules::AssociativityRight:
      return stream << "right)";
    default:
      return stream << "none)";
  }
}

ostream &operator<<(ostream &stream, const PrecedenceRange &range) {
  if (range.empty)
    return stream << string("{empty}");
  else
    return stream << string("{") << to_string(range.min) << string(", ") << to_string(range.max) << string("}");
}

namespace build_tables {

ostream &operator<<(ostream &stream, const LexItem &item) {
  return stream << string("(item ") << item.lhs << string(" ") << *item.rule
                << string(")");
}

ostream &operator<<(ostream &stream, const LexItemSet &item_set) {
  return stream << item_set.entries;
}

ostream &operator<<(ostream &stream, const LexItemSet::Transition &transition) {
  return stream << "{dest: " << transition.destination << ", prec: " << transition.precedence << "}";
}

ostream &operator<<(ostream &stream, const ParseItem &item) {
  return stream << string("(item variable:") << to_string(item.variable_index)
                << string(" production:") << to_string((size_t)item.production % 1000)
                << string(" step:") << to_string(item.step_index)
                << string(")");
}

std::ostream &operator<<(std::ostream &stream, const ParseItemSet &item_set) {
  return stream << item_set.entries;
}

std::ostream &operator<<(std::ostream &stream, const LookaheadSet &set) {
  if (set.entries.get()) {
    return stream << *set.entries;
  } else {
    return stream << "{}";
  }
}

}  // namespace build_tables

} // namespace tree_sitter
