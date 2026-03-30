/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2026 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vfvalue.h"

#include "mathlib.h"
#include "token.h"
#include "utils.h"

#include <sstream>
#include <string>

namespace ValueFlow {
    Value::Value(const Token *c, MathLib::bigint val, Bound b)
        : bound(b),
        safe(false),
        conditional(false),
        macro(false),
        defaultArg(false),
        intvalue(val),
        varvalue(val),
        condition(c) {
        errorPath.emplace_back(c, "Assuming that condition '" + c->expressionString() + "' is not redundant");
    }

    void Value::assumeCondition(const Token *tok) {
        condition = tok;
        errorPath.emplace_back(tok, "Assuming that condition '" + tok->expressionString() + "' is not redundant");
    }

    std::string Value::toString() const {
        std::stringstream ss;
        if (this->isImpossible())
            ss << "!";
        if (this->bound == Bound::Lower)
            ss << ">=";
        if (this->bound == Bound::Upper)
            ss << "<=";
        switch (this->valueType) {
        case ValueType::INT:
            ss << this->intvalue;
            break;
        case ValueType::TOK:
            ss << this->tokvalue->str();
            break;
        case ValueType::FLOAT:
            ss << MathLib::toString(this->floatValue);
            break;
        case ValueType::MOVED:
            ss << toString(this->moveKind);
            break;
        case ValueType::UNINIT:
            ss << "Uninit";
            break;
        case ValueType::BUFFER_SIZE:
        case ValueType::CONTAINER_SIZE:
            ss << "size=" << this->intvalue;
            break;
        case ValueType::ITERATOR_START:
            ss << "start=" << this->intvalue;
            break;
        case ValueType::ITERATOR_END:
            ss << "end=" << this->intvalue;
            break;
        case ValueType::LIFETIME:
            ss << "lifetime[" << toString(this->lifetimeKind) << "]=("
               << this->tokvalue->expressionString() << ")";
            break;
        case ValueType::SYMBOLIC:
            ss << "symbolic=(" << this->tokvalue->expressionString();
            if (this->intvalue > 0)
                ss << "+" << this->intvalue;
            else if (this->intvalue < 0)
                ss << "-" << -this->intvalue;
            ss << ")";
            break;
        }
        if (this->indirect > 0)
            for (int i = 0; i < this->indirect; i++)
                ss << "*";
        if (this->path > 0)
            ss << "@" << this->path;
        return ss.str();
    }

    std::string Value::infoString() const {
        switch (valueType) {
        case ValueType::INT:
            return MathLib::toString(intvalue);
        case ValueType::TOK:
            return tokvalue->str();
        case ValueType::FLOAT:
            return MathLib::toString(floatValue);
        case ValueType::MOVED:
            return "<Moved>";
        case ValueType::UNINIT:
            return "<Uninit>";
        case ValueType::BUFFER_SIZE:
        case ValueType::CONTAINER_SIZE:
            return "size=" + MathLib::toString(intvalue);
        case ValueType::ITERATOR_START:
            return "start=" + MathLib::toString(intvalue);
        case ValueType::ITERATOR_END:
            return "end=" + MathLib::toString(intvalue);
        case ValueType::LIFETIME:
            return "lifetime=" + tokvalue->str();
        case ValueType::SYMBOLIC:
        {
            std::string result = "symbolic=" + tokvalue->expressionString();
            if (intvalue > 0)
                result += "+" + MathLib::toString(intvalue);
            else if (intvalue < 0)
                result += "-" + MathLib::toString(-intvalue);
            return result;
        }
        }
        cppcheck::unreachable();
    }

    const char *Value::toString(MoveKind moveKind) {
        switch (moveKind) {
        case MoveKind::NonMovedVariable:
            return "NonMovedVariable";
        case MoveKind::MovedVariable:
            return "MovedVariable";
        case MoveKind::ForwardedVariable:
            return "ForwardedVariable";
        }
        return "";
    }

    const char *Value::toString(LifetimeKind lifetimeKind) {
        switch (lifetimeKind) {
        case LifetimeKind::Object:
            return "Object";
        case LifetimeKind::SubObject:
            return "SubObject";
        case LifetimeKind::Lambda:
            return "Lambda";
        case LifetimeKind::Iterator:
            return "Iterator";
        case LifetimeKind::Address:
            return "Address";
        }
        return "";
    }

    bool Value::sameToken(const Token *tok1, const Token *tok2) {
        if (tok1 == tok2)
            return true;
        if (!tok1)
            return false;
        if (tok1->exprId() == 0 || tok2->exprId() == 0)
            return false;
        return tok1->exprId() == tok2->exprId();
    }

    const char *Value::toString(LifetimeScope lifetimeScope) {
        switch (lifetimeScope) {
        case LifetimeScope::Local:
            return "Local";
        case LifetimeScope::Argument:
            return "Argument";
        case LifetimeScope::SubFunction:
            return "SubFunction";
        case LifetimeScope::ThisPointer:
            return "ThisPointer";
        case LifetimeScope::ThisValue:
            return "ThisValue";
        }
        return "";
    }

    const char *Value::toString(Bound bound) {
        switch (bound) {
        case Bound::Point:
            return "Point";
        case Bound::Upper:
            return "Upper";
        case Bound::Lower:
            return "Lower";
        }
        return "";
    }
}
