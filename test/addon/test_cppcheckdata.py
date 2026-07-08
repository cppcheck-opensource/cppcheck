"""Unit tests for addons/cppcheckdata.py

Most tests parse dump files generated with the cppcheck binary
(see conftest.py); tests of pure helper classes construct the
objects directly from dicts (which provide the same .get() API
as the XML elements).
"""
import json
import sys

import pytest

import cppcheckdata


def find_tokens(cfg, token_str):
    return [tok for tok in cfg.tokenlist if tok.str == token_str]


def find_token(cfg, token_str, skip=0):
    tokens = find_tokens(cfg, token_str)
    assert len(tokens) > skip, "token '%s' (skip=%d) not found" % (token_str, skip)
    return tokens[skip]


def find_function(cfg, name):
    for function in cfg.functions:
        if function.name == name:
            return function
    assert False, "function '%s' not found" % name


def find_scope(cfg, scope_type, className=None):
    for scope in cfg.scopes:
        if scope.type == scope_type and (className is None or scope.className == className):
            return scope
    assert False, "scope '%s' not found" % scope_type


def find_variable(cfg, name):
    for variable in cfg.variables:
        if variable.nameToken and variable.nameToken.str == name:
            return variable
    assert False, "variable '%s' not found" % name


class TestCppcheckData:
    def test_language(self, sample_data, sample_cpp_data):
        assert sample_data.language == 'c'
        assert sample_cpp_data.language == 'cpp'

    def test_platform(self, sample_data):
        platform = sample_data.platform
        assert platform.name
        assert platform.char_bit == 8
        assert platform.short_bit >= 16
        assert platform.int_bit >= 16
        assert platform.long_bit >= 32
        assert platform.long_long_bit >= 64
        assert platform.pointer_bit > 0
        assert 'char_bit=8' in repr(platform)

    def test_files(self, sample_data):
        assert len(sample_data.files) == 1
        assert sample_data.files[0].endswith('sample.c')

    def test_rawtokens(self, sample_data):
        raw = sample_data.rawTokens
        # the not yet preprocessed code contains the directive tokens
        assert [tok.str for tok in raw[:4]] == ['#', 'define', 'ANSWER', '42']
        assert raw[-1].str == '}'
        assert raw[0].file.endswith('sample.c')
        assert raw[0].linenr == 1
        # next/previous chain
        assert raw[0].previous is None
        assert raw[-1].next is None
        for i in range(len(raw) - 1):
            assert raw[i].next is raw[i + 1]
            assert raw[i + 1].previous is raw[i]

    def test_configurations(self, sample_data):
        cfgs = sample_data.configurations
        assert len(cfgs) == 1
        assert cfgs[0].name == ''

    def test_iterconfigurations(self, multi_cfg_data):
        it = multi_cfg_data.iterconfigurations()
        cfgs = list(it)
        assert len(cfgs) == 2
        assert cfgs[0].name == ''
        assert 'FOO' in cfgs[1].name
        # 'foo()' is only tokenized in the FOO configuration
        assert not find_tokens(cfgs[0], 'foo')
        assert find_tokens(cfgs[1], 'foo')

    def test_standards(self, sample_cfg, sample_cpp_cfg):
        assert sample_cfg.standards.c.startswith('c')
        assert sample_cpp_cfg.standards.cpp.startswith('c++')
        assert sample_cfg.standards.posix is False
        assert 'c=' in repr(sample_cfg.standards)


class TestToken:
    def test_tokenlist(self, sample_cfg):
        code = ' '.join(tok.str for tok in sample_cfg.tokenlist)
        assert 'static int add ( int a , int b )' in code
        # the macro has been expanded
        assert 'x = 42' in code

    def test_next_previous(self, sample_cfg):
        tokens = sample_cfg.tokenlist
        assert tokens[0].previous is None
        assert tokens[-1].next is None
        for i in range(len(tokens) - 1):
            assert tokens[i].next is tokens[i + 1]
            assert tokens[i + 1].previous is tokens[i]

    def test_location(self, sample_cfg):
        tok = sample_cfg.tokenlist[0]
        assert tok.str == 'static'
        assert tok.file.endswith('sample.c')
        assert tok.linenr == 2
        assert tok.column == 1

    def test_link(self, sample_cfg):
        parenthesis = find_token(sample_cfg, '(')
        assert parenthesis.link.str == ')'
        assert parenthesis.link.link is parenthesis
        bracket = find_token(sample_cfg, '[')
        assert bracket.link.str == ']'
        brace = find_token(sample_cfg, '{')
        assert brace.link.str == '}'

    def test_scope(self, sample_cfg):
        assert sample_cfg.tokenlist[0].scope.type == 'Global'
        return_tok = find_token(sample_cfg, 'return')
        assert return_tok.scope.type == 'Function'
        assert return_tok.scope.className == 'add'

    def test_name_number_flags(self, sample_cfg):
        name_tok = find_token(sample_cfg, 'main')
        assert name_tok.isName
        assert not name_tok.isNumber
        num_tok = find_token(sample_cfg, '42')
        assert num_tok.isNumber
        assert num_tok.isInt
        assert not num_tok.isFloat
        float_tok = find_token(sample_cfg, '2.0')
        assert float_tok.isNumber
        assert float_tok.isFloat

    def test_operator_flags(self, sample_cfg, sample_cpp_cfg):
        plus = find_token(sample_cfg, '+')
        assert plus.isOp
        assert plus.isArithmeticalOp
        assign = find_token(sample_cfg, '=')
        assert assign.isOp
        assert assign.isAssignmentOp
        logical = find_token(sample_cpp_cfg, '&&')
        assert logical.isOp
        assert logical.isLogicalOp
        comparison = find_token(sample_cpp_cfg, '>')
        assert comparison.isOp
        assert comparison.isComparisonOp

    def test_string_char(self, sample_cpp_cfg):
        string_tok = find_token(sample_cpp_cfg, '"hello"')
        assert string_tok.isString
        assert string_tok.strlen == 5
        char_tok = find_token(sample_cpp_cfg, "'x'")
        assert char_tok.isChar

    @pytest.mark.xfail(strict=False,
                       reason='Tokenizer::dump checks Token::isName() before Token::isBoolean() and '
                              'eBoolean tokens are names, so type="boolean" is never dumped')
    def test_boolean(self, sample_cpp_cfg):
        bool_tok = find_token(sample_cpp_cfg, 'true')
        assert bool_tok.isBoolean

    def test_cast(self, sample_cpp_cfg):
        # static_cast<int>(...) is simplified to a C-style cast
        assert any(tok.isCast for tok in sample_cpp_cfg.tokenlist)

    def test_macro_expansion(self, sample_cfg):
        tok = find_token(sample_cfg, '42')
        assert tok.isExpandedMacro
        assert tok.macroName == 'ANSWER'

    def test_removed_void_parameter(self, sample_cfg):
        main_tok = find_token(sample_cfg, 'main')
        assert main_tok.next.isRemovedVoidParameter

    def test_splitted_var_decl(self, sample_cfg):
        # 'int x = ANSWER;' is simplified to 'int x ; x = 42 ;'
        assert any(tok.isSplittedVarDeclEq for tok in sample_cfg.tokenlist)

    def test_variable_and_var_id(self, sample_cfg):
        x_decl = find_token(sample_cfg, 'x')
        assert x_decl.varId
        assert x_decl.variable.nameToken is x_decl
        # all 'x' tokens refer to the same variable and varId
        for tok in find_tokens(sample_cfg, 'x'):
            assert tok.varId == x_decl.varId
            assert tok.variable is x_decl.variable

    def test_function(self, sample_cfg):
        add_call = find_token(sample_cfg, 'add', skip=1)
        assert add_call.function
        assert add_call.function.name == 'add'

    def test_value_type(self, sample_cfg):
        x_tok = find_token(sample_cfg, 'x')
        assert x_tok.valueType.type == 'int'
        assert x_tok.valueType.sign == 'signed'
        assert x_tok.valueType.pointer == 0
        assert x_tok.valueType.isIntegral()
        assert not x_tok.valueType.isFloat()
        assert not x_tok.valueType.isEnum()
        d_tok = find_token(sample_cfg, 'd')
        assert d_tok.valueType.type == 'double'
        assert d_tok.valueType.isFloat()
        # in 'arr[0] = ...' the array decays to a pointer
        arr_use = find_token(sample_cfg, 'arr', skip=1)
        assert arr_use.valueType.pointer == 1

    def test_value_type_enum(self, sample_cpp_cfg):
        color_tok = find_token(sample_cpp_cfg, 'color')
        assert color_tok.valueType.isEnum()
        assert color_tok.valueType.typeScope.className == 'Color'

    def test_ast(self, sample_cfg):
        plus = find_token(sample_cfg, '+')
        assert plus.astOperand1.str == 'a'
        assert plus.astOperand2.str == 'b'
        assert plus.astParent.str == 'return'
        assert plus.isBinaryOp()
        assert not plus.isUnaryOp('+')

    def test_ast_unary_op(self, sample_cfg):
        # the '-' in 'int neg = -x;'
        minus = find_token(sample_cfg, '-')
        assert minus.isUnaryOp('-')
        assert not minus.isBinaryOp()
        assert minus.astOperand1.str == 'x'
        assert minus.astOperand2 is None

    def test_ast_parents_top(self, sample_cfg):
        a_tok = find_token(sample_cfg, 'a', skip=1)  # the 'a' in 'return a + b;'
        assert [tok.str for tok in a_tok.astParents()] == ['+', 'return']
        assert a_tok.astTop().str == 'return'

    def test_values(self, sample_cfg):
        # 'x' has the known value 42 when it is used
        x_use = find_token(sample_cfg, 'x', skip=2)
        assert x_use.getKnownIntValue() == 42
        value = x_use.getValue(42)
        assert value.intvalue == 42
        assert value.isKnown()
        assert not value.isPossible()
        assert x_use.getValue(43) is None
        assert 'intvalue=42' in repr(value)

    def test_values_possible(self, sample_cfg):
        # inside add() the argument 'a' has the possible value 42
        a_use = find_token(sample_cfg, 'a', skip=1)
        assert a_use.getKnownIntValue() is None
        value = a_use.getValue(42)
        assert value is not None
        assert value.isPossible()

    def test_impossible_values(self, sample_cfg):
        # impossible values are separated from the possible/known ones
        arr_tokens = find_tokens(sample_cfg, 'arr')
        impossible = [v for tok in arr_tokens for v in tok.impossible_values]
        assert impossible
        assert all(v.isImpossible() for v in impossible)
        possible_or_known = [v for tok in arr_tokens for v in tok.values]
        assert all(not v.isImpossible() for v in possible_or_known)

    def test_forward_backward(self, sample_cfg):
        add_def = find_token(sample_cfg, 'add')
        strs = [tok.str for tok in add_def.forward()]
        assert strs[:4] == ['add', '(', 'int', 'a']
        end = add_def.tokAt(2)
        assert [tok.str for tok in add_def.forward(end=end)] == ['add', '(']
        strs = [tok.str for tok in add_def.backward()]
        assert strs[:3] == ['add', 'int', 'static']
        start = sample_cfg.tokenlist[0]
        assert [tok.str for tok in add_def.backward(start=start)] == ['add', 'int']

    def test_tokAt_linkAt(self, sample_cfg):
        add_def = find_token(sample_cfg, 'add')
        assert add_def.tokAt(0) is add_def
        assert add_def.tokAt(1).str == '('
        assert add_def.tokAt(-1).str == 'int'
        assert add_def.linkAt(1).str == ')'

    def test_repr(self, sample_cfg):
        tok = sample_cfg.tokenlist[0]
        assert "str='static'" in repr(tok)


