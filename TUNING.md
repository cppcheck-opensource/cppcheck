# Tuning Cppcheck Analysis

There are several ways you can potentially improve the runtime of a Cppcheck analysis.

Note: Most of these suggestions highly depend on your code base so they need to be evaluated before being applied. They might also depend the code of Cppcheck itself and your environment. So you might want to evaluate these on a regular basis.

## Collecting Information

To see the time each file took to analyze just specify the `--showtime=file-total` CLI option.

## Tuning At Build-Level

It is most likely that a pre-built binary is being used - either an official one or one provided by the packaging manager of my operating system.

But if you build your own binary it is possible to apply several steps to potentially improve the performance.

Note: Recently more distribution have opted to use more advanced optimization for their packages so several of the following suggestions might have already been applied.
Please refer to the documentation of your distribution for more insight on this.

### Use Matchcompiler

(TODO: document how to use it when using the in-tree Visual Studio project)
(TODO: check with a CMake generated Visual Studio project)

### Use Boost

Boost.Container (https://www.boost.org/doc/libs/release/libs/container) is being used for some containers which have a smaller overhead.

As the used library is header-only implementation you only need to install the package on the system you build the binary on but not on the system you run the analysis on.

The official Windows binary is always using this.

This will be used by default if Boost is detected in CMake. If you want to enforce the usage, you can use the CMake option `-DUSE_BOOST=On` which will cause the build to fail if no Boost was detected.

Using Visual Studio you need to provide a full Boost release (i.e. including binaries) for it to be detected by CMake. If you are not able to do this you can specify the CMake option `-DBOOST_INCLUDEDIR=<in>` (pointing to the directory which *contains* the `boost` include directory) to work around this (this is a Cppcheck specific hack) - see https://trac.cppcheck.net/ticket/13822 for more details.

If you are using `make` instead you need to specify `-DHAVE_BOOST` in the flags.

If you are using the Visual Studio project you need to specify the properties `HaveBoost` (always needs to be set to `HAVE_BOOST`) and `BoostInclude` (set to the Boost folder). On the command-line you would need to add `-p:HaveBoost=HAVE_BOOST -p:BoostInclude=<boostdir>`.

### Use A Different Compiler

Analyzing our own code base has shown that using a different compiler might lead to slightly better performance.

In our case Clang is mostly faster than GCC. See https://trac.cppcheck.net/ticket/10778 for some details.

### Use More Advanced Optimizations

By default we enforce the `-O2` optimization level. Even when using the `Release` build type in CMake which defaults to `-O3`. It might be possible that building with `-O3` might yield a performance increase.

There are also no additional code generation flags provided so the resulting binary can run on any system. You might be able to tune this and apply more optimization which is tailored to the system you will be running the binary on.

Needs to be evaluated. (TODO: file tickets)

### Use LTO

Needs to be evaluated. See https://trac.cppcheck.net/ticket/11671.

### Use profile-guided optimizations (PGO/BOLT/AutoFDO/Propeller)

Needs to be evaluated. See https://trac.cppcheck.net/ticket/11672.

## Tuning At Analysis-Level

### Use A More Performant Platform

It seems people assume that you need to run the analysis on the same system you build your code on/for. That is not necessary as the analysis is system-agnostic.
As system headers are not required for analyzing your code you only need to specify the configuration which matches the system you run your code on.

In case you are using a project as input which can only be generated on the build/target system you can just transfer that to a different system and still run the analysis.

### Specify A Build Dir

Using the `--cppcheck-build-dir` allows you to perform incremental runs which omit files which have not been changed.

Important: As this is currently seriously lacking in testing coverage it might have shortcomings and need to be used with care. (TODO: file ticket)

### Exclude Static/Generated Files

If your code base contains files which rarely change (e.g. local copies of external dependencies) or you have generated files (e.g. `moc_*.cpp` for Qt projects) you might consider excluding these from the analysis.
This can be done by using the `-i` option on the CLI, `<ignore>` in GUI projects or by including them to begin with into the files passed to the analysis.

Depending on your setup you might also consider to scan these files in a less frequent run (e.g. only when the files have changed or Cppcheck was updated).

This could also be handled using `--cppcheck-build-dir` (see above).

### Exclude System Headers

System headers are not necessary to perform the analysis. So you might consider not providing those to the analysis and specify a library configuration via `--library` instead.

`pkg-config` for instance will always provide non-system includes.

(TODO: file ticket about ignoring all braced includes)

### Limit The Configuration

By default configuration to analyze will be determined automatically for each file based on the code. The maximum amount is limited and can be controlled by CLI options.

Depending on your setup you might want to limit it to specific configuration by using `-D` (CLI) or `--project-configuration=` (Visual Studio project).
When you are using a compilation database generated by CMake it is already using a fixed configuration.

### Use Multiple Jobs

By default only a single process/thread is being used. You might to scale this up using the `-j` CLI option. Please note that specifying a value that will max out your systems resources might have a detrimental effect.

### Use A Different Threading Model

When using multiple job for the analysis (see above) on Linux it will default to using processes. This is done so the analysis is not aborted prematurely aborted in case of a crash. 
Unfortunately it has overhead because of a suboptimal implementation and the fact that data needs to be transferred from the child processes to the main process.
So if you do not require the additional safety you might want to switch to the usage of thread instead using `--executor=thread`.

Note: For Windows binaries we currently do not provide the possibility of using processes so this does not apply.

### Disable Analyzing Of Unused Templated Functions

Currently all templated functions (either locally or in headers) will be analyzed regardless if they are instantiated or not. If you have template-heavy includes that might lead to unnecessary work and findings, and might slow down the analysis. This behavior can be disabled with `--no-check-unused-templates`.

Note: This might lead to "false negatives" in such functions if they are never instantiated. You should make sure that you have proper coverage of the affected functions in your code before enabling this.

### Limit Analysis Of Projects 

If you specify a project all files will be analyzed by default. But in some cases you might only be interested in the results in a subset of those (e.g. in IDE integrations).

Using the `--file-filter=<pattern>` CLI option you can select files using a globbing syntax. Using `--file-filter=-` you can provide the filters directly on the CLI.

## Advanced Tuning

### Re-order The Files

Files which take longer to analyze should be processed at first so they might not extended the run time. As the order how the files provided on the CLI or via the project are being honored it is as simple as that.

### Adjust Thresholds

There are lots of internal thresholds which limit the work which is put into parts of the analysis. The defaults were chosen as a compromise of time being spent vs. issues being detected but not might not be a good fit in all cases.

These thresholds are currently neither exposed nor documented so they cannot be changed without the modifying the source which is *highly discouraged*.

They are being utilized internally by `-check-level` though (see below).

(TODO: file ticket about providing all bailouts)
(TODO: file ticket about expose these)
(TODO: file ticket about specifying these per file)

Note: As these will lead to less data being collected for the analysis it might lead to false negatives *and* false positives.

### Adjust Check Level

There are several check levels which are basically a collection of different threshold values (see above). This can be adjusted by the CLI option `--check-level`.

Note: The current default is the lowest available check level.

Note: As these will lead to less data being collected for the analysis it might lead to false negatives *and* false positives.

## Reporting Issues

If you encounter a file which has an unreasonable slow analysis please consider reporting this as an issue.

Also consider reporting major upticks in the runtime of the analysis after updating to a newer version. Some of these might be expected as the analysis is constantly improved but out-of-the-box we still need aim for reasonable times.

In all cases please try to provide a small reproducer if possible.

Note: There might even be cases the analysis will never finish because it is stuck in a cycle. This is quite uncommon but there are still several unresolved known cases so it is possible to encounter this.

### Known Issues

https://trac.cppcheck.net/ticket/10663
https://trac.cppcheck.net/ticket/10765
https://trac.cppcheck.net/ticket/10778
https://trac.cppcheck.net/ticket/11262
https://trac.cppcheck.net/ticket/12528
https://trac.cppcheck.net/ticket/13698