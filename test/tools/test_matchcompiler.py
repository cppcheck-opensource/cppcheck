#!/usr/bin/env python3
#
# Cppcheck - A tool for static C/C++ code analysis
# Copyright (C) 2007-2021 Cppcheck team.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import unittest
import matchcompiler


class MatchCompilerTest(unittest.TestCase):

    def setUp(self):
        self.mc = matchcompiler.MatchCompiler(verify_mode=False)

    def test_parseMatch(self):
        self.assertEqual(self.mc.parseMatch('  Token::Match(tok, ";") ', 2), [
                         'Token::Match(tok, ";")', 'tok', ' ";"'])
        self.assertEqual(self.mc.parseMatch('  Token::Match(tok,', 2), None)
        # multiline Token::Match is not supported yet
        self.assertEqual(self.mc.parseMatch('  Token::Match(Token::findsimplematch(tok,")"), ";")', 2), [
                         'Token::Match(Token::findsimplematch(tok,")"), ";")',
                         'Token::findsimplematch(tok,")")', ' ";"'])  # inner function call

    def test_replaceTokenMatch(self):
        input = 'if (Token::Match(tok, "foobar")) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (match1(tok)) {')

        input = 'if (Token::simpleMatch(tok, "foobar")) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (match1(tok)) {')

        input = 'if (Token::Match(tok->next()->next(), "foobar %type% %num%")) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (match2(tok->next()->next())) {')

        input = 'if (Token::Match(tok, "foo\"special\"bar %num%")) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(
            output, 'if (match3(tok)) {')

        # test that non-static patterns get passed on unmatched
        input = 'if (Token::Match(tok, "struct " + varname)) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(
            output, 'if (Token::Match(tok, "struct " + varname)) {')

        # test that non-static patterns get passed on unmatched
        input = 'if (Token::Match(tok, "extern \"C\" " + varname)) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(
            output, 'if (Token::Match(tok, "extern \"C\" " + varname)) {')

        # test that multiple patterns on the same line are replaced
        input = 'if (Token::Match(tok, "foo") && Token::Match(tok->next(), "baz")) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (match4(tok) && match5(tok->next())) {')

        # test that second pattern is replaced, even if there is a bailout on the first pattern
        input = 'if (Token::Match(tok, foo) && Token::Match(tok->next(), "foobaz")) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (Token::Match(tok, foo) && match6(tok->next())) {')

        # test mixing Match and simpleMatch on the same line
        input = 'if (Token::Match(tok, "a") && Token::simpleMatch(tok->next(), "b")) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (match7(tok) && match8(tok->next())) {')

        input = 'if (Token::simpleMatch(tok, "a") && Token::Match(tok->next(), "b")) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (match7(tok) && match8(tok->next())) {')

    def test_replaceTokenMatchWithVarId(self):
        input = 'if (Token::Match(tok, "foobar %varid%", 123)) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (match1(tok, 123)) {')

        input = 'if (Token::Match(tok->next()->next(), "%varid% foobar", tok->varId())) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(
            output, 'if (match2(tok->next()->next(), tok->varId())) {')

        input = 'if (Token::Match(tok, "foo\"special\"bar %type% %varid%", my_varid_cache)) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(
            output, 'if (match3(tok, my_varid_cache)) {')

        # test caching: reuse existing matchX()
        input = 'if (Token::Match(tok, "foobar %varid%", 123)) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (match1(tok, 123)) {')

        # two in one line
        input = 'if (Token::Match(tok, "foobar2 %varid%", 123) || Token::Match(tok, "%type% %varid%", 123)) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (match4(tok, 123) || match5(tok, 123)) {')

    def test_replaceTokenSimpleMatch(self):
        input = 'if (Token::simpleMatch(tok, "foobar")) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (match1(tok)) {')

        input = 'if (Token::simpleMatch(tok->next()->next(), "foobar")) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (match1(tok->next()->next())) {')

        input = 'if (Token::simpleMatch(tok, "foo\"special\"bar")) {'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(
            output, 'if (match2(tok)) {')

    def test_replaceTokenFindSimpleMatch(self):
        input = 'if (Token::findsimplematch(tok, "foobar")) {'
        output = self.mc._replaceTokenFindMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (findmatch1(tok) ) {')

        input = 'if (Token::findsimplematch(tok->next()->next(), "foobar", tok->link())) {'
        output = self.mc._replaceTokenFindMatch(input, 0, "foo.cpp")
        self.assertEqual(
            output, 'if (findmatch2(tok->next()->next(), tok->link()) ) {')

        input = 'if (Token::findsimplematch(tok, "foo\"special\"bar")) {'
        output = self.mc._replaceTokenFindMatch(input, 0, "foo.cpp")
        self.assertEqual(
            output, 'if (findmatch3(tok) ) {')

    def test_replaceTokenFindMatch(self):
        input = 'if (Token::findmatch(tok, "foobar")) {'
        output = self.mc._replaceTokenFindMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (findmatch1(tok) ) {')

        # findmatch with varid
        input = 'if (Token::findmatch(tok, "foobar %varid%", tok->varId())) {'
        output = self.mc._replaceTokenFindMatch(input, 0, "foo.cpp")
        self.assertEqual(output, 'if (findmatch2(tok, tok->varId()) ) {')

        # findmatch with end token
        input = 'if (Token::findmatch(tok->next()->next(), "foobar %type%", tok->link())) {'
        output = self.mc._replaceTokenFindMatch(input, 0, "foo.cpp")
        self.assertEqual(
            output, 'if (findmatch3(tok->next()->next(), tok->link()) ) {')

        # findmatch with end token and varid
        input = 'if (Token::findmatch(tok->next()->next(), "foobar %type% %varid%", tok->link(), 123)) {'
        output = self.mc._replaceTokenFindMatch(input, 0, "foo.cpp")
        self.assertEqual(
            output, 'if (findmatch4(tok->next()->next(), tok->link(), 123) ) {')

    def test_parseStringComparison(self):
        input = 'str == "abc"'
        # offset '5' is chosen as an abritary start offset to look for
        res = self.mc._parseStringComparison(input, 5)
        self.assertEqual(2, len(res))
        self.assertEqual('str == MatchCompiler::makeConstString("abc")', input[:res[0]] +
                         "MatchCompiler::makeConstString(" + input[res[0]:res[1]] + ")" + input[res[1]:])

        input = 'str == "a\\"b\\"c"'
        res = self.mc._parseStringComparison(input, 5)
        self.assertEqual(2, len(res))
        self.assertEqual('str == MatchCompiler::makeConstString("a\\"b\\"c")', input[:res[0]] +
                         "MatchCompiler::makeConstString(" + input[res[0]:res[1]] + ")" + input[res[1]:])

    def test_replaceCStrings(self):
        # str() ==
        input = 'if (tok2->str() == "abc") {'
        output = self.mc._replaceCStrings(input)
        self.assertEqual('if (tok2->str() == MatchCompiler::makeConstString("abc")) {', output)

        # str() !=
        input = 'if (tok2->str() != "xyz") {'
        output = self.mc._replaceCStrings(input)
        self.assertEqual('if (tok2->str() != MatchCompiler::makeConstString("xyz")) {', output)

        # strAt()
        input = 'if (match16(parent->tokAt(-3)) && tok->strAt(1) == ")")'
        output = self.mc._replaceCStrings(input)
        self.assertEqual(
            'if (match16(parent->tokAt(-3)) && tok->strAt(1) == MatchCompiler::makeConstString(")"))',
            output)

    def test_parseMatchSkipComments(self):
        input = '// TODO: suggest Token::exactMatch() for Token::simpleMatch() when pattern contains no whitespaces'
        output = self.mc._replaceTokenMatch(input, 0, "foo.cpp")
        self.assertEqual(output, '// TODO: suggest Token::exactMatch() for Token::simpleMatch() when pattern contains no whitespaces')

if __name__ == '__main__':
    unittest.main()
