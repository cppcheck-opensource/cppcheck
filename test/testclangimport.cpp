// Cppcheck - A tool for static C/C++ code analysis
// Copyright (C) 2007-2026 Cppcheck team.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "clangimport.h"
#include "fixture.h"
#include "mathlib.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <cstdint>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>


class TestClangImport : public TestFixture {
public:
    TestClangImport()
        : TestFixture("TestClangImport") {}


private:
    void run() override {
        TEST_CASE(breakStmt);
        TEST_CASE(callExpr);
        TEST_CASE(caseStmt1);
        TEST_CASE(characterLiteral);
        TEST_CASE(class1);
        TEST_CASE(classTemplateDecl1);
        TEST_CASE(classTemplateDecl2);
        TEST_CASE(conditionalExpr);
        TEST_CASE(compoundAssignOperator);
        TEST_CASE(continueStmt);
        TEST_CASE(cstyleCastExpr);
        TEST_CASE(cxxBoolLiteralExpr);
        TEST_CASE(cxxConstructorDecl1);
        TEST_CASE(cxxConstructorDecl2);
        TEST_CASE(cxxConstructExpr1);
        TEST_CASE(cxxConstructExpr2);
        TEST_CASE(cxxConstructExpr3);
        TEST_CASE(cxxDeleteExpr);
        TEST_CASE(cxxDestructorDecl);
        TEST_CASE(cxxForRangeStmt1);
        TEST_CASE(cxxForRangeStmt2);
        TEST_CASE(cxxFunctionalCastExpr);
        TEST_CASE(cxxMemberCall);
        TEST_CASE(cxxMethodDecl1);
        TEST_CASE(cxxMethodDecl2);
        TEST_CASE(cxxMethodDecl3);
        TEST_CASE(cxxMethodDecl4);
        TEST_CASE(cxxNewExpr1);
        TEST_CASE(cxxNewExpr2);
        TEST_CASE(cxxNullPtrLiteralExpr);
        TEST_CASE(cxxOperatorCallExpr);
        TEST_CASE(cxxRecordDecl1);
        TEST_CASE(cxxRecordDecl2);
        TEST_CASE(cxxRecordDeclDerived);
        TEST_CASE(cxxStaticCastExpr1);
        TEST_CASE(cxxStaticCastExpr2);
        TEST_CASE(cxxStaticCastExpr3);
        TEST_CASE(cxxStdInitializerListExpr);
        TEST_CASE(cxxThrowExpr);
        TEST_CASE(defaultStmt);
        TEST_CASE(doStmt);
        TEST_CASE(enumDecl1);
        TEST_CASE(enumDecl2);
        TEST_CASE(enumDecl3);
        TEST_CASE(enumDecl4);
        TEST_CASE(forStmt);
        TEST_CASE(funcdecl1);
        TEST_CASE(funcdecl2);
        TEST_CASE(funcdecl3);
        TEST_CASE(funcdecl4);
        TEST_CASE(funcdecl5);
        TEST_CASE(funcdecl6);
        TEST_CASE(functionTemplateDecl1);
        TEST_CASE(functionTemplateDecl2);
        TEST_CASE(initListExpr);
        TEST_CASE(ifelse);
        TEST_CASE(ifStmt);
        TEST_CASE(labelStmt);
        TEST_CASE(memberExpr);
        TEST_CASE(namespaceDecl1);
        TEST_CASE(namespaceDecl2);
        TEST_CASE(recordDecl1);
        TEST_CASE(recordDecl2);
        TEST_CASE(switchStmt);
        TEST_CASE(typedefDeclPrologue);
        TEST_CASE(unaryExprOrTypeTraitExpr1);
        TEST_CASE(unaryExprOrTypeTraitExpr2);
        TEST_CASE(unaryOperator);
        TEST_CASE(vardecl1);
        TEST_CASE(vardecl2);
        TEST_CASE(vardecl3);
        TEST_CASE(vardecl4);
        TEST_CASE(vardecl5);
        TEST_CASE(vardecl6);
        TEST_CASE(vardecl7);
        TEST_CASE(whileStmt1);
        TEST_CASE(whileStmt2);

        TEST_CASE(tokenIndex);
        TEST_CASE(symbolDatabaseEnum1);
        TEST_CASE(symbolDatabaseFunction1);
        TEST_CASE(symbolDatabaseFunction2);
        TEST_CASE(symbolDatabaseFunction3);
        TEST_CASE(symbolDatabaseFunctionConst);
        TEST_CASE(symbolDatabaseVariableRef);
        TEST_CASE(symbolDatabaseVariableRRef);
        TEST_CASE(symbolDatabaseVariablePointerRef);
        TEST_CASE(symbolDatabaseNodeType1);
        TEST_CASE(symbolDatabaseForVariable);
        TEST_CASE(stdinLoc);

        TEST_CASE(valueFlow1);
        TEST_CASE(valueFlow2);

        TEST_CASE(valueType1);
        TEST_CASE(valueType2);

        TEST_CASE(crash);
    }

    std::string parse(const char clang[]) {
        const Settings settings = settingsBuilder().clang().build();
        TokenList tokenlist{settings, Standards::Language::CPP};
        Tokenizer tokenizer(std::move(tokenlist), *this);
        std::istringstream istr(clang);
        clangimport::parseClangAstDump(tokenizer, istr);
        if (!tokenizer.tokens()) {
            return std::string();
        }
        return tokenizer.tokens()->stringifyList(true, false, false, false, false);
    }

    void breakStmt() {
        const char clang[] = "`-FunctionDecl 0x2c31b18 <1.c:1:1, col:34> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x2c31c40 <col:12, col:34>\n"
                             "    `-WhileStmt 0x2c31c20 <col:14, col:24>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-IntegerLiteral 0x2c31bf8 <col:21> 'int' 0\n"
                             "      `-BreakStmt 0x3687c18 <col:24>";
        ASSERT_EQUALS("void foo ( ) { while ( 0 ) { break ; } }", parse(clang));
    }

    void callExpr() {
        const char clang[] = "`-FunctionDecl 0x2444b60 <1.c:1:1, line:8:1> line:1:6 foo 'void (int)'\n"
                             "  |-ParmVarDecl 0x2444aa0 <col:10, col:14> col:14 used x 'int'\n"
                             "  `-CompoundStmt 0x2444e00 <col:17, line:8:1>\n"
                             "    `-CallExpr 0x7f5a6c04b158 <line:1:16, col:60> 'bool'\n"
                             "      |-ImplicitCastExpr 0x7f5a6c04b140 <col:16, col:23> 'bool (*)(const Token *, const char *, int)' <FunctionToPointerDecay>\n"
                             "      | `-DeclRefExpr 0x7f5a6c04b0a8 <col:16, col:23> 'bool (const Token *, const char *, int)' lvalue CXXMethod 0x43e5600 'Match' 'bool (const Token *, const char *, int)'\n"
                             "      |-ImplicitCastExpr 0x7f5a6c04b1c8 <col:29> 'const Token *' <NoOp>\n"
                             "      | `-ImplicitCastExpr 0x7f5a6c04b1b0 <col:29> 'Token *' <LValueToRValue>\n"
                             "      |   `-DeclRefExpr 0x7f5a6c04b0e0 <col:29> 'Token *' lvalue Var 0x7f5a6c045968 'tokAfterCondition' 'Token *'\n"
                             "      |-ImplicitCastExpr 0x7f5a6c04b1e0 <col:48> 'const char *' <ArrayToPointerDecay>\n"
                             "      | `-StringLiteral 0x7f5a6c04b108 <col:48> 'const char [11]' lvalue \"%name% : {\"\n"
                             "      `-CXXDefaultArgExpr 0x7f5a6c04b1f8 <<invalid sloc>> 'int'\n";
        ASSERT_EQUALS("void foo ( int x@1 ) { Match ( tokAfterCondition , \"%name% : {\" ) ; }", parse(clang));
    }

    void caseStmt1() {
        const char clang[] = "`-FunctionDecl 0x2444b60 <1.c:1:1, line:8:1> line:1:6 foo 'void (int)'\n"
                             "  |-ParmVarDecl 0x2444aa0 <col:10, col:14> col:14 used x 'int'\n"
                             "  `-CompoundStmt 0x2444e00 <col:17, line:8:1>\n"
                             "    `-SwitchStmt 0x2444c88 <line:2:5, line:7:5>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-ImplicitCastExpr 0x2444c70 <line:2:13> 'int' <LValueToRValue>\n"
                             "      | `-DeclRefExpr 0x2444c48 <col:13> 'int' lvalue ParmVar 0x2444aa0 'x' 'int'\n"
                             "      `-CompoundStmt 0x2444de0 <col:16, line:7:5>\n"
                             "        |-CaseStmt 0x2444cd8 <line:3:9, line:5:15>\n"
                             "        | |-IntegerLiteral 0x2444cb8 <line:3:14> 'int' 16\n"
                             "        | |-<<<NULL>>>\n"
                             "        | `-CaseStmt 0x2444d30 <line:4:9, line:5:15>\n"
                             "        |   |-IntegerLiteral 0x2444d10 <line:4:14> 'int' 32\n"
                             "        |   |-<<<NULL>>>\n"
                             "        |   `-BinaryOperator 0x2444db0 <line:5:13, col:15> 'int' '='\n"
                             "        |     |-DeclRefExpr 0x2444d68 <col:13> 'int' lvalue ParmVar 0x2444aa0 'x' 'int'\n"
                             "        |     `-IntegerLiteral 0x2444d90 <col:15> 'int' 123\n"
                             "        `-BreakStmt 0x2444dd8 <line:6:13>";
        ASSERT_EQUALS("void foo ( int x@1 ) { switch ( x@1 ) { case 16 : case 32 : x@1 = 123 ; break ; } }", parse(clang));
    }

    void characterLiteral() {
        const char clang[] = "`-VarDecl 0x3df8608 <a.cpp:1:1, col:10> col:6 c 'char' cinit\n"
                             "  `-CharacterLiteral 0x3df86a8 <col:10> 'char' 120";
        ASSERT_EQUALS("char c@1 = 'x' ;", parse(clang));
    }

    void class1() {
        const char clang[] = "`-CXXRecordDecl 0x274c638 <a.cpp:1:1, col:25> col:7 class C definition\n"
                             "  |-DefinitionData pass_in_registers empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init\n"
                             "  | |-DefaultConstructor exists trivial constexpr needs_implicit defaulted_is_constexpr\n"
                             "  | |-CopyConstructor simple trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "  | |-MoveConstructor exists simple trivial needs_implicit\n"
                             "  | |-CopyAssignment trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "  | |-MoveAssignment exists simple trivial needs_implicit\n"
                             "  | `-Destructor simple irrelevant trivial needs_implicit\n"
                             "  |-CXXRecordDecl 0x274c758 <col:1, col:7> col:7 implicit class C\n"
                             "  `-CXXMethodDecl 0x274c870 <col:11, col:23> col:16 foo 'void ()'\n"
                             "    `-CompoundStmt 0x274c930 <col:22, col:23>";
        ASSERT_EQUALS("class C { void foo ( ) { } } ;", parse(clang));
    }

    void classTemplateDecl1() {
        const char clang[] = "`-ClassTemplateDecl 0x29d1748 <a.cpp:1:1, col:59> col:25 C\n"
                             "  |-TemplateTypeParmDecl 0x29d15f8 <col:10, col:16> col:16 referenced class depth 0 index 0 T\n"
                             "  `-CXXRecordDecl 0x29d16b0 <col:19, col:59> col:25 class C definition\n"
                             "    |-DefinitionData empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init\n"
                             "    | |-DefaultConstructor exists trivial constexpr needs_implicit defaulted_is_constexpr\n"
                             "    | |-CopyConstructor simple trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "    | |-MoveConstructor exists simple trivial needs_implicit\n"
                             "    | |-CopyAssignment trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "    | |-MoveAssignment exists simple trivial needs_implicit\n"
                             "    | `-Destructor simple irrelevant trivial needs_implicit\n"
                             "    |-CXXRecordDecl 0x29d19b0 <col:19, col:25> col:25 implicit class C\n"
                             "    |-AccessSpecDecl 0x29d1a48 <col:29, col:35> col:29 public\n"
                             "    `-CXXMethodDecl 0x29d1b20 <col:37, col:57> col:39 foo 'T ()'\n"
                             "      `-CompoundStmt 0x29d1c18 <col:45, col:57>\n"
                             "        `-ReturnStmt 0x29d1c00 <col:47, col:54>\n"
                             "          `-IntegerLiteral 0x29d1be0 <col:54> 'int' 0";
        ASSERT_EQUALS("", parse(clang));
    }

