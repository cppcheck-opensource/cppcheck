"""
cppcheckdata

This is a Python module that helps you access Cppcheck dump data.

License: No restrictions, use this as you need.
"""

import argparse
import json
import os
import sys
import subprocess

try:
    import pathlib
except ImportError:
    message = "Failed to load pathlib. Upgrade Python to 3.x or install pathlib with 'pip install pathlib'."
    error_id = 'pythonError'
    if '--cli' in sys.argv:
        msg = { 'file': '',
                'linenr': 0,
                'column': 0,
                'severity': 'error',
                'message': message,
                'addon': 'cppcheckdata',
                'errorId': error_id,
                'extra': ''}
        sys.stdout.write(json.dumps(msg) + '\n')
    else:
        sys.stderr.write('{} [{}]\n'.format(message, error_id))
    sys.exit(1)

from xml.etree import ElementTree
from fnmatch import fnmatch

EXIT_CODE = 0

current_dumpfile_suppressions = []

def _load_location(location, element):
    """Load location from element/dict"""
    location.file = element.get('file')
    line = element.get('line')
    if line is None:
        line = element.get('linenr')
    if line is None:
        line = '0'
    location.linenr = int(line)
    location.column = int(element.get('column', '0'))


class Location:
    """Utility location class"""
    file = None
    linenr = None
    column = None
    def __init__(self, element):
        _load_location(self, element)


class Directive:
    """
    Directive class. Contains information about each preprocessor directive in the source code.

    Attributes:
        str      The directive line, with all C or C++ comments removed
        file     Name of (possibly included) file where directive is defined
        linenr   Line number in (possibly included) file where directive is defined

    To iterate through all directives use such code:
    @code
    data = cppcheckdata.parsedump(...)
    for cfg in data.configurations:
      for directive in cfg.directives:
        print(directive.str)
    @endcode
    """
    #preprocessor.cpp/Preprocessor::dump

    str = None
    file = None
    linenr = None
    column = None

    def __init__(self, element):
        self.str = element.get('str')
        _load_location(self, element)

    def __repr__(self):
        attrs = ["str", "file", "linenr"]
        return "{}({})".format(
            "Directive",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )

class MacroUsage:
    """
    Tracks preprocessor macro usage

    Attributes:
        name         Name of the macro
        usefile
        useline
        usecolumn
        isKnownValue
    """
    #preprocessor.cpp/Preprocessor::dump

    name = None  # Macro name
    file = None
    linenr = None
    column = None
    usefile = None
    uselinenr = None
    usecolumn = None

    def __init__(self, element):
        self.name = element.get('name')
        _load_location(self, element)
        self.usefile = element.get('usefile')
        self.useline = element.get('useline')
        self.usecolumn = element.get('usecolumn')
        self.isKnownValue = element.get('is-known-value', 'false') == 'true'

    def __repr__(self):
        attrs = ["name", "file", "linenr", "column", "usefile", "useline", "usecolumn", "isKnownValue"]
        return "{}({})".format(
            "MacroUsage",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )


class PreprocessorIfCondition:
    """
    Information about #if/#elif conditions

    Attributes:
        E
        result
    """
    #preprocessor.cpp/Preprocessor::dump

    file = None
    linenr = None
    column = None
    E = None
    result = None

    def __init__(self, element):
        _load_location(self, element)
        self.E = element.get('E')
        self.result = int(element.get('result'))

    def __repr__(self):
        attrs = ["file", "linenr", "column", "E", "result"]
        return "{}({})".format(
            "PreprocessorIfCondition",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )

class ValueType:
    """
    ValueType class. Contains (promoted) type information for each node in the AST.

    Attributes:
        type             nonstd/pod/record/smart-pointer/container/iterator/void/bool/char/short/wchar_t/int/long/long long/unknown int/float/double/long double
        sign             signed/unsigned
        bits             bit count for bit-fields, otherwise None
        pointer
        constness
        reference
        typeScopeId
        originalTypeName bool/const char */long/char */size_t/int/double/std::string/..

    """
    #symboldatabase.cpp/ValueType::dump

    type = None
    sign = None
    bits = None
    constness = 0
    pointer = 0
    typeScopeId = None
    typeScope = None
    originalTypeName = None

    def __init__(self, element):
        self.type = element.get('valueType-type')
        self.sign = element.get('valueType-sign')
        self.bits = element.get('valueType-bits', None)
        self.bits = int(self.bits) if self.bits else None
        self.pointer = int(element.get('valueType-pointer', 0))
        self.constness = int(element.get('valueType-constness', 0))
        self.reference = element.get('valueType-reference')
        self.typeScopeId = element.get('valueType-typeScope')
        self.originalTypeName = element.get('valueType-originalTypeName')
        #valueType-containerId TODO add


    def __repr__(self):
        attrs = ["type", "sign", "bits", "typeScopeId", "originalTypeName",
                  "constness", "pointer"]
        return "{}({})".format(
            "ValueType",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )


    def setId(self, IdMap):
        self.typeScope = IdMap[self.typeScopeId]

    def isIntegral(self):
        return self.type in {'bool', 'char', 'short', 'int', 'long', 'long long'}

    def isFloat(self):
        return self.type in {'float', 'double', 'long double'}

    def isEnum(self):
        return self.typeScope and self.typeScope.type == "Enum"


