# Holds information about an array, struct or union's element definition.
class ElementDef:
    def __init__(self, elementType, name, valueType, dimensions = None):
        self.elementType = elementType      # 'array', 'record' or 'value'
        self.name = str(name)
        self.valueType = valueType
        self.children = []
        self.dimensions = dimensions
        self.parent = None

        self.isDesignated = False
        self.isPositional = False
        self.numInits = 0
        self.childIndex = -1

        self.flexibleToken = None
        self.isFlexible = False
        self.structureViolationToken = None

    def __repr__(self):
        inits = ""
        if self.isPositional:
            inits += 'P'
        if self.isDesignated:
            inits += 'D'
        if not (self.isPositional or self.isDesignated) and self.numInits == 0:
            inits += '_'
        if self.numInits > 1:
            inits += str(self.numInits)

        attrs = ["childIndex", "elementType", "valueType"]
        return "{}({}, {}, {})".format(
            "ElementDef",
            self.getLongName(),
            inits,
            ", ".join(("{}={}".format(a, repr(getattr(self, a))) for a in attrs))
        )

    @property
    def isArray(self):
        return self.elementType == 'array'

    @property
    def isRecord(self):
        return self.elementType == 'record'

    @property
    def isValue(self):
        return self.elementType == 'value'


    def getLongName(self):
        return self.parent.getLongName() + "." + self.name if self.parent else self.name

    def getInitDump(self):
        t = []
        if self.isPositional:
            t.append('P')
        if self.isDesignated:
            t.append('D')
        if self.numInits == 0:
            t.append('_')
        if self.numInits > 1:
            t.append(str(self.numInits))

        myDump = "".join(t)

        if len(self.children):
            childDumps = []
            for c in self.children:
                childDumps.append(c.getInitDump())
            if self.structureViolationToken is not None:
                myDump += "!"
            myDump += "{ " + ", ".join(childDumps) + " }"

        return myDump

    def addChild(self, child):
        self.children.append(child)
        child.parent = self

    def getNextChild(self):
        self.childIndex += 1
        return self.getChildByIndex(self.childIndex)

    def getChildByIndex(self, index):
        if self.isFlexible:
            while len(self.children) <= index:
                createChild(self, self.flexibleToken, len(self.children), None)
        return self.children[index] if 0 <= index < len(self.children) else None

    def getChildByName(self, name):
        for c in self.children:
            if c.name == name:
                return c
        return None

    def getNextValueElement(self, root):
        current = self
        while current != root:
            if not current.parent:
                return None
            # Get next index of parent
            i = current.parent.children.index(current) + 1
            # Next index of parent exists
            if i < len(current.parent.children):
                current = current.parent.children[i]
                return current.getFirstValueElement()

            # Next index of parent doesn't exist. Move up
            current = current.parent
        return None

    def getFirstValueElement(self):
        current = self

        # Move to first child as long as children exists
        next_child = current.getChildByIndex(0)
        while next_child:
            current = next_child
            next_child = current.getChildByIndex(0)
        return current

    def getLastValueElement(self):
        current = self
        # Move to last child as long as children exists
        while len(current.children) > 0:
            current = current.children[-1]
        return current

    def getChildByValueElement(self, ed):
        potentialChild = ed
        while potentialChild and potentialChild not in self.children:
            potentialChild = potentialChild.parent

        return self.children[self.children.index(potentialChild)] if potentialChild else None

    def getEffectiveLevel(self):
        if self.parent and self.parent.elementType == "array":
            return self.parent.getEffectiveLevel() + 1
        return 0

    def setInitialized(self, designated=False, positional=False):
        if designated:
            self.isDesignated = True
        if positional or not designated:
            self.isPositional = True
        self.numInits += 1

    def initializeChildren(self):
        for child in self.children:
            child.setInitialized(positional=True)
            child.initializeChildren()

    def unset(self):
        self.isDesignated = False
        self.isPositional = False

        # Unset is always recursive
        for child in self.children:
            child.unset()

    def markStuctureViolation(self, token):
        if self.name == '->':
            self.children[0].markStuctureViolation(token)
        elif not self.structureViolationToken:
            self.structureViolationToken = token

    def markAsFlexibleArray(self, token):
        self.flexibleToken = token
        self.isFlexible = True

    def markAsCurrent(self):
        if self.parent:
            if self.name == '<-':
                self.parent.childIndex = self.parent.children.index(self.children[0])
            else:
                self.parent.childIndex = self.parent.children.index(self)

            self.parent.markAsCurrent()

    def isAllChildrenSet(self):
        myself = len(self.children) == 0 and (self.isDesignated or self.isPositional)
        mychildren = len(self.children) > 0 and all([child.isAllChildrenSet() for child in self.children])
        return myself or mychildren

    def isAllSet(self):
        return all([child.isPositional or child.isDesignated for child in self.children])

    def isOnlyDesignated(self):
        return all([not child.isPositional for child in self.children])

    def isMisra92Compliant(self):
        return self.structureViolationToken is None and all([child.isMisra92Compliant() for child in self.children])

    def isMisra93Compliant(self):
        if self.elementType == 'array':
            result = self.isAllChildrenSet() or \
                ((self.isAllSet() or
                  self.isOnlyDesignated()) and
                 all([not (child.isDesignated or child.isPositional) or child.isMisra93Compliant() for child in self.children]))
            return result
        if self.elementType == 'record':
            result = all([child.isMisra93Compliant() for child in self.children])
            return result
        return True

    def isMisra94Compliant(self):
        return self.numInits <= 1 and all([child.isMisra94Compliant() for child in self.children])

    def isMisra95Compliant(self):
        return not self.isFlexible or all([not child.isDesignated for child in self.children])