class TestScope:
    def test_scopes(self, sample_cfg):
        types = [scope.type for scope in sample_cfg.scopes]
        assert types.count('Global') == 1
        assert types.count('Function') == 3

    def test_global_scope(self, sample_cfg):
        global_scope = find_scope(sample_cfg, 'Global')
        assert global_scope.nestedIn is None
        assert not global_scope.isExecutable
        assert len(global_scope.nestedList) == 3

    def test_function_scope(self, sample_cfg):
        scope = find_scope(sample_cfg, 'Function', className='add')
        assert scope.bodyStart.str == '{'
        assert scope.bodyEnd.str == '}'
        assert scope.bodyStart.link is scope.bodyEnd
        assert scope.nestedIn.type == 'Global'
        assert scope.function.name == 'add'
        assert scope.isExecutable

    def test_varlist(self, sample_cfg):
        scope = find_scope(sample_cfg, 'Function', className='main')
        names = [var.nameToken.str for var in scope.varlist]
        assert names == ['x', 'arr', 'neg']

    def test_cpp_scopes(self, sample_cpp_cfg):
        types = {scope.type for scope in sample_cpp_cfg.scopes}
        assert {'Global', 'Namespace', 'Class', 'Enum', 'Function', 'If'} <= types
        assert find_scope(sample_cpp_cfg, 'Namespace', className='ns')
        assert find_scope(sample_cpp_cfg, 'Class', className='Shape')
        if_scope = find_scope(sample_cpp_cfg, 'If')
        assert if_scope.isExecutable
        class_scope = find_scope(sample_cpp_cfg, 'Class')
        assert not class_scope.isExecutable