class Token:
    """
    Token class. Contains information about each token in the source code.

    The CppcheckData.tokenlist is a list of Token items

    C++ class: https://cppcheck.sourceforge.io/devinfo/doxyoutput/classToken.html

    Attributes:
        str                Token string
        next               Next token in tokenlist. For last token, next is None.
        previous           Previous token in tokenlist. For first token, previous is None.
        link               Linked token in tokenlist. Each '(', '[' and '{' are linked to the
                           corresponding '}', ']' and ')'. For templates, the '<' is linked to
                           the corresponding '>'.
        scope              Scope information for this token. See the Scope class.
        type               Type information: name/op/number/string/..
        isName             Is this token a symbol name
        isUnsigned         Is this token a unsigned type
        isSigned           Is this token a signed type
        isNumber           Is this token a number, for example 123, 12.34
        isInt              Is this token a int value such as 1234
        isFloat            Is this token a float value such as 12.34
        isString           Is this token a string literal such as "hello"
        strlen             string length for string literal
        isChar             Is this token a char literal such as 'x'
        isBoolean          Is this token a boolean
        isOp               Is this token a operator
        isArithmeticalOp   Is this token a arithmetic operator
        isAssignmentOp     Is this token a assignment operator
        isComparisonOp     Is this token a comparison operator
        isLogicalOp        Is this token a logical operator: && ||
        isCast
        externLang
        isExpandedMacro    Is this token a expanded macro token
        macroName          Macro name that this token is expanded from
        isRemovedVoidParameter  Has void parameter been removed?
        isSplittedVarDeclComma  Is this a comma changed to semicolon in a split variable declaration ('int a,b;' => 'int a; int b;')
        isSplittedVarDeclEq     Is this a '=' changed to semicolon in a split variable declaration ('int a=5;' => 'int a; a=5;')
        isImplicitInt      Is this token an implicit "int"?
        isComplex
        isRestrict
        isAttributeExport
        isAnonymous
        varId              varId for token, each variable has a unique non-zero id
        exprId             exprId for token, each expression has a unique non-zero id
        variable           Variable information for this token. See the Variable class.
        function           If this token points at a function call, this attribute has the Function
                           information. See the Function class.
        values             Possible/Known values of token
        typeScope          type scope (token->type()->classScope)
        astParent          ast parent
        astOperand1        ast operand1
        astOperand2        ast operand2
        orriginalName      original name of the token
        valueType          type information: container/..
        file               file name
        linenr             line number
        column             column

    To iterate through all tokens use such code:
    @code
    data = cppcheckdata.parsedump(...)
    for cfg in data.configurations:
      code = ''
      for token in cfg.tokenlist:
        code = code + token.str + ' '
      print(code)
    @endcode
    """
    #tokenize.cpp/Tokenizer::dump

    Id = None
    str = None
    next = None
    previous = None
    linkId = None
    link = None
    scopeId = None
    scope = None
    isName = False
    isNumber = False
    isInt = False
    isFloat = False
    isString = False
    strlen = None
    isChar = False
    isBoolean = False
    isOp = False
    isArithmeticalOp = False
    isAssignmentOp = False
    isComparisonOp = False
    isLogicalOp = False
    isCast = False
    isUnsigned = False
    isSigned = False
    macroName = None
    isExpandedMacro = False
    isRemovedVoidParameter = False
    isSplittedVarDeclComma = False
    isSplittedVarDeclEq = False
    isImplicitInt = False
    isComplex = False
    isRestrict = False
    isAttributeExport = False
    isAnonymous = False
    exprId = None
    varId = None
    variableId = None
    variable = None
    functionId = None
    function = None
    valuesId = None
    values = None
    impossible_values = None
    valueType = None

    typeScopeId = None
    typeScope = None
    type = None

    astParentId = None
    astParent = None
    astOperand1Id = None
    astOperand1 = None
    astOperand2Id = None
    astOperand2 = None

    file = None
    linenr = None
    column = None

    def __init__(self, element):
        self.Id = element.get('id')
        self.str = element.get('str')
        self.next = None
        self.previous = None
        self.scopeId = element.get('scope')
        self.scope = None
        self.type = element.get('type')
        if self.type == 'name':
            self.isName = True
            if element.get('isUnsigned'):
                self.isUnsigned = True
            if element.get('isSigned'):
                self.isSigned = True
        elif self.type == 'number':
            self.isNumber = True
            if element.get('isInt'):
                self.isInt = True
            elif element.get('isFloat'):
                self.isFloat = True
        elif self.type == 'string':
            self.isString = True
            self.strlen = int(element.get('strlen'))
        elif self.type == 'char':
            self.isChar = True
        elif self.type == 'boolean':
            self.isBoolean = True
        elif self.type == 'op':
            self.isOp = True
            if element.get('isArithmeticalOp'):
                self.isArithmeticalOp = True
            elif element.get('isAssignmentOp'):
                self.isAssignmentOp = True
            elif element.get('isComparisonOp'):
                self.isComparisonOp = True
            elif element.get('isLogicalOp'):
                self.isLogicalOp = True
        if element.get('isCast'):
            self.isCast = True
        self.externLang = element.get('externLang')
        self.macroName = element.get('macroName')
        if self.macroName or element.get('isExpandedMacro'):
            self.isExpandedMacro = True
        if element.get('isRemovedVoidParameter'):
            self.isRemovedVoidParameter = True
        if element.get('isSplittedVarDeclComma'):
            self.isSplittedVarDeclComma = True
        if element.get('isSplittedVarDeclEq'):
            self.isSplittedVarDeclEq = True
        if element.get('isImplicitInt'):
            self.isImplicitInt = True
        if element.get('isComplex'):
            self.isComplex = True
        if element.get('isRestrict'):
            self.isRestrict = True
        if element.get('isAttributeExport'):
            self.isAttributeExport = True
        if element.get('isAnonymous'):
            self.isAnonymous = True
        self.linkId = element.get('link')
        self.link = None
        if element.get('varId'):
            self.varId = int(element.get('varId'))
        if element.get('exprId'):
            self.exprId = int(element.get('exprId'))
        self.variableId = element.get('variable')
        self.variable = None
        self.functionId = element.get('function')
        self.function = None
        self.valuesId = element.get('values')
        self.values = None
        self.typeScopeId = element.get('type-scope')
        self.typeScope = None
        self.astParentId = element.get('astParent')
        self.astParent = None
        self.astOperand1Id = element.get('astOperand1')
        self.astOperand1 = None
        self.astOperand2Id = element.get('astOperand2')
        self.astOperand2 = None
        self.originalName = element.get('originalName')
        if element.get('valueType-type'):
            self.valueType = ValueType(element)
        else:
            self.valueType = None
        _load_location(self, element)

    def __repr__(self):
        attrs = ["Id", "str", "scopeId", "isName", "isUnsigned", "isSigned",
                "isNumber", "isInt", "isFloat", "isString", "strlen",
                "isChar", "isBoolean", "isOp", "isArithmeticalOp", "isAssignmentOp", 
                "isComparisonOp", "isLogicalOp", "isCast", "externLang", "isExpandedMacro", 
                "isRemovedVoidParameter", "isSplittedVarDeclComma", "isSplittedVarDeclEq", 
                "isImplicitInt", "isComplex", "isRestrict", "isAttributeExport", "isAnonymous", "linkId",
                "varId", "variableId", "functionId", "valuesId", "valueType",
                "typeScopeId", "astParentId", "astOperand1Id", "file",
                "linenr", "column"]
        return "{}({})".format(
            "Token",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )

    def setId(self, IdMap):
        self.scope = IdMap[self.scopeId]
        self.link = IdMap[self.linkId]
        self.variable = IdMap[self.variableId]
        self.function = IdMap[self.functionId]
        self.values = []
        self.impossible_values = []
        if IdMap[self.valuesId]:
            for v in IdMap[self.valuesId]:
                if v.isImpossible():
                    self.impossible_values.append(v)
                else:
                    self.values.append(v)
                v.setId(IdMap)
        self.typeScope = IdMap[self.typeScopeId]
        self.astParent = IdMap[self.astParentId]
        self.astOperand1 = IdMap[self.astOperand1Id]
        self.astOperand2 = IdMap[self.astOperand2Id]
        if self.valueType:
            self.valueType.setId(IdMap)

    def getValue(self, v):
        """
        Get value if it exists
        Returns None if it doesn't exist
        """

        if not self.values:
            return None
        for value in self.values:
            if value.intvalue == v:
                return value
        return None

    def getKnownIntValue(self):
        """
        If token has a known int value then return that.
        Otherwise returns None
        """
        if not self.values:
            return None
        for value in self.values:
            if value.valueKind == 'known':
                return value.intvalue
        return None

    def isUnaryOp(self, op):
        return self.astOperand1 and (self.astOperand2 is None) and self.str == op

    def isBinaryOp(self):
        return self.astOperand1 and self.astOperand2

    def forward(self, end=None):
        token = self
        while token and token != end:
            yield token
            token = token.next

    def backward(self, start=None):
        token = self
        while token and token != start:
            yield token
            token = token.previous

    def astParents(self):
        token = self
        while token and token.astParent:
            token = token.astParent
            yield token

    def astTop(self):
        top = None
        for parent in self.astParents():
            top = parent
        return top

    def tokAt(self, n):
        tl = self.forward()
        if n < 0:
            tl = self.backward()
            n = -n
        for i, t in enumerate(tl):
            if i == n:
                return t
        return None

    def linkAt(self, n):
        token = self.tokAt(n)
        if token:
            return token.link
        return None