# Parses the initializers and update the ElementDefs status accordingly
class InitializerParser:
    def __init__(self):
        self.token = None
        self.root = None
        self.ed = None
        self.rootStack = []

    def parseInitializer(self, root, token):
        self.root = root
        self.token = token
        dummyRoot = ElementDef('array', '->', self.root.valueType)
        dummyRoot.children = [self.root]

        self.rootStack = []
        self.root = dummyRoot
        self.ed = self.root.getFirstValueElement()
        isFirstElement = False
        isDesignated = False

        while self.token:
            if self.token.str == ',':
                self.token = self.token.astOperand1
                isFirstElement = False

            # Designated initializer ( [2]=...  or .name=... )
            elif self.token.isAssignmentOp and not self.token.valueType:
                self.popFromStackIfExitElement()

                self.ed = getElementByDesignator(self.root, self.token.astOperand1)
                if self.ed:
                    # Update root
                    self.pushToRootStackAndMarkAsDesignated()
                    # Make sure ed points to valueElement
                    self.ed = self.ed.getFirstValueElement()

                self.token = self.token.astOperand2
                isFirstElement = False
                isDesignated = True

            elif self.token.isString and self.ed and self.ed.isArray:
                self.ed.setInitialized(isDesignated)
                if self.token == self.token.astParent.astOperand1 and self.token.astParent.astOperand2:
                    self.token = self.token.astParent.astOperand2
                    self.ed.markAsCurrent()
                    self.ed = self.root.getNextChild()
                else:
                    self.unwindAndContinue()
                continue

            elif self.token.str == '{':
                nextChild = self.root.getNextChild() if self.root is not None else None

                if nextChild:
                    if nextChild.isArray or nextChild.isRecord:
                        nextChild.unset()
                        nextChild.setInitialized(isDesignated)
                        self.ed = nextChild.getFirstValueElement()
                        isDesignated = False
                    elif nextChild.valueType is None:
                        # No type information available - unable to check structure - assume correct initialization
                        nextChild.setInitialized(isDesignated)
                        self.unwindAndContinue()
                        continue

                    elif self.token.astOperand1:
                        # Create dummy nextChild to represent excess levels in initializer
                        dummyRoot = ElementDef('array', '<-', self.root.valueType)
                        dummyRoot.parent = self.root
                        dummyRoot.childIndex = 0
                        dummyRoot.children = [nextChild]
                        nextChild.parent = dummyRoot

                        self.root.markStuctureViolation(self.token)

                        # Fake dummy as nextChild (of current root)
                        nextChild = dummyRoot

                if nextChild and self.token.astOperand1:
                    self.root = nextChild
                    self.token = self.token.astOperand1
                    isFirstElement = True
                else:
                    if self.root:
                        # {}
                        if self.root.name == '<-':
                            self.root.parent.markStuctureViolation(self.token)
                        else:
                            self.root.markStuctureViolation(self.token)
                    self.ed = None
                    self.unwindAndContinue()

            else:
                if self.ed and self.ed.isValue:
                    if not isDesignated and len(self.rootStack) > 0 and self.rootStack[-1][1] == self.root:
                        self.rootStack[-1][0].markStuctureViolation(self.token)
                    if isFirstElement and self.token.str == '0' and self.token.next.str == '}':
                        # Zero initializer causes recursive initialization
                        self.root.initializeChildren()
                    elif self.token.isString and self.ed.valueType and self.ed.valueType.pointer > 0:
                        if self.ed.valueType.pointer - self.ed.getEffectiveLevel() == 1:
                            if self.ed.parent != self.root:
                                self.root.markStuctureViolation(self.token)
                            self.ed.setInitialized(isDesignated)
                        elif self.ed.valueType.pointer == self.ed.getEffectiveLevel():
                            if(self.root.name != '->' and self.ed.parent.parent != self.root) or (self.root.name == '->' and self.root.children[0] != self.ed.parent):
                                self.root.markStuctureViolation(self.token)
                            else:
                                self.ed.parent.setInitialized(isDesignated)
                            self.ed.parent.initializeChildren()

                    else:
                        if self.root is not None and self.ed.parent != self.root:
                            # Check if token is correct value type for self.root.children[?]
                            child = self.root.getChildByValueElement(self.ed)
                            if self.token.valueType:
                                if child.elementType != 'record' or self.token.valueType.type != 'record' or child.valueType.typeScope != self.token.valueType.typeScope:
                                    self.root.markStuctureViolation(self.token)

                        self.ed.setInitialized(isDesignated)

                    # Mark all elements up to root with positional or designated
                    # (for complex designators, or missing structure)
                    parent = self.ed.parent
                    while parent and parent != self.root:
                        parent.isDesignated = isDesignated if isDesignated and not parent.isPositional else parent.isDesignated
                        parent.isPositional = not isDesignated if not isDesignated and not parent.isDesignated else parent.isPositional
                        parent = parent.parent
                    isDesignated = False

                    if self.token.isString and self.ed.parent.isArray:
                        self.ed = self.ed.parent
                self.unwindAndContinue()

    def pushToRootStackAndMarkAsDesignated(self):
        new = self.ed.parent
        if new != self.root:
            # Mark all elements up to self.root root as designated
            parent = new
            while parent and parent != self.root:
                parent.isDesignated = True
                parent = parent.parent
            self.rootStack.append((self.root, new))
            new.markAsCurrent()
        new.childIndex = new.children.index(self.ed) - 1
        self.root = new

    def popFromStackIfExitElement(self):
        if len(self.rootStack) > 0 and self.rootStack[-1][1] == self.root:
            old = self.rootStack.pop()[0]
            old.markAsCurrent()
            self.root = old

    def unwindAndContinue(self):
        while self.token:
            if self.token.astParent.astOperand1 == self.token and self.token.astParent.astOperand2:
                if self.ed:
                    if self.token.astParent.astOperand2.str == "{" and self.ed.isDesignated:
                        self.popFromStackIfExitElement()
                    else:
                        self.ed.markAsCurrent()
                        self.ed = self.ed.getNextValueElement(self.root)

                self.token = self.token.astParent.astOperand2
                break

            self.token = self.token.astParent
            if self.token.str == '{':
                if self.root:
                    self.ed = self.root.getLastValueElement()
                    self.ed.markAsCurrent()

                    # Cleanup if root is dummy node representing excess levels in initializer
                    if self.root.name == '<-':
                        self.root.children[0].parent = self.root.parent

                    self.root = self.root.parent

            if self.token.astParent is None:
                self.token = None
                break

def misra_9_x(self, data, rule, rawTokens = None):

    parser = InitializerParser()

    for variable in data.variables:
        if variable.nameToken is None:
            continue

        nameToken = variable.nameToken

        # Check if declaration and initialization is
        # split into two separate statements in ast.
        if nameToken.next and nameToken.next.isSplittedVarDeclEq:
            nameToken = nameToken.next.next

        # Find declarations with initializer assignment
        eq = nameToken
        while not eq.isAssignmentOp and eq.astParent:
            eq = eq.astParent

        # We are only looking for initializers
        if not eq.isAssignmentOp or eq.astOperand2.isName:
            continue

        if variable.isArray or variable.isClass:
            ed = getElementDef(nameToken, rawTokens)
            # No need to check non-arrays if valueType is missing,
            # since we can't say anything useful about the structure
            # without it.
            if ed.valueType is None and not variable.isArray:
                continue
            parser.parseInitializer(ed, eq.astOperand2)
            # print(rule, nameToken.str + '=', ed.getInitDump())
            if rule == 902 and not ed.isMisra92Compliant():
                self.reportError(nameToken, 9, 2)
            if rule == 903 and not ed.isMisra93Compliant():
                # Do not check when variable is pointer type
                type_token = variable.nameToken
                while type_token and type_token.isName:
                    type_token = type_token.previous
                if type_token and type_token.str == '*':
                    continue

                self.reportError(nameToken, 9, 3)
            if rule == 904 and not ed.isMisra94Compliant():
                self.reportError(nameToken, 9, 4)
            if rule == 905 and not ed.isMisra95Compliant():
                self.reportError(nameToken, 9, 5)

