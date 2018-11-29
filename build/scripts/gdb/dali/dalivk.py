# Copyright (c) 2018 Samsung Electronics Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Dali pretty printers
# Copy of dali.py to handle temporary DaliVk namespace nonsense

import gdb
import re
import sys

# Copied from stl printers
if sys.version_info[0] > 2:
    ### Python 3 stuff
    Iterator = object
    # Python 3 folds these into the normal functions.
    imap = map
    izip = zip
    # Also, int subsumes long
    long = int
else:
    ### Python 2 stuff
    class Iterator:
        """Compatibility mixin for iterators

        Instead of writing next() methods for iterators, write
        __next__() methods and use this mixin to make them work in
        Python 2 as well as Python 3.

        Idea stolen from the "six" documentation:
        <http://pythonhosted.org/six/#six.Iterator>
        """

        def next(self):
            return self.__next__()

    # In Python 2, we still need these from itertools
    from itertools import imap, izip

_use_gdb_pp = True
try:
    import gdb.printing
except ImportError:
    _use_gdb_pp = False


## Look at
## http://sourceware.org/gdb/onlinedocs/gdb/Values-From-Inferior.html
## for info on converting types from C/C++ to python

## CREATE PRINTER CLASS:
## Constructor takes (self, typename, val) tuple
## Requires a to_string(self) method
## Can also add other methods, such as children(), display_hint(),


class DaliTreeNodePrinter:
    "Print a Dali tree node"

    def __init__(self, typename, val):
        self.typename = typename;
        self.val = val

    def to_string(self):
        nodeTypeEnum = gdb.lookup_type("DaliVk::Toolkit::TreeNode::NodeType");
        enumDict={}
        revEnumDict={}
        for field in nodeTypeEnum.fields():
            enumDict[field.name] = field.enumval
            revEnumDict[field.enumval] = field.name

        nodeType = self.val['mType']
        name = '{ name:'
        if self.val['mName'] != 0:
            name += '\"%s\", ' % self.val['mName'].string();
        else:
            name += 'None, '

        if nodeType == enumDict['DaliVk::Toolkit::TreeNode::IS_NULL']:
            return name + 'value:NULL}'

        elif nodeType == enumDict['DaliVk::Toolkit::TreeNode::STRING']:
            return name + 'value:\"%s\"}' % self.val['mStringValue'].string()

        elif nodeType == enumDict['DaliVk::Toolkit::TreeNode::INTEGER']:
            return name + 'value:%d }' % self.val['mIntValue']

        elif nodeType == enumDict['DaliVk::Toolkit::TreeNode::FLOAT']:
            return name + 'value:%f }' % self.val['mFloatValue']

        elif nodeType == enumDict['DaliVk::Toolkit::TreeNode::BOOLEAN']:
            if self.val['mIntValue'] != 0:
                return name + "value:true }"
            else:
                return name + "value:false }"
        return name + 'value:OTHER }'


class DaliVectorPrinter:
   "Print a DaliVk::Vector"

   class _iterator(Iterator):
       def __init__ (self, itemCount, itemPtr):
           self.itemCount = itemCount
           self.itemPtr = itemPtr
           self.count = 0

       def __iter__(self):
           return self

       def __next__(self):
           #print( "\nITER::NEXT curCount:%d itemCount:%d PtrType:%s\n" % ( self.count, self.itemCount, self.itemPtr.type ) )
           count = self.count
           self.count = self.count + 1
           if count == self.itemCount:
               raise StopIteration
           elt = self.itemPtr.dereference()
           self.itemPtr = self.itemPtr + 1
           if elt.type.code == gdb.TYPE_CODE_PTR:
               if elt != 0:
                   #ptr = elt.cast(elt.dynamic_type)
                   #eltderef = ptr.dereference()
                   #return ('[%d]' % count, eltderef)
                   # Should also try and return elt.dynamic_type if not already
                   return ('[%d]' % count, elt)
               else:
                   return ('[%d]' % count, 0 )
           return ('[%d]' % count, elt)

   def __init__(self, typename, val):
       self.typename = typename
       self.val = val
       self.dataPtr = val['mData'].cast(gdb.lookup_type("DaliVk::VectorBase::SizeType").pointer())
       self.itemCount = 0
       if self.dataPtr != 0:
           self.itemCount = (self.dataPtr-1).dereference()
       #print( "\ntemplateArg:%s\n" % self.val.type.template_argument(0) )
       self.itemType = self.val.type.template_argument(0)
       self.itemPtr = val['mData'].cast(self.itemType.pointer())

   def children(self):
       return self._iterator(self.itemCount, self.itemPtr)

   def to_string(self):
       #return "0x%x [%d]" % ((long(self.dataPtr)), self.itemCount)
       return ( "Count:%d" % self.itemCount )

