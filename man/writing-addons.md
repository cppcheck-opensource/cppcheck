---
title: Writing addons
subtitle: Version 2.21 dev
author: Cppcheck team
lang: en
documentclass: report
---

# Introduction

This document provides an overview about writing Cppcheck addons.


# Overview of data

## Tokens

See class `Token` in cppcheckdata.py

Cppcheck splits the code up in tokens: operators, numbers, identifiers, etc.

Example C code:

    ab = a + b;

Addon code:

    import cppcheck

    @cppcheck.checker
    def func(cfg, data):
        for token in cfg.tokenlist:
            print(token.str)

Output:

    $ cppcheck --dump test.c
    $ python3 runaddon.py myaddon.py test.c.dump
    Checking 1.c.dump...
    Checking 1.c.dump, config ...
    ab
    =
    a
    +
    b
    ;

The `cfg.tokenlist` does not always match the raw input code exactly. For instance:
 * The `cfg.tokenlist` is preprocessed.
 * There is no typedefs in `cfg.tokenlist`.
 * C++ templates are instantiated when possible in `cfg.tokenlist`.
 * Variable declarations are sometimes split up.
 * If you don't write {} around the body for a if/else/while/for etc then those are inserted in the `cfg.tokenlist`.
 * ...

There are various properties in the `Token` class and some of those will be discussed below.


## AST - Abstract syntax tree

Cppcheck creates a syntax tree for every expression.

Example C code:

    ab = a + b;

Addon code:

    import cppcheck

    @cppcheck.checker
    def func(cfg, data):
        for token in cfg.tokenlist:
            out = token.str
            if token.astParent:
                out += f' parent:"{token.astParent.str}"'
            if token.astOperand1:
                out += f' astOperand1:"{token.astOperand1.str}"'
            if token.astOperand2:
                out += f' astOperand2:"{token.astOperand2.str}"'
            print(out)

Output:

    $ cppcheck --dump test.c
    $ python3 runaddon.py myaddon.py test.c.dump
    Checking 1.c.dump...
    Checking 1.c.dump, config ...
    ab parent:"="
    = astOperand1:"ab" astOperand2:"+"
    a parent:"+"
    + parent:"=" astOperand1:"a" astOperand2:"b"
    b parent:"+"
    ;

## ValueType

Data type of expressions are provided by `class ValueType` in cppcheckdata.py.

Example C code:

    short a;
    a = a + 10;

Addon code:

    import cppcheck

    @cppcheck.checker
    def func(cfg, data):
        for token in cfg.tokenlist:
            print(f'{token.str} : {token.valueType}')

Output:

    $ cppcheck --dump test.c
    $ python3 runaddon.py myaddon.py test.c.dump
    Checking 1.c.dump...
    Checking 1.c.dump, config ...
    short : None
    a : ValueType(type='short', sign='signed', bits=0, typeScopeId=None, originalTypeName=None, constness=0, pointer=0)
    ; : None
    a : ValueType(type='short', sign='signed', bits=0, typeScopeId=None, originalTypeName=None, constness=0, pointer=0)
    = : ValueType(type='short', sign='signed', bits=0, typeScopeId=None, originalTypeName=None, constness=0, pointer=0)
    a : ValueType(type='short', sign='signed', bits=0, typeScopeId=None, originalTypeName=None, constness=0, pointer=0)
    + : ValueType(type='int', sign='signed', bits=0, typeScopeId=None, originalTypeName=None, constness=0, pointer=0)
    10 : ValueType(type='int', sign='signed', bits=0, typeScopeId=None, originalTypeName=None, constness=0, pointer=0)
    ; : None

The `pointer` property is a simple counter.

    int p    =>  pointer=0
    int *p   =>  pointer=1
    int **p  =>  pointer=2

The `constness` property is a bitmask.

    int * *                     =>  constness=0
    const int * *               =>  constness=1
    int * const *               =>  constness=2
    int * * const               =>  constness=4
    const int * const *         =>  constness=3
    const int * const * const   =>  constness=7


## Variable

Information about a variable is provided by `class Variable` in cppcheckdata.py.

Example code:

    short a[10];
    a[0] = 0;