    void classTemplateDecl2() {
        const char clang[] = "|-ClassTemplateDecl 0x244e748 <a.cpp:1:1, col:59> col:25 C\n"
                             "| |-TemplateTypeParmDecl 0x244e5f8 <col:10, col:16> col:16 referenced class depth 0 index 0 T\n"
                             "| |-CXXRecordDecl 0x244e6b0 <col:19, col:59> col:25 class C definition\n"
                             "| | |-DefinitionData empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init\n"
                             "| | | |-DefaultConstructor exists trivial constexpr needs_implicit defaulted_is_constexpr\n"
                             "| | | |-CopyConstructor simple trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "| | | |-MoveConstructor exists simple trivial needs_implicit\n"
                             "| | | |-CopyAssignment trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "| | | |-MoveAssignment exists simple trivial needs_implicit\n"
                             "| | | `-Destructor simple irrelevant trivial needs_implicit\n"
                             "| | |-CXXRecordDecl 0x244e9b0 <col:19, col:25> col:25 implicit class C\n"
                             "| | |-AccessSpecDecl 0x244ea48 <col:29, col:35> col:29 public\n"
                             "| | `-CXXMethodDecl 0x244eb20 <col:37, col:57> col:39 foo 'T ()'\n"
                             "| |   `-CompoundStmt 0x244ec18 <col:45, col:57>\n"
                             "| |     `-ReturnStmt 0x244ec00 <col:47, col:54>\n"
                             "| |       `-IntegerLiteral 0x244ebe0 <col:54> 'int' 0\n"
                             "| `-ClassTemplateSpecializationDecl 0x244ed78 <col:1, col:59> col:25 class C definition\n"
                             "|   |-DefinitionData pass_in_registers empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init\n"
                             "|   | |-DefaultConstructor exists trivial constexpr defaulted_is_constexpr\n"
                             "|   | |-CopyConstructor simple trivial has_const_param implicit_has_const_param\n"
                             "|   | |-MoveConstructor exists simple trivial\n"
                             "|   | |-CopyAssignment trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "|   | |-MoveAssignment exists simple trivial needs_implicit\n"
                             "|   | `-Destructor simple irrelevant trivial needs_implicit\n"
                             "|   |-TemplateArgument type 'int'\n"
                             "|   |-CXXRecordDecl 0x244eff0 prev 0x244ed78 <col:19, col:25> col:25 implicit class C\n"
                             "|   |-AccessSpecDecl 0x244f088 <col:29, col:35> col:29 public\n"
                             "|   |-CXXMethodDecl 0x244f160 <col:37, col:57> col:39 used foo 'int ()'\n"
                             "|   | `-CompoundStmt 0x247cb40 <col:45, col:57>\n"
                             "|   |   `-ReturnStmt 0x247cb28 <col:47, col:54>\n"
                             "|   |     `-IntegerLiteral 0x244ebe0 <col:54> 'int' 0\n"
                             "|   |-CXXConstructorDecl 0x247c540 <col:25> col:25 implicit used constexpr C 'void () noexcept' inline default trivial\n"
                             "|   | `-CompoundStmt 0x247ca00 <col:25>\n"
                             "|   |-CXXConstructorDecl 0x247c658 <col:25> col:25 implicit constexpr C 'void (const C<int> &)' inline default trivial noexcept-unevaluated 0x247c658\n"
                             "|   | `-ParmVarDecl 0x247c790 <col:25> col:25 'const C<int> &'\n"
                             "|   `-CXXConstructorDecl 0x247c828 <col:25> col:25 implicit constexpr C 'void (C<int> &&)' inline default trivial noexcept-unevaluated 0x247c828\n"
                             "|     `-ParmVarDecl 0x247c960 <col:25> col:25 'C<int> &&'\n";
        ASSERT_EQUALS("class C { int foo ( ) { return 0 ; } C ( ) { } C ( const C<int> & ) = default ; C ( C<int> && ) = default ; } ;", parse(clang));
    }

    void conditionalExpr() {
        const char clang[] = "`-VarDecl 0x257cc88 <a.cpp:4:1, col:13> col:5 x 'int' cinit\n"
                             "  `-ConditionalOperator 0x257cda8 <col:9, col:13> 'int'\n"
                             "    |-ImplicitCastExpr 0x257cd60 <col:9> 'int' <LValueToRValue>\n"
                             "    | `-DeclRefExpr 0x257cce8 <col:9> 'int' lvalue Var 0x257cae0 'a' 'int'\n"
                             "    |-ImplicitCastExpr 0x257cd78 <col:11> 'int' <LValueToRValue>\n"
                             "    | `-DeclRefExpr 0x257cd10 <col:11> 'int' lvalue Var 0x257cb98 'b' 'int'\n"
                             "    `-ImplicitCastExpr 0x257cd90 <col:13> 'int' <LValueToRValue>\n"
                             "      `-DeclRefExpr 0x257cd38 <col:13> 'int' lvalue Var 0x257cc10 'c' 'int'";
        ASSERT_EQUALS("int x@1 = a ? b : c ;", parse(clang));
    }

    void compoundAssignOperator() {
        const char clang[] = "`-FunctionDecl 0x3570690 <1.cpp:2:1, col:25> col:6 f 'void ()'\n"
                             "  `-CompoundStmt 0x3570880 <col:10, col:25>\n"
                             "    `-CompoundAssignOperator 0x3570848 <col:19, col:22> 'int' lvalue '+=' ComputeLHSTy='int' ComputeResultTy='int'\n"
                             "      |-DeclRefExpr 0x3570800 <col:19> 'int' lvalue Var 0x3570788 'x' 'int'\n"
                             "      `-IntegerLiteral 0x3570828 <col:22> 'int' 1";
        ASSERT_EQUALS("void f ( ) { x += 1 ; }", parse(clang));
    }

    void continueStmt() {
        const char clang[] = "`-FunctionDecl 0x2c31b18 <1.c:1:1, col:34> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x2c31c40 <col:12, col:34>\n"
                             "    `-WhileStmt 0x2c31c20 <col:14, col:24>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-IntegerLiteral 0x2c31bf8 <col:21> 'int' 0\n"
                             "      `-ContinueStmt 0x2c31c18 <col:24>";
        ASSERT_EQUALS("void foo ( ) { while ( 0 ) { continue ; } }", parse(clang));
    }

    void cstyleCastExpr() {
        const char clang[] = "`-VarDecl 0x2336aa0 <1.c:1:1, col:14> col:5 x 'int' cinit\n"
                             "  `-CStyleCastExpr 0x2336b70 <col:9, col:14> 'int' <NoOp>\n"
                             "    `-CharacterLiteral 0x2336b40 <col:14> 'int' 97";
        ASSERT_EQUALS("int x@1 = ( int ) 'a' ;", parse(clang));
    }

    void cxxBoolLiteralExpr() {
        const char clang[] = "`-VarDecl 0x3940608 <a.cpp:1:1, col:10> col:6 x 'bool' cinit\n"
                             "  `-CXXBoolLiteralExpr 0x39406a8 <col:10> 'bool' true";
        ASSERT_EQUALS("bool x@1 = true ;", parse(clang));
    }

    void cxxConstructorDecl1() {
        const char clang[] = "|-CXXConstructorDecl 0x428e890 <a.cpp:11:1, col:24> col:11 C 'void ()'\n"
                             "| `-CompoundStmt 0x428ea58 <col:15, col:24>\n"
                             "|   `-BinaryOperator 0x428ea30 <col:17, col:21> 'int' lvalue '='\n"
                             "|     |-MemberExpr 0x428e9d8 <col:17> 'int' lvalue ->x 0x428e958\n"
                             "|     | `-CXXThisExpr 0x428e9c0 <col:17> 'C *' this\n"
                             "|     `-IntegerLiteral 0x428ea10 <col:21> 'int' 0\n"
                             "`-FieldDecl 0x428e958 <col:26, col:30> col:30 referenced x 'int'";
        ASSERT_EQUALS("C ( ) { this . x@1 = 0 ; } int x@1", parse(clang));
    }

    void cxxConstructorDecl2() {
        const char clang[] = "`-CXXConstructorDecl 0x1c208c0 <a.cpp:1:11> col:11 implicit constexpr basic_string 'void (std::basic_string<char> &&)' inline default trivial noexcept-unevaluated 0x1c208c0\n"
                             "  `-ParmVarDecl 0x1c209f0 <col:11> col:11 'std::basic_string<char> &&'";
        ASSERT_EQUALS("basic_string ( std::basic_string<char> && ) = default ;", parse(clang));
    }

    void cxxConstructExpr1() {
        const char clang[] = "`-FunctionDecl 0x2dd7940 <a.cpp:2:1, col:30> col:5 f 'Foo (Foo)'\n"
                             "  |-ParmVarDecl 0x2dd7880 <col:7, col:11> col:11 used foo 'Foo'\n"
                             "  `-CompoundStmt 0x2dd80c0 <col:16, col:30>\n"
                             "    `-ReturnStmt 0x2dd80a8 <col:18, col:25>\n"
                             "      `-CXXConstructExpr 0x2dd8070 <col:25> 'Foo' 'void (Foo &&) noexcept'\n"
                             "        `-ImplicitCastExpr 0x2dd7f28 <col:25> 'Foo' xvalue <NoOp>\n"
                             "          `-DeclRefExpr 0x2dd7a28 <col:25> 'Foo' lvalue ParmVar 0x2dd7880 'foo' 'Foo'";
        ASSERT_EQUALS("Foo f ( Foo foo@1 ) { return foo@1 ; }", parse(clang));
    }

    void cxxConstructExpr2() {
        const char clang[] = "`-FunctionDecl 0x3e44180 <1.cpp:2:1, col:30> col:13 f 'std::string ()'\n"
                             "  `-CompoundStmt 0x3e4cb80 <col:17, col:30>\n"
                             "    `-ReturnStmt 0x3e4cb68 <col:19, col:27>\n"
                             "      `-CXXConstructExpr 0x3e4cb38 <col:26, col:27> 'std::string':'std::__cxx11::basic_string<char>' '....' list";
        ASSERT_EQUALS("std :: string f ( ) { return std :: string ( ) ; }", parse(clang));
    }

    void cxxConstructExpr3() {
        const char clang[] = "`-FunctionDecl 0x2c585b8 <1.cpp:4:1, col:39> col:6 f 'void ()'\n"
                             "  `-CompoundStmt 0x2c589d0 <col:10, col:39>\n"
                             "    |-DeclStmt 0x2c586d0 <col:12, col:19>\n"
                             "    | `-VarDecl 0x2c58670 <col:12, col:18> col:18 used p 'char *'\n"
                             "    `-DeclStmt 0x2c589b8 <col:21, col:37>\n"
                             "      `-VarDecl 0x2c58798 <col:21, col:36> col:33 s 'std::string':'std::__cxx11::basic_string<char>' callinit\n"
                             "        `-ExprWithCleanups 0x2c589a0 <col:33, col:36> 'std::string':'std::__cxx11::basic_string<char>'\n"
                             "          `-CXXConstructExpr 0x2c58960 <col:33, col:36> 'std::string':'std::__cxx11::basic_string<char>' 'void (const char *, const std::allocator<char> &)'\n"
                             "            |-ImplicitCastExpr 0x2c58870 <col:35> 'const char *' <NoOp>\n"
                             "            | `-ImplicitCastExpr 0x2c58858 <col:35> 'char *' <LValueToRValue>\n"
                             "            |   `-DeclRefExpr 0x2c58750 <col:35> 'char *' lvalue Var 0x2c58670 'p' 'char *'\n"
                             "            `-CXXDefaultArgExpr 0x2c58940 <<invalid sloc>> 'const std::allocator<char>':'const std::allocator<char>' lvalue\n";
        ASSERT_EQUALS("void f ( ) { char * p@1 ; std :: string s@2 ( p@1 ) ; }", parse(clang));
    }

    void cxxDeleteExpr() {
        const char clang[] = "|-FunctionDecl 0x2e0e740 <1.cpp:1:1, col:28> col:6 f 'void (int *)'\n"
                             "| |-ParmVarDecl 0x2e0e680 <col:8, col:13> col:13 used p 'int *'\n"
                             "| `-CompoundStmt 0x2e0ee70 <col:16, col:28>\n"
                             "|   `-CXXDeleteExpr 0x2e0ee48 <col:18, col:25> 'void' Function 0x2e0ebb8 'operator delete' 'void (void *) noexcept'\n"
                             "|     `-ImplicitCastExpr 0x2e0e850 <col:25> 'int *' <LValueToRValue>\n"
                             "|       `-DeclRefExpr 0x2e0e828 <col:25> 'int *' lvalue ParmVar 0x2e0e680 'p' 'int *'";
        ASSERT_EQUALS("void f ( int * p@1 ) { delete p@1 ; }", parse(clang));
    }

    void cxxDestructorDecl() {
        const char clang[] = "`-CXXRecordDecl 0x8ecd60 <1.cpp:1:1, line:4:1> line:1:8 struct S definition\n"
                             "  `-CXXDestructorDecl 0x8ed088 <line:3:3, col:9> col:3 ~S 'void () noexcept'\n"
                             "    `-CompoundStmt 0x8ed1a8 <col:8, col:9>";
        ASSERT_EQUALS("struct S { ~S ( ) { } } ;", parse(clang));
    }

