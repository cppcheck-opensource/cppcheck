# Cppcheck Safety Notes

## Process Stability

### Note On Code Handling

The internal parsing of Cppcheck is very lenient and is able to handle code which will not be compilable. This is done so you can analyze code without providing all dependencies and code which is written for specific compilers.

### Providing Process Stability

The leniency means that code which is invalid will not be rejected outright. So it is possible to trigger an assertion or crash within Cppcheck causing the process to exit prematurely.

Since the analysis is by default performed in the main process such a failure will cause the analysis to abort without providing a result.

If you use `-j<n>` multiple jobs (up to the total amount of `n`) will be used to perform the analysis. On supported platforms a new process will be spawned for each file which is analyzed (this is currently only implemented on non-Windows platforms - see https://trac.cppcheck.net/ticket/12464). This will make sure the analysis will always finish and provide a result. Premature termination failures will be provided as `cppcheckError` findings.

If you encountered any such bugs please report them at https://trac.cppcheck.net/.

### Note On Handling Unknown/Untrusted Code

Triggering an abnormal process terminations is most likely caused by unknown/untrusted code (provided by e.g. a pull request). To reduce the chance of this happening you should invoke the analysis *after* that code has been successfully compiled. This might also limit the possibility of someone leveraging a known cause for a abnormal process termination for malicious purposes.

### Fuzzing

We are utilizing fuzzing based on libFuzzer and OSS-Fuzz to catch such issues. Currently this is mainly done off-tree and manually - see https://trac.cppcheck.net/ticket/12442 and https://github.com/danmar/simplecpp/issues/341 for details.

As those methods mostly result in non-sensical and invalid code there is also a plan to use a targeted approach which would be closer to examples of real-life invalid/incomplete code (like when used via an IDE integration). See https://trac.cppcheck.net/ticket/12357 for details.

## Code Execution

The analyzed code will only be parsed and/or compiled but no executable code is being generated. So there is no risk in analyzing code containing known exploits as it will never be executed.

## Required Permissions

By default you only need the permissions to read the files specified for analysis and the specified include paths. No administrator permissions should be necessary to perform the analysis.

Additional permissions might be necessary for specific options being provided. Those are lined out below.

## Process Elevation

No process elevation is being performed for any of the spawned processes. They currently use the same permissions as the main process.

See https://trac.cppcheck.net/ticket/14237 about a potential future improvement by reducing the permissions of spawned processes.

## Invoked executables

By default no additional external executables will be invoked by the analysis.

But there are some options which will utilize additional executables.

- `python`/`python3`<br/>
Used for the execution of addons. Triggered by using the `--addon=<addon>` option or specifying addons in `cppcheck.cfg`.<br/>
The executable will be looked for in `PATH` by default and can be configured by `--addon-python=<exe>` or the `python` field when passing a JSON to `--addon`.

- `clang-tidy`<br/>
Used for invoking an additional analysis via Clang-Tidy. Triggered by the `--clang-tidy` option.<br/>
The executable will be looked for in `PATH` by default. A versioned executable or absolute path can be specified by using `--clang-tidy=<exe>`.

- `clang`<br/>
Used to generate a CLang AST instead of an internal one. Triggered by the `--clang` option.<br/>
The executable will be looked for in `PATH` by default. A versioned executable or absolute path can be specified by using `--clang=<exe>`.

## Output Files

By default no files will be written.

But there are some options which will cause files to be written.

- `--cppcheck-build-dir=<dir>`<br/>
Will create files inside the specified directory only - the specified directory needs to be created by the user before running the analysis

- `--dump`<br/>
When `--cppcheck-build-dir` is *not* specified `<file>.dump` files will be created next to the files which are being analyzed

- `--plist-output=<dir>`<br/>
Will create files inside the specified directory only - the specified directory needs to be created by the user before running the analysis

- `--addon=<addon>`<br/>
When `--cppcheck-build-dir` is *not* specified a `<pid>.ctu-info` will be generated in the CWD and `<file>.<pid>.dump` files will be created next to the files which are being analyzed. All files will be deleted if the analysis is complete