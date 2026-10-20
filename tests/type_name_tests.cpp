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
}