def getElementDef(nameToken, rawTokens = None):
    if nameToken.variable.isArray:
        ed = ElementDef("array", nameToken.str, nameToken.valueType)
        createArrayChildrenDefs(ed, nameToken.astParent, nameToken.variable, rawTokens)
    elif nameToken.variable.isClass:
        ed = ElementDef("record", nameToken.str, nameToken.valueType)
        createRecordChildrenDefs(ed, nameToken.variable)
    else:
        ed = ElementDef("value", nameToken.str, nameToken.valueType)
    return ed

def createArrayChildrenDefs(ed, token, var, rawTokens = None):
    if token and token.str == '[':
        if rawTokens is not None:
            foundToken = next((rawToken for rawToken in rawTokens
                               if rawToken.file == token.file
                               and rawToken.linenr == token.linenr
                               and rawToken.column == token.column
                               ), None)

            if foundToken and foundToken.next and foundToken.next.str == ']':
                ed.markAsFlexibleArray(token)

        if (token.astOperand2 is not None) and (token.astOperand2.getKnownIntValue() is not None):
            for i in range(token.astOperand2.getKnownIntValue()):
                createChild(ed, token, i, var)
        else:
            ed.markAsFlexibleArray(token)


def createChild(ed, token, name, var):
    if token.astParent and token.astParent.str == '[':
        child = ElementDef("array", name, ed.valueType)
        createArrayChildrenDefs(child, token.astParent, var)
    else:
        if ed.valueType and ed.valueType.type == "record":
            child = ElementDef("record", name, ed.valueType)
            createRecordChildrenDefs(child, var)
        else:
            child = ElementDef("value", name, ed.valueType)

    ed.addChild(child)

def createRecordChildrenDefs(ed, var):
    valueType = ed.valueType
    if not valueType or not valueType.typeScope:
        return
    if var is None:
        return
    typeToken = var.typeEndToken
    while typeToken and typeToken.isName:
        typeToken = typeToken.previous
    if typeToken and typeToken.str == '*':
        child = ElementDef("pointer", var.nameToken, var.nameToken.valueType)
        ed.addChild(child)
        return
    child_dict = {}
    for variable in valueType.typeScope.varlist:
        if variable is var:
            continue
        child = getElementDef(variable.nameToken)
        child_dict[variable.nameToken] = child
    for scopes in valueType.typeScope.nestedList:
        varscope = False
        if scopes.nestedIn == valueType.typeScope:
            for variable in valueType.typeScope.varlist:
                if variable.nameToken and variable.nameToken.valueType and variable.nameToken.valueType.typeScope == scopes:
                    varscope = True
                    break
            if not varscope:
                ed1 = ElementDef("record", scopes.Id, valueType)
                for variable in scopes.varlist:
                    child = getElementDef(variable.nameToken)
                    ed1.addChild(child)
                child_dict[scopes.bodyStart] = ed1
    sorted_keys = sorted(list(child_dict.keys()), key=lambda k: (k.file, k.linenr, k.column))
    for _key in sorted_keys:
        ed.addChild(child_dict[_key])


def getElementByDesignator(ed, token):
    if not token.str in [ '.', '[' ]:
        return None

    while token.str in [ '.', '[' ]:
        token = token.astOperand1

    while ed and not token.isAssignmentOp:
        token = token.astParent

        if token.str == '[':
            if not ed.isArray:
                ed.markStuctureViolation(token)

            chIndex = -1
            if token.astOperand2 is not None:
                chIndex = token.astOperand2.getKnownIntValue()
            elif token.astOperand1 is not None:
                chIndex = token.astOperand1.getKnownIntValue()

            ed = ed.getChildByIndex(chIndex) if chIndex is not None else None

        elif token.str == '.':
            if not ed.isRecord:
                ed.markStuctureViolation(token)

            name = ""
            if token.astOperand2 is not None:
                name = token.astOperand2.str
            elif token.astOperand1 is not None:
                name = token.astOperand1.str

            ed = ed.getChildByName(name)

    return ed