    void cxxForRangeStmt1() {
        const char clang[] = "`-FunctionDecl 0x4280820 <a.cpp:4:1, line:8:1> line:4:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x42810f0 <col:12, line:8:1>\n"
                             "    `-CXXForRangeStmt 0x4281090 <line:5:3, line:7:3>\n"
                             "      |-DeclStmt 0x4280c30 <line:5:17>\n"
                             "      | `-VarDecl 0x42809c8 <col:17> col:17 implicit referenced __range1 'char const (&)[6]' cinit\n"
                             "      |   `-DeclRefExpr 0x42808c0 <col:17> 'const char [6]' lvalue Var 0x4280678 'hello' 'const char [6]'\n"
                             "      |-DeclStmt 0x4280ef8 <col:15>\n"
                             "      | `-VarDecl 0x4280ca8 <col:15> col:15 implicit used __begin1 'const char *':'const char *' cinit\n"
                             "      |   `-ImplicitCastExpr 0x4280e10 <col:15> 'const char *' <ArrayToPointerDecay>\n"
                             "      |     `-DeclRefExpr 0x4280c48 <col:15> 'char const[6]' lvalue Var 0x42809c8 '__range1' 'char const (&)[6]'\n"
                             "      |-DeclStmt 0x4280f10 <col:15>\n"
                             "      | `-VarDecl 0x4280d18 <col:15, col:17> col:15 implicit used __end1 'const char *':'const char *' cinit\n"
                             "      |   `-BinaryOperator 0x4280e60 <col:15, col:17> 'const char *' '+'\n"
                             "      |     |-ImplicitCastExpr 0x4280e48 <col:15> 'const char *' <ArrayToPointerDecay>\n"
                             "      |     | `-DeclRefExpr 0x4280c70 <col:15> 'char const[6]' lvalue Var 0x42809c8 '__range1' 'char const (&)[6]'\n"
                             "      |     `-IntegerLiteral 0x4280e28 <col:17> 'long' 6\n"
                             "      |-BinaryOperator 0x4280fa8 <col:15> 'bool' '!='\n"
                             "      | |-ImplicitCastExpr 0x4280f78 <col:15> 'const char *':'const char *' <LValueToRValue>\n"
                             "      | | `-DeclRefExpr 0x4280f28 <col:15> 'const char *':'const char *' lvalue Var 0x4280ca8 '__begin1' 'const char *':'const char *'\n"
                             "      | `-ImplicitCastExpr 0x4280f90 <col:15> 'const char *':'const char *' <LValueToRValue>\n"
                             "      |   `-DeclRefExpr 0x4280f50 <col:15> 'const char *':'const char *' lvalue Var 0x4280d18 '__end1' 'const char *':'const char *'\n"
                             "      |-UnaryOperator 0x4280ff8 <col:15> 'const char *':'const char *' lvalue prefix '++'\n"
                             "      | `-DeclRefExpr 0x4280fd0 <col:15> 'const char *':'const char *' lvalue Var 0x4280ca8 '__begin1' 'const char *':'const char *'\n"
                             "      |-DeclStmt 0x4280958 <col:8, col:22>\n"
                             "      | `-VarDecl 0x42808f8 <col:8, col:15> col:13 c1 'char' cinit\n"
                             "      |   `-ImplicitCastExpr 0x4281078 <col:15> 'char' <LValueToRValue>\n"
                             "      |     `-UnaryOperator 0x4281058 <col:15> 'const char' lvalue prefix '*' cannot overflow\n"
                             "      |       `-ImplicitCastExpr 0x4281040 <col:15> 'const char *':'const char *' <LValueToRValue>\n"
                             "      |         `-DeclRefExpr 0x4281018 <col:15> 'const char *':'const char *' lvalue Var 0x4280ca8 '__begin1' 'const char *':'const char *'\n"
                             "      `-CompoundStmt 0x42810e0 <col:24, line:7:3>";
        ASSERT_EQUALS("void foo ( ) { for ( char c1@1 : hello ) { } }",
                      parse(clang));
    }

    void cxxForRangeStmt2() {
        // clang 9
        const char clang[] = "`-FunctionDecl 0xc15d98 <a.cpp:3:1, col:36> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0xc16668 <col:12, col:36>\n"
                             "    `-CXXForRangeStmt 0xc165f8 <col:14, col:34>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-DeclStmt 0xc161c0 <col:25>\n"
                             "      | `-VarDecl 0xc15f48 <col:25> col:25 implicit referenced __range1 'int const (&)[4]' cinit\n"
                             "      |   `-DeclRefExpr 0xc15e38 <col:25> 'const int [4]' lvalue Var 0xc15ac0 'values' 'const int [4]'\n"
                             "      |-DeclStmt 0xc16498 <col:24>\n"
                             "      | `-VarDecl 0xc16228 <col:24> col:24 implicit used __begin1 'const int *':'const int *' cinit\n"
                             "      |   `-ImplicitCastExpr 0xc163b0 <col:24> 'const int *' <ArrayToPointerDecay>\n"
                             "      |     `-DeclRefExpr 0xc161d8 <col:24> 'int const[4]' lvalue Var 0xc15f48 '__range1' 'int const (&)[4]' non_odr_use_constant\n"
                             "      |-DeclStmt 0xc164b0 <col:24>\n"
                             "      | `-VarDecl 0xc162a0 <col:24, col:25> col:24 implicit used __end1 'const int *':'const int *' cinit\n"
                             "      |   `-BinaryOperator 0xc16400 <col:24, col:25> 'const int *' '+'\n"
                             "      |     |-ImplicitCastExpr 0xc163e8 <col:24> 'const int *' <ArrayToPointerDecay>\n"
                             "      |     | `-DeclRefExpr 0xc161f8 <col:24> 'int const[4]' lvalue Var 0xc15f48 '__range1' 'int const (&)[4]' non_odr_use_constant\n"
                             "      |     `-IntegerLiteral 0xc163c8 <col:25> 'long' 4\n"
                             "      |-BinaryOperator 0xc16538 <col:24> 'bool' '!='\n"
                             "      | |-ImplicitCastExpr 0xc16508 <col:24> 'const int *':'const int *' <LValueToRValue>\n"
                             "      | | `-DeclRefExpr 0xc164c8 <col:24> 'const int *':'const int *' lvalue Var 0xc16228 '__begin1' 'const int *':'const int *'\n"
                             "      | `-ImplicitCastExpr 0xc16520 <col:24> 'const int *':'const int *' <LValueToRValue>\n"
                             "      |   `-DeclRefExpr 0xc164e8 <col:24> 'const int *':'const int *' lvalue Var 0xc162a0 '__end1' 'const int *':'const int *'\n"
                             "      |-UnaryOperator 0xc16578 <col:24> 'const int *':'const int *' lvalue prefix '++'\n"
                             "      | `-DeclRefExpr 0xc16558 <col:24> 'const int *':'const int *' lvalue Var 0xc16228 '__begin1' 'const int *':'const int *'\n"
                             "      |-DeclStmt 0xc15ed8 <col:19, col:31>\n"
                             "      | `-VarDecl 0xc15e70 <col:19, col:24> col:23 v 'int' cinit\n"
                             "      |   `-ImplicitCastExpr 0xc165e0 <col:24> 'int' <LValueToRValue>\n"
                             "      |     `-UnaryOperator 0xc165c8 <col:24> 'const int' lvalue prefix '*' cannot overflow\n"
                             "      |       `-ImplicitCastExpr 0xc165b0 <col:24> 'const int *':'const int *' <LValueToRValue>\n"
                             "      |         `-DeclRefExpr 0xc16590 <col:24> 'const int *':'const int *' lvalue Var 0xc16228 '__begin1' 'const int *':'const int *'\n"
                             "      `-CompoundStmt 0xc16658 <col:33, col:34>";
        ASSERT_EQUALS("void foo ( ) { for ( int v@1 : values ) { } }",
                      parse(clang));
    }

    void cxxFunctionalCastExpr() {
        const char clang[] = "`-FunctionDecl 0x156fe98 <a.cpp:1:1, line:3:1> line:1:5 main 'int (int, char **)'\n"
                             "  |-ParmVarDecl 0x156fd00 <col:10, col:14> col:14 argc 'int'\n"
                             "  |-ParmVarDecl 0x156fdb8 <col:20, col:27> col:27 argv 'char **'\n"
                             "  `-CompoundStmt 0x1596410 <line:2:1, line:2:1>\n"
                             "    |-DeclStmt 0x15946a8 <line:2:15, line:2:29>\n"
                             "    | `-VarDecl 0x1570118 <line:2:15, line:2:28> col:11 used setCode 'MyVar<int>':'MyVar<int>' cinit\n"
                             "    |   `-ExprWithCleanups 0x1594690 <line:2:15, line:3:28> 'MyVar<int>':'MyVar<int>'\n"
                             "    |     `-CXXConstructExpr 0x1594660 <line:2:15, line:3:28> 'MyVar<int>':'MyVar<int>' 'void (MyVar<int> &&) noexcept' elidable\n"
                             "    |       `-MaterializeTemporaryExpr 0x1592b68 <line:2:15, line:3:28> 'MyVar<int>':'MyVar<int>' xvalue\n"
                             "    |         `-CXXFunctionalCastExpr 0x1592b40 <line:2:15, line:3:28> 'MyVar<int>':'MyVar<int>' functional cast to MyVar<int> <ConstructorConversion>\n"
                             "    |           `-CXXConstructExpr 0x15929f0 <line:2:15, line:3:28> 'MyVar<int>':'MyVar<int>' 'void (int)'\n"
                             "    |             `-IntegerLiteral 0x1570248 <col:27> 'int' 5\n";
        ASSERT_EQUALS("int main ( int argc@1 , char * * argv@2 ) { MyVar<int> setCode@3 = MyVar<int> ( 5 ) ; }",
                      parse(clang));
    }

    void cxxMemberCall() {
        const char clang[] = "`-FunctionDecl 0x320dc80 <a.cpp:2:1, col:33> col:6 bar 'void ()'\n"
                             "  `-CompoundStmt 0x323bb08 <col:12, col:33>\n"
                             "    |-DeclStmt 0x323ba40 <col:14, col:22>\n"
                             "    | `-VarDecl 0x320df28 <col:14, col:21> col:21 used c 'C<int>':'C<int>' callinit\n"
                             "    |   `-CXXConstructExpr 0x323ba10 <col:21> 'C<int>':'C<int>' 'void () noexcept'\n"
                             "    `-CXXMemberCallExpr 0x323bab8 <col:24, col:30> 'int':'int'\n"
                             "      `-MemberExpr 0x323ba80 <col:24, col:26> '<bound member function type>' .foo 0x320e160\n"
                             "        `-DeclRefExpr 0x323ba58 <col:24> 'C<int>':'C<int>' lvalue Var 0x320df28 'c' 'C<int>':'C<int>'";
        ASSERT_EQUALS("void bar ( ) { C<int> c@1 ( C<int> ( ) ) ; c@1 . foo ( ) ; }", parse(clang));
    }

    void cxxMethodDecl1() {
        const char clang[] = "|-CXXMethodDecl 0x55c786f5ad60 <a.cpp:56:5, col:179> col:10 analyzeFile '_Bool (const std::string &, const std::string &, const std::string &, unsigned long long, std::list<ErrorLogger::ErrorMessage> *)'\n"
                             "| |-ParmVarDecl 0x55c786f5a4c8 <col:22, col:41> col:41 buildDir 'const std::string &'\n"
                             "| |-ParmVarDecl 0x55c786f5a580 <col:51, col:70> col:70 sourcefile 'const std::string &'\n"
                             "| |-ParmVarDecl 0x55c786f5a638 <col:82, col:101> col:101 cfg 'const std::string &'\n"
                             "| |-ParmVarDecl 0x55c786f5a6a8 <col:106, col:125> col:125 checksum 'unsigned long long'\n"
                             "| |-ParmVarDecl 0x55c786f5ac00 <col:135, col:173> col:173 errors 'std::list<ErrorLogger::ErrorMessage> *'\n"
                             "  `-CompoundStmt 0x0 <>";
        ASSERT_EQUALS("_Bool analyzeFile ( const std :: string & buildDir@1 , const std :: string & sourcefile@2 , const std :: string & cfg@3 , unsigned long long checksum@4 , std::list<ErrorLogger::ErrorMessage> * errors@5 ) { }", parse(clang));
    }