#   def display_hint(self):
#       return "array"


class DaliHandlePrinter:
    "Print a DaliVk::Handle"

    def __init__(self, typename, val):
        self.typename = typename
        self.value = val;
        self.obj = val['mObjectHandle']['mPtr']
        #self.val = self.obj.cast(gdb.lookup_type("DaliVk::Internal::Actor").pointer())

    def to_string (self):
        if self.typename != 'WeakHandleBase':
            ptr = self.obj.cast(self.obj.dynamic_type)
        else:
            ptr = self.obj
        object=ptr.dereference()
        return 'Handle(%s) => %s' % ( self.value.address, object )

class DaliPropertyMetadataPrinter:
    "Print DaliVk::PropertyMetadata"

    def __init__(self, typename, val):
        self.typename = typename
        self.value = val['value']
        self.componentIndex = val['componentIndex']

    def to_string (self):
        return 'value = %s, componentIndex=%s' % ( self.value, self.componentIndex )

class DaliCustomPropertyMetadataPrinter:
    "Print DaliVk::CustomPropertyMetadata"

    def __init__(self, typename, val):
        self.typename = typename
        self.name = val['name']
        self.key = val['key']
        self.childPropertyIndex = val['childPropertyIndex']
        self.value = val['value']
        self.componentIndex = val['componentIndex']

    def to_string (self):
        return '(name = %s, key = %d, childPropertyIndex = %d, value = %s, componentIndex=%s)' % ( self.name, self.key, self.childPropertyIndex, self.value, self.componentIndex )


#Root|Rend|Layer|Stg|Sens|Fcsble|RotInh|ScaleInh|Norm|InhPPos|OwnColr
#Twig|Cont|Actor|Off|Insn|Unfcbl|RotLoc|ScaleLoc|Ovly|UsePPos|PrntCol
#                                                Stnc|PPosLoc|BlndCol
#                                                     LocIsWd|BlndAlf
class DaliActorPrinter:
    "Print a DaliVk::Actor"

    def __init__(self, typename, val):
        self.typename = typename
        self.instance = val['mObjectHandle']
        self.obj = self.instance['mPtr']
        self.val = self.obj.cast(gdb.lookup_type("DaliVk::Internal::Actor").pointer())

    def isRoot (self):
        return "Root" if self.actor['mIsRoot'] == 1 else "Twig"

    def BoolRenderable(self):
        return "+Drw" if self.actor['mIsRenderable'] else "-Drw"

    def BoolLayer(self):
        return "Layer" if self.actor['mIsLayer'] else "Actor"

    def BoolStaged(self):
        return "Stg" if self.actor['mIsOnStage'] else "Off"

    def BoolSensitive(self):
        return "Sens" if self.actor['mSensitive'] else "Insn"

    def BoolFocusable(self):
        return "Fcsble" if self.actor['mKeyboardFocusable'] else "Unfcbl"

    def BoolRotation(self):
        return "RotInh" if self.actor['mInheritRotation'] else "RotLoc"

    def BoolScale(self):
        return "SclInh" if self.actor['mInheritScale'] else "SclLoc"

    def DrawMode(self):
        if self.actor['mDrawMode'] == 'STENCIL':
            return "Stnc"
        elif self.actor['mDrawMode'] == 'OVERLAY':
            return "Ovly"
        return "Norm"

    def to_string (self):
        self.actor = self.val.dereference()
        return '[%s(%d) Node:0x%x]' % (self.actor['mName'], self.actor['mId'], int(self.actor['mNode']) )

#        return '[%s(%d) Node:0x%x Flags:%s|%s|%s|%s|%s|%s|%s|%s]' % (self.actor['mName'], self.actor['mId'], self.actor['mNode'], self.BoolRoot(), self.BoolRenderable(), self.BoolLayer(), self.BoolStaged(), self.BoolSensitive(), self.BoolFocusable(), self.BoolRotation(), self.BoolScale() )


class DaliVector2Printer:
    "Print a DaliVk::Vector2"

    def __init__(self, typename, val):
        self.typename = typename
        self.val = val

    def to_string(self):
        return '<%g, %g>' % ( self.val['x'], self.val['y'] )