Addon code:

    import cppcheck
    
    @cppcheck.checker
    def func(cfg, data):
        for token in cfg.tokenlist:
            print(f'{token.str} : {token.variable}')

Output:

    $ cppcheck --dump test.c
    $ python3 runaddon.py myaddon.py test.c.dump
    Checking 1.c.dump...
    Checking 1.c.dump, config ...
    a : Variable(Id='0x55f432fc1580', nameTokenId='0x55f432fe0850', typeStartTokenId='0x55f432fc0eb0', typeEndTokenId='0x55f432fc0eb0', access='Global', scopeId='0x55f432fe0360', isArgument=False, isArray=True, isClass=False, isConst=False, isGlobal=True, isExtern=False, isLocal=False, isPointer=False, isReference=False, isStatic=False, constness=0)
    [ : None
    10 : None
    ] : None
    ; : None
    a : Variable(Id='0x55f432fc1580', nameTokenId='0x55f432fe0850', typeStartTokenId='0x55f432fc0eb0', typeEndTokenId='0x55f432fc0eb0', access='Global', scopeId='0x55f432fe0360', isArgument=False, isArray=True, isClass=False, isConst=False, isGlobal=True, isExtern=False, isLocal=False, isPointer=False, isReference=False, isStatic=False, constness=0)
    [ : None
    0 : None
    ] : None
    = : None
    0 : None
    ; : None


## Scope

See `class Scope` in cppcheckdata.py.

Every token is in some scope.

If you see {} in the code then that is a scope of some kind. There is one scope that is not surrounded by {}; the global scope.

Example c code:

    int x;
    void foo()
    {
        if (x)
        {
            x = 0;
        }
    }

Example addon #1 (list all scopes):

    import cppcheck
    
    @cppcheck.checker
    def func(cfg, data):
        for scope in cfg.scopes:
            print(scope.type)
    
Output:

    $ cppcheck --dump test.c
    $ python3 runaddon.py myaddon.py test.c.dump
    Checking 1.c.dump...
    Checking 1.c.dump, config ...
    Global
    Function
    If

Example addon #2 (show scope for each token):

    import cppcheck
    
    @cppcheck.checker
    def func(cfg, data):
        for token in cfg.tokenlist:
            print(f'{token.str} : {scope.type}')
    
Output:

    $ cppcheck --dump test.c
    $ python3 runaddon.py myaddon.py test.c.dump
    Checking 1.c.dump...
    Checking 1.c.dump, config ...
    int : Global
    x : Global
    ; : Global
    void : Global
    foo : Global
    ( : Global
    ) : Global
    { : Function
    if : Function
    ( : Function
    x : Function
    ) : Function
    { : If
    x : If
    = : If
    0 : If
    ; : If
    } : If
    } : Function
    
### Special tokenlist tweaks and else if

The `cfg.tokenlist` has some tweaks. In C/C++ code it is optional to use `{` and `}` around the if/else/for/while body,
if the body is only a single statement. However Cppcheck adds "{" and "}" tokens if those are missing.

One more tweak is that in cfg.tokenlist there is no "else if" scope.

Example C code:

    void foo(int x)
    {
        if (x > 0)
            --x;
        else if (x < 0)
            ++x;
    }

The tokens in the `cfg.tokenlist` will look like this:

    void foo(int x)
    {
        if (x > 0)
        {
            --x;
        }
        else
        {
            if (x < 0)
            {
                ++x;
            }
        }
    }

And there are 2 "If" scopes here. And 1 "Else" scope.


## Function

The class `Function` in cppcheckdata.py represents a function that is declared somewhere.

Example code:

    void foo(int x);

    void bar()
    {
        foo(1);
    }


Example addon #1:

    import cppcheck
    
    @cppcheck.checker
    def func(cfg, data):
        for token in cfg.tokenlist:
            print(f'{token.str} : {token.function}')
    