    void cxxMethodDecl2() { // "unexpanded" template method
        const char clang[] = "`-CXXMethodDecl 0x220ecb0 parent 0x21e4c28 prev 0x21e5338 <a.cpp:11:1, line:18:1> line:14:1 find 'const typename char_traits<_CharT>::char_type *(const char_traits::char_type *, int, const char_traits::char_type &)'\n"
                             "  `-CompoundStmt 0x220ede0 <line:15:1, line:18:1>\n"
                             "    `-ReturnStmt 0x220edd0 <line:17:5, col:12>\n"
                             "      `-IntegerLiteral 0x220edb0 <col:12> 'int' 0";
        ASSERT_EQUALS("", parse(clang));
    }

    void cxxMethodDecl3() {
        const char clang[] = "|-CXXRecordDecl 0x21cca40 <2.cpp:2:1, line:4:1> line:2:7 class Fred definition\n"
                             "| |-DefinitionData pass_in_registers empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init\n"
                             "| | |-DefaultConstructor exists trivial constexpr needs_implicit defaulted_is_constexpr\n"
                             "| | |-CopyConstructor simple trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "| | |-MoveConstructor exists simple trivial needs_implicit\n"
                             "| | |-CopyAssignment trivial has_const_param needs_implicit implicit_has_const_param\n"
                             "| | |-MoveAssignment exists simple trivial needs_implicit\n"
                             "| | `-Destructor simple irrelevant trivial needs_implicit\n"
                             "| |-CXXRecordDecl 0x21ccb58 <col:1, col:7> col:7 implicit class Fred\n"
                             "| `-CXXMethodDecl 0x21ccc68 <line:3:1, col:10> col:6 foo 'void ()'\n"
                             "`-CXXMethodDecl 0x21ccd60 parent 0x21cca40 prev 0x21ccc68 <line:6:1, col:19> col:12 foo 'void ()'\n"
                             "  `-CompoundStmt 0x21cce50 <col:18, col:19>";
        ASSERT_EQUALS("class Fred { void foo ( ) ; } ; void Fred :: foo ( ) { }", parse(clang));
    }

    void cxxMethodDecl4() {
        const char clang[] = "|-ClassTemplateSpecializationDecl 0x15d82f8 <a.cpp:7:3, line:18:3> line:8:12 struct char_traits definition\n"
                             "| |-TemplateArgument type 'char'\n"
                             "| | `-BuiltinType 0x15984c0 'char'\n"
                             "| |-CXXRecordDecl 0x15d8520 <col:5, col:12> col:12 implicit struct char_traits\n"
                             "| |-CXXMethodDecl 0x15d8738 <line:12:7, line:16:7> line:13:7 move 'char *(char *)' static\n"
                             "| | |-ParmVarDecl 0x15d8630 <col:12, col:18> col:18 used __s1 'char *'\n"
                             "| | `-CompoundStmt 0x15d88e8 <line:14:7, line:16:7>\n";
        ASSERT_EQUALS("struct char_traits<char> { static char * move ( char * __s1@1 ) { } } ;",
                      parse(clang));
    }

    void cxxNewExpr1() {
        const char clang[] = "|-VarDecl 0x3a97680 <1.cpp:2:1, col:14> col:6 i 'int *' cinit\n"
                             "| `-CXXNewExpr 0x3a97d18 <col:10, col:14> 'int *' Function 0x3a97778 'operator new' 'void *(unsigned long)'\n"
                             "`-VarDecl 0x3a97d80 <line:3:1, col:21> col:6 j 'int *' cinit\n"
                             "  `-CXXNewExpr 0x3a97e68 <col:10, col:21> 'int *' array Function 0x3a978c0 'operator new[]' 'void *(unsigned long)'\n"
                             "    `-ImplicitCastExpr 0x3a97e18 <col:18> 'unsigned long' <IntegralCast>\n"
                             "      `-IntegerLiteral 0x3a97de0 <col:18> 'int' 100";
        ASSERT_EQUALS("int * i@1 = new int ; int * j@2 = new int [ 100 ] ;",
                      parse(clang));
    }

    void cxxNewExpr2() {
        const char clang[] = "|-FunctionDecl 0x59a188 <a.cpp:7:1, line:9:1> line:7:11 f 'struct S *()'\n"
                             "| `-CompoundStmt 0x5c4318 <col:15, line:9:1>\n"
                             "|   `-ReturnStmt 0x5c4308 <line:8:3, col:14>\n"
                             "|     `-CXXNewExpr 0x5c42c8 <col:10, col:14> 'S *' Function 0x59a378 'operator new' 'void *(unsigned long)'\n"
                             "|       `-CXXConstructExpr 0x5c42a0 <col:14> 'S' 'void () noexcept'";
        ASSERT_EQUALS("struct S * f ( ) { return new S ( ) ; }",
                      parse(clang));
    }

    void cxxNullPtrLiteralExpr() {
        const char clang[] = "`-VarDecl 0x2a7d650 <1.cpp:1:1, col:17> col:13 p 'const char *' cinit\n"
                             "  `-ImplicitCastExpr 0x2a7d708 <col:17> 'const char *' <NullToPointer>\n"
                             "    `-CXXNullPtrLiteralExpr 0x2a7d6f0 <col:17> 'nullptr_t'";
        ASSERT_EQUALS("const char * p@1 = nullptr ;", parse(clang));
    }

    void cxxOperatorCallExpr() {
        const char clang[] = "`-FunctionDecl 0x3c099f0 <a.cpp:2:1, col:24> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x3c37308 <col:12, col:24>\n"
                             "    |-DeclStmt 0x3c0a060 <col:14, col:17>\n"
                             "    | `-VarDecl 0x3c09ae0 <col:14, col:16> col:16 used c 'C' callinit\n"
                             "    |   `-CXXConstructExpr 0x3c0a030 <col:16> 'C' 'void () noexcept'\n"
                             "    `-CXXOperatorCallExpr 0x3c372c0 <col:19, col:21> 'void'\n"
                             "      |-ImplicitCastExpr 0x3c372a8 <col:20> 'void (*)(int)' <FunctionToPointerDecay>\n"
                             "      | `-DeclRefExpr 0x3c37250 <col:20> 'void (int)' lvalue CXXMethod 0x3c098c0 'operator=' 'void (int)'\n"
                             "      |-DeclRefExpr 0x3c0a078 <col:19> 'C' lvalue Var 0x3c09ae0 'c' 'C'\n"
                             "      `-IntegerLiteral 0x3c0a0a0 <col:21> 'int' 4";
        ASSERT_EQUALS("void foo ( ) { C c@1 ( C ( ) ) ; c@1 . operator= ( 4 ) ; }", parse(clang));
    }

    void cxxRecordDecl1() {
        const char* clang = "`-CXXRecordDecl 0x34cc5f8 <1.cpp:2:1, col:7> col:7 class Foo";
        ASSERT_EQUALS("class Foo ;", parse(clang));

        clang = "`-CXXRecordDecl 0x34cc5f8 <C:\\Foo\\Bar Baz\\1.cpp:2:1, col:7> col:7 class Foo";
        ASSERT_EQUALS("class Foo ;", parse(clang));

        clang = "`-CXXRecordDecl 0x34cc5f8 <C:/Foo/Bar Baz/1.cpp:2:1, col:7> col:7 class Foo";
        ASSERT_EQUALS("class Foo ;", parse(clang));
    }

    void cxxRecordDecl2() {
        const char clang[] = "`-CXXRecordDecl 0x34cc5f8 <1.cpp:2:1, col:7> col:7 struct Foo definition";
        ASSERT_EQUALS("struct Foo { } ;", parse(clang));
    }

    void cxxRecordDeclDerived() {
        const char clang[] = "|-CXXRecordDecl 0x19ccd38 <e.cpp:4:1, line:6:1> line:4:8 referenced struct base definition\n"
                             "| `-VarDecl 0x19ccf00 <line:5:5, col:35> col:27 value 'const bool' static constexpr cinit\n"
                             "|   |-value: Int 0\n"
                             "|   `-CXXBoolLiteralExpr 0x19ccf68 <col:35> 'bool' false\n"
                             "`-CXXRecordDecl 0x19ccfe8 <line:8:1, col:32> col:8 struct derived definition\n"
                             "  |-public 'base'\n"
                             "  `-CXXRecordDecl 0x19cd150 <col:1, col:8> col:8 implicit struct derived";

        ASSERT_EQUALS("struct base { static const bool value@1 = false ; } ; struct derived : public base { } ;", parse(clang));
    }

    void cxxStaticCastExpr1() {
        const char clang[] = "`-VarDecl 0x2e0e650 <a.cpp:2:1, col:27> col:5 a 'int' cinit\n"
                             "  `-CXXStaticCastExpr 0x2e0e728 <col:9, col:27> 'int' static_cast<int> <NoOp>\n"
                             "    `-IntegerLiteral 0x2e0e6f0 <col:26> 'int' 0";
        ASSERT_EQUALS("int a@1 = static_cast<int> ( 0 ) ;", parse(clang));
    }

    void cxxStaticCastExpr2() {
        const char clang[] = "`-VarDecl 0x2e0e650 <a.cpp:2:1, col:27> col:5 a 'int' cinit\n"
                             "  `-CXXStaticCastExpr 0x3e453e8 <col:12> 'std::_Rb_tree_iterator<std::pair<const std::__cxx11::basic_string<char>, Library::AllocFunc> >' xvalue static_cast<struct std::_Rb_tree_iterator<struct std::pair<const class std::__cxx11::basic_string<char>, struct Library::AllocFunc> > &&> <NoOp>\n"
                             "    `-DeclRefExpr 0x3e453b0 <col:12> 'std::_Rb_tree_iterator<std::pair<const std::__cxx11::basic_string<char>, Library::AllocFunc> >' lvalue ParmVar 0x3e45250 '' 'std::_Rb_tree_iterator<std::pair<const std::__cxx11::basic_string<char>, Library::AllocFunc> > &&'";
        ASSERT_EQUALS("int a@1 = static_cast<structstd::_Rb_tree_iterator<structstd::pair<constclassstd::__cxx11::basic_string<char>,structLibrary::AllocFunc>>&&> ( <NoName> ) ;", parse(clang));
    }

    void cxxStaticCastExpr3() {
        const char clang[] = "`-ClassTemplateSpecializationDecl 0xd842d8 <a.cpp:4:3, line:7:3> line:4:21 struct char_traits definition\n"
                             "  |-TemplateArgument type 'char'\n"
                             "  | `-BuiltinType 0xd444c0 'char'\n"
                             "  |-CXXRecordDecl 0xd84500 <col:14, col:21> col:21 implicit struct char_traits\n"
                             "  |-TypedefDecl 0xd845a0 <line:5:7, col:20> col:20 referenced char_type 'char'\n"
                             "  | `-BuiltinType 0xd444c0 'char'\n"
                             "  `-CXXMethodDecl 0xd847b0 <line:6:7, col:80> col:18 assign 'char_traits<char>::char_type *(char_traits<char>::char_type *)'\n"
                             "    |-ParmVarDecl 0xd84670 <col:25, col:36> col:36 used __s 'char_traits<char>::char_type *'\n"
                             "    `-CompoundStmt 0xd848f8 <col:41, col:80>\n"
                             "      `-ReturnStmt 0xd848e8 <col:43, col:77>\n"
                             "        `-CXXStaticCastExpr 0xd848b8 <col:50, col:77> 'char_traits<char>::char_type *' static_cast<char_traits<char>::char_type *> <NoOp>\n"
                             "          `-ImplicitCastExpr 0xd848a0 <col:74> 'char_traits<char>::char_type *' <LValueToRValue> part_of_explicit_cast\n"
                             "            `-DeclRefExpr 0xd84870 <col:74> 'char_traits<char>::char_type *' lvalue ParmVar 0xd84670 '__s' 'char_traits<char>::char_type *'\n";

        ASSERT_EQUALS("struct char_traits<char> { typedef char char_type ; char_traits<char>::char_type * assign ( char_traits<char>::char_type * __s@1 ) { return static_cast<char_traits<char>::char_type*> ( __s@1 ) ; } } ;", parse(clang));
    }

    void cxxStdInitializerListExpr() {
        const char clang[] = "`-VarDecl 0x2f92060 <1.cpp:3:1, col:25> col:18 x 'std::vector<int>':'std::vector<int, std::allocator<int> >' listinit\n"
                             "  `-ExprWithCleanups 0x2fb0b40 <col:18, col:25> 'std::vector<int>':'std::vector<int, std::allocator<int> >'\n"
                             "    `-CXXConstructExpr 0x2fb0b00 <col:18, col:25> 'std::vector<int>':'std::vector<int, std::allocator<int> >' 'void (initializer_list<std::vector<int, std::allocator<int> >::value_type>, const std::vector<int, std::allocator<int> >::allocator_type &)' list std::initializer_list\n"
                             "      |-CXXStdInitializerListExpr 0x2fb0928 <col:19, col:25> 'initializer_list<std::vector<int, std::allocator<int> >::value_type>':'std::initializer_list<int>'\n"
                             "      | `-MaterializeTemporaryExpr 0x2fb0910 <col:19, col:25> 'const int [3]' xvalue\n"
                             "      |   `-InitListExpr 0x2fb08b8 <col:19, col:25> 'const int [3]'\n"
                             "      |     |-IntegerLiteral 0x2f920c0 <col:20> 'int' 1\n"
                             "      |     |-IntegerLiteral 0x2f920e0 <col:22> 'int' 2\n"
                             "      |     `-IntegerLiteral 0x2f92100 <col:24> 'int' 3\n"
                             "      `-CXXDefaultArgExpr 0x2fb0ae0 <<invalid sloc>> 'const std::vector<int, std::allocator<int> >::allocator_type':'const std::allocator<int>' lvalue";
        ASSERT_EQUALS("std :: vector<int> x@1 { 1 , 2 , 3 } ;", parse(clang));
    }