class DaliVector3Printer:
    "Print a DaliVk::Vector3"

    def __init__(self, typename, val):
        self.typename = typename
        self.val = val

    def to_string(self):
        return '<%g, %g, %g>' % ( self.val['x'], self.val['y'], self.val['z'] )


class DaliVector4Printer:
    "Print a DaliVk::Vector4"

    def __init__(self, typename, val):
        self.typename = typename
        self.val = val

    def to_string(self):
        return '<%g, %g, %g, %g>' % ( self.val['x'], self.val['y'], self.val['z'], self.val['w'] )

class DaliQuaternionPrinter:
    "Print a DaliVk::Quaternion"

    def __init__(self, typename, val):
        self.typename = typename
        self.val = val

    def to_string(self):
        return '<(%g, %g, %g) w=%g>' % ( self.val['mVector']['x'], self.val['mVector']['y'], self.val['mVector']['z'], self.val['mVector']['w'] )

class DaliMatrixPrinter:
    "Print a DaliVk::Matrix"

    def __init__(self, typename, val):
        self.typename = typename
        self.val = val

    def to_string(self):
        return '[ %s ]' % ( self.val['mMatrix'] )

class DaliMatrix3Printer:
    "Print a DaliVk::Matrix"

    def __init__(self, typename, val):
        self.typename = typename
        self.val = val

    def to_string(self):
        return  self.val['mElements']

class DaliMaterialPrinter:
    "Print a DaliVk::Material"

    def __init__(self, typename, val):
        self.typename = typename
        self.val = val

    def to_string(self):
        return  self.val['mObjectHandle']


class DaliPropertyValuePrinter:
    "Print a Dali property value"

    def __init__(self, typename, val):
        self.typename = typename;
        self.val = val

    def to_string(self):
        valueTypeEnum = gdb.lookup_type("DaliVk::Property::Type");
        enumDict={}
        revEnumDict={}
        for field in valueTypeEnum.fields():
            enumDict[field.name] = field.enumval
            revEnumDict[field.enumval] = field.name

        valueImplPtr = self.val['mImpl']
        # valueImpl is opaque, but is really 2 values, an enum and a ptr/int/float union
        valueType = valueImplPtr.cast(gdb.lookup_type("int").pointer()).dereference()
        valueUnionPtr = valueImplPtr.cast(gdb.lookup_type("long int").pointer()) + 1
        valueUnion = valueUnionPtr.dereference()
        #print('Type:%d Union %08X' % (valueType,valueUnion))

        valueInt = valueUnion.cast(gdb.lookup_type("int"))
        if valueType == enumDict['DaliVk::Property::BOOLEAN']:
            if valueInt != 0:
                return 'true'
            else:
                return 'false'

        elif valueType == enumDict['DaliVk::Property::INTEGER']:
            return valueInt

        elif valueType == enumDict['DaliVk::Property::FLOAT']:
            return '%f' % valueUnionPtr.cast(gdb.lookup_type("float").pointer()).dereference()

        elif valueType == enumDict['DaliVk::Property::STRING']:
            try:
                stdStringType = gdb.lookup_type("std::string")
            except RuntimeError:
                stdStringType = gdb.lookup_type("std::__cxx11::string")
            stdStringPtrType = stdStringType.pointer()
            return valueUnion.cast(stdStringPtrType).dereference()

        elif valueType == enumDict['DaliVk::Property::VECTOR2']:
            return valueUnion.cast(gdb.lookup_type("DaliVk::Vector2").pointer()).dereference()
        elif valueType == enumDict['DaliVk::Property::VECTOR3']:
            return valueUnion.cast(gdb.lookup_type("DaliVk::Vector3").pointer()).dereference()
        elif valueType == enumDict['DaliVk::Property::VECTOR4']:
            return valueUnion.cast(gdb.lookup_type("DaliVk::Vector4").pointer()).dereference()
        elif valueType == enumDict['DaliVk::Property::MATRIX']:
            return valueUnion.cast(gdb.lookup_type("DaliVk::Matrix").pointer()).dereference()
        elif valueType == enumDict['DaliVk::Property::MATRIX3']:
            return valueUnion.cast(gdb.lookup_type("DaliVk::Matrix3").pointer()).dereference()
        elif valueType == enumDict['DaliVk::Property::ROTATION']:
            return valueUnion.cast(gdb.lookup_type("DaliVk::Vector4").pointer()).dereference()
        elif valueType == enumDict['DaliVk::Property::RECTANGLE']:
            return valueUnion.cast(gdb.lookup_type("DaliVk::Vector4").pointer()).dereference()
        elif valueType == enumDict['DaliVk::Property::ARRAY']:
            return valueUnion.cast(gdb.lookup_type("DaliVk::Property::Array").pointer()).dereference()
        elif valueType == enumDict['DaliVk::Property::MAP']:
            return valueUnion.cast(gdb.lookup_type("DaliVk::Property::Map").pointer()).dereference()