Output:

    void : None
    foo : Function(Id='0x55c5f9330f60', tokenId='0', tokenDefId='0x55c5f93510d0', name='foo', type='Function', isVirtual=False, isImplicitlyVirtual=False, isInlineKeyword=False, isStatic=False, argumentId={1: '0x55c5f9351260'})
    ( : None
    int : None
    x : None
    ) : None
    ; : None
    void : None
    bar : Function(Id='0x55c5f9331040', tokenId='0x55c5f93314a0', tokenDefId='0x55c5f93314a0', name='bar', type='Function', isVirtual=False, isImplicitlyVirtual=False, isInlineKeyword=False, isStatic=False, argumentId={})
    ( : None
    ) : None
    { : None
    foo : Function(Id='0x55c5f9330f60', tokenId='0', tokenDefId='0x55c5f93510d0', name='foo', type='Function', isVirtual=False, isImplicitlyVirtual=False, isInlineKeyword=False, isStatic=False, argumentId={1: '0x55c5f9351260'})
    ( : None
    1 : None
    ) : None
    ; : None
    } : None
    

## Value flow

For Cppcheck, by default variables and expressions have "unknown" values. If the value is only constrained by the data type that is typically not enough information to write warnings with precision. Unless all the values possible in the data type are bad.

In Cppcheck terminology an expression will have a "possible" value if Cppcheck can determine that there will be a specific value in some control flow paths.

In Cppcheck terminology an expression will have a "known" value if Cppcheck can determine that there will be a specific value in all control flow paths.

An expression can have several "possible" values. But it can't have several "known" values.

Example code:

    void foo(int x)  // <- values of x is only constrained by data type. there are no "possible" or "known" values here.
    {
        a = x;  // <- assuming that condition below is not redundant, x can have value 2.
        if (x == 2)
        {
            b = x + 2;  // <- value of x is always 2 when this code is executed. It's "known".
        }
        else
        {
            c = x;
        }
        d = x + 10;  // <- value of x can be 2 when this code is executed. It's "possible".
    }

Addon:

    import cppcheck
    
    @cppcheck.checker
    def func(cfg, data):
        for token in cfg.tokenlist:
            if token.values:
                print(f'line {token.linenr} str="{token.str}"')
                for value in token.values:
                    print(f'    {value}')

Output:

    $ ../cppcheck --dump 1.c ; python3 runaddon.py myaddon.py 1.c.dump 
    Checking 1.c ...
    Checking 1.c.dump...
    Checking 1.c.dump, config ...
    line 3 str="="
        Value(intvalue=2, tokvalue=None, floatvalue=None, containerSize=None, condition=4, valueKind='possible', inconclusive=False)
    line 3 str="x"
        Value(intvalue=2, tokvalue=None, floatvalue=None, containerSize=None, condition=4, valueKind='possible', inconclusive=False)
    line 4 str="2"
        Value(intvalue=2, tokvalue=None, floatvalue=None, containerSize=None, condition=None, valueKind='known', inconclusive=False)
    line 6 str="="
        Value(intvalue=4, tokvalue=None, floatvalue=None, containerSize=None, condition=4, valueKind='known', inconclusive=False)
    line 6 str="x"
        Value(intvalue=2, tokvalue=None, floatvalue=None, containerSize=None, condition=4, valueKind='known', inconclusive=False)
    line 6 str="+"
        Value(intvalue=4, tokvalue=None, floatvalue=None, containerSize=None, condition=4, valueKind='known', inconclusive=False)
    line 6 str="2"
        Value(intvalue=2, tokvalue=None, floatvalue=None, containerSize=None, condition=None, valueKind='known', inconclusive=False)
    line 12 str="="
        Value(intvalue=12, tokvalue=None, floatvalue=None, containerSize=None, condition=4, valueKind='possible', inconclusive=False)
    line 12 str="x"
        Value(intvalue=2, tokvalue=None, floatvalue=None, containerSize=None, condition=4, valueKind='possible', inconclusive=False)
    line 12 str="+"
        Value(intvalue=12, tokvalue=None, floatvalue=None, containerSize=None, condition=4, valueKind='possible', inconclusive=False)
    line 12 str="10"
        Value(intvalue=10, tokvalue=None, floatvalue=None, containerSize=None, condition=None, valueKind='known', inconclusive=False)
    
If the value is determined from a condition then the attribute `condition` points out the line that has the condition.

