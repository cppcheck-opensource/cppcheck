
# unknownMacro

**Message**: There is an unknown macro here somewhere. Configuration is required. If AAA is a macro then please configure it. [unknownMacro]
<br/>
**Category**: Configuration<br/>
**Severity**: Error<br/>
**Language**: C and C++

## Description

Cppcheck has found code that is confusing and does not know how to analyze it. This is a critical
error, the analysis of the whole translation unit is aborted. Such error in a header file can mean
that analysis of many source files are aborted.

Your code is probably OK but you need to configure Cppcheck to make Cppcheck understand the code
better.

## How to fix

Review the configuration.

If Cppcheck warns about a macro that is defined in a 3rd party library, and there is a cfg file for
that, then a `--library=` option may be a proper solution.

If Cppcheck warns about a macro that is defined in a header that should be included, make sure that
this header is included properly. Cppcheck must have the include path.

If Cppcheck warns about a compiler keyword add a `-D` that defines this keyword somehow. I.e. if
cppcheck should just ignore the keyword then an `-DKEYWORD=` option is suggested.

## Example

### Example code 1
```
    fprintf(stderr, "Generating up to " F_U64 " sequences and up to " F_U64 " bases.\n", nSeqs, nBases);
```

Warning:

canu-2.2/src/seqrequester/src/seqrequester/generate.H:72:41: error: There is an unknown macro here somewhere. Configuration is required. If F_U64 is a macro then please configure it. [unknownMacro]

Fix:

Somehow `F_U64` must be specified for Cppcheck to be able to analyse this properly. Either:
 * Add `-DF_U64="x"` to explicitly tell Cppcheck what it should replace F_U64 with. Or;
 * Add `-I..` so that headers are included properly.
 * If the symbol is defined in a 3rd party library adding a corresponding `--library=` might solve such issue.

### Example code 2
```
BOTAN_FUNC_ISA("crypto")
void AES_128::hw_aes_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
```

Warning:

botan-2.19.5+dfsg/src/lib/block/aes/aes_power8/aes_power8.cpp:103:1: error: There is an unknown macro here somewhere. Configuration is required. If BOTAN_FUNC_ISA is a macro then please configure it. [unknownMacro]

Fix:

Somehow `BOTAN_FUNC_ISA` must be specified for Cppcheck to be able to analyse this properly. Either:
 * Add `-DBOTAN_FUNC_ISA(X)=` to explicitly tell Cppcheck that BOTAN_FUNC_ISA("crypto") should be ignored. Or;
 * Add `-I..` so that headers are included properly.
 * If the symbol is defined in a 3rd party library adding a corresponding `--library=` might solve such issue.