    void cxxThrowExpr() {
        const char clang[] = "`-FunctionDecl 0x3701690 <1.cpp:2:1, col:23> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x37017b0 <col:12, col:23>\n"
                             "    `-CXXThrowExpr 0x3701790 <col:14, col:20> 'void'\n"
                             "      `-IntegerLiteral 0x3701770 <col:20> 'int' 1";
        ASSERT_EQUALS("void foo ( ) { throw 1 ; }", parse(clang));
    }

    void defaultStmt() {
        const char clang[] = "`-FunctionDecl 0x18476b8 <1.c:3:1, line:9:1> line:3:5 foo 'int (int)'\n"
                             "  |-ParmVarDecl 0x18475e0 <col:9, col:13> col:13 used rc 'int'\n"
                             "  `-CompoundStmt 0x1847868 <line:4:1, line:9:1>\n"
                             "    `-SwitchStmt 0x18477e0 <line:5:3, line:8:3>\n"
                             "      |-ImplicitCastExpr 0x18477c8 <line:5:10> 'int' <LValueToRValue>\n"
                             "      | `-DeclRefExpr 0x18477a8 <col:10> 'int' lvalue ParmVar 0x18475e0 'rc' 'int'\n"
                             "      `-CompoundStmt 0x1847850 <col:14, line:8:3>\n"
                             "        `-DefaultStmt 0x1847830 <line:6:3, line:7:12>\n"
                             "          `-ReturnStmt 0x1847820 <col:5, col:12>\n"
                             "            `-IntegerLiteral 0x1847800 <col:12> 'int' 1";
        ASSERT_EQUALS("int foo ( int rc@1 ) { switch ( rc@1 ) { default : return 1 ; } }", parse(clang));
    }

    void doStmt() {
        const char clang[] = "`-FunctionDecl 0x27fbbc8 <a.cpp:2:1, col:34> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x27fbd08 <col:12, col:34>\n"
                             "    `-DoStmt 0x27fbce8 <col:14, col:31>\n"
                             "      |-CompoundStmt 0x27fbcb0 <col:17, col:22>\n"
                             "      | `-UnaryOperator 0x27fbc90 <col:18, col:19> 'int' postfix '++'\n"
                             "      |   `-DeclRefExpr 0x27fbc68 <col:18> 'int' lvalue Var 0x27fbae0 'x' 'int'\n"
                             "      `-IntegerLiteral 0x27fbcc8 <col:30> 'int' 1";
        ASSERT_EQUALS("void foo ( ) { do { ++ x ; } while ( 1 ) ; }", parse(clang));
    }

    void enumDecl1() {
        const char clang[] = "`-EnumDecl 0x2660660 <a.cpp:3:1, col:16> col:6 referenced abc\n"
                             "  |-EnumConstantDecl 0x2660720 <col:11> col:11 referenced a 'abc'\n"
                             "  |-EnumConstantDecl 0x2660768 <col:13> col:13 b 'abc'\n"
                             "  `-EnumConstantDecl 0x26607b0 <col:15> col:15 c 'abc'";
        ASSERT_EQUALS("enum abc { a , b , c }", parse(clang));
    }

    void enumDecl2() {
        const char clang[] = "`-EnumDecl 0xb55d50 <2.cpp:4:3, col:44> col:8 syntax_option_type 'unsigned int'";
        ASSERT_EQUALS("enum syntax_option_type : unsigned int { }", parse(clang));
    }

    void enumDecl3() {
        const char clang[] = "|-EnumDecl 0x1586e48 <2.cpp:1:3, line:5:3> line:1:8 __syntax_option\n"
                             "| |-EnumConstantDecl 0x1586f18 <line:3:5> col:5 referenced _S_polynomial '__syntax_option'\n"
                             "| `-EnumConstantDecl 0x1586f68 <line:4:5> col:5 _S_syntax_last '__syntax_option'";
        ASSERT_EQUALS("enum __syntax_option { _S_polynomial , _S_syntax_last }", parse(clang));
    }

    void enumDecl4() {
        const char clang[] = "|-EnumDecl 0xace1f8 <e1.cpp:3:1, col:51> col:1\n"
                             "| |-EnumConstantDecl 0xace2c8 <col:7> col:7 A '(anonymous enum at e1.cpp:3:1)'\n"
                             "| |-EnumConstantDecl 0xace318 <col:16> col:16 B '(anonymous enum at e1.cpp:3:1)'\n"
                             "| `-EnumConstantDecl 0xace3b8 <col:46> col:46 referenced C '(anonymous enum at e1.cpp:3:1)'\n"
                             "`-VarDecl 0xace470 <col:1, col:66> col:53 x 'enum (anonymous enum at e1.cpp:3:1)':'(anonymous enum at e1.cpp:3:1)' cinit\n"
                             "  `-DeclRefExpr 0xace520 <col:66> '(anonymous enum at e1.cpp:3:1)' EnumConstant 0xace3b8 'C' '(anonymous enum at e1.cpp:3:1)'";
        ASSERT_EQUALS("enum { A , B , C } x@1 = C ;", parse(clang));
    }

    void forStmt() {
        const char clang[] = "`-FunctionDecl 0x2f93ae0 <1.c:1:1, col:56> col:5 main 'int ()'\n"
                             "  `-CompoundStmt 0x2f93dc0 <col:12, col:56>\n"
                             "    |-ForStmt 0x2f93d50 <col:14, col:44>\n"
                             "    | |-DeclStmt 0x2f93c58 <col:19, col:28>\n"
                             "    | | `-VarDecl 0x2f93bd8 <col:19, col:27> col:23 used i 'int' cinit\n"
                             "    | |   `-IntegerLiteral 0x2f93c38 <col:27> 'int' 0\n"
                             "    | |-<<<NULL>>>\n"
                             "    | |-BinaryOperator 0x2f93cd0 <col:30, col:34> 'int' '<'\n"
                             "    | | |-ImplicitCastExpr 0x2f93cb8 <col:30> 'int' <LValueToRValue>\n"
                             "    | | | `-DeclRefExpr 0x2f93c70 <col:30> 'int' lvalue Var 0x2f93bd8 'i' 'int'\n"
                             "    | | `-IntegerLiteral 0x2f93c98 <col:34> 'int' 10\n"
                             "    | |-UnaryOperator 0x2f93d20 <col:38, col:39> 'int' postfix '++'\n"
                             "    | | `-DeclRefExpr 0x2f93cf8 <col:38> 'int' lvalue Var 0x2f93bd8 'i' 'int'\n"
                             "    | `-CompoundStmt 0x2f93d40 <col:43, col:44>\n"
                             "    `-ReturnStmt 0x2f93da8 <col:46, col:53>\n"
                             "      `-IntegerLiteral 0x2f93d88 <col:53> 'int' 0";
        ASSERT_EQUALS("int main ( ) { for ( int i@1 = 0 ; i@1 < 10 ; ++ i@1 ) { } return 0 ; }", parse(clang));
    }

    void funcdecl1() {
        const char clang[] = "`-FunctionDecl 0x3122c30 <1.c:1:1, col:22> col:6 foo 'void (int, int)'\n"
                             "  |-ParmVarDecl 0x3122ae0 <col:10, col:14> col:14 x 'int'\n"
                             "  `-ParmVarDecl 0x3122b58 <col:17, col:21> col:21 y 'int'";
        ASSERT_EQUALS("void foo ( int x@1 , int y@2 ) ;", parse(clang));
    }

    void funcdecl2() {
        const char clang[] = "`-FunctionDecl 0x24b2c38 <1.c:1:1, line:4:1> line:1:5 foo 'int (int, int)'\n"
                             "  |-ParmVarDecl 0x24b2ae0 <col:9, col:13> col:13 used x 'int'\n"
                             "  |-ParmVarDecl 0x24b2b58 <col:16, col:20> col:20 used y 'int'\n"
                             "  `-CompoundStmt 0x24b2de8 <line:2:1, line:4:1>\n"
                             "    `-ReturnStmt 0x24b2dd0 <line:3:5, col:16>\n"
                             "      `-BinaryOperator 0x24b2da8 <col:12, col:16> 'int' '/'\n"
                             "        |-ImplicitCastExpr 0x24b2d78 <col:12> 'int' <LValueToRValue>\n"
                             "        | `-DeclRefExpr 0x24b2d28 <col:12> 'int' lvalue ParmVar 0x24b2ae0 'x' 'int'\n"
                             "        `-ImplicitCastExpr 0x24b2d90 <col:16> 'int' <LValueToRValue>\n"
                             "          `-DeclRefExpr 0x24b2d50 <col:16> 'int' lvalue ParmVar 0x24b2b58 'y' 'int'";
        ASSERT_EQUALS("int foo ( int x@1 , int y@2 ) { return x@1 / y@2 ; }", parse(clang));
    }

    void funcdecl3() {
        const char clang[] = "|-FunctionDecl 0x27cb6b8 <a.cpp:865:1, col:35> col:12 __overflow 'int (FILE *, int)' extern\n"
                             "| |-ParmVarDecl 0x27cb528 <col:24, col:29> col:30 'FILE *'\n"
                             "| `-ParmVarDecl 0x27cb5a0 <col:32> col:35 'int'";
        ASSERT_EQUALS("int __overflow ( FILE * , int ) ;", parse(clang));
    }

    void funcdecl4() {
        const char clang[] = "|-FunctionDecl 0x272bb60 <a.cpp:658:15> col:15 implicit fwrite 'unsigned long (const void *, unsigned long, unsigned long, FILE *)' extern\n"
                             "| |-ParmVarDecl 0x272cc40 <<invalid sloc>> <invalid sloc> 'const void *'\n"
                             "| |-ParmVarDecl 0x272cca0 <<invalid sloc>> <invalid sloc> 'unsigned long'\n"
                             "| |-ParmVarDecl 0x272cd00 <<invalid sloc>> <invalid sloc> 'unsigned long'\n"
                             "| `-ParmVarDecl 0x272cd60 <<invalid sloc>> <invalid sloc> 'FILE *'";
        ASSERT_EQUALS("unsigned long fwrite ( const void * , unsigned long , unsigned long , FILE * ) ;", parse(clang));
    }

    void funcdecl5() {
        const char clang[] = "`-FunctionDecl 0x59d670 <1.c:1:1, col:28> col:20 foo 'void (void)' static inline";
        ASSERT_EQUALS("static inline void foo ( ) ;", parse(clang));
    }

    void funcdecl6() {
        const char clang[] = "`-FunctionDecl 0x196eea8 <1.cpp:3:5, col:27> col:12 foo 'void **(int)'\n"
                             "  `-ParmVarDecl 0x196eda0 <col:17, col:21> col:21 count 'int'";
        ASSERT_EQUALS("void * * foo ( int count@1 ) ;", parse(clang));
    }

    void functionTemplateDecl1() {
        const char clang[] = "`-FunctionTemplateDecl 0x3242860 <a.cpp:1:1, col:46> col:21 foo";
        ASSERT_EQUALS("", parse(clang));
    }