class Scope:
    """
    Scope. Information about global scope, function scopes, class scopes, inner scopes, etc.
    C++ class: https://cppcheck.sourceforge.io/devinfo/doxyoutput/classScope.html

    Attributes
        bodyStart      The { Token for this scope
        bodyEnd        The } Token for this scope
        className      Name of this scope.
                       For a function scope, this is the function name;
                       For a class scope, this is the class name.
        function       If this scope belongs at a function call, this attribute
                       has the Function information. See the Function class.
        functions      if this is a Class type, it may have functions defined
        nestedIn
        type           Type of scope: Function, If/Else/For/While/Switch/Global/Enum/Struct/Namespace/Class/Constructor/Destructor
        isExecutable   True when the type is: Function/If/Else/For/While/Do/Switch/Try/Catch/Unconditional/Lambda
        definedType
    """
    #symboldatabase.cpp/SymbolDatabase::printXml

    Id = None
    bodyStartId = None
    bodyStart = None
    bodyEndId = None
    bodyEnd = None
    className = None
    functionId = None
    function = None
    nestedInId = None
    nestedIn = None
    nestedList = None
    type = None
    isExecutable = None
    varlistId = None
    varlist = None

    def __init__(self, element):
        self.Id = element.get('id')
        self.className = element.get('className')
        self.functionId = element.get('function')
        self.function = None
        self.functions = []
        self.bodyStartId = element.get('bodyStart')
        self.bodyStart = None
        self.bodyEndId = element.get('bodyEnd')
        self.bodyEnd = None
        self.nestedInId = element.get('nestedIn')
        self.nestedIn = None
        self.nestedList = []
        self.type = element.get('type')
        self.definedType = element.get('definedType')
        self.isExecutable = (self.type in ('Function', 'If', 'Else', 'For', 'While', 'Do',
                                           'Switch', 'Try', 'Catch', 'Unconditional', 'Lambda'))

        self.varlistId = []
        self.varlist = []

    def __repr__(self):
        attrs = ["Id", "className", "functionId", "bodyStartId", "bodyEndId",
                 "nestedInId", "nestedIn", "type", "definedType", "isExecutable", "functions"]
        return "{}({})".format(
            "Scope",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )

    def setId(self, IdMap):
        self.bodyStart = IdMap[self.bodyStartId]
        self.bodyEnd = IdMap[self.bodyEndId]
        self.nestedIn = IdMap[self.nestedInId]
        if self.nestedIn:
            self.nestedIn.nestedList.append(self)
        self.function = IdMap[self.functionId]
        for v in self.varlistId:
            value = IdMap.get(v)
            if value:
                self.varlist.append(value)


class Function:
    """
    Information about a function
    C++ class:
    https://cppcheck.sourceforge.io/devinfo/doxyoutput/classFunction.html

    Attributes
        argument                Argument list (dict of argument number and variable)
        token                   Token in function implementation
        tokenDef                Token in function definition
        name
        type                    Constructor/CopyConstructor/MoveConstructor/OperatorEqual/Destructor/Function/Lambda/Unknown
        hasVirtualSpecifier     Is this function is virtual
        isImplicitlyVirtual     Is this function is virtual this in the base classes
        access                  Public/Protected/Private
        isInlineKeyword         Is inline keyword used
        isStatic                Is this function static
        isAttributeNoreturn
        overriddenFunction
    """
    #symboldatabase.cpp/SymbolDatabase::printXml

    Id = None
    argument = None
    argumentId = None
    token = None
    tokenId = None
    tokenDef = None
    tokenDefId = None
    name = None
    type = None
    access = None
    isImplicitlyVirtual = None
    hasVirtualSpecifier = None
    isInlineKeyword = None
    isStatic = None
    isAttributeNoreturn = None
    overriddenFunction = None
    nestedIn = None

    def __init__(self, element, nestedIn):
        self.Id = element.get('id')
        self.tokenId = element.get('token')
        self.tokenDefId = element.get('tokenDef')
        self.name = element.get('name')
        self.type = element.get('type')
        self.hasVirtualSpecifier = element.get('hasVirtualSpecifier', 'false') == 'true'
        self.isImplicitlyVirtual = element.get('isImplicitlyVirtual', 'false') == 'true'
        self.access = element.get('access')
        self.isInlineKeyword = element.get('isInlineKeyword', 'false') == 'true'
        self.isStatic = element.get('isStatic', 'false') == 'true'
        self.isAttributeNoreturn = element.get('isAttributeNoreturn', 'false') == 'true'
        self.overriddenFunction = element.get('overriddenFunction', 'false') == 'true'
        self.nestedIn = nestedIn

        self.argument = {}
        self.argumentId = {}

    def __repr__(self):
        attrs = ["Id", "tokenId", "tokenDefId", "name", "type", "hasVirtualSpecifier",
                 "isImplicitlyVirtual", "access", "isInlineKeyword", "isStatic", 
                 "isAttributeNoreturn", "overriddenFunction", "nestedIn", "argumentId"]
        return "{}({})".format(
            "Function",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )

    def setId(self, IdMap):
        for argnr, argid in self.argumentId.items():
            self.argument[argnr] = IdMap[argid]
        self.token = IdMap.get(self.tokenId, None)
        self.tokenDef = IdMap[self.tokenDefId]