class DaliPropertyArrayPrinter:
    "Print a Dali Property Array"

    def __init__(self, typename, val):
        self.typename = typename;
        self.val = val
        self.valueImplPtr = val['mImpl']
        # valueImpl is opaque, but is really a std::vector of Values
        self.valueVector = self.valueImplPtr.cast(gdb.lookup_type("DaliVk::DebugPropertyValueArray").pointer()).dereference()
        self.start = self.valueVector['_M_impl']['_M_start']
        self.finish = self.valueVector['_M_impl']['_M_finish']

    class _iterator(Iterator):
        def __init__ (self, start, finish):
            self.item = start
            self.finish = finish
            self.count = 0

        def __iter__(self):
            return self

        def __next__(self):
            count = self.count
            self.count = self.count + 1
            if self.item == self.finish:
                raise StopIteration
            elt = self.item.dereference()
            self.item = self.item + 1
            return ('[%d]' % count, elt)

    def children(self):
        # std::vector specific
        return self._iterator( self.start, self.finish )

    def to_string(self):
        return ('[#%d]' % int(self.finish - self.start) )

    def display_hint(self):
        return 'array'


class DaliPropertyMapPrinter:
    "Print a Dali Property Map"

    def __init__(self, typename, val):
        self.typename = typename;
        self.valueImplPtr = val['mImpl']
        # valueImpl is opaque, but is really a std::vector of Values
        self.valueStruct = self.valueImplPtr.cast(gdb.lookup_type("DaliVk::DebugPropertyValueMap").pointer()).dereference()
        self.valueStrKeys = self.valueStruct['stringValues']
        self.strStart = self.valueStrKeys['_M_impl']['_M_start']
        self.strFinish = self.valueStrKeys['_M_impl']['_M_finish']

        self.valueIntKeys = self.valueStruct['intValues']
        self.intStart = self.valueIntKeys['_M_impl']['_M_start']
        self.intFinish = self.valueIntKeys['_M_impl']['_M_finish']

    class _iterator(Iterator):
        def __init__ (self, strStart, strFinish, intStart, intFinish):
            self.item = strStart
            self.strFinish = strFinish
            self.intStart  = intStart
            self.intFinish = intFinish
            self.count = 0
            self.state=0

        def __iter__(self):
            return self

        def __next__(self):
            count = self.count
            self.count = self.count + 1
            if self.state == 0:
                if self.item == self.strFinish:
                    self.count = 0
                    self.state = 1
                    self.item = self.intStart
            else:
                if self.item == self.intFinish:
                    raise StopIteration
            if self.item == 0:
                raise StopIteration

            elt = self.item.dereference()
            self.item = self.item + 1
            elValue = elt['second']
            if self.state == 0:
                return ('[%d]' % count, '%s:%s' % (elt['first'], elValue) )
            else:
                return ('[%d]' % count, '%d:%s' % (elt['first'], elValue) )

    def children(self):
        # std::vector specific
        return self._iterator( self.strStart, self.strFinish, self.intStart, self.intFinish )

    def to_string(self):
        return None

    def display_hint(self):
        return 'array'



# Added from http://rethinkdb.com/blog/make-debugging-easier-with-custom-pretty-printers/
class GenericPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "Generic object with the following members:"

    def children(self):
        for k, v in process_kids(self.val, self.val):
            for k2, v2 in printer(k, v): yield k2, v2

# Added from http://rethinkdb.com/blog/make-debugging-easier-with-custom-pretty-printers/
def process_kids(state, PF):
    for field in PF.type.fields():
        if field.artificial or field.type == gdb.TYPE_CODE_FUNC or \
        field.type == gdb.TYPE_CODE_VOID or field.type == gdb.TYPE_CODE_METHOD or \
        field.type == gdb.TYPE_CODE_METHODPTR or field.type == None: continue
        key = field.name
        if key is None: continue
        try: state[key]
        except RuntimeError: continue
        val = PF[key]
        if field.is_base_class and len(field.type.fields()) != 0:
            for k, v in process_kids(state, field):
                yield key + " :: " + k, v
        else:
            yield key, val