    void functionTemplateDecl2() {
        const char clang[] = "|-FunctionTemplateDecl 0x333a860 <a.cpp:1:1, col:46> col:21 foo\n"
                             "| |-TemplateTypeParmDecl 0x333a5f8 <col:10, col:16> col:16 referenced class depth 0 index 0 T\n"
                             "| |-FunctionDecl 0x333a7c0 <col:19, col:46> col:21 foo 'T (T)'\n"
                             "| | |-ParmVarDecl 0x333a6c0 <col:25, col:27> col:27 referenced t 'T'\n"
                             "| | `-CompoundStmt 0x333a980 <col:30, col:46>\n"
                             "| |   `-ReturnStmt 0x333a968 <col:32, col:43>\n"
                             "| |     `-BinaryOperator 0x333a940 <col:39, col:43> '<dependent type>' '+'\n"
                             "| |       |-DeclRefExpr 0x333a8f8 <col:39> 'T' lvalue ParmVar 0x333a6c0 't' 'T'\n"
                             "| |       `-IntegerLiteral 0x333a920 <col:43> 'int' 1\n"
                             "| `-FunctionDecl 0x333ae00 <col:19, col:46> col:21 used foo 'int (int)'\n"
                             "|   |-TemplateArgument type 'int'\n"
                             "|   |-ParmVarDecl 0x333ad00 <col:25, col:27> col:27 used t 'int':'int'\n"
                             "|   `-CompoundStmt 0x333b0a8 <col:30, col:46>\n"
                             "|     `-ReturnStmt 0x333b090 <col:32, col:43>\n"
                             "|       `-BinaryOperator 0x333b068 <col:39, col:43> 'int' '+'\n"
                             "|         |-ImplicitCastExpr 0x333b050 <col:39> 'int':'int' <LValueToRValue>\n"
                             "|         | `-DeclRefExpr 0x333b028 <col:39> 'int':'int' lvalue ParmVar 0x333ad00 't' 'int':'int'\n"
                             "|         `-IntegerLiteral 0x333a920 <col:43> 'int' 1\n"
                             "`-FunctionDecl 0x333a9f8 <line:2:1, col:22> col:1 invalid bar 'int ()'\n"
                             "  `-CompoundStmt 0x333b010 <col:7, col:22>\n"
                             "    `-CallExpr 0x333afe0 <col:9, col:19> 'int':'int'\n"
                             "      |-ImplicitCastExpr 0x333afc8 <col:9, col:16> 'int (*)(int)' <FunctionToPointerDecay>\n"
                             "      | `-DeclRefExpr 0x333af00 <col:9, col:16> 'int (int)' lvalue Function 0x333ae00 'foo' 'int (int)' (FunctionTemplate 0x333a860 'foo')\n"
                             "      `-IntegerLiteral 0x333ab48 <col:18> 'int' 1";
        ASSERT_EQUALS("int foo<int> ( int t@1 ) { return t@1 + 1 ; } int bar ( ) { foo ( 1 ) ; }", parse(clang));
    }

    void ifelse() {
        const char clang[] = "`-FunctionDecl 0x2637ba8 <1.c:1:1, line:4:1> line:1:5 foo 'int (int)'\n"
                             "  |-ParmVarDecl 0x2637ae0 <col:9, col:13> col:13 used x 'int'\n"
                             "  `-CompoundStmt 0x2637d70 <col:16, line:4:1>\n"
                             "    `-IfStmt 0x2637d38 <line:2:5, line:3:11>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-BinaryOperator 0x2637cf0 <line:2:9, col:13> 'int' '>'\n"
                             "      | |-ImplicitCastExpr 0x2637cd8 <col:9> 'int' <LValueToRValue>\n"
                             "      | | `-DeclRefExpr 0x2637c90 <col:9> 'int' lvalue ParmVar 0x2637ae0 'x' 'int'\n"
                             "      | `-IntegerLiteral 0x2637cb8 <col:13> 'int' 10\n"
                             "      |-CompoundStmt 0x2637d18 <col:17, col:18>\n"
                             "      `-CompoundStmt 0x2637d28 <line:3:10, col:11>";
        ASSERT_EQUALS("int foo ( int x@1 ) { if ( x@1 > 10 ) { } else { } }", parse(clang));
    }

    void ifStmt() {
        // Clang 8 in cygwin
        const char clang[] = "`-FunctionDecl 0x41d0690 <2.cpp:1:1, col:24> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x41d07f0 <col:12, col:24>\n"
                             "    `-IfStmt 0x41d07b8 <col:14, col:22>\n"
                             "      |-ImplicitCastExpr 0x41d0790 <col:18> 'bool' <IntegralToBoolean>\n"
                             "      | `-IntegerLiteral 0x41d0770 <col:18> 'int' 1\n"
                             "      |-CompoundStmt 0x41d07a8 <col:21, col:22>\n";
        ASSERT_EQUALS("void foo ( ) { if ( 1 ) { } }", parse(clang));
    }

    void initListExpr() {
        const char clang[] = "|-VarDecl 0x397c680 <1.cpp:2:1, col:26> col:11 used ints 'const int [3]' cinit\n"
                             "| `-InitListExpr 0x397c7d8 <col:20, col:26> 'const int [3]'\n"
                             "|   |-IntegerLiteral 0x397c720 <col:21> 'int' 1\n"
                             "|   |-IntegerLiteral 0x397c740 <col:23> 'int' 2\n"
                             "|   `-IntegerLiteral 0x397c760 <col:25> 'int' 3";
        ASSERT_EQUALS("const int [3] ints@1 = { 1 , 2 , 3 } ;", parse(clang));
    }

    void labelStmt() {
        const char clang[] = "`-FunctionDecl 0x2ed1ba0 <1.c:1:1, col:36> col:6 foo 'void (int)'\n"
                             "  `-CompoundStmt 0x2ed1d00 <col:17, col:36>\n"
                             "    `-LabelStmt 0x2ed1ce8 <col:19, col:30> 'loop'\n"
                             "      `-GotoStmt 0x2ed1cd0 <col:25, col:30> 'loop' 0x2ed1c88";
        ASSERT_EQUALS("void foo ( ) { loop : goto loop ; }", parse(clang));
    }

    void memberExpr() {
        // C code:
        // struct S { int x };
        // int foo(struct S s) { return s.x; }
        const char clang[] = "|-RecordDecl 0x2441a88 <1.c:1:1, col:18> col:8 struct S definition\n"
                             "| `-FieldDecl 0x2441b48 <col:12, col:16> col:16 referenced x 'int'\n"
                             "`-FunctionDecl 0x2441cf8 <line:2:1, col:35> col:5 foo 'int (struct S)'\n"
                             "  |-ParmVarDecl 0x2441be8 <col:9, col:18> col:18 used s 'struct S':'struct S'\n"
                             "  `-CompoundStmt 0x2441e70 <col:21, col:35>\n"
                             "    `-ReturnStmt 0x2441e58 <col:23, col:32>\n"
                             "      `-ImplicitCastExpr 0x2441e40 <col:30, col:32> 'int' <LValueToRValue>\n"
                             "        `-MemberExpr 0x2441e08 <col:30, col:32> 'int' lvalue .x 0x2441b48\n"
                             "          `-DeclRefExpr 0x2441de0 <col:30> 'struct S':'struct S' lvalue ParmVar 0x2441be8 's' 'struct S':'struct S'";
        ASSERT_EQUALS("struct S { int x@1 ; } ; int foo ( struct S s@2 ) { return s@2 . x@1 ; }",
                      parse(clang));
    }

    void namespaceDecl1() {
        const char clang[] = "`-NamespaceDecl 0x2e5f658 <hello.cpp:1:1, col:24> col:11 x\n"
                             "  `-VarDecl 0x2e5f6d8 <col:15, col:19> col:19 var 'int'";
        ASSERT_EQUALS("namespace x { int var@1 ; }",
                      parse(clang));
    }

    void namespaceDecl2() {
        const char clang[] = "`-NamespaceDecl 0x1753e60 <1.cpp:1:1, line:4:1> line:1:11 std\n"
                             "  |-VisibilityAttr 0x1753ed0 <col:31, col:56> Default\n"
                             "  `-VarDecl 0x1753f40 <line:3:5, col:9> col:9 x 'int'";
        ASSERT_EQUALS("namespace std { int x@1 ; }",
                      parse(clang));
    }

    void recordDecl1() {
        const char clang[] = "`-RecordDecl 0x354eac8 <1.c:1:1, line:4:1> line:1:8 struct S definition\n"
                             "  |-FieldDecl 0x354eb88 <line:2:3, col:7> col:7 x 'int'\n"
                             "  `-FieldDecl 0x354ebe8 <line:3:3, col:7> col:7 y 'int'";
        ASSERT_EQUALS("struct S { int x@1 ; int y@2 ; } ;",
                      parse(clang));
    }

    void recordDecl2() {
        const char clang[] = "`-RecordDecl 0x3befac8 <2.c:2:1, col:22> col:1 struct definition\n"
                             "  `-FieldDecl 0x3befbf0 <col:10, col:19> col:14 val 'int'";
        ASSERT_EQUALS("struct { int val@1 ; } ;",
                      parse(clang));
    }

    void switchStmt() {
        const char clang[] = "`-FunctionDecl 0x2796ba0 <1.c:1:1, col:35> col:6 foo 'void (int)'\n"
                             "  |-ParmVarDecl 0x2796ae0 <col:10, col:14> col:14 used x 'int'\n"
                             "  `-CompoundStmt 0x2796d18 <col:17, col:35>\n"
                             "    |-SwitchStmt 0x2796cc8 <col:19, col:32>\n"
                             "    | |-<<<NULL>>>\n"
                             "    | |-<<<NULL>>>\n"
                             "    | |-ImplicitCastExpr 0x2796cb0 <col:27> 'int' <LValueToRValue>\n"
                             "    | | `-DeclRefExpr 0x2796c88 <col:27> 'int' lvalue ParmVar 0x2796ae0 'x' 'int'\n"
                             "    | `-CompoundStmt 0x2796cf8 <col:30, col:32>\n"
                             "    `-NullStmt 0x2796d08 <col:33>";
        ASSERT_EQUALS("void foo ( int x@1 ) { switch ( x@1 ) { } ; }",
                      parse(clang));
    }

    void typedefDeclPrologue()
    {
        // these TypedefDecl are included in *any* AST dump and we should ignore them as they should not be of interest to us
        // see https://github.com/llvm/llvm-project/issues/120228#issuecomment-2549212109 for an explanation
        const char clang[] =
            "TranslationUnitDecl 0x60efd80ef9f8 <<invalid sloc>> <invalid sloc>\n"
            "|-TypedefDecl 0x60efd80f0228 <<invalid sloc>> <invalid sloc> implicit __int128_t '__int128'\n"
            "| `-BuiltinType 0x60efd80effc0 '__int128'\n"
            "|-TypedefDecl 0x60efd80f0298 <<invalid sloc>> <invalid sloc> implicit __uint128_t 'unsigned __int128'\n"
            "| `-BuiltinType 0x60efd80effe0 'unsigned __int128'\n"
            "|-TypedefDecl 0x60efd80f05a0 <<invalid sloc>> <invalid sloc> implicit __NSConstantString 'struct __NSConstantString_tag'\n"
            "| `-RecordType 0x60efd80f0370 'struct __NSConstantString_tag'\n"
            "|   `-Record 0x60efd80f02f0 '__NSConstantString_tag'\n"
            "|-TypedefDecl 0x60efd80f0648 <<invalid sloc>> <invalid sloc> implicit __builtin_ms_va_list 'char *'\n"
            "| `-PointerType 0x60efd80f0600 'char *'\n"
            "|   `-BuiltinType 0x60efd80efaa0 'char'\n"
            "|-TypedefDecl 0x60efd80f0940 <<invalid sloc>> <invalid sloc> implicit __builtin_va_list 'struct __va_list_tag[1]'\n"
            "| `-ConstantArrayType 0x60efd80f08e0 'struct __va_list_tag[1]' 1\n"
            "|   `-RecordType 0x60efd80f0720 'struct __va_list_tag'\n"
            "|     `-Record 0x60efd80f06a0 '__va_list_tag'\n"
            "`-FunctionDecl 0x60efd8151470 <a.cpp:1:1, line:3:1> line:1:6 f 'void ()'\n"
            "`-CompoundStmt 0x60efd8151560 <line:2:1, line:3:1>\n";

        ASSERT_EQUALS("void f ( ) ;", parse(clang));
    }

    void unaryExprOrTypeTraitExpr1() {
        const char clang[] = "`-VarDecl 0x24cc610 <a.cpp:1:1, col:19> col:5 x 'int' cinit\n"
                             "  `-ImplicitCastExpr 0x24cc6e8 <col:9, col:19> 'int' <IntegralCast>\n"
                             "    `-UnaryExprOrTypeTraitExpr 0x24cc6c8 <col:9, col:19> 'unsigned long' sizeof 'int'\n";
        ASSERT_EQUALS("int x@1 = sizeof ( int ) ;", parse(clang));
    }

    void unaryExprOrTypeTraitExpr2() {
        const char clang[] = "`-VarDecl 0x27c6c00 <a.cpp:3:5, col:23> col:9 x 'int' cinit\n"
                             "  `-ImplicitCastExpr 0x27c6cc8 <col:13, col:23> 'int' <IntegralCast>\n"
                             "    `-UnaryExprOrTypeTraitExpr 0x27c6ca8 <col:13, col:23> 'unsigned long' sizeof\n"
                             "      `-ParenExpr 0x27c6c88 <col:19, col:23> 'char [10]' lvalue\n"
                             "        `-DeclRefExpr 0x27c6c60 <col:20> 'char [10]' lvalue Var 0x27c6b48 'buf' 'char [10]'";
        ASSERT_EQUALS("int x@1 = sizeof ( buf ) ;", parse(clang));
    }