class TestFunction:
    def test_functions(self, sample_cfg):
        names = {function.name for function in sample_cfg.functions}
        assert names == {'add', 'half', 'main'}

    def test_arguments(self, sample_cfg):
        add = find_function(sample_cfg, 'add')
        assert sorted(add.argument.keys()) == [1, 2]
        assert add.argument[1].nameToken.str == 'a'
        assert add.argument[2].nameToken.str == 'b'
        main = find_function(sample_cfg, 'main')
        assert main.argument == {}

    def test_attributes(self, sample_cfg):
        add = find_function(sample_cfg, 'add')
        assert add.isStatic
        assert add.type == 'Function'
        assert add.tokenDef.str == 'add'
        assert add.token.str == 'add'
        assert add.nestedIn.type == 'Global'
        main = find_function(sample_cfg, 'main')
        assert not main.isStatic

    def test_virtual(self, sample_cpp_cfg):
        area = find_function(sample_cpp_cfg, 'area')
        assert area.hasVirtualSpecifier
        twice = find_function(sample_cpp_cfg, 'twice')
        assert not twice.hasVirtualSpecifier
        assert twice.nestedIn.className == 'ns'


class TestVariable:
    def test_local(self, sample_cfg):
        x = find_variable(sample_cfg, 'x')
        assert x.access == 'Local'
        assert x.isLocal
        assert not x.isArgument
        assert not x.isGlobal
        assert not x.isArray
        assert x.typeStartToken.str == 'int'
        assert x.typeEndToken.str == 'int'
        assert x.scope.className == 'main'

    def test_argument(self, sample_cfg):
        a = find_variable(sample_cfg, 'a')
        assert a.access == 'Argument'
        assert a.isArgument
        assert not a.isLocal

    def test_array(self, sample_cfg):
        arr = find_variable(sample_cfg, 'arr')
        assert arr.isArray
        assert not arr.isPointer

    def test_global_const(self, sample_cpp_cfg):
        limit = find_variable(sample_cpp_cfg, 'limit')
        assert limit.access == 'Global'
        assert limit.isGlobal
        assert limit.isConst
        assert not limit.isStatic

    def test_pointer(self, sample_cpp_cfg):
        msg = find_variable(sample_cpp_cfg, 'msg')
        assert msg.isPointer
        assert not msg.isArray

    def test_class_member(self, sample_cpp_cfg):
        member = find_variable(sample_cpp_cfg, 'mScale')
        assert member.access == 'Protected'
        assert member.isClass is False


class TestPreprocessor:
    def test_directives(self, sample_cfg):
        directives = sample_cfg.directives
        assert len(directives) == 1
        assert directives[0].str == '#define ANSWER 42'
        assert directives[0].file.endswith('sample.c')
        assert directives[0].linenr == 1
        assert "#define ANSWER 42" in repr(directives[0])

    def test_macro_usage(self, sample_cfg):
        macros = sample_cfg.macro_usage
        assert len(macros) == 1
        macro = macros[0]
        assert macro.name == 'ANSWER'
        assert macro.usefile.endswith('sample.c')
        assert int(macro.useline) == 14
        assert macro.isKnownValue
        assert "name='ANSWER'" in repr(macro)

    def test_if_conditions(self, multi_cfg_data):
        cfgs = multi_cfg_data.configurations
        for cfg in cfgs:
            assert len(cfg.preprocessor_if_conditions) == 1
            assert cfg.preprocessor_if_conditions[0].linenr == 3
        assert cfgs[0].preprocessor_if_conditions[0].result == 0
        assert cfgs[1].preprocessor_if_conditions[0].result == 1
        assert 'result=' in repr(cfgs[0].preprocessor_if_conditions[0])

    def test_typedef_info(self, multi_cfg_data):
        cfg = multi_cfg_data.configurations[0]
        typedefs = {info.name: info for info in cfg.typedefInfo}
        assert set(typedefs.keys()) == {'myint', 'myfloat'}
        assert typedefs['myint'].used
        assert not typedefs['myfloat'].used
        assert typedefs['myint'].linenr == 1


