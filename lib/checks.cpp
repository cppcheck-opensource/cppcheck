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

#include "checks.h"

#include "check64bit.h"
#include "checkassert.h"
#include "checkautovariables.h"
#include "checkbool.h"
#include "checkbufferoverrun.h"
#include "checkclass.h"
#include "checkcondition.h"
#include "checkexceptionsafety.h"
#include "checkfunctions.h"
#include "checkinternal.h"
#include "checkio.h"
#include "checkleakautovar.h"
#include "checkmemoryleak.h"
#include "checknullpointer.h"
#include "checkother.h"
#include "checkpostfixoperator.h"
#include "checksizeof.h"
#include "checkstl.h"
#include "checkstring.h"
#include "checktype.h"
#include "checkuninitvar.h"
#include "checkunusedvar.h"
#include "checkvaarg.h"

#include <memory>

class CheckInstancesImpl
{
private:
/* *INDENT-OFF* */
#define UPI(c) std::unique_ptr<c> m##c{new c}
/* *INDENT-ON* */
    UPI(Check64BitPortability);
    UPI(CheckAssert);
    UPI(CheckAutoVariables);
    UPI(CheckBool);
    UPI(CheckBufferOverrun);
    UPI(CheckClass);
    UPI(CheckCondition);
    UPI(CheckExceptionSafety);
    UPI(CheckFunctions);
#ifdef CHECK_INTERNAL
    UPI(CheckInternal);
#endif
    UPI(CheckIO);
    UPI(CheckLeakAutoVar);
    UPI(CheckMemoryLeakInFunction);
    UPI(CheckMemoryLeakInClass);
    UPI(CheckMemoryLeakStructMember);
    UPI(CheckMemoryLeakNoVar);
    UPI(CheckNullPointer);
    UPI(CheckOther);
    UPI(CheckPostfixOperator);
    UPI(CheckSizeof);
    UPI(CheckStl);
    UPI(CheckString);
    UPI(CheckType);
    UPI(CheckUninitVar);
    UPI(CheckUnusedVar);
    UPI(CheckVaarg);
#undef UPI

public:
    const std::list<Check *>& get() const
    {
        static std::list<Check*> s_checks{
            mCheck64BitPortability.get(),
            mCheckAssert.get(),
            mCheckAutoVariables.get(),
            mCheckBool.get(),
            mCheckBufferOverrun.get(),
            mCheckClass.get(),
            mCheckCondition.get(),
            mCheckExceptionSafety.get(),
            mCheckFunctions.get(),
    #ifdef CHECK_INTERNAL
            mCheckInternal.get(),
    #endif
            mCheckIO.get(),
            mCheckLeakAutoVar.get(),
            mCheckMemoryLeakInFunction.get(),
            mCheckMemoryLeakInClass.get(),
            mCheckMemoryLeakStructMember.get(),
            mCheckMemoryLeakNoVar.get(),
            mCheckNullPointer.get(),
            mCheckOther.get(),
            mCheckPostfixOperator.get(),
            mCheckSizeof.get(),
            mCheckStl.get(),
            mCheckString.get(),
            mCheckType.get(),
            mCheckUninitVar.get(),
            mCheckUnusedVar.get(),
            mCheckVaarg.get()
        };

        return s_checks;
    }
};

const std::list<Check*>& CheckInstances::get()
{
    static const CheckInstancesImpl s_impl;
    return s_impl.get();
}