#todo add class Types:
    #symboldatabase.cpp/SymbolDatabase::printXml


class Variable:
    """
    Information about a variable
    C++ class:
    https://cppcheck.sourceforge.io/devinfo/doxyoutput/classVariable.html

    Attributes:
        nameToken       Name token in variable declaration
        typeStartToken  Start token of variable declaration
        typeEndToken    End token of variable declaration
        access          Global/Local/Namespace/Public/Protected/Public/Throw/Argument/Unknown
        scope           Variable scope
        constness       Variable constness (same encoding as ValueType::constness)
        isArgument      Is this variable a function argument?
        isGlobal        Is this variable a global variable?
        isLocal         Is this variable a local variable?
        isArray         Is this variable an array?
        isClass         Is this variable a class or struct?
        isConst         Is this variable a const variable?
        isExtern        Is this variable an extern variable?
        isPointer       Is this variable a pointer
        isReference     Is this variable a reference
        isStatic        Is this variable static?
        isVolatile      Is this variable volatile?
    """
    #symboldatabase.cpp/SymbolDatabase::printXml

    Id = None
    nameTokenId = None
    nameToken = None
    typeStartTokenId = None
    typeStartToken = None
    typeEndTokenId = None
    typeEndToken = None
    access = None
    scopeId = None
    scope = None
    isArgument = False
    isArray = False
    isClass = False
    isConst = False
    isExtern = False
    isGlobal = False
    isLocal = False
    isPointer = False
    isReference = False
    isStatic = False
    isVolatile = False
    constness = 0

    def __init__(self, element):
        self.Id = element.get('id')
        self.nameTokenId = element.get('nameToken')
        self.nameToken = None
        self.typeStartTokenId = element.get('typeStartToken')
        self.typeStartToken = None
        self.typeEndTokenId = element.get('typeEndToken')
        self.typeEndToken = None
        self.access = element.get('access')
        self.isArgument = (self.access and self.access == 'Argument')
        self.isGlobal = (self.access and self.access == 'Global')
        self.isLocal = (self.access and self.access == 'Local')
        self.scopeId = element.get('scope')
        self.scope = None
        self.constness = int(element.get('constness',0))
        self.isArray = element.get('isArray') == 'true'
        self.isClass = element.get('isClass') == 'true'
        self.isConst = element.get('isConst') == 'true'
        self.isExtern = element.get('isExtern') == 'true'
        self.isPointer = element.get('isPointer') == 'true'
        self.isReference = element.get('isReference') == 'true'
        self.isStatic = element.get('isStatic') == 'true'
        self.isVolatile = element.get('isVolatile') == 'true'

    def __repr__(self):
        attrs = ["Id", "nameTokenId", "typeStartTokenId", "typeEndTokenId",
                 "access", "scopeId", "isArgument", "isArray", "isClass",
                 "isConst", "isGlobal", "isExtern", "isLocal", "isPointer",
                 "isReference", "isStatic", "isVolatile", "constness"]
        return "{}({})".format(
            "Variable",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )

    def setId(self, IdMap):
        self.nameToken = IdMap[self.nameTokenId]
        self.typeStartToken = IdMap[self.typeStartTokenId]
        self.typeEndToken = IdMap[self.typeEndTokenId]
        self.scope = IdMap[self.scopeId]

class Container:
    """
    Container class -- information about containers

    Attributes:
        array-like-index-op true/false
        stdStringLike       true/false
    """
    #tokenizer.cpp/tokenizer::dump
    Id = None

    def __init__(self, element):
        self.Id = element.get('id')
        self.arrayLikeIndexOp = element.get('array-like-index-op') == 'true'
        self.stdStringLike = element.get('std-string-like') == 'true'

class TypedefInfo:
    """
    TypedefInfo class -- information about typedefs

    Attributes:
        name   name of the typedef
        used   0/1
    """
    #tokenizer.cpp/tokenizer::dump

    name = None
    used = None
    file = None
    linenr = None
    column = None

    def __init__(self, element):
        self.name = element.get('name')
        _load_location(self, element)
        self.used = (element.get('used') == '1')

class Value:
    """
    Value class

    Attributes:
        intvalue         integer value
        tokvalue         token value
        floatvalue       float value
        movedvalue
        uninit
        containerSize    container size
        bufferSize       buffer size
        lifetimeScope    Local/Argument/SubFunction/ThisPointer/ThisValue
        lifetimeKind     Object/SubObject/Lambda/Iterator/Address
        symbolicDelta
        condition        condition where this Value comes from
        bound            Upper/Lower/Point
        valueKind        known/possible/impossible/inconclusive
        path             0/1/2/3/..
    """
    #token.cpp/token::printValueFlow

    intvalue = None
    tokvalue = None
    floatvalue = None
    containerSize = None
    condition = None
    valueKind = None

    def isKnown(self):
        return self.valueKind and self.valueKind == 'known'

    def isPossible(self):
        return self.valueKind and self.valueKind == 'possible'

    def isImpossible(self):
        return self.valueKind and self.valueKind == 'impossible'

    def isInconclusive(self):
        return self.valueKind and self.valueKind == 'inconclusive'

    def __init__(self, element):
        self.intvalue = element.get('intvalue')
        if self.intvalue:
            self.intvalue = int(self.intvalue)
        self._tokvalueId = element.get('tokvalue')
        self.floatvalue = element.get('floatvalue')
        self.movedvalue = element.get('movedvalue')
        self.uninit = element.get('uninit')
        self.bufferSize = element.get('buffer-size')
        self.containerSize = element.get('container-size')
        self.iteratorStart = element.get('iterator-start')
        self.iteratorEnd = element.get('iterator-end')
        self._lifetimeId = element.get('lifetime')
        self.lifetimeScope = element.get('lifetime-scope')
        self.lifetimeKind = element.get('lifetime-kind')
        self._symbolicId = element.get('symbolic')
        self.symbolicDelta = element.get('symbolic-delta')
        self.bound = element.get('bound')
        self.condition = element.get('condition-line')
        if self.condition:
            self.condition = int(self.condition)
        if element.get('known'):
            self.valueKind = 'known'
        elif element.get('possible'):
            self.valueKind = 'possible'
        elif element.get('impossible'):
            self.valueKind = 'impossible'
        elif element.get('inconclusive'):
            self.valueKind = 'inconclusive'
        self.path = element.get('path')

    def setId(self, IdMap):
        self.tokvalue = IdMap.get(self._tokvalueId)
        self.lifetime = IdMap.get(self._lifetimeId)
        self.symbolic = IdMap.get(self._symbolicId)

    def __repr__(self):
        attrs = ["intvalue", "tokvalue", "floatvalue", "movedvalue", "uninit",
                 "bufferSize", "containerSize", "condition", "valueKind"]
        return "{}({})".format(
            "Value",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )


class ValueFlow:
    """
    ValueFlow::Value class
    Each possible value has a ValueFlow::Value item.
    Each ValueFlow::Value either has a intvalue or tokvalue
    C++ class:
    https://cppcheck.sourceforge.io/devinfo/doxyoutput/classValueFlow_1_1Value.html

    Attributes:
        values    Possible values
    """

    Id = None
    values = None

    def __init__(self, element):
        self.Id = element.get('id')
        self.values = []

    def __repr__(self):
        attrs = ["Id", "values"]
        return "{}({})".format(
            "ValueFlow",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )


class Suppression:
    """
    Suppression class
    This class contains a suppression entry to suppress a warning.

    Attributes
      errorId     The id string of the error to suppress, can be a wildcard
      fileName    The name of the file to suppress warnings for, can include wildcards
      lineNumber  The number of the line to suppress warnings from, can be 0 to represent any line
      symbolName  The name of the symbol to match warnings for, can include wildcards
      lineBegin   The first line to suppress warnings from
      lineEnd     The last line to suppress warnings from
      suppressionType   The type of suppression which is applied (unique = None (default), file, block, blockBegin, blockEnd, macro)
    """

    errorId = None
    fileName = None
    lineNumber = None
    symbolName = None
    lineBegin = None
    lineEnd = None
    suppressionType = None

    def __init__(self, element):
        self.errorId = element.get('errorId')
        self.fileName = element.get('fileName')
        self.lineNumber = element.get('lineNumber')
        self.symbolName = element.get('symbolName')
        self.lineBegin = element.get('lineBegin')
        self.lineEnd = element.get('lineEnd')
        self.suppressionType = element.get('type')

    def __repr__(self):
        attrs = ["errorId", "fileName", "lineNumber", "symbolName", "lineBegin", "lineEnd","suppressionType"]
        return "{}({})".format(
            "Suppression",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )

    def isMatch(self, file, line, message, errorId):
        # Line Suppression
        if ((self.fileName is None or fnmatch(file, self.fileName))
                and (self.suppressionType is None) # Verify use of default suppression type (None = unique)
                and (self.lineNumber is not None and int(line) == int(self.lineNumber))
                and (self.symbolName is None or fnmatch(message, '*'+self.symbolName+'*'))
                and fnmatch(errorId, self.errorId)):
            return True
        # File Suppression
        if ((self.fileName is None or fnmatch(file, self.fileName))
                and (self.suppressionType is not None and self.suppressionType == "file") # Verify use of file (global) suppression type
                and (self.symbolName is None or fnmatch(message, '*'+self.symbolName+'*'))
                and fnmatch(errorId, self.errorId)):
            return True
        # Block Suppression Mode
        if ((self.fileName is None or fnmatch(file, self.fileName))
                and (self.suppressionType is not None and self.suppressionType == "block") # Type for Block suppression
                and (self.lineBegin is not None and int(line) > int(self.lineBegin)) # Code Match is between the Block suppression
                and (self.lineEnd is not None and int(line) < int(self.lineEnd)) # Code Match is between the Block suppression
                and (self.symbolName is None or fnmatch(message, '*'+self.symbolName+'*'))
                and fnmatch(errorId, self.errorId)):
            return True
        # Other Suppression (Globally set via suppression file or cli command)
        if ((self.fileName is None or fnmatch(file, self.fileName))
                and (self.suppressionType is None)
                and (self.symbolName is None or fnmatch(message, '*'+self.symbolName+'*'))
                and fnmatch(errorId, self.errorId)):
            return True
        return False


class Configuration:
    """
    Configuration class
    This class contains the directives, tokens, scopes, functions,
    variables, value flows, and suppressions for one configuration.

    Attributes:
        name          Name of the configuration, "" for default
        directives    List of Directive items
        macro_usage   List of used macros
        preprocessor_if_conditions  List of preprocessor if conditions that was evaluated during preprocessing
        tokenlist     List of Token items
        scopes        List of Scope items
        containers    List of Container items
        functions     List of Function items
        variables     List of Variable items
        valueflow     List of ValueFlow values
        standards     List of Standards values
    """

    name = ''
    directives = []
    macro_usage = []
    preprocessor_if_conditions = []
    tokenlist = []
    scopes = []
    containers = []
    functions = []
    variables = []
    typedefInfo = []
    valueflow = []
    standards = None
    clang_warnings = []

    def __init__(self, name):
        self.name = name
        self.directives = []
        self.macro_usage = []
        self.preprocessor_if_conditions = []
        self.tokenlist = []
        self.scopes = []
        self.containers = []
        self.functions = []
        self.variables = []
        self.typedefInfo = []
        self.valueflow = []
        self.standards = Standards()
        self.clang_warnings = []

    def set_tokens_links(self):
        """Set next/previous links between tokens."""
        prev = None
        for token in self.tokenlist:
            token.previous = prev
            if prev:
                prev.next = token
            prev = token

    def set_id_map(self, arguments):
        IdMap = {None: None, '0': None, '00000000': None, '0000000000000000': None, '0x0': None}
        for token in self.tokenlist:
            IdMap[token.Id] = token
        for scope in self.scopes:
            IdMap[scope.Id] = scope
        for container in self.containers:
            IdMap[container.Id] = container
        for function in self.functions:
            IdMap[function.Id] = function
        for variable in self.variables:
            IdMap[variable.Id] = variable
        for variable in arguments:
            IdMap[variable.Id] = variable
        for values in self.valueflow:
            IdMap[values.Id] = values.values
        for token in self.tokenlist:
            token.setId(IdMap)
        for scope in self.scopes:
            scope.setId(IdMap)
        #for container in self.containers:
        #    container.setId(IdMap)
        for function in self.functions:
            function.setId(IdMap)
        for variable in self.variables:
            variable.setId(IdMap)
        for variable in arguments:
            variable.setId(IdMap)

    def setIdMap(self, functions_arguments):
        """Set relationships between objects stored in this configuration.
        :param functions_arguments: List of Variable objects which are function arguments
        """
        self.set_tokens_links()
        self.set_id_map(functions_arguments)