class TestHelperFunctions:
    def test_getArguments(self, sample_cfg):
        add_call = find_token(sample_cfg, 'add', skip=1)
        args = cppcheckdata.getArguments(add_call)
        assert [tok.str for tok in args] == ['x', '1']

    def test_getArguments_no_call(self, sample_cfg):
        int_tok = find_token(sample_cfg, 'int')
        assert cppcheckdata.getArguments(int_tok) is None

    def test_get_function_call_name_args(self, sample_cfg):
        add_call = find_token(sample_cfg, 'add', skip=1)
        name, args = cppcheckdata.get_function_call_name_args(add_call)
        assert name == 'add'
        assert [tok.str for tok in args] == ['x', '1']

    def test_get_function_call_name_args_namespace(self, sample_cpp_cfg):
        twice_call = find_token(sample_cpp_cfg, 'twice', skip=1)
        name, args = cppcheckdata.get_function_call_name_args(twice_call)
        assert name == 'ns::twice'
        assert [tok.str for tok in args] == ['21']

    def test_get_function_call_name_args_not_a_call(self, sample_cfg):
        # the function definition is not a call
        add_def = find_token(sample_cfg, 'add')
        name, args = cppcheckdata.get_function_call_name_args(add_def)
        assert name is None
        assert args is None

    def test_astIsFloat(self, sample_cfg):
        division = find_token(sample_cfg, '/')
        assert cppcheckdata.astIsFloat(division)
        plus = find_token(sample_cfg, '+')  # a + b with int operands
        assert not cppcheckdata.astIsFloat(plus)
        assert not cppcheckdata.astIsFloat(None)


class TestMatch:
    def test_simpleMatch(self, sample_cfg):
        add_def = find_token(sample_cfg, 'add')
        assert cppcheckdata.simpleMatch(add_def, 'add ( int a , int b )')
        assert not cppcheckdata.simpleMatch(add_def, 'add ( int b')
        assert not cppcheckdata.simpleMatch(None, 'add')

    def test_match_plain(self, sample_cfg):
        add_def = find_token(sample_cfg, 'add')
        res = cppcheckdata.match(add_def, 'add ( int')
        assert res
        assert res.end.str == 'int'
        assert not cppcheckdata.match(add_def, 'sub (')
        assert not cppcheckdata.match(add_def, '')
        assert not cppcheckdata.match(None, 'add')

    def test_match_patterns(self, sample_cfg):
        x_assign = find_token(sample_cfg, 'x', skip=1)  # the 'x' in 'x = 42 ;'
        assert cppcheckdata.match(x_assign, '%name% %assign% %any% ;')
        assert cppcheckdata.match(x_assign, '%var% = 42')
        assert not cppcheckdata.match(x_assign, '%op%')
        plus = find_token(sample_cfg, '+')
        assert cppcheckdata.match(plus, '%op%')
        comparison = find_token(sample_cfg, '+')
        assert not cppcheckdata.match(comparison, '%comp%')

    def test_match_bindings(self, sample_cfg):
        x_assign = find_token(sample_cfg, 'x', skip=1)
        res = cppcheckdata.match(x_assign, '%var%@lhs = %any%@rhs ;')
        assert res
        assert res.lhs.str == 'x'
        assert res.rhs.str == '42'
        # bindings are None on a failed match
        res = cppcheckdata.match(x_assign, '%var%@lhs + %any%@rhs')
        assert not res
        assert res.lhs is None
        assert res.rhs is None

    def test_match_alternatives(self, sample_cfg):
        x_assign = find_token(sample_cfg, 'x', skip=1)
        assert cppcheckdata.match(x_assign, 'y|x =')
        assert not cppcheckdata.match(x_assign, 'y|z =')

    def test_match_negation(self, sample_cfg):
        x_assign = find_token(sample_cfg, 'x', skip=1)
        assert cppcheckdata.match(x_assign, 'x !!+')
        assert not cppcheckdata.match(x_assign, 'x !!=')

    def test_match_link(self, sample_cfg):
        add_def = find_token(sample_cfg, 'add')
        res = cppcheckdata.match(add_def.next, '(*)')
        assert res
        assert res.end.str == ')'


