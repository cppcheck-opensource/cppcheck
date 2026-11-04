# Cppcheck addons

Addons are scripts that analyses Cppcheck dump files to check compatibility with secure coding standards and to locate various issues.

## Supported addons

+ [misra.py](https://github.com/cppcheck-opensource/cppcheck/blob/main/addons/misra.py) 
  Used to verify compliance with MISRA C 2012 - a proprietary set of guidelines to avoid such questionable code, developed for embedded systems. Since this standard is proprietary, cppcheck does not display error text by specifying only the number of violated rules (for example, [c2012-21.3]). If you want to display full texts for violated rules, you will need to create a text file containing MISRA rules, which you will have to pass when calling the script with `--rule-texts` key. Some examples of rule texts files available in [tests directory](https://github.com/cppcheck-opensource/cppcheck/blob/main/addons/test/misra/).
+ [y2038.py](https://github.com/cppcheck-opensource/cppcheck/blob/main/addons/y2038.py) 
  Checks code for [year 2038 problem](https://en.wikipedia.org/wiki/Year_2038_problem) safety. See complete description [here](https://github.com/cppcheck-opensource/cppcheck/blob/main/addons/doc/y2038.md).
+ [threadsafety.py](https://github.com/cppcheck-opensource/cppcheck/blob/main/addons/threadsafety.py) 
  Analyse Cppcheck dump files to locate threadsafety issues like static local objects used by multiple threads.
+ [naming.py](https://github.com/cppcheck-opensource/cppcheck/blob/main/addons/naming.py)
  Enforces naming conventions across the code.
+ [namingng.py](https://github.com/cppcheck-opensource/cppcheck/blob/main/addons/namingng.py)
  Enforces naming conventions across the code. Enhanced version with support for type prefixes in variable and function names.
+ [findcasts.py](https://github.com/cppcheck-opensource/cppcheck/blob/main/addons/findcasts.py)
  Locates casts in the code.
+ [misc.py](https://github.com/cppcheck-opensource/cppcheck/blob/main/addons/misc.py)
  Performs miscellaneous checks.

### Other files

- doc
  Additional files for documentation generation.
- tests
  Contains various unit tests for the addons.
- cppcheck.py
  Internal helper used by Cppcheck binary to run the addons.
- cppcheckdata.doxyfile
  Configuration file for documentation generation.
- cppcheckdata.py
  Helper class for reading Cppcheck dump files within an addon.
- misra_9.py
  Implementation of the MISRA 9.x rules used by `misra` addon. 
- namingng.config.json
  Example configuration for `namingng` addon. 
- namingng.json
  Example JSON file that can be used using --addon=namingng.json, referring to namingng.py and namingng.config.json
- ROS_naming.json
  Example configuration for the `namingng` addon enforcing the [ROS naming convention for C++ ](http://wiki.ros.org/CppStyleGuide#Files).
- runaddon.py
  Internal helper used by Cppcheck binary to run the addons.

## Usage

### Command line interface

```bash
cppcheck --addon=misc src/test.c
```

For project-wide analysis with compile_commands.json:
```bash
cppcheck --project=build/compile_commands.json --addon=y2038
```

It is also possible to call scripts as follows:
```bash
cppcheck --dump --quiet src/test.c
python misc.py src/test.c.dump
python misra.py --rule-texts=~/misra_rules.txt src/test.c.dump
```

This allows you to add additional parameters when calling the script (for example, `--rule-texts` for `misra.py`). The full list of available parameters can be found by calling any script with the `--help` flag.

### GUI

When using the graphical interface `cppcheck-gui`, the selection and configuration of addons is carried out on the tab `Addons and tools` in the project settings (`Edit Project File`):

![Screenshot](https://raw.githubusercontent.com/cppcheck-opensource/cppcheck/main/addons/doc/img/cppcheck-gui-addons.png)