    void unaryOperator() {
        const char clang[] = "`-FunctionDecl 0x2dd9748 <1.cpp:2:1, col:44> col:5 foo 'int (int *)'\n"
                             "  |-ParmVarDecl 0x2dd9680 <col:14, col:19> col:19 used p 'int *'\n"
                             "  `-CompoundStmt 0x2dd9908 <col:22, col:44>\n"
                             "    `-ReturnStmt 0x2dd98f0 <col:24, col:41>\n"
                             "      `-BinaryOperator 0x2dd98c8 <col:31, col:41> 'int' '/'\n"
                             "        |-IntegerLiteral 0x2dd9830 <col:31> 'int' 100000\n"
                             "        `-ImplicitCastExpr 0x2dd98b0 <col:40, col:41> 'int' <LValueToRValue>\n"
                             "          `-UnaryOperator 0x2dd9890 <col:40, col:41> 'int' lvalue prefix '*' cannot overflow\n"
                             "            `-ImplicitCastExpr 0x2dd9878 <col:41> 'int *' <LValueToRValue>\n"
                             "              `-DeclRefExpr 0x2dd9850 <col:41> 'int *' lvalue ParmVar 0x2dd9680 'p' 'int *'";
        ASSERT_EQUALS("int foo ( int * p@1 ) { return 100000 / * p@1 ; }", parse(clang));
    }

    void vardecl1() {
        const char clang[] = "|-VarDecl 0x32b8aa0 <1.c:1:1, col:9> col:5 used a 'int' cinit\n"
                             "| `-IntegerLiteral 0x32b8b40 <col:9> 'int' 1\n"
                             "`-VarDecl 0x32b8b78 <line:2:1, col:9> col:5 b 'int' cinit\n"
                             "  `-ImplicitCastExpr 0x32b8c00 <col:9> 'int' <LValueToRValue>\n"
                             "    `-DeclRefExpr 0x32b8bd8 <col:9> 'int' lvalue Var 0x32b8aa0 'a' 'int'";

        ASSERT_EQUALS("int a@1 = 1 ; int b@2 = a@1 ;",
                      parse(clang));
    }

    void vardecl2() {
        const char clang[] = "|-VarDecl 0x3873b50 <1.c:1:1, col:9> col:5 used a 'int [10]'\n"
                             "`-FunctionDecl 0x3873c38 <line:3:1, line:6:1> line:3:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x3873dd0 <line:4:1, line:6:1>\n"
                             "    `-BinaryOperator 0x3873da8 <line:5:3, col:10> 'int' '='\n"
                             "      |-ArraySubscriptExpr 0x3873d60 <col:3, col:6> 'int' lvalue\n"
                             "      | |-ImplicitCastExpr 0x3873d48 <col:3> 'int *' <ArrayToPointerDecay>\n"
                             "      | | `-DeclRefExpr 0x3873cd8 <col:3> 'int [10]' lvalue Var 0x3873b50 'a' 'int [10]'\n"
                             "      | `-IntegerLiteral 0x3873d00 <col:5> 'int' 0\n"
                             "      `-IntegerLiteral 0x3873d88 <col:10> 'int' 0\n";

        ASSERT_EQUALS("int [10] a@1 ; void foo ( ) { a@1 [ 0 ] = 0 ; }",
                      parse(clang));
    }

    void vardecl3() {
        const char clang[] = "`-VarDecl 0x25a8aa0 <1.c:1:1, col:12> col:12 p 'const int *'";
        ASSERT_EQUALS("const int * p@1 ;", parse(clang));
    }

    void vardecl4() {
        const char clang[] = "|-VarDecl 0x23d6c78 <a.cpp:137:1, col:14> col:14 stdin 'FILE *' extern";
        ASSERT_EQUALS("FILE * stdin@1 ;", parse(clang));
    }

    void vardecl5() {
        const char clang[] = "|-VarDecl 0x2e31fc0 <a.cpp:27:1, col:38> col:26 sys_errlist 'const char *const []' extern";
        ASSERT_EQUALS("const char * const [] sys_errlist@1 ;", parse(clang));
    }

    void vardecl6() {
        const char clang[] = "`-VarDecl 0x278e170 <1.c:1:1, col:16> col:12 x 'int' static cinit\n"
                             "  `-IntegerLiteral 0x278e220 <col:16> 'int' 3";
        ASSERT_EQUALS("static int x@1 = 3 ;", parse(clang));
    }

    void vardecl7() {
        const char clang[] = "`-VarDecl 0x2071f20 <1.cpp:2:1, col:23> col:9 start 'void *(*)(void *)'";
        ASSERT_EQUALS("void * * start@1 ;", parse(clang));
    }

    void whileStmt1() {
        const char clang[] = "`-FunctionDecl 0x3d45b18 <1.c:1:1, line:3:1> line:1:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x3d45c48 <col:12, line:3:1>\n"
                             "    `-WhileStmt 0x3d45c28 <line:2:5, col:14>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-IntegerLiteral 0x3d45bf8 <col:12> 'int' 0\n"
                             "      `-NullStmt 0x3d45c18 <col:14>";
        ASSERT_EQUALS("void foo ( ) { while ( 0 ) { ; } }",
                      parse(clang));
    }

    void whileStmt2() {
        // clang 9
        const char clang[] = "`-FunctionDecl 0x1c99ac8 <1.cpp:1:1, col:27> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x1c99c10 <col:12, col:27>\n"
                             "    `-WhileStmt 0x1c99bf8 <col:14, col:25>\n"
                             "      |-ImplicitCastExpr 0x1c99bd0 <col:21> 'bool' <IntegralToBoolean>\n"
                             "      | `-IntegerLiteral 0x1c99bb0 <col:21> 'int' 1\n"
                             "      `-CompoundStmt 0x1c99be8 <col:24, col:25>";
        ASSERT_EQUALS("void foo ( ) { while ( 1 ) { } }", parse(clang));
    }


#define GET_SYMBOL_DB(AST) \
    const Settings settings = settingsBuilder().clang().platform(Platform::Type::Unix64).build(); \
    TokenList tokenlist{settings, Standards::Language::CPP}; \
    Tokenizer tokenizer(std::move(tokenlist), *this); \
    { \
        std::istringstream istr(AST); \
        clangimport::parseClangAstDump(tokenizer, istr); \
    } \
    const SymbolDatabase *db = tokenizer.getSymbolDatabase(); \
    ASSERT(db)

    void tokenIndex() {
        const char clang[] = "`-FunctionDecl 0x1e07dd0 <67.cpp:1:1, col:13> col:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x1e07eb8 <col:12, col:13>";
        ASSERT_EQUALS("void foo ( ) { }", parse(clang));

        GET_SYMBOL_DB(clang);
        const Token *tok = tokenizer.tokens();
        ASSERT_EQUALS(tok->index() + 1, tok->next()->index());
    }

    void symbolDatabaseEnum1() {
        const char clang[] = "|-NamespaceDecl 0x29ad5f8 <1.cpp:1:1, line:3:1> line:1:11 ns\n"
                             "| `-EnumDecl 0x29ad660 <line:2:1, col:16> col:6 referenced abc\n"
                             "|   |-EnumConstantDecl 0x29ad720 <col:11> col:11 a 'ns::abc'\n"
                             "|   |-EnumConstantDecl 0x29ad768 <col:13> col:13 b 'ns::abc'\n"
                             "|   `-EnumConstantDecl 0x29ad7b0 <col:15> col:15 referenced c 'ns::abc'\n"
                             "`-VarDecl 0x29ad898 <line:5:1, col:22> col:9 x 'ns::abc':'ns::abc' cinit\n"
                             "  `-DeclRefExpr 0x29ad998 <col:13, col:22> 'ns::abc' EnumConstant 0x29ad7b0 'c' 'ns::abc'\n";

        ASSERT_EQUALS("namespace ns { enum abc { a , b , c } } ns :: abc x@1 = c ;", parse(clang));

        GET_SYMBOL_DB(clang);

        // Enum scope and type
        ASSERT_EQUALS(3, db->scopeList.size());
        const Scope &enumScope = db->scopeList.back();
        ASSERT_EQUALS_ENUM(ScopeType::eEnum, enumScope.type);
        ASSERT_EQUALS("abc", enumScope.className);
        const Type *enumType = enumScope.definedType;
        ASSERT_EQUALS("abc", enumType->name());

        // Variable
        const Token *vartok = Token::findsimplematch(tokenizer.tokens(), "x");
        ASSERT(vartok);
        ASSERT(vartok->variable());
        ASSERT(vartok->variable()->valueType());
        ASSERT_EQUALS(uintptr_t(&enumScope), uintptr_t(vartok->variable()->valueType()->typeScope));
    }

    void symbolDatabaseFunction1() {
        const char clang[] = "|-FunctionDecl 0x3aea7a0 <1.cpp:2:1, col:22> col:6 used foo 'void (int, int)'\n"
                             "  |-ParmVarDecl 0x3aea650 <col:10, col:14> col:14 x 'int'\n"
                             "  |-ParmVarDecl 0x3aea6c8 <col:17, col:21> col:21 y 'int'\n"
                             "  `-CompoundStmt 0x3d45c48 <col:12>\n";

        GET_SYMBOL_DB(clang);

        // There is a function foo that has 2 arguments
        ASSERT_EQUALS(1, db->functionScopes.size());
        const Scope *scope = db->functionScopes[0];
        const Function *func = scope->function;
        ASSERT_EQUALS(2, func->argCount());
        ASSERT_EQUALS("x", func->getArgumentVar(0)->name());
        ASSERT_EQUALS("y", func->getArgumentVar(1)->name());
        ASSERT_EQUALS(ValueType::Type::INT, func->getArgumentVar(0)->valueType()->type);
        ASSERT_EQUALS(ValueType::Type::INT, func->getArgumentVar(1)->valueType()->type);
    }

    void symbolDatabaseFunction2() {
        const char clang[] = "|-FunctionDecl 0x3aea7a0 <1.cpp:2:1, col:22> col:6 used foo 'void (int, int)'\n"
                             "| |-ParmVarDecl 0x3aea650 <col:10, col:14> col:14 'int'\n"
                             "| |-ParmVarDecl 0x3aea6c8 <col:17, col:21> col:21 'int'\n"
                             "  `-CompoundStmt 0x3d45c48 <col:12>\n";

        GET_SYMBOL_DB(clang);

        // There is a function foo that has 2 arguments
        ASSERT_EQUALS(1, db->functionScopes.size());
        const Scope *scope = db->functionScopes[0];
        const Function *func = scope->function;
        ASSERT_EQUALS(2, func->argCount());
        ASSERT(!func->getArgumentVar(0)->nameToken());
        ASSERT(!func->getArgumentVar(1)->nameToken());
    }

    void symbolDatabaseFunction3() { // #9640
        const char clang[] = "`-FunctionDecl 0x238fcd8 <9640.cpp:1:1, col:26> col:6 used bar 'bool (const char, int &)'\n"
                             "  |-ParmVarDecl 0x238fb10 <col:10, col:16> col:20 'const char'\n"
                             "  |-ParmVarDecl 0x238fbc0 <col:22, col:25> col:26 'int &'\n"
                             "  `-CompoundStmt 0x3d45c48 <col:12>\n";

        GET_SYMBOL_DB(clang);

        // There is a function foo that has 2 arguments
        ASSERT_EQUALS(1, db->functionScopes.size());
        const Scope *scope = db->functionScopes[0];
        const Function *func = scope->function;
        ASSERT_EQUALS(2, func->argCount());
        ASSERT_EQUALS(false, func->getArgumentVar(0)->isReference());
        ASSERT_EQUALS(true, func->getArgumentVar(1)->isReference());
    }

    void symbolDatabaseFunctionConst() {
        const char clang[] = "`-CXXRecordDecl 0x7e2d98 <1.cpp:2:1, line:5:1> line:2:7 class foo definition\n"
                             "  `-CXXMethodDecl 0x7e3000 <line:4:3, col:12> col:8 f 'void () const'";

        GET_SYMBOL_DB(clang);

        // There is a function f that is const
        ASSERT_EQUALS(2, db->scopeList.size());
        ASSERT_EQUALS(1, db->scopeList.back().functionList.size());
        const Function &func = db->scopeList.back().functionList.back();
        ASSERT(func.isConst());
    }

    void symbolDatabaseVariableRef() {
        const char clang[] = "`-FunctionDecl 0x1593df0 <3.cpp:1:1, line:4:1> line:1:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x15940b0 <col:12, line:4:1>\n"
                             "    |-DeclStmt 0x1593f58 <line:2:3, col:8>\n"
                             "    | `-VarDecl 0x1593ef0 <col:3, col:7> col:7 used x 'int'\n"
                             "    `-DeclStmt 0x1594098 <line:3:3, col:15>\n"
                             "      `-VarDecl 0x1593fb8 <col:3, col:14> col:8 ref 'int &' cinit\n"
                             "        `-DeclRefExpr 0x1594020 <col:14> 'int' lvalue Var 0x1593ef0 'x' 'int'";
        GET_SYMBOL_DB(clang);
        const Variable *refVar = db->variableList().back();
        ASSERT(refVar->isReference());
    }