# A "regular expression" printer which conforms to the
# "SubPrettyPrinter" protocol from gdb.printing.
class RxPrinter(object):
    def __init__(self, name, function):
        super(RxPrinter, self).__init__()
        self.name = name
        self.function = function
        self.enabled = True

    def invoke(self, value):
        if not self.enabled:
            return None
        return self.function(self.name, value)

# A pretty-printer that conforms to the "PrettyPrinter" protocol from
# gdb.printing.  It can also be used directly as an old-style printer.
class Printer(object):
    def __init__(self, name):
        super(Printer, self).__init__()
        self.name = name
        self.subprinters = []
        self.lookup = {}
        self.enabled = True
        self.compiled_rx = re.compile('^([a-zA-Z0-9_:]+)<.*>$')
        self.generic_rx = re.compile('.*Generic.*')
        self.genericPrinter = RxPrinter('Generic', GenericPrinter)

    def add(self, name, function):
        # A small sanity check.
        # FIXME
        #if not self.compiled_rx.match(name + '<>'):
        #  raise ValueError, 'Dali programming error: "%s" does not match' % name
        printer = RxPrinter(name, function)
        self.subprinters.append(printer)
        self.lookup[name] = printer

    @staticmethod
    def get_basic_type(type):
        # If it points to a reference, get the reference.
        if type.code == gdb.TYPE_CODE_REF:
            type = type.target ()

        # Get the unqualified type, stripped of typedefs.
        type = type.unqualified ().strip_typedefs ()

        return type.tag

    def __call__(self, val):
        typename = self.get_basic_type(val.type)
        if not typename:
            return None

        # Try non-templated type
        if typename in self.lookup:
            return self.lookup[typename].invoke(val)

        # Try instead templated type
        match = self.compiled_rx.match(typename)
        if match:
            basename = match.group(1)
            if basename in self.lookup:
                return self.lookup[basename].invoke(val)

        if self.generic_rx.match(typename):
            return self.genericPrinter.invoke(val)

        # Cannot find a pretty printer.  Return None.
        return None

libdalivk_printer = None

def register_dalivk_printers(obj):
    "Registers DaliVK pretty printers"

    global libdalivk_printer
    if _use_gdb_pp:
        gdb.printing.register_pretty_printer(obj, libdalivk_printer)
    else:
        if obj is None:
            obj = gdb
        obj.pretty_printers.append(libdalivk_printer)


def build_libdalivk_dictionary ():
    global libdalivk_printer
    libdalivk_printer = Printer("libdali-toolkit-vk")

    libdalivk_printer.add('DaliVk::Handle', DaliHandlePrinter)
    libdalivk_printer.add('DaliVk::Actor', DaliActorPrinter)
    libdalivk_printer.add('DaliVk::Vector2', DaliVector2Printer)
    libdalivk_printer.add('DaliVk::Vector3', DaliVector3Printer)
    libdalivk_printer.add('DaliVk::Vector4', DaliVector4Printer)
    libdalivk_printer.add('DaliVk::Quaternion', DaliQuaternionPrinter)
    libdalivk_printer.add('DaliVk::Matrix',  DaliMatrixPrinter)
    libdalivk_printer.add('DaliVk::Matrix3', DaliMatrix3Printer)
    libdalivk_printer.add('DaliVk::Material',  DaliMaterialPrinter)
    libdalivk_printer.add('DaliVk::Material',  DaliMaterialPrinter)
    libdalivk_printer.add('DaliVk::Property::Value', DaliPropertyValuePrinter)
    libdalivk_printer.add('DaliVk::Property::Array', DaliPropertyArrayPrinter)
    libdalivk_printer.add('DaliVk::Property::Map', DaliPropertyMapPrinter)
    libdalivk_printer.add('DaliVk::Internal::PropertyMetadata', DaliPropertyMetadataPrinter)
    libdalivk_printer.add('DaliVk::Internal::CustomPropertyMetadata', DaliCustomPropertyMetadataPrinter)
    libdalivk_printer.add('DaliVk::Vector', DaliVectorPrinter)
    libdalivk_printer.add('DaliVk::Toolkit::TreeNode', DaliTreeNodePrinter)

## ADD PRINTERS ABOVE HERE ^^^^^

build_libdalivk_dictionary ()