class TestSuppressions:
    def make_suppression(self, **kwargs):
        return cppcheckdata.Suppression(kwargs)

    def test_line_suppression(self):
        supp = self.make_suppression(errorId='zerodiv', lineNumber='5')
        assert supp.isMatch('a.c', '5', 'msg', 'zerodiv')
        assert not supp.isMatch('a.c', '5', 'msg', 'nullPointer')

    @pytest.mark.xfail(strict=False,
                       reason='the "other suppression" fallback in Suppression.isMatch() does not check '
                              'that lineNumber is unset, so a line suppression matches every line')
    def test_line_suppression_other_line(self):
        supp = self.make_suppression(errorId='zerodiv', lineNumber='5')
        assert not supp.isMatch('a.c', '6', 'msg', 'zerodiv')

    def test_wildcard_errorId(self):
        supp = self.make_suppression(errorId='*', lineNumber='5')
        assert supp.isMatch('a.c', '5', 'msg', 'anything')

    def test_file_suppression(self):
        supp = self.make_suppression(errorId='zerodiv', fileName='a.c', type='file')
        assert supp.isMatch('a.c', '1', 'msg', 'zerodiv')
        assert supp.isMatch('a.c', '99', 'msg', 'zerodiv')
        assert not supp.isMatch('b.c', '1', 'msg', 'zerodiv')

    def test_block_suppression(self):
        supp = self.make_suppression(errorId='zerodiv', type='block', lineBegin='3', lineEnd='7')
        assert supp.isMatch('a.c', '5', 'msg', 'zerodiv')
        assert not supp.isMatch('a.c', '3', 'msg', 'zerodiv')
        assert not supp.isMatch('a.c', '7', 'msg', 'zerodiv')

    def test_global_suppression(self):
        supp = self.make_suppression(errorId='zerodiv')
        assert supp.isMatch('a.c', '1', 'msg', 'zerodiv')
        assert supp.isMatch('b.c', '99', 'other', 'zerodiv')

    def test_symbolName(self):
        supp = self.make_suppression(errorId='zerodiv', symbolName='xyz')
        assert supp.isMatch('a.c', '1', 'error on xyz here', 'zerodiv')
        assert not supp.isMatch('a.c', '1', 'other message', 'zerodiv')

    def test_dumpfile_suppressions(self, dump_factory):
        code = ('void f(void)\n'
                '{\n'
                '    int a;\n'
                '    // cppcheck-suppress uninitvar\n'
                '    a++;\n'
                '}\n')
        data = dump_factory.parse(code, extra_args=['--inline-suppr'])
        assert len(data.suppressions) == 1
        supp = data.suppressions[0]
        assert supp.errorId == 'uninitvar'
        assert supp.fileName.endswith('test.c')
        assert int(supp.lineNumber) == 5
        # parsedump() also sets the suppressions used by is_suppressed()
        location = cppcheckdata.Location({'file': supp.fileName, 'line': '5', 'column': '5'})
        assert cppcheckdata.is_suppressed(location, 'msg', 'uninitvar')
        assert not cppcheckdata.is_suppressed(location, 'msg', 'nullPointer')
        # NOTE: a non-matching line is not checked here - see test_line_suppression_other_line


class TestPureHelpers:
    """Tests that construct objects from dicts and do not need a dump file."""

    def test_location(self):
        loc = cppcheckdata.Location({'file': 'a.c', 'line': '3', 'column': '7'})
        assert loc.file == 'a.c'
        assert loc.linenr == 3
        assert loc.column == 7

    def test_location_defaults(self):
        loc = cppcheckdata.Location({'file': 'a.c'})
        assert loc.linenr == 0
        assert loc.column == 0

    def test_location_linenr_alias(self):
        loc = cppcheckdata.Location({'file': 'a.c', 'linenr': '4'})
        assert loc.linenr == 4

    def test_value_kinds(self):
        known = cppcheckdata.Value({'intvalue': '42', 'known': 'true'})
        assert known.intvalue == 42
        assert known.isKnown()
        assert not known.isPossible()
        possible = cppcheckdata.Value({'intvalue': '1', 'possible': 'true'})
        assert possible.isPossible()
        impossible = cppcheckdata.Value({'intvalue': '0', 'impossible': 'true'})
        assert impossible.isImpossible()
        inconclusive = cppcheckdata.Value({'intvalue': '2', 'inconclusive': 'true'})
        assert inconclusive.isInconclusive()

    def test_value_condition(self):
        value = cppcheckdata.Value({'intvalue': '1', 'possible': 'true', 'condition-line': '12'})
        assert value.condition == 12

    def test_argument_parser(self):
        parser = cppcheckdata.ArgumentParser()
        args = parser.parse_args(['a.dump', 'b.ctu-info', '--cli', '-q'])
        assert args.dumpfile == ['a.dump', 'b.ctu-info']
        assert args.cli
        assert args.quiet
        assert args.file_list is None
        assert '{severity}' in args.template

    def test_get_files(self, tmp_path):
        file_list = tmp_path / 'files.txt'
        file_list.write_text('c.dump\nd.ctu-info\n')
        parser = cppcheckdata.ArgumentParser()
        args = parser.parse_args(['a.dump', 'b.ctu-info', '--file-list', str(file_list)])
        dump_files, ctu_info_files = cppcheckdata.get_files(args)
        assert dump_files == ['a.dump', 'c.dump']
        assert ctu_info_files == ['b.ctu-info', 'd.ctu-info']