    void symbolDatabaseVariableRRef() {
        const char clang[] = "`-FunctionDecl 0x1a40df0 <3.cpp:1:1, line:4:1> line:1:6 foo 'void ()'\n"
                             "  `-CompoundStmt 0x1a41180 <col:12, line:4:1>\n"
                             "    |-DeclStmt 0x1a40f58 <line:2:3, col:8>\n"
                             "    | `-VarDecl 0x1a40ef0 <col:3, col:7> col:7 used x 'int'\n"
                             "    `-DeclStmt 0x1a41168 <line:3:3, col:18>\n"
                             "      `-VarDecl 0x1a40fb8 <col:3, col:17> col:9 ref 'int &&' cinit\n"
                             "        `-ExprWithCleanups 0x1a410f8 <col:15, col:17> 'int' xvalue\n"
                             "          `-MaterializeTemporaryExpr 0x1a41098 <col:15, col:17> 'int' xvalue extended by Var 0x1a40fb8 'ref' 'int &&'\n"
                             "            `-BinaryOperator 0x1a41078 <col:15, col:17> 'int' '+'\n"
                             "              |-ImplicitCastExpr 0x1a41060 <col:15> 'int' <LValueToRValue>\n"
                             "              | `-DeclRefExpr 0x1a41020 <col:15> 'int' lvalue Var 0x1a40ef0 'x' 'int'\n"
                             "              `-IntegerLiteral 0x1a41040 <col:17> 'int' 1\n";

        ASSERT_EQUALS("void foo ( ) { int x@1 ; int && ref@2 = x@1 + 1 ; }", parse(clang));

        GET_SYMBOL_DB(clang);
        const Variable *refVar = db->variableList().back();
        ASSERT(refVar->isReference());
        ASSERT(refVar->isRValueReference());
    }

    void symbolDatabaseVariablePointerRef() {
        const char clang[] = "`-FunctionDecl 0x9b4f10 <3.cpp:1:1, col:17> col:6 used foo 'void (int *&)'\n"
                             "  `-ParmVarDecl 0x9b4e40 <col:10, col:16> col:16 p 'int *&'\n";

        ASSERT_EQUALS("void foo ( int * & p@1 ) ;", parse(clang));

        GET_SYMBOL_DB(clang);
        const Variable *p = db->variableList().back();
        ASSERT(p->isPointer());
        ASSERT(p->isReference());
    }

    void symbolDatabaseNodeType1() {
        const char clang[] = "`-FunctionDecl 0x32438c0 <a.cpp:5:1, line:7:1> line:5:6 foo 'a::b (a::b)'\n"
                             "  |-ParmVarDecl 0x32437b0 <col:10, col:15> col:15 used i 'a::b':'long'\n"
                             "  `-CompoundStmt 0x3243a60 <col:18, line:7:1>\n"
                             "    `-ReturnStmt 0x3243a48 <line:6:3, col:12>\n"
                             "      `-BinaryOperator 0x3243a20 <col:10, col:12> 'long' '+'\n"
                             "        |-ImplicitCastExpr 0x32439f0 <col:10> 'a::b':'long' <LValueToRValue>\n"
                             "        | `-DeclRefExpr 0x32439a8 <col:10> 'a::b':'long' lvalue ParmVar 0x32437b0 'i' 'a::b':'long'\n"
                             "        `-ImplicitCastExpr 0x3243a08 <col:12> 'long' <IntegralCast>\n"
                             "          `-IntegerLiteral 0x32439d0 <col:12> 'int' 1\n";

        GET_SYMBOL_DB(clang);

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "i + 1");
        ASSERT(!!tok);
        ASSERT(!!tok->valueType());
        ASSERT_EQUALS("signed long", tok->valueType()->str());
    }

    void symbolDatabaseForVariable() {
        const char clang[] = "`-FunctionDecl 0x38f8bb0 <10100.c:2:1, line:4:1> line:2:6 f 'void (void)'\n"
                             "  `-CompoundStmt 0x38f8d88 <col:14, line:4:1>\n"
                             "    `-ForStmt 0x38f8d50 <line:3:5, col:26>\n"
                             "      |-DeclStmt 0x38f8d28 <col:10, col:19>\n"
                             "      | `-VarDecl 0x38f8ca8 <col:10, col:18> col:14 i 'int' cinit\n"
                             "      |   `-IntegerLiteral 0x38f8d08 <col:18> 'int' 0\n"
                             "      |-<<<NULL>>>\n"
                             "      |-<<<NULL>>>\n"
                             "      |-<<<NULL>>>\n"
                             "      `-CompoundStmt 0x38f8d40 <col:25, col:26>";

        ASSERT_EQUALS("void f ( ) { for ( int i@1 = 0 ; ; ) { } }", parse(clang));

        GET_SYMBOL_DB(clang);

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "i");
        ASSERT(!!tok);
        ASSERT(!!tok->variable());
        ASSERT_EQUALS_ENUM(ScopeType::eFor, tok->variable()->scope()->type);
    }

    void valueFlow1() {
        // struct S { int x; int buf[10]; } ; int sz = sizeof(struct S);
        const char clang[] = "|-RecordDecl 0x2fc5a88 <1.c:1:1, line:4:1> line:1:8 struct S definition\n"
                             "| |-FieldDecl 0x2fc5b48 <line:2:3, col:7> col:7 x 'int'\n"
                             "| `-FieldDecl 0x2fc5c10 <line:3:3, col:13> col:7 buf 'int [10]'\n"
                             "`-VarDecl 0x2fc5c70 <line:6:1, col:25> col:5 sz 'int' cinit\n"
                             "  `-ImplicitCastExpr 0x2fc5d88 <col:10, col:25> 'int' <IntegralCast>\n"
                             "    `-UnaryExprOrTypeTraitExpr 0x2fc5d68 <col:10, col:25> 'unsigned long' sizeof 'struct S':'struct S'";
        GET_SYMBOL_DB(clang);

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "sizeof (");
        ASSERT(!!tok);
        tok = tok->next();
        ASSERT(tok->hasKnownIntValue());
        ASSERT_EQUALS(MathLib::bigint(44), tok->getKnownIntValue());
    }

    void valueFlow2() {
        // int buf[42];
        // int x = sizeof(buf);
        const char clang[] = "|-VarDecl 0x10f6de8 <66.cpp:3:1, col:11> col:5 referenced buf 'int [42]'\n"
                             "`-VarDecl 0x10f6eb0 <line:4:1, col:19> col:5 x 'int' cinit\n"
                             "  `-ImplicitCastExpr 0x10f6f78 <col:9, col:19> 'int' <IntegralCast>\n"
                             "    `-UnaryExprOrTypeTraitExpr 0x10f6f58 <col:9, col:19> 'unsigned long' sizeof\n"
                             "      `-ParenExpr 0x10f6f38 <col:15, col:19> 'int [42]' lvalue\n"
                             "        `-DeclRefExpr 0x10f6f18 <col:16> 'int [42]' lvalue Var 0x10f6de8 'buf' 'int [42]' non_odr_use_unevaluated";

        GET_SYMBOL_DB(clang);

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "sizeof (");
        ASSERT(!!tok);
        tok = tok->next();
        TODO_ASSERT_EQUALS(true, false, tok->hasKnownIntValue() && tok->getKnownIntValue() == 10);
    }

    void valueType1() {
        const char clang[] = "`-FunctionDecl 0x32438c0 <a.cpp:5:1, line:7:1> line:5:6 foo 'a::b (a::b)'\n"
                             "  |-ParmVarDecl 0x32437b0 <col:10, col:15> col:15 used i 'a::b':'long'\n"
                             "  `-CompoundStmt 0x3243a60 <col:18, line:7:1>\n"
                             "    `-ReturnStmt 0x3243a48 <line:6:3, col:12>\n"
                             "      `-ImplicitCastExpr 0x2176ca8 <col:9> 'int' <IntegralCast>\n"
                             "        `-ImplicitCastExpr 0x2176c90 <col:9> 'bool' <LValueToRValue>\n"
                             "          `-DeclRefExpr 0x2176c60 <col:9> 'bool' lvalue Var 0x2176bd0 'e' 'bool'\n";

        GET_SYMBOL_DB(clang);

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "e");
        ASSERT(!!tok);
        ASSERT(!!tok->valueType());
        ASSERT_EQUALS("bool", tok->valueType()->str());
    }

    void valueType2() {
        const char clang[] = "`-VarDecl 0xc9eda0 <1.cpp:2:1, col:17> col:13 s 'const char *' cinit\n"
                             "  `-ImplicitCastExpr 0xc9eef0 <col:17> 'const char *' <ArrayToPointerDecay>\n"
                             "    `-StringLiteral 0xc9eed0 <col:17> 'const char [6]' lvalue \"hello\"\n";

        GET_SYMBOL_DB(clang);

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "\"hello\"");
        ASSERT(!!tok);
        ASSERT(!!tok->valueType());
        ASSERT_EQUALS("const char *", tok->valueType()->str());
    }

    void stdinLoc() {
        // we should never encounter this
        /*
           int i;
         */
        const char clang[] = "`-VarDecl 0x5776bb240470 <<stdin>:1:1, col:5> col:5 i 'int'\n";

        ASSERT_THROW_INTERNAL_EQUALS(parse(clang), AST, "invalid AST location: <<stdin>");
    }

    void crash() {
        const char* clang = "TranslationUnitDecl 0x56037914f998 <<invalid sloc>> <invalid sloc>\n"
                            "|-ClassTemplateDecl 0x560379196b58 <test1.cpp:1:1, col:50> col:37 A\n"
                            "| |-TemplateTypeParmDecl 0x560379196950 <col:11> col:19 typename depth 0 index 0\n"
                            "| |-TemplateTypeParmDecl 0x5603791969f8 <col:21> col:29 typename depth 0 index 1\n"
                            "| `-CXXRecordDecl 0x560379196ac8 <col:31, col:50> col:37 class A definition\n"
                            "|   |-DefinitionData empty aggregate standard_layout trivially_copyable pod trivial literal has_constexpr_non_copy_move_ctor can_const_default_init\n"
                            "|   | |-DefaultConstructor exists trivial constexpr needs_implicit defaulted_is_constexpr\n"
                            "|   | |-CopyConstructor simple trivial has_const_param needs_implicit implicit_has_const_param\n"
                            "|   | |-MoveConstructor exists simple trivial needs_implicit\n"
                            "|   | |-CopyAssignment simple trivial has_const_param needs_implicit implicit_has_const_param\n"
                            "|   | |-MoveAssignment exists simple trivial needs_implicit\n"
                            "|   | `-Destructor simple irrelevant trivial needs_implicit\n"
                            "|   |-CXXRecordDecl 0x560379196de0 <col:31, col:37> col:37 implicit referenced class A\n"
                            "|   `-CXXRecordDecl 0x560379196e70 <col:41, col:47> col:47 class b\n"
                            "|-CXXRecordDecl 0x560379197110 parent 0x560379196ac8 prev 0x560379196e70 <line:2:1, col:63> col:50 class b definition\n"
                            "| |-DefinitionData empty standard_layout trivially_copyable has_user_declared_ctor can_const_default_init\n"
                            "| | |-DefaultConstructor defaulted_is_constexpr\n"
                            "| | |-CopyConstructor simple trivial has_const_param needs_implicit implicit_has_const_param\n"
                            "| | |-MoveConstructor exists simple trivial needs_implicit\n"
                            "| | |-CopyAssignment simple trivial has_const_param needs_implicit implicit_has_const_param\n"
                            "| | |-MoveAssignment exists simple trivial needs_implicit\n"
                            "| | `-Destructor simple irrelevant trivial needs_implicit\n"
                            "| |-CXXRecordDecl 0x560379197250 <col:35, col:50> col:50 implicit referenced class b\n"
                            "| `-CXXConstructorDecl 0x5603791974b8 <col:54, col:60> col:54 b 'void (A<type-parameter-0-0, type-parameter-0-1> &)'\n"
                            "|   `-ParmVarDecl 0x560379197380 <col:56, col:59> col:59 a 'A<type-parameter-0-0, type-parameter-0-1> &'\n"
                            "`-CXXConstructorDecl 0x5603791b5600 parent 0x560379197110 prev 0x5603791974b8 <line:3:1, col:55> col:47 b 'void (A<type-parameter-0-0, type-parameter-0-1> &)'\n"
                            "  |-ParmVarDecl 0x5603791b5570 <col:49, col:51> col:52 'A<type-parameter-0-0, type-parameter-0-1> &'\n"
                            "  `-CompoundStmt 0x5603791b5700 <col:54, col:55>\n";
        (void)parse(clang); // don't crash
    }
};

REGISTER_TEST(TestClangImport)