class Platform:
    """
    Platform class
    This class contains type sizes

    Attributes:
        name          Name of the platform: unspecified/native/win32A/win32W/win64/unix32/unix64/platformFile
        char_bit      CHAR_BIT value
        short_bit     SHORT_BIT value
        int_bit       INT_BIT value
        long_bit      LONG_BIT value
        long_long_bit LONG_LONG_BIT value
        pointer_bit   POINTER_BIT value
    """

    name = ''
    char_bit = 0
    short_bit = 0
    int_bit = 0
    long_bit = 0
    long_long_bit = 0
    pointer_bit = 0

    def __init__(self, platformnode):
        self.name = platformnode.get('name')
        self.char_bit = int(platformnode.get('char_bit'))
        self.short_bit = int(platformnode.get('short_bit'))
        self.int_bit = int(platformnode.get('int_bit'))
        self.long_bit = int(platformnode.get('long_bit'))
        self.long_long_bit = int(platformnode.get('long_long_bit'))
        self.pointer_bit = int(platformnode.get('pointer_bit'))

    def __repr__(self):
        attrs = ["name", "char_bit", "short_bit", "int_bit",
                 "long_bit", "long_long_bit", "pointer_bit"]
        return "{}({})".format(
            "Platform",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )


class Standards:
    """
    Standards class
    This class contains versions of standards that were used for the cppcheck

    Attributes:
        c            C Standard used
        cpp          C++ Standard used
        posix        If Posix was used
    """

    c = ""
    cpp = ""
    posix = False

    def set_c(self, node):
        self.c = node.get("version")

    def set_cpp(self, node):
        self.cpp = node.get("version")

    def set_posix(self, node):
        self.posix = node.get("posix") is not None

    def __repr__(self):
        attrs = ["c", "cpp", "posix"]
        return "{}({})".format(
            "Standards",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )


class CppcheckData:
    """
    Class that makes cppcheck dump data available
    Contains a list of Configuration instances

    Attributes:
        filename          Path to Cppcheck dump file
        rawTokens         List of rawToken elements
        suppressions      List of Suppressions
        files             Source files for elements occurred in this configuration

    To iterate through all configurations use such code:
    @code
    data = cppcheckdata.parsedump(...)
    for cfg in data.configurations:
        print('cfg: ' + cfg.name)
    @endcode

    To iterate through all tokens in each configuration use such code:
    @code
    data = cppcheckdata.parsedump(...)
    for cfg in data.configurations:
        print('cfg: ' + cfg.name)
        code = ''
            for token in cfg.tokenlist:
                code = code + token.str + ' '
        print('    ' + code)
    @endcode

    To iterate through all scopes (functions, types, etc) use such code:
    @code
    data = cppcheckdata.parsedump(...)
    for cfg in data.configurations:
        print('cfg: ' + cfg.name)
        for scope in cfg.scopes:
            print('    type:' + scope.type + ' name:' + scope.className)
    @endcode
    """

    def __init__(self, filename):
        """
        :param filename: Path to Cppcheck dump file
        """
        self.language = None
        self.filename = filename
        self.rawTokens = []
        self.platform = None
        self.suppressions = []
        self.files = [] # source files for elements occurred in this configuration

        platform_done = False
        rawtokens_done = False
        suppressions_done = False

        # Parse general configuration options from <dumps> node
        # We intentionally don't clean node resources here because we
        # want to serialize in memory only small part of the XML tree.
        for event, node in ElementTree.iterparse(self.filename, events=('start', 'end')):
            if platform_done and rawtokens_done and suppressions_done:
                break
            if node.tag == 'dumps':
                self.language = node.get('language')
            if node.tag == 'platform' and event == 'start':
                self.platform = Platform(node)
                platform_done = True
            elif node.tag == 'rawtokens' and event == 'end':
                for rawtokens_node in node:
                    if rawtokens_node.tag == 'file':
                        self.files.append(rawtokens_node.get('name'))
                    elif rawtokens_node.tag == 'tok':
                        tok = Token(rawtokens_node)
                        tok.file = self.files[int(rawtokens_node.get('fileIndex'))]
                        self.rawTokens.append(tok)
                rawtokens_done = True
            elif node.tag == 'suppressions' and event == 'end':
                for suppressions_node in node:
                    self.suppressions.append(Suppression(suppressions_node))
                suppressions_done = True

        global current_dumpfile_suppressions
        current_dumpfile_suppressions = self.suppressions

        # Set links between rawTokens.
        for i in range(len(self.rawTokens)-1):
            self.rawTokens[i+1].previous = self.rawTokens[i]
            self.rawTokens[i].next = self.rawTokens[i+1]

    @property
    def configurations(self):
        """
        Return the list of all available Configuration objects.
        """
        return list(self.iterconfigurations())

    def iterconfigurations(self):
        """
        Create and return iterator for the available Configuration objects.
        The iterator loops over all Configurations in the dump file tree, in document order.
        """
        cfg = None
        cfg_arguments = []  # function arguments for Configuration node initialization
        cfg_function = None
        cfg_valueflow = None

        # Iterating <varlist> in a <scope>.
        iter_scope_varlist = False

        # Iterating <typedef-info>
        iter_typedef_info = False

        # Iterating <directive>
        iter_directive = False

        # Use iterable objects to traverse XML tree for dump files incrementally.
        # Iterative approach is required to avoid large memory consumption.
        # Calling .clear() is necessary to let the element be garbage collected.
        for event, node in ElementTree.iterparse(self.filename, events=('start', 'end')):
            # Serialize new configuration node
            if node.tag == 'dump':
                if event == 'start':
                    cfg = Configuration(node.get('cfg'))
                    continue
                if event == 'end':
                    cfg.setIdMap(cfg_arguments)
                    yield cfg
                    cfg = None
                    cfg_arguments = []

            elif node.tag == 'clang-warning' and event == 'start':
                cfg.clang_warnings.append({'file': node.get('file'),
                                           'line': int(node.get('line')),
                                           'column': int(node.get('column')),
                                           'message': node.get('message')})
            # Parse standards
            elif node.tag == "standards" and event == 'start':
                continue
            elif node.tag == 'c' and event == 'start':
                cfg.standards.set_c(node)
            elif node.tag == 'cpp' and event == 'start':
                cfg.standards.set_cpp(node)
            elif node.tag == 'posix' and event == 'start':
                cfg.standards.set_posix(node)

            # Parse directives list
            elif node.tag == 'directive':
                if event == 'start':
                    cfg.directives.append(Directive(node))
                    iter_directive = True
                elif event == 'end':
                    iter_directive = False
            # Parse macro usage
            elif node.tag == 'macro' and event == 'start':
                cfg.macro_usage.append(MacroUsage(node))

            # Preprocessor #if/#elif condition
            elif node.tag == "if-cond" and event == 'start':
                cfg.preprocessor_if_conditions.append(PreprocessorIfCondition(node))

            # Parse tokens
            elif node.tag == 'tokenlist' and event == 'start':
                continue
            elif node.tag == 'token' and event == 'start' and not iter_directive and not iter_typedef_info:
                cfg.tokenlist.append(Token(node))

            # Parse scopes
            elif node.tag == 'scopes' and event == 'start':
                continue
            elif node.tag == 'scope' and event == 'start':
                cfg.scopes.append(Scope(node))
            elif node.tag == 'varlist':
                if event == 'start':
                    iter_scope_varlist = True
                elif event == 'end':
                    iter_scope_varlist = False

            # Parse functions
            elif node.tag == 'functionList' and event == 'start':
                continue
            elif node.tag == 'function':
                if event == 'start':
                    cfg_function = Function(node, cfg.scopes[-1])
                    continue
                if event == 'end':
                    cfg.functions.append(cfg_function)
                    cfg_function = None

            # Parse function arguments
            elif node.tag == 'arg' and event == 'start':
                arg_nr = int(node.get('nr'))
                arg_variable_id = node.get('variable')
                cfg_function.argumentId[arg_nr] = arg_variable_id

            # Parse variables
            elif node.tag == 'var' and event == 'start':
                if iter_scope_varlist:
                    cfg.scopes[-1].varlistId.append(node.get('id'))
                else:
                    var = Variable(node)
                    if var.nameTokenId:
                        cfg.variables.append(var)
                    else:
                        cfg_arguments.append(var)

            # Parse containers
            elif node.tag == 'containers' and event == 'start':
                continue
            elif node.tag == 'container' and event == 'start':
                cfg.containers.append(Container(node))

            # Parse typedef info
            elif node.tag == 'typedef-info':
                iter_typedef_info = (event == 'start')
            elif iter_typedef_info and node.tag == 'info' and event == 'start':
                cfg.typedefInfo.append(TypedefInfo(node))

            # Parse template-token
            #elif node.tag == 'TokenAndName' and event == 'start': #todo add processing of containers
            #    cfg.containers.append(Container(node))

            # Parse valueflows (list of values)
            elif node.tag == 'valueflow' and event == 'start':
                continue
            elif node.tag == 'values':
                if event == 'start':
                    cfg_valueflow = ValueFlow(node)
                    continue
                if event == 'end':
                    cfg.valueflow.append(cfg_valueflow)
                    cfg_valueflow = None

            # Parse values
            elif node.tag == 'value' and event == 'start':
                cfg_valueflow.values.append(Value(node))

            # Remove links to the sibling nodes
            node.clear()

    def __repr__(self):
        attrs = ["configurations", "platform"]
        return "{}({})".format(
            "CppcheckData",
            ", ".join(f"{a}={repr(getattr(self, a))}" for a in attrs)
        )


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
    if (not ftok.isName) or (ftok.next is None) or ftok.next.str != '(':
        return None
    args = []
    getArgumentsRecursive(ftok.next.astOperand2, args)
    return args


def parsedump(filename):
    """
    parse a cppcheck dump file
    """
    return CppcheckData(filename)


def astIsFloat(token):
    """
    Check if type of ast node is float/double
    """

    if not token:
        return False
    if token.str == '.':
        return astIsFloat(token.astOperand2)
    if token.str in '+-*/%':
        return astIsFloat(token.astOperand1) or astIsFloat(token.astOperand2)
    if not token.variable:
        # float literal?
        if token.str[0].isdigit():
            for c in token.str:
                if c == 'f' or c == '.' or c == 'E':
                    return True
        return False
    typeToken = token.variable.typeStartToken
    endToken = token.variable.typeEndToken
    while typeToken != endToken:
        if typeToken.str == 'float' or typeToken.str == 'double':
            return True
        typeToken = typeToken.next
    if typeToken.str == 'float' or typeToken.str == 'double':
        return True
    return False


class CppCheckFormatter(argparse.HelpFormatter):
    """
    Properly formats multiline argument helps
    """
    def _split_lines(self, text, width):
        # this is the RawTextHelpFormatter._split_lines
        if text.startswith('R|'):
            return text[2:].splitlines()
        return argparse.HelpFormatter._split_lines(self, text, width)


def ArgumentParser():
    """
    Returns an argparse argument parser with an already-added
    argument definition for -t/--template
    """
    parser = argparse.ArgumentParser(formatter_class=CppCheckFormatter)
    parser.add_argument('-t', '--template', metavar='<text>',
                        default='{callstack}: ({severity}) {message}',
                        help="R|Format the error messages. E.g.\n"
                        "'{file}:{line},{severity},{id},{message}' or\n"
                        "'{file}({line}):({severity}) {message}' or\n"
                        "'{callstack} {message}'\n"
                        "Pre-defined templates: gcc, vs, edit")
    parser.add_argument("dumpfile", nargs='*',
                        help="Path of dump files from cppcheck.")
    parser.add_argument("--cli",
                        help="Addon is executed from Cppcheck",
                        action="store_true")
    parser.add_argument("--file-list", metavar='<text>',
                        default=None,
                        help="file list in a text file")
    parser.add_argument("-q", "--quiet",
                        help='do not print "Checking ..." lines',
                        action="store_true")
    return parser


