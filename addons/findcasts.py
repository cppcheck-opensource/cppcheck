#!/usr/bin/env python3
#
# Locate casts in the code
#

import cppcheck

@cppcheck.checker
def cast(cfg, data):
    for token in cfg.tokenlist:
        if token.str != '(' or not token.astOperand1 or token.astOperand2:
            continue

        # Is it a lambda?
        if token.astOperand1.str == '{':
            continue

        # we probably have a cast.. if there is something inside the parentheses
        # there is a cast. Otherwise this is a function call.
        typetok = token.next
        if not typetok.isName:
            continue

        # cast number => skip output
        if token.astOperand1.isNumber:
            continue

        # void cast => often used to suppress compiler warnings
        if typetok.str == 'void':
            continue

        cppcheck.reportError(token, 'information', 'found a cast')
