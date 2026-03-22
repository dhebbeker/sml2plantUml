# Doxygen filter for Boost.SML conversion to PlantUML

This tool can be used to automatically generate [PlantUML][] diagrams
from [Boost.SML][] state machines when using [doxygen][],
making it easy to visualize and document your state machine designs.

## Requirements

- [cmake][]
- [Boost.SML][]
- a C++ compiler which supports the requirements of [Boost.SML][]
- [PlantUML][]

## Limitations

- The transition table of the state machine must be defined in a file
  which has no dependencies other than `<boost/sml.h>` and STL.
  - This limits the use of lambdas in the transition table to those which
    don't depend on external defintions.
  - Dependencies to external definitions can be outsourced by
    - Declare function call operators in used defined types for
      actions and guards.
    - Use forward declarations for their parameter types if necessary.
    - Define those functions in a different file.
    - Usually the transision table, guards, actions and
      state types would be defined in a header files.
      The guards and actions would contain only the declaration of the
      call operator functions; not their definition.
      Events are only declared; not defined.
- The name of the file containing the definition of a transition table
  must adhere to a specified pattern.
  The name of the type creating the transition table must be part of
  the file name. The required pattern is `**/XYZ_state_machine_sml.hpp` where
  - `**/` is the path to the file
  - `XYZ` is the name of the state machine to be processed
  - `_state_machine_sml.hpp` is a fixed pattern to filter for those files
- In each such file, only one transition table may be transformed to
  a PlantUML diagram.

## Direct Usage (without Doxygen)

```
cmake -P sml2plantUml_filter.cmake <path_to_header>
```

## Integration in Doxygen configuration

Add a [`FILTER_PATTERNS`](https://www.doxygen.nl/manual/config.html#cfg_filter_patterns)
entry to your doxygen configuration with
the path to this tool and the file pattern for files which define
transition tables.

## How it works

1. This tool will be called by doxygen with the path to a file which defines
   a transition table for a Boost.SML state machine.
2. This tool compiles a helper program including the provided file.
   The compiler will build the transistion table using metaprogramming
   provided by the Boost.SML library.
3. This tool will then run the helper program.
   The helper program will write the PlantUML diagram source to the
   specified target directory.
4. On success this tool will add a [`\file`](https://www.doxygen.nl/manual/commands.html#cmdfile)
   documentation block to the input file which include the diagram.
   Doxygen will include the rendered state machine diagram here.
5. Finally this tool will write the input file back to standard output for
   Doxygen to process it.


[Boost.SML]: https://boost-ext.github.io/sml/
[doxygen]: https://www.doxygen.nl/
[cmake]: https://cmake.org/
[PlantUML]: https://plantuml.com/

## Credits

Based on the [PlantUML integration example](https://boost-ext.github.io/sml/examples.html#plant-uml-integration) from Boost.SML.
Original code by Kris Jusiak, distributed under the Boost Software License 1.0.