def get_files(args):
    """Return dump_files, ctu_info_files"""
    all_files = args.dumpfile
    if args.file_list:
        with open(args.file_list) as f:
            for line in f.readlines():
                all_files.append(line.rstrip())
    dump_files = []
    ctu_info_files = []
    for f in all_files:
        if f.endswith('.ctu-info'):
            ctu_info_files.append(f)
        else:
            dump_files.append(f)
    return dump_files, ctu_info_files

def simpleMatch(token, pattern):
    for p in pattern.split(' '):
        if not token or token.str != p:
            return False
        token = token.next
    return True

patterns = {
    '%any%': lambda tok: tok,
    '%assign%': lambda tok: tok if tok.isAssignmentOp else None,
    '%comp%': lambda tok: tok if tok.isComparisonOp else None,
    '%name%': lambda tok: tok if tok.isName else None,
    '%op%': lambda tok: tok if tok.isOp else None,
    '%or%': lambda tok: tok if tok.str == '|' else None,
    '%oror%': lambda tok: tok if tok.str == '||' else None,
    '%var%': lambda tok: tok if tok.variable else None,
    '(*)': lambda tok: tok.link if tok.str == '(' else None,
    '[*]': lambda tok: tok.link if tok.str == '[' else None,
    '{*}': lambda tok: tok.link if tok.str == '{' else None,
    '<*>': lambda tok: tok.link if tok.str == '<' and tok.link else None,
}

def match_atom(token, p):
    if not token:
        return None
    if not p:
        return None
    if token.str == p:
        return token
    if p in ['!', '|', '||', '%', '!=', '*']:
        return None
    if p in patterns:
        return patterns[p](token)
    if '|' in p:
        for x in p.split('|'):
            t = match_atom(token, x)
            if t:
                return t
    elif p.startswith('!!'):
        t = match_atom(token, p[2:])
        if not t:
            return token
    elif p.startswith('**'):
        a = p[2:]
        t = token
        while t:
            if match_atom(t, a):
                return t
            if t.link and t.str in ['(', '[', '<', '{']:
                t = t.link
            t = t.next
    return None

class MatchResult:
    def __init__(self, matches, bindings=None, keys=None):
        self.__dict__.update(bindings or {})
        self._matches = matches
        self._keys = keys or []

    def __bool__(self):
        return self._matches

    def __nonzero__(self):
        return self._matches

    def __getattr__(self, k):
        if k in self._keys:
            return None
        raise AttributeError

def bind_split(s):
    if '@' in s:
        p = s.partition('@')
        return (p[0], p[2])
    return (s, None)

def match(token, pattern):
    if not pattern:
        return MatchResult(False)
    end = None
    bindings = {}
    words = [bind_split(word) for word in pattern.split()]
    for p, b in words:
        t = match_atom(token, p)
        if b:
            bindings[b] = token
        if not t:
            return MatchResult(False, keys=[xx for pp, xx in words]+['end'])
        end = t
        token = t.next
    bindings['end'] = end
    return MatchResult(True, bindings=bindings)

def get_function_call_name_args(token):
    """Get function name and arguments for function call
    name, args = get_function_call_name_args(tok)
    """
    if token is None:
        return None, None
    if not token.isName or not token.scope.isExecutable:
        return None, None
    if not simpleMatch(token.next, '('):
        return None, None
    if token.function:
        nametok = token.function.token
        if nametok is None:
            nametok = token.function.tokenDef
        if token in (token.function.token, token.function.tokenDef):
            return None, None
        name = nametok.str
        while nametok.previous and nametok.previous.previous and nametok.previous.str == '::' and nametok.previous.previous.isName:
            name = nametok.previous.previous.str + '::' + name
            nametok = nametok.previous.previous
        scope = token.function.nestedIn
        while scope:
            if scope.className:
                name = scope.className + '::' + name
            scope = scope.nestedIn
    else:
        nametok = token
        name = nametok.str
        while nametok.previous and nametok.previous.previous and nametok.previous.str == '::' and nametok.previous.previous.isName:
            name = nametok.previous.previous.str + '::' + name
            nametok = nametok.previous.previous
    return name, getArguments(token)

def is_suppressed(location, message, errorId):
    for suppression in current_dumpfile_suppressions:
        if suppression.isMatch(location.file, location.linenr, message, errorId):
            return True
    return False

def log_checker(message, addon):
    if '--cli' in sys.argv:
        msg = { 'addon': addon,
                'severity': 'none',
                'message': message,
                'errorId': 'logChecker'}
        sys.stdout.write(json.dumps(msg) + '\n')

def reportError(location, severity, message, addon, errorId, extra='', columnOverride=None):
    if '--cli' in sys.argv:
        msg = { 'file': location.file,
                'linenr': location.linenr,
                'column': location.column if columnOverride is None else columnOverride,
                'severity': severity,
                'message': message,
                'addon': addon,
                'errorId': errorId,
                'extra': extra}
        sys.stdout.write(json.dumps(msg) + '\n')
    else:
        if is_suppressed(location, message, '{}-{}'.format(addon, errorId)):
            return
        loc = '[%s:%i]' % (location.file, location.linenr)
        if len(extra) > 0:
            message += ' (' + extra + ')'
        sys.stderr.write('{} ({}) {} [{}-{}]\n'.format(loc, severity, message, addon, errorId))
        global EXIT_CODE
        EXIT_CODE = 1

def reportSummary(dumpfile, summary_type, summary_data):
    msg = {'summary': summary_type, 'data': summary_data}
    if '--cli' in sys.argv:
        sys.stdout.write(json.dumps(msg) + '\n')
    else:
        # dumpfile ends with ".dump"
        ctu_info_file = dumpfile[:-4] + "ctu-info"
        with open(ctu_info_file, 'a') as f:
            f.write(json.dumps(msg) + '\n')


def get_path_premium_addon():
    p = pathlib.Path(sys.argv[0]).parent.parent

    for ext in ('.exe', ''):
        p1 = os.path.join(p, 'premiumaddon' + ext)
        p2 = os.path.join(p, 'cppcheck' + ext)
        if os.path.isfile(p1) and os.path.isfile(p2):
            return p1
    return None


def cmd_output(cmd):
    with subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as p:
        stdout, stderr = p.communicate()
        rc = p.returncode
    out = stdout
    if rc == 1 and len(stderr) > 2:
        out = stderr
    return out.decode(encoding='utf-8', errors='ignore')
