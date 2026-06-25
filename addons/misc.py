#!/usr/bin/env python3
#
# Misc: Uncategorized checks that might be moved to some better addon later
#
# Example usage of this addon (scan a sourcefile main.cpp)
# cppcheck --dump main.cpp
# python misc.py main.cpp.dump

import cppcheckdata
import sys
import re

DEBUG = ('-debug' in sys.argv)
VERIFY = ('-verify' in sys.argv)
VERIFY_EXPECTED = []
VERIFY_ACTUAL = []

def reportError(token, severity, msg, id):
    if id == 'debug' and not DEBUG:
        return
    if VERIFY:
        VERIFY_ACTUAL.append(str(token.linenr) + ':' + id)
    else:
        cppcheckdata.reportError(token, severity, msg, 'misc', id)

def simpleMatch(token, pattern):
    return cppcheckdata.simpleMatch(token, pattern)

# Get function arguments
def getArgumentsRecursive(tok, arguments):
    if tok is None:
        return
    if tok.str == ',':
        getArgumentsRecursive(tok.astOperand1, arguments)
        getArgumentsRecursive(tok.astOperand2, arguments)
    else:
        arguments.append(tok)

def getArguments(ftok):
    arguments = []
    getArgumentsRecursive(ftok.astOperand2, arguments)
    return arguments

def isStringLiteral(tokenString):
    return tokenString.startswith('"')

# check data
def stringConcatInArrayInit(data):
    # Get all string macros
    stringMacros = []
    for cfg in data.iterconfigurations():
        for directive in cfg.directives:
            res = re.match(r'#define[ ]+([A-Za-z0-9_]+)[ ]+".*', directive.str)
            if res:
                macroName = res.group(1)
                if macroName not in stringMacros:
                    stringMacros.append(macroName)

    # Check code
    arrayInit = False
    for i in range(len(data.rawTokens)):
        if i < 2:
            continue
        tok1 = data.rawTokens[i-2].str
        tok2 = data.rawTokens[i-1].str
        tok3 = data.rawTokens[i-0].str
        if tok3 == '}':
            arrayInit = False
        elif tok1 == ']' and tok2 == '=' and tok3 == '{':
            arrayInit = True
        elif arrayInit and (tok1 in [',', '{']):
            isString2 = (isStringLiteral(tok2) or (tok2 in stringMacros))
            isString3 = (isStringLiteral(tok3) or (tok3 in stringMacros))
            if isString2 and isString3:
                reportError(data.rawTokens[i], 'style', 'String concatenation in array initialization, missing comma?', 'stringConcatInArrayInit')


def implicitlyVirtual(data):
    for cfg in data.iterconfigurations():
        for function in cfg.functions:
            if function.isImplicitlyVirtual is None:
                continue
            if not function.isImplicitlyVirtual:
                continue
            reportError(function.tokenDef, 'style', 'Function \'' + function.name + '\' overrides base class function but is not marked with \'virtual\' keyword.', 'implicitlyVirtual')

def ellipsisStructArg(data):
    for cfg in data.iterconfigurations():
        for tok in cfg.tokenlist:
            if tok.str != '(':
                continue
            if tok.astOperand1 is None or tok.astOperand2 is None:
                continue
            if tok.astOperand2.str != ',':
                continue
            if tok.scope.type in ['Global', 'Class']:
                continue
            if tok.astOperand1.function is None:
                continue
            for argnr, argvar in tok.astOperand1.function.argument.items():
                if argnr < 1:
                    continue
                if not simpleMatch(argvar.typeStartToken, '...'):
                    continue
                callArgs = getArguments(tok)
                for i in range(argnr-1, len(callArgs)):
                    valueType = callArgs[i].valueType
                    if valueType is None:
                        argStart = callArgs[i].previous
                        while argStart.str != ',':
                            if argStart.str == ')':
                                argStart = argStart.link
                            argStart = argStart.previous
                        argEnd = callArgs[i]
                        while argEnd.str != ',' and argEnd.str != ')':
                            if argEnd.str == '(':
                                argEnd = argEnd.link
                            argEnd = argEnd.next
                        expression = ''
                        argStart = argStart.next
                        while argStart != argEnd:
                            expression = expression + argStart.str
                            argStart = argStart.next
                        reportError(tok, 'debug', 'Bailout, unknown argument type for argument \'' + expression + '\'.', 'debug')
                        continue
                    if valueType.pointer > 0:
                        continue
                    if valueType.type != 'record' and valueType.type != 'container':
                        continue
                    reportError(tok, 'style', 'Passing record to ellipsis function \'' + tok.astOperand1.function.name + '\'.', 'ellipsisStructArg')
                break

for arg in sys.argv[1:]:
    if arg in ['-debug', '-verify', '--cli']:
        continue

    print("Checking %s..." % arg)
    data = cppcheckdata.CppcheckData(arg)

    if VERIFY:
        VERIFY_ACTUAL = []
        VERIFY_EXPECTED = []
        for tok in data.rawTokens:
            if tok.str.startswith('//'):
                for word in tok.str[2:].split(' '):
                    if word in ['stringConcatInArrayInit', 'implicitlyVirtual', 'ellipsisStructArg']:
                        VERIFY_EXPECTED.append(str(tok.linenr) + ':' + word)

    stringConcatInArrayInit(data)
    implicitlyVirtual(data)
    ellipsisStructArg(data)

    if VERIFY:
        for expected in VERIFY_EXPECTED:
            if expected not in VERIFY_ACTUAL:
                print('Expected but not seen: ' + expected)
                sys.exit(1)
        for actual in VERIFY_ACTUAL:
            if actual not in VERIFY_EXPECTED:
                print('Not expected: ' + actual)
                sys.exit(1)

sys.exit(cppcheckdata.EXIT_CODE)