class TestReporting:
    def test_reportError_cli(self, monkeypatch, capsys):
        monkeypatch.setattr(sys, 'argv', ['myaddon.py', '--cli'])
        location = cppcheckdata.Location({'file': 'a.c', 'line': '3', 'column': '7'})
        cppcheckdata.reportError(location, 'error', 'the message', 'myaddon', 'myid', extra='extra info')
        msg = json.loads(capsys.readouterr().out)
        assert msg == {'file': 'a.c',
                       'linenr': 3,
                       'column': 7,
                       'severity': 'error',
                       'message': 'the message',
                       'addon': 'myaddon',
                       'errorId': 'myid',
                       'extra': 'extra info'}

    def test_reportError_cli_column_override(self, monkeypatch, capsys):
        monkeypatch.setattr(sys, 'argv', ['myaddon.py', '--cli'])
        location = cppcheckdata.Location({'file': 'a.c', 'line': '3', 'column': '7'})
        cppcheckdata.reportError(location, 'error', 'the message', 'myaddon', 'myid', columnOverride=42)
        msg = json.loads(capsys.readouterr().out)
        assert msg['column'] == 42

    def test_reportError_stderr(self, monkeypatch, capsys):
        monkeypatch.setattr(sys, 'argv', ['myaddon.py'])
        monkeypatch.setattr(cppcheckdata, 'current_dumpfile_suppressions', [])
        monkeypatch.setattr(cppcheckdata, 'EXIT_CODE', 0)
        location = cppcheckdata.Location({'file': 'a.c', 'line': '3', 'column': '7'})
        cppcheckdata.reportError(location, 'style', 'the message', 'myaddon', 'myid')
        stderr = capsys.readouterr().err
        assert stderr == '[a.c:3] (style) the message [myaddon-myid]\n'
        assert cppcheckdata.EXIT_CODE == 1

    def test_reportError_stderr_suppressed(self, monkeypatch, capsys):
        monkeypatch.setattr(sys, 'argv', ['myaddon.py'])
        suppression = cppcheckdata.Suppression({'errorId': 'myaddon-myid'})
        monkeypatch.setattr(cppcheckdata, 'current_dumpfile_suppressions', [suppression])
        monkeypatch.setattr(cppcheckdata, 'EXIT_CODE', 0)
        location = cppcheckdata.Location({'file': 'a.c', 'line': '3', 'column': '7'})
        cppcheckdata.reportError(location, 'style', 'the message', 'myaddon', 'myid')
        assert capsys.readouterr().err == ''
        assert cppcheckdata.EXIT_CODE == 0

    def test_log_checker_cli(self, monkeypatch, capsys):
        monkeypatch.setattr(sys, 'argv', ['myaddon.py', '--cli'])
        cppcheckdata.log_checker('SomeChecker', 'myaddon')
        msg = json.loads(capsys.readouterr().out)
        assert msg == {'addon': 'myaddon',
                       'severity': 'none',
                       'message': 'SomeChecker',
                       'errorId': 'logChecker'}

    def test_log_checker_no_cli(self, monkeypatch, capsys):
        monkeypatch.setattr(sys, 'argv', ['myaddon.py'])
        cppcheckdata.log_checker('SomeChecker', 'myaddon')
        assert capsys.readouterr().out == ''
