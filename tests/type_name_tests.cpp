#include "sml2plantUml.hpp"
#include "exampleSM_state_machine_sml.hpp"
#include <cstdlib>
#include <sstream>

void expect_equal(const std::string &actual, const std::string &expected) {
  if (actual != expected) {
    std::cerr << "Expected:\n" << expected << "\nActual:\n" << actual << '\n';
    std::exit(EXIT_FAILURE);
  }
}

void expect_contains(const std::string &text, const std::string &expected) {
  if (text.find(expected) == std::string::npos) {
    std::cerr << "Missing: " << expected << "\nActual:\n" << text;
    std::exit(EXIT_FAILURE);
  }
}

class class_state {};
enum class enum_event {};
union union_state {};

struct deferred_machine {
  auto operator()() const {
    using namespace sml;
    return make_transition_table(
        *"idle"_s + event<e1> / defer,
        "idle"_s + "struct literal event"_e / defer);
  }
};

namespace ns {
enum class sequence_id { first, second };

// Sub state machines sharing the names of their inner states.
template <sequence_id ID> struct sequence {
  auto operator()() const {
    using namespace sml;
    return make_transition_table(*"prompt"_s + event<e1> = "measure"_s,
                                 "measure"_s + event<e2> = X);
  }
};

// Same as sequence, but chooses its own display name.
template <sequence_id ID> struct named_sequence : sequence<ID> {
  static constexpr const char *c_str() {
    return ID == sequence_id::first ? "FirstSequence" : "SecondSequence";
  }
};
} // namespace ns

struct sequences_machine {
  auto operator()() const {
    using namespace sml;
    using ns::sequence_id;
    return make_transition_table(
        *"idle"_s + event<e1> = state<ns::sequence<sequence_id::first>>,
        state<ns::sequence<sequence_id::first>> + event<e3> =
            state<ns::named_sequence<sequence_id::second>>,
        state<ns::named_sequence<sequence_id::second>> + event<e3> =
            "idle"_s);
  }
};

int main() {
  expect_equal(normalize_type_name("struct sub_machine"), "sub_machine");
  expect_equal(normalize_type_name("class ns::State"), "ns::State");
  expect_equal(normalize_type_name("enum ns::Event"), "ns::Event");
  expect_equal(normalize_type_name("union ns::State"), "ns::State");
  expect_equal(normalize_type_name(
                   "struct ns::Outer<class Inner<struct A>, enum E,union U>"),
               "ns::Outer<Inner<A>, E,U>");
  expect_equal(normalize_type_name("void(struct A, class B,enum E, union U)"),
               "void(A, B,E, U)");
  expect_equal(normalize_type_name("Outer< \tstruct A>"), "Outer< \tA>");
  expect_equal(normalize_type_name("ns::structure<classify, mystruct>"),
               "ns::structure<classify, mystruct>");
  expect_equal(normalize_type_name(""), "");
  expect_equal(normalize_type_name("sub_machine"), "sub_machine");

  expect_equal(plantuml_name<sub_machine>(), "sub_machine");
  expect_equal(plantuml_name<class_state>(), "class_state");
  expect_equal(plantuml_name<enum_event>(), "enum_event");
  expect_equal(plantuml_name<union_state>(), "union_state");
  using namespace sml;
  expect_equal(plantuml_name<typename decltype("struct literal"_s)::type>(),
               "struct literal");
  std::ostringstream diagram;
  dump<exampleSM>(diagram);
  expect_contains(diagram.str(), "s2 --> sub_machine : e4 /\\n action\n");
  expect_contains(diagram.str(), "state sub_machine {\n");
  expect_contains(diagram.str(), "sub_machine --> [*] : e5 /\\n (action,\\n action2)\n");
  expect_contains(diagram.str(), "[!guard &&\\n guard2]");
  expect_contains(diagram.str(), "[guard ||\\n guard2]");
  expect_contains(diagram.str(), "s1 : on_entry /\\n on_s1_entry\n");
  expect_contains(diagram.str(), "s2 : on_exit /\\n on_s2_exit\n");

  state_initialized = false;
  completed_submachines.clear();
  std::ostringstream deferred;
  dump<deferred_machine>(deferred);
  expect_contains(deferred.str(), "idle : e1 / defer\n");
  expect_contains(deferred.str(), "idle : struct literal event / defer\n");

  expect_equal(plantuml_id("", "ns::sequence<ns::sequence_id::first>"),
               "ns__sequence_ns__sequence_id__first_");
  expect_equal(plantuml_id("outer", "s 1"), "outer__s_1");

  std::ostringstream sequences;
  dump<sequences_machine>(sequences);
  const auto sequences_text = sequences.str();
  // Without c_str(), the type name is displayed.
  expect_contains(sequences_text, "state \"ns::sequence<");
  // With c_str(), its name is displayed.
  expect_contains(sequences_text, "state \"SecondSequence\" as ");
  // Equal inner state names are not merged across sub state machines.
  const auto first_id =
      submachine_id<ns::sequence<ns::sequence_id::first>>("");
  const auto second_id =
      submachine_id<ns::named_sequence<ns::sequence_id::second>>("");
  expect_contains(sequences_text, "idle --> " + first_id + " : e1\n");
  expect_contains(sequences_text,
                  "state \"prompt\" as " + first_id + "__prompt\n");
  expect_contains(sequences_text,
                  "state \"prompt\" as " + second_id + "__prompt\n");
  expect_contains(sequences_text, first_id + "__prompt --> " + first_id +
                                      "__measure : e1\n");
  expect_contains(sequences_text, second_id + "__measure --> [*] : e2\n");
  expect_contains(sequences_text,
                  first_id + " --> " + second_id + " : e3\n");
  for (const auto &id : {first_id, second_id}) {
    for (const char ch : id) {
      if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '_') {
        std::cerr << "Invalid identifier: " << id << '\n';
        std::exit(EXIT_FAILURE);
      }
    }
  }
}
