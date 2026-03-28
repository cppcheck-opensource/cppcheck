/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#include "errortypes.h"

#include "utils.h"

static std::string typeToString(InternalError::Type type)
{
    switch (type) {
    case InternalError::Type::AST:
        return "internalAstError";
    case InternalError::Type::SYNTAX:
        return "syntaxError";
    case InternalError::Type::UNKNOWN_MACRO:
        return "unknownMacro";
    case InternalError::Type::INTERNAL:
        return "internalError";
    case InternalError::Type::LIMIT:
        return "cppcheckLimit";
    case InternalError::Type::INSTANTIATION:
        return "instantiationError";
    }
    cppcheck::unreachable();
}

InternalError::InternalError(const Token *tok, std::string errorMsg, Type type) :
    InternalError(tok, std::move(errorMsg), "", type)
{}

InternalError::InternalError(const Token *tok, std::string errorMsg, std::string details, Type type) :
    token(tok), errorMessage(std::move(errorMsg)), details(std::move(details)), type(type), id(typeToString(type))
{}

std::string severityToString(Severity severity)
{
    switch (severity) {
    case Severity::none:
        return "";
    case Severity::error:
        return "error";
    case Severity::warning:
        return "warning";
    case Severity::style:
        return "style";
    case Severity::performance:
        return "performance";
    case Severity::portability:
        return "portability";
    case Severity::information:
        return "information";
    case Severity::debug:
        return "debug";
    case Severity::internal:
        return "internal";
    }
    cppcheck::unreachable();
}

std::string coloredSeverityToString(Severity severity)
{
    switch (severity) {
    case Severity::none:
        return "";
    case Severity::error:
        return "{red}error";
    case Severity::warning:
        return "{magenta}warning";
    case Severity::style:
        return "{bold}style";
    case Severity::performance:
        return "{bold}performance";
    case Severity::portability:
        return "{bold}portability";
    case Severity::information:
        return "{green}information";
    case Severity::debug:
        return "debug";
    case Severity::internal:
        return "internal";
    }
    throw InternalError(nullptr, "Unknown severity");
}

// TODO: bail out on invalid severity
Severity severityFromString(const std::string& severity)
{
    if (severity.empty())
        return Severity::none;
    if (severity == "none")
        return Severity::none;
    if (severity == "error")
        return Severity::error;
    if (severity == "warning")
        return Severity::warning;
    if (severity == "style")
        return Severity::style;
    if (severity == "performance")
        return Severity::performance;
    if (severity == "portability")
        return Severity::portability;
    if (severity == "information")
        return Severity::information;
    if (severity == "debug")
        return Severity::debug;
    if (severity == "internal")
        return Severity::internal;
    return Severity::none;
}
