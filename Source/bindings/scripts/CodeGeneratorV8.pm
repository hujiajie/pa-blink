# Copyright (C) 2005, 2006 Nikolas Zimmermann <zimmermann@kde.org>
# Copyright (C) 2006 Anders Carlsson <andersca@mac.com>
# Copyright (C) 2006 Samuel Weinig <sam.weinig@gmail.com>
# Copyright (C) 2006 Alexey Proskuryakov <ap@webkit.org>
# Copyright (C) 2006 Apple Computer, Inc.
# Copyright (C) 2007, 2008, 2009, 2012 Google Inc.
# Copyright (C) 2009 Cameron McCormack <cam@mcc.id.au>
# Copyright (C) Research In Motion Limited 2010. All rights reserved.
# Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
# Copyright (C) 2012 Ericsson AB. All rights reserved.
# Copyright (C) 2013 Samsung Electronics. All rights reserved.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; see the file COPYING.LIB.  If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#

package Block;

# Sample code:
#   my $outer = new Block("Free Name 1", "namespace Foo {", "} // namespace Foo");
#   $outer->add("    void foo() {}");
#   my $inner = new Block("Free Name 2", "namespace Bar {", "} // namespace Bar");
#   $inner->add("    void bar() {}");
#   $outer->add($inner);
#   print $outer->toString();
#
# Output code:
#   namespace Foo {
#       void foo() {}
#   namespace Bar {
#       void bar() {}
#   } // namespace Bar
#   } // namespace Foo

sub new
{
    my $package = shift;
    my $name = shift || "Anonymous block";
    my $header = shift || "";
    my $footer = shift || "";

    my $object = {
        "name" => $name,
        "header" => [$header],
        "footer" => [$footer],
        "contents" => [],
    };
    bless $object, $package;
    return $object;
}

sub addHeader
{
    my $object = shift;
    my $header = shift || "";

    push(@{$object->{header}}, $header);
}

sub addFooter
{
    my $object = shift;
    my $footer = shift || "";

    push(@{$object->{footer}}, $footer);
}

sub add
{
    my $object = shift;
    my $content = shift || "";

    push(@{$object->{contents}}, $content);
}

sub toString
{
    my $object = shift;

    my $header = join "", @{$object->{header}};
    my $footer = join "", @{$object->{footer}};
    my $code = "";
    $code .= "/* BEGIN " . $object->{name} . " */\n" if $verbose;
    $code .=  $header . "\n" if $header;
    for my $content (@{$object->{contents}}) {
        if (ref($content) eq "Block") {
            $code .= $content->toString();
        } else {
            $code .= $content;
        }
    }
    $code .=  $footer . "\n" if $footer;
    $code .= "/* END " . $object->{name} . " */\n" if $verbose;
    return $code;
}


package CodeGeneratorV8;

use strict;
use Cwd;
use File::Basename;
use File::Find;
use File::Spec;

my $idlDocument;
my $idlDirectories;
my $preprocessor;
my $defines;
my $verbose;
my $dependentIdlFiles;
my $writeFileOnlyIfChanged;
my $sourceRoot;

# Cache of IDL file pathnames.
my $idlFiles;
my $cachedInterfaces = {};

my %implIncludes = ();
my %headerIncludes = ();

# Header code structure:
# Root                    ... Copyright, include duplication check
#   Conditional           ... #if FEATURE ... #endif  (to be removed soon)
#     Includes
#     NameSpaceWebCore
#       Class
#         ClassPublic
#         ClassPrivate
my %header;

# Implementation code structure:
# Root                    ... Copyright
#   Conditional           ... #if FEATURE ... #endif  (to be removed soon)
#     Includes
#     NameSpaceWebCore
#     NameSpaceInternal   ... namespace ${implClassName}V8Internal in case of non-callback
my %implementation;

my %primitiveTypeHash = ("boolean" => 1,
                         "void" => 1,
                         "Date" => 1,
                         "byte" => 1,
                         "octet" => 1,
                         "short" => 1,
                         "long" => 1,
                         "long long" => 1,
                         "unsigned short" => 1,
                         "unsigned long" => 1,
                         "unsigned long long" => 1,
                         "float" => 1,
                         "double" => 1,
                        );

my %nonWrapperTypes = ("CompareHow" => 1,
                       "DOMTimeStamp" => 1,
                       "Dictionary" => 1,
                       "EventListener" => 1,
                       # FIXME: When EventTarget is an interface and not a mixin, fix this so that
                       # EventTarget is treated as a wrapper type.
                       "EventTarget" => 1,
                       "MediaQueryListListener" => 1,
                       "NodeFilter" => 1,
                       "SerializedScriptValue" => 1,
                       "any" => 1,
                      );

my %typedArrayHash = ("ArrayBuffer" => [],
                      "ArrayBufferView" => [],
                      "Uint8Array" => ["unsigned char", "v8::kExternalUnsignedByteArray"],
                      "Uint8ClampedArray" => ["unsigned char", "v8::kExternalPixelArray"],
                      "Uint16Array" => ["unsigned short", "v8::kExternalUnsignedShortArray"],
                      "Uint32Array" => ["unsigned int", "v8::kExternalUnsignedIntArray"],
                      "Int8Array" => ["signed char", "v8::kExternalByteArray"],
                      "Int16Array" => ["short", "v8::kExternalShortArray"],
                      "Int32Array" => ["int", "v8::kExternalIntArray"],
                      "Float32Array" => ["float", "v8::kExternalFloatArray"],
                      "Float64Array" => ["double", "v8::kExternalDoubleArray"],
                     );

my %callbackFunctionTypeHash = ();

my %enumTypeHash = ();

my %svgAnimatedTypeHash = ("SVGAnimatedAngle" => 1, "SVGAnimatedBoolean" => 1,
                           "SVGAnimatedEnumeration" => 1, "SVGAnimatedInteger" => 1,
                           "SVGAnimatedLength" => 1, "SVGAnimatedLengthList" => 1,
                           "SVGAnimatedNumber" => 1, "SVGAnimatedNumberList" => 1,
                           "SVGAnimatedPreserveAspectRatio" => 1,
                           "SVGAnimatedRect" => 1, "SVGAnimatedString" => 1,
                           "SVGAnimatedTransformList" => 1);

my %svgAttributesInHTMLHash = ("class" => 1, "id" => 1, "onabort" => 1, "onclick" => 1,
                               "onerror" => 1, "onload" => 1, "onmousedown" => 1,
                               "onmousemove" => 1, "onmouseout" => 1, "onmouseover" => 1,
                               "onmouseup" => 1, "onresize" => 1, "onscroll" => 1,
                               "onunload" => 1);

my %svgTypeNeedingTearOff = (
    "SVGAngle" => "SVGPropertyTearOff<SVGAngle>",
    "SVGLength" => "SVGPropertyTearOff<SVGLength>",
    "SVGLengthList" => "SVGListPropertyTearOff<SVGLengthList>",
    "SVGMatrix" => "SVGPropertyTearOff<SVGMatrix>",
    "SVGNumber" => "SVGPropertyTearOff<float>",
    "SVGNumberList" => "SVGListPropertyTearOff<SVGNumberList>",
    "SVGPathSegList" => "SVGPathSegListPropertyTearOff",
    "SVGPoint" => "SVGPropertyTearOff<FloatPoint>",
    "SVGPointList" => "SVGListPropertyTearOff<SVGPointList>",
    "SVGPreserveAspectRatio" => "SVGPropertyTearOff<SVGPreserveAspectRatio>",
    "SVGRect" => "SVGPropertyTearOff<FloatRect>",
    "SVGStringList" => "SVGStaticListPropertyTearOff<SVGStringList>",
    "SVGTransform" => "SVGPropertyTearOff<SVGTransform>",
    "SVGTransformList" => "SVGTransformListPropertyTearOff"
);

my %svgTypeWithWritablePropertiesNeedingTearOff = (
    "SVGPoint" => 1,
    "SVGMatrix" => 1
);

# Default .h template
my $headerTemplate = <<EOF;
/*
    This file is part of the Blink open source project.
    This file has been auto-generated by CodeGeneratorV8.pm. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
EOF

sub new
{
    my $object = shift;
    my $reference = { };

    $idlDocument = shift;
    $idlDirectories = shift;
    $preprocessor = shift;
    $defines = shift;
    $verbose = shift;
    $dependentIdlFiles = shift;
    $writeFileOnlyIfChanged = shift;

    $sourceRoot = getcwd();

    bless($reference, $object);
    return $reference;
}


sub IDLFileForInterface
{
    my $interfaceName = shift;

    unless ($idlFiles) {
        my @directories = map { $_ = "$sourceRoot/$_" if -d "$sourceRoot/$_"; $_ } @$idlDirectories;
        push(@directories, ".");

        $idlFiles = { };
        foreach my $idlFile (@$dependentIdlFiles) {
            $idlFiles->{fileparse(basename($idlFile), ".idl")} = $idlFile;
        }

        my $wanted = sub {
            $idlFiles->{$1} = $File::Find::name if /^([A-Z].*)\.idl$/;
            $File::Find::prune = 1 if /^\../;
        };
        find($wanted, @directories);
    }

    return $idlFiles->{$interfaceName};
}

sub ParseInterface
{
    my $interfaceName = shift;

    if (exists $cachedInterfaces->{$interfaceName}) {
        return $cachedInterfaces->{$interfaceName};
    }

    # Step #1: Find the IDL file associated with 'interface'
    my $filename = IDLFileForInterface($interfaceName)
        or die("Could NOT find IDL file for interface \"$interfaceName\" $!\n");

    print "  |  |>  Parsing parent IDL \"$filename\" for interface \"$interfaceName\"\n" if $verbose;

    # Step #2: Parse the found IDL file (in quiet mode).
    my $parser = IDLParser->new(1);
    my $document = $parser->Parse($filename, $defines, $preprocessor);

    foreach my $interface (@{$document->interfaces}) {
        if ($interface->name eq $interfaceName or $interface->isPartial) {
            $cachedInterfaces->{$interfaceName} = $interface;
            return $interface;
        }
    }

    die("Could NOT find interface definition for $interfaceName in $filename");
}

sub GenerateInterface
{
    my $object = shift;
    my $interface = shift;

    %callbackFunctionTypeHash = map { $_->name => $_ } @{$idlDocument->callbackFunctions};
    %enumTypeHash = map { $_->name => $_->values } @{$idlDocument->enumerations};
    my $v8ClassName = GetV8ClassName($interface);
    my $defineName = $v8ClassName . "_h";
    my $internalNamespace = GetImplName($interface) . "V8Internal";

    my $conditionalString = GenerateConditionalString($interface);
    my $conditionalIf = "";
    my $conditionalEndif = "";
    if ($conditionalString) {
        $conditionalIf = "#if ${conditionalString}";
        $conditionalEndif = "#endif // ${conditionalString}";
    }

    $header{root} = new Block("ROOT", "", "");
    # FIXME: newlines should be generated by Block::toString().
    $header{conditional} = new Block("Conditional", "$conditionalIf", $conditionalEndif ? "$conditionalEndif\n" : "");
    $header{includes} = new Block("Includes", "", "");
    $header{nameSpaceWebCore} = new Block("Namespace WebCore", "\nnamespace WebCore {\n", "}\n");
    $header{class} = new Block("Class definition", "", "");
    $header{classPublic} = new Block("Class public:", "public:", "");
    $header{classPrivate} = new Block("Class private:", "private:", "");

    $header{root}->add($header{conditional});
    $header{conditional}->add($header{includes});
    $header{conditional}->add($header{nameSpaceWebCore});
    $header{nameSpaceWebCore}->add($header{class});
    $header{class}->add($header{classPublic});
    $header{class}->add($header{classPrivate});

    # - Add default header template
    $header{root}->addHeader($headerTemplate . "\n");
    $header{root}->addHeader("#ifndef $defineName\n#define $defineName\n");
    $header{root}->addFooter("#endif // $defineName");

    $implementation{root} = new Block("ROOT", "", "");
    $conditionalEndif = "\n$conditionalEndif" if !$interface->isCallback and $conditionalEndif;
    $implementation{conditional} = new Block("Conditional", $conditionalIf, $conditionalEndif);
    $implementation{includes} = new Block("Includes", "", "");

    # FIXME: newlines should be generated by Block::toString().
    my $nameSpaceWebCoreBegin = "namespace WebCore {\n";
    my $nameSpaceWebCoreEnd = "} // namespace WebCore";
    $nameSpaceWebCoreBegin = "$nameSpaceWebCoreBegin\n" if !$interface->isCallback;
    $nameSpaceWebCoreEnd = "\n$nameSpaceWebCoreEnd\n" if $interface->isCallback;
    $implementation{nameSpaceWebCore} = new Block("Namespace WebCore", $nameSpaceWebCoreBegin, $nameSpaceWebCoreEnd);
    $implementation{nameSpaceInternal} = new Block("Internal namespace", "namespace $internalNamespace {\n", "} // namespace $internalNamespace\n");

    $implementation{root}->add($implementation{conditional});
    $implementation{conditional}->add($implementation{includes});
    $implementation{conditional}->add($implementation{nameSpaceWebCore});
    if (!$interface->isCallback) {
        $implementation{nameSpaceWebCore}->add($implementation{nameSpaceInternal});
    }

    # - Add default header template
    $implementation{root}->addHeader($headerTemplate);
    $implementation{root}->addHeader("\n#include \"config.h\"");
    $implementation{includes}->add("#include \"${v8ClassName}.h\"\n\n");

    # Start actual generation
    if ($interface->isCallback) {
        $object->GenerateCallbackHeader($interface);
        $object->GenerateCallbackImplementation($interface);
    } else {
        $object->GenerateHeader($interface);
        $object->GenerateImplementation($interface);
    }
}

sub AddToImplIncludes
{
    my $header = shift;
    $implIncludes{$header} = 1;
}

sub AddToHeaderIncludes
{
    my @includes = @_;

    for my $include (@includes) {
        $headerIncludes{$include} = 1;
    }
}

sub SkipIncludeHeader
{
    my $type = shift;

    return 1 if IsPrimitiveType($type);
    return 1 if IsEnumType($type);
    return 1 if IsCallbackFunctionType($type);
    return 1 if $type eq "DOMString";

    # Special case: SVGPoint.h / SVGNumber.h do not exist.
    return 1 if $type eq "SVGPoint" or $type eq "SVGNumber";
    return 0;
}

sub AddIncludesForType
{
    my $type = shift;

    return if SkipIncludeHeader($type);

    # Default includes
    if ($type eq "EventListener") {
        AddToImplIncludes("core/dom/EventListener.h");
    } elsif ($type eq "SerializedScriptValue") {
        AddToImplIncludes("bindings/v8/SerializedScriptValue.h");
    } elsif ($type eq "any" || IsCallbackFunctionType($type)) {
        AddToImplIncludes("bindings/v8/ScriptValue.h");
    } else {
        AddToImplIncludes("V8${type}.h");
    }

    # Additional includes
    if ($type eq "CSSStyleSheet") {
        AddToImplIncludes("core/css/CSSImportRule.h");
    }
}

sub HeaderFilesForInterface
{
    my $interfaceName = shift;
    my $implClassName = shift;

    my @includes = ();
    if (IsTypedArrayType($interfaceName)) {
        push(@includes, "wtf/${interfaceName}.h");
    } elsif ($interfaceName =~ /SVGPathSeg/) {
        $interfaceName =~ s/Abs|Rel//;
        push(@includes, "core/svg/${interfaceName}.h");
    } elsif (!SkipIncludeHeader($interfaceName)) {
        my $idlFilename = IDLFileForInterface($interfaceName) or die("Could NOT find IDL file for interface \"$interfaceName\" $!\n");
        my $idlRelPath= "bindings/" . File::Spec->abs2rel($idlFilename, $sourceRoot);
        push(@includes, dirname($idlRelPath) . "/" . $implClassName . ".h");
    }
    return @includes;
}

sub NeedsOpaqueRootForGC
{
    my $interface = shift;
    return $interface->extendedAttributes->{"GenerateIsReachable"} || $interface->extendedAttributes->{"CustomIsReachable"};
}

sub GenerateOpaqueRootForGC
{
    my $interface = shift;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);

    if ($interface->extendedAttributes->{"CustomIsReachable"}) {
        return;
    }

    my $code = <<END;
void* ${v8ClassName}::opaqueRootForGC(void* object, v8::Isolate* isolate)
{
    ${implClassName}* impl = static_cast<${implClassName}*>(object);
END
    my $isReachableMethod = $interface->extendedAttributes->{"GenerateIsReachable"};
    if ($isReachableMethod) {
        AddToImplIncludes("bindings/v8/V8GCController.h");
        AddToImplIncludes("core/dom/Element.h");
        $code .= <<END;
    if (Node* owner = impl->${isReachableMethod}())
        return V8GCController::opaqueRootForGC(owner, isolate);
END
    }

    $code .= <<END;
    return object;
}

END
    $implementation{nameSpaceWebCore}->add($code);
}

sub GetSVGPropertyTypes
{
    my $implType = shift;

    my $svgPropertyType;
    my $svgListPropertyType;
    my $svgNativeType;

    return ($svgPropertyType, $svgListPropertyType, $svgNativeType) if not $implType =~ /SVG/;

    $svgNativeType = GetSVGTypeNeedingTearOff($implType);
    return ($svgPropertyType, $svgListPropertyType, $svgNativeType) if not $svgNativeType;

    # Append space to avoid compilation errors when using  PassRefPtr<$svgNativeType>
    $svgNativeType = "$svgNativeType ";

    my $svgWrappedNativeType = GetSVGWrappedTypeNeedingTearOff($implType);
    if ($svgNativeType =~ /SVGPropertyTearOff/) {
        $svgPropertyType = $svgWrappedNativeType;
        AddToImplIncludes("core/svg/properties/SVGAnimatedPropertyTearOff.h");
    } elsif ($svgNativeType =~ /SVGListPropertyTearOff/ or $svgNativeType =~ /SVGStaticListPropertyTearOff/ or $svgNativeType =~ /SVGTransformListPropertyTearOff/) {
        $svgListPropertyType = $svgWrappedNativeType;
        AddToHeaderIncludes("core/svg/properties/SVGAnimatedListPropertyTearOff.h");
    } elsif ($svgNativeType =~ /SVGPathSegListPropertyTearOff/) {
        $svgListPropertyType = $svgWrappedNativeType;
        AddToHeaderIncludes("core/svg/properties/SVGPathSegListPropertyTearOff.h");
    }

    if ($svgPropertyType) {
        $svgPropertyType = "SVGPoint" if $svgPropertyType eq "FloatPoint";
    }

    return ($svgPropertyType, $svgListPropertyType, $svgNativeType);
}

sub GetIndexedGetterFunction
{
    my $interface = shift;

    # FIXME: Expose indexed getter of CSSMixFunctionValue by removing this special case
    # because CSSValueList(which is parent of CSSMixFunctionValue) has indexed property getter.
    if ($interface->name eq "CSSMixFunctionValue") {
        return 0;
    }

    return GetSpecialAccessorFunctionForType($interface, "getter", "unsigned long", 1);
}

sub GetIndexedSetterFunction
{
    my $interface = shift;

    return GetSpecialAccessorFunctionForType($interface, "setter", "unsigned long", 2);
}

sub GetIndexedDeleterFunction
{
    my $interface = shift;

    return GetSpecialAccessorFunctionForType($interface, "deleter", "unsigned long", 1);
}

sub GetNamedGetterFunction
{
    my $interface = shift;
    return GetSpecialAccessorFunctionForType($interface, "getter", "DOMString", 1);
}

sub GetNamedSetterFunction
{
    my $interface = shift;
    return GetSpecialAccessorFunctionForType($interface, "setter", "DOMString", 2);
}

sub GetNamedDeleterFunction
{
    my $interface = shift;
    return GetSpecialAccessorFunctionForType($interface, "deleter", "DOMString", 1);
}

sub GetSpecialAccessorFunctionForType
{
    my $interface = shift;
    my $special = shift;
    my $firstParameterType = shift;
    my $numberOfParameters = shift;

    foreach my $function (@{$interface->functions}) {
        my $specials = $function->specials;
        my $specialExists = grep { $_ eq $special } @$specials;
        my $parameters = $function->parameters;
        if ($specialExists and scalar(@$parameters) == $numberOfParameters and $parameters->[0]->type eq $firstParameterType) {
            return $function;
        }
    }

    return 0;
}

sub GenerateHeader
{
    my $object = shift;
    my $interface = shift;

    my $interfaceName = $interface->name;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);

    # Copy contents of parent interfaces except the first parent.
    my @parents;
    AddMethodsConstantsAndAttributesFromParentInterfaces($interface, \@parents);
    LinkOverloadedFunctions($interface);

    # Ensure the IsDOMNodeType function is in sync.
    die("IsDOMNodeType is out of date with respect to $interfaceName") if IsDOMNodeType($interfaceName) != InheritsInterface($interface, "Node");

    my ($svgPropertyType, $svgListPropertyType, $svgNativeType) = GetSVGPropertyTypes($interfaceName);

    if ($v8ClassName !~ /SVG/) {
        for my $parent (@{$interface->parents}) {
            AddToHeaderIncludes("V8${parent}.h");
        }
    }
    AddToHeaderIncludes("bindings/v8/WrapperTypeInfo.h");
    AddToHeaderIncludes("bindings/v8/V8Binding.h");
    AddToHeaderIncludes("bindings/v8/V8DOMWrapper.h");
    AddToHeaderIncludes(HeaderFilesForInterface($interfaceName, $implClassName));
    foreach my $headerInclude (sort keys(%headerIncludes)) {
        $header{includes}->add("#include \"${headerInclude}\"\n");
    }

    $header{nameSpaceWebCore}->addHeader("\ntemplate<typename PropertyType> class SVGPropertyTearOff;\n") if $svgPropertyType;
    if ($svgNativeType) {
        if ($svgNativeType =~ /SVGStaticListPropertyTearOff/) {
            $header{nameSpaceWebCore}->addHeader("\ntemplate<typename PropertyType> class SVGStaticListPropertyTearOff;\n");
        } else {
            $header{nameSpaceWebCore}->addHeader("\ntemplate<typename PropertyType> class SVGListPropertyTearOff;\n");
        }
    }

    $header{nameSpaceWebCore}->addHeader("class FloatRect;\n") if $svgPropertyType && $svgPropertyType eq "FloatRect";
    $header{nameSpaceWebCore}->addHeader("\nclass Dictionary;") if IsConstructorTemplate($interface, "Event");

    my $nativeType = GetNativeTypeForConversions($interface);
    if ($interface->extendedAttributes->{"NamedConstructor"}) {
        $header{nameSpaceWebCore}->addHeader(<<END);

class V8${nativeType}Constructor {
public:
    static v8::Handle<v8::FunctionTemplate> GetTemplate(v8::Isolate*, WrapperWorldType);
    static WrapperTypeInfo info;
};
END
    }

    $header{class}->addHeader("class $v8ClassName {");
    $header{class}->addFooter("};");

    $header{classPublic}->add(<<END);
    static bool HasInstance(v8::Handle<v8::Value>, v8::Isolate*, WrapperWorldType);
    static bool HasInstanceInAnyWorld(v8::Handle<v8::Value>, v8::Isolate*);
    static v8::Handle<v8::FunctionTemplate> GetTemplate(v8::Isolate*, WrapperWorldType);
    static ${nativeType}* toNative(v8::Handle<v8::Object> object)
    {
        return reinterpret_cast<${nativeType}*>(object->GetAlignedPointerFromInternalField(v8DOMWrapperObjectIndex));
    }
    static void derefObject(void*);
    static WrapperTypeInfo info;
END

    if (NeedsOpaqueRootForGC($interface)) {
        $header{classPublic}->add("    static void* opaqueRootForGC(void*, v8::Isolate*);\n");
    }

    if (InheritsExtendedAttribute($interface, "ActiveDOMObject")) {
        $header{classPublic}->add("    static ActiveDOMObject* toActiveDOMObject(v8::Handle<v8::Object>);\n");
    }

    if (InheritsExtendedAttribute($interface, "EventTarget")) {
        $header{classPublic}->add("    static EventTarget* toEventTarget(v8::Handle<v8::Object>);\n");
    }

    if ($interfaceName eq "Window") {
        $header{classPublic}->add(<<END);
    static v8::Handle<v8::ObjectTemplate> GetShadowObjectTemplate(v8::Isolate*, WrapperWorldType);
END
    }

    my @enabledPerContextFunctions;
    foreach my $function (@{$interface->functions}) {
        my $name = $function->name;
        next if $name eq "";
        my $attrExt = $function->extendedAttributes;

        if (HasCustomMethod($attrExt) && !$attrExt->{"ImplementedBy"} && $function->{overloadIndex} == 1) {
            my $conditionalString = GenerateConditionalString($function);
            $header{classPublic}->add("#if ${conditionalString}\n") if $conditionalString;
            $header{classPublic}->add(<<END);
    static void ${name}MethodCustom(const v8::FunctionCallbackInfo<v8::Value>&);
END
            $header{classPublic}->add("#endif // ${conditionalString}\n") if $conditionalString;
        }
        if ($attrExt->{"EnabledPerContext"}) {
            push(@enabledPerContextFunctions, $function);
        }
    }

    if (IsConstructable($interface)) {
        $header{classPublic}->add("    static void constructorCallback(const v8::FunctionCallbackInfo<v8::Value>&);\n");
END
    }
    if (HasCustomConstructor($interface)) {
        $header{classPublic}->add("    static void constructorCustom(const v8::FunctionCallbackInfo<v8::Value>&);\n");
    }

    my @enabledPerContextAttributes;
    foreach my $attribute (@{$interface->attributes}) {
        my $name = $attribute->name;
        my $attrExt = $attribute->extendedAttributes;
        my $conditionalString = GenerateConditionalString($attribute);
        if (HasCustomGetter($attrExt) && !$attrExt->{"ImplementedBy"}) {
            $header{classPublic}->add("#if ${conditionalString}\n") if $conditionalString;
            $header{classPublic}->add(<<END);
    static void ${name}AttrGetterCustom(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>&);
END
            $header{classPublic}->add("#endif // ${conditionalString}\n") if $conditionalString;
        }
        if (HasCustomSetter($attrExt) && !$attrExt->{"ImplementedBy"}) {
            $header{classPublic}->add("#if ${conditionalString}\n") if $conditionalString;
            $header{classPublic}->add(<<END);
    static void ${name}AttrSetterCustom(v8::Local<v8::String> name, v8::Local<v8::Value>, const v8::PropertyCallbackInfo<void>&);
END
            $header{classPublic}->add("#endif // ${conditionalString}\n") if $conditionalString;
        }
        if ($attrExt->{"EnabledPerContext"}) {
            push(@enabledPerContextAttributes, $attribute);
        }
    }

    GenerateHeaderNamedAndIndexedPropertyAccessors($interface);
    GenerateHeaderLegacyCall($interface);
    GenerateHeaderCustomInternalFieldIndices($interface);

    if ($interface->name eq "Window") {
        $header{classPublic}->add(<<END);
    static bool namedSecurityCheckCustom(v8::Local<v8::Object> host, v8::Local<v8::Value> key, v8::AccessType, v8::Local<v8::Value> data);
    static bool indexedSecurityCheckCustom(v8::Local<v8::Object> host, uint32_t index, v8::AccessType, v8::Local<v8::Value> data);
END
    }

    if (@enabledPerContextAttributes) {
        $header{classPublic}->add(<<END);
    static void installPerContextProperties(v8::Handle<v8::Object>, ${nativeType}*, v8::Isolate*);
END
    } else {
        $header{classPublic}->add(<<END);
    static void installPerContextProperties(v8::Handle<v8::Object>, ${nativeType}*, v8::Isolate*) { }
END
    }

    if (@enabledPerContextFunctions) {
        $header{classPublic}->add(<<END);
    static void installPerContextPrototypeProperties(v8::Handle<v8::Object>, v8::Isolate*);
END
    } else {
        $header{classPublic}->add(<<END);
    static void installPerContextPrototypeProperties(v8::Handle<v8::Object>, v8::Isolate*) { }
END
    }

    if ($interfaceName eq "HTMLElement") {
        $header{classPublic}->add(<<END);
    friend v8::Handle<v8::Object> createV8HTMLWrapper(HTMLElement*, v8::Handle<v8::Object> creationContext, v8::Isolate*);
    friend v8::Handle<v8::Object> createV8HTMLDirectWrapper(HTMLElement*, v8::Handle<v8::Object> creationContext, v8::Isolate*);
END
    } elsif ($interfaceName eq "SVGElement") {
        $header{classPublic}->add(<<END);
    friend v8::Handle<v8::Object> createV8SVGWrapper(SVGElement*, v8::Handle<v8::Object> creationContext, v8::Isolate*);
    friend v8::Handle<v8::Object> createV8SVGDirectWrapper(SVGElement*, v8::Handle<v8::Object> creationContext, v8::Isolate*);
    friend v8::Handle<v8::Object> createV8SVGFallbackWrapper(SVGElement*, v8::Handle<v8::Object> creationContext, v8::Isolate*);
END
    } elsif ($interfaceName eq "HTMLUnknownElement") {
        $header{classPublic}->add(<<END);
    friend v8::Handle<v8::Object> createV8HTMLFallbackWrapper(HTMLUnknownElement*, v8::Handle<v8::Object> creationContext, v8::Isolate*);
END
    } elsif ($interfaceName eq "Element") {
        $header{classPublic}->add(<<END);
    // This is a performance optimization hack. See V8Element::wrap.
    friend v8::Handle<v8::Object> wrap(Node*, v8::Handle<v8::Object> creationContext, v8::Isolate*);
END
    }

    my $noToV8 = $interface->extendedAttributes->{"DoNotGenerateToV8"};
    my $noWrap = $interface->extendedAttributes->{"DoNotGenerateWrap"} || $noToV8;
    if (!$noWrap) {
        my $createWrapperArgumentType = GetPassRefPtrType($nativeType);
        $header{classPrivate}->add(<<END);
    friend v8::Handle<v8::Object> wrap(${nativeType}*, v8::Handle<v8::Object> creationContext, v8::Isolate*);
    static v8::Handle<v8::Object> createWrapper(${createWrapperArgumentType}, v8::Handle<v8::Object> creationContext, v8::Isolate*);
END
    }

    $header{nameSpaceWebCore}->add(<<END);

template<>
class WrapperTypeTraits<${nativeType} > {
public:
    static WrapperTypeInfo* info() { return &${v8ClassName}::info; }
};

END

    my $customWrap = $interface->extendedAttributes->{"CustomToV8"};
    if ($noToV8) {
        die "Can't suppress toV8 for subclass\n" if @parents;
    } elsif ($noWrap) {
        die "Must have custom toV8\n" if !$customWrap;
        $header{nameSpaceWebCore}->add(<<END);
class ${nativeType};
v8::Handle<v8::Value> toV8(${nativeType}*, v8::Handle<v8::Object> creationContext, v8::Isolate*);
v8::Handle<v8::Value> toV8ForMainWorld(${nativeType}*, v8::Handle<v8::Object> creationContext, v8::Isolate*);

template<class HolderContainer, class Wrappable>
inline v8::Handle<v8::Value> toV8Fast(${nativeType}* impl, const HolderContainer& container, Wrappable*)
{
    return toV8(impl, container.Holder(), container.GetIsolate());
}

template<class HolderContainer, class Wrappable>
inline v8::Handle<v8::Value> toV8FastForMainWorld(${nativeType}* impl, const HolderContainer& container, Wrappable*)
{
    return toV8ForMainWorld(impl, container.Holder(), container.GetIsolate());
}
END
    } else {

        my $createWrapperCall = $customWrap ? "${v8ClassName}::wrap" : "${v8ClassName}::createWrapper";

        if ($customWrap) {
            $header{nameSpaceWebCore}->add(<<END);

v8::Handle<v8::Object> wrap(${nativeType}* impl, v8::Handle<v8::Object> creationContext, v8::Isolate*);
END
        } else {
            $header{nameSpaceWebCore}->add(<<END);

inline v8::Handle<v8::Object> wrap(${nativeType}* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(DOMDataStore::getWrapper(impl, isolate).IsEmpty());
    if (ScriptWrappable::wrapperCanBeStoredInObject(impl)) {
        const WrapperTypeInfo* actualInfo = ScriptWrappable::getTypeInfoFromObject(impl);
        // Might be a XXXConstructor::info instead of an XXX::info. These will both have
        // the same object de-ref functions, though, so use that as the basis of the check.
        RELEASE_ASSERT_WITH_SECURITY_IMPLICATION(actualInfo->derefObjectFunction == ${v8ClassName}::info.derefObjectFunction);
    }
    return $createWrapperCall(impl, creationContext, isolate);
}
END
        }

        $header{nameSpaceWebCore}->add(<<END);

inline v8::Handle<v8::Value> toV8(${nativeType}* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    if (UNLIKELY(!impl))
        return v8NullWithCheck(isolate);
    v8::Handle<v8::Value> wrapper = DOMDataStore::getWrapper(impl, isolate);
    if (!wrapper.IsEmpty())
        return wrapper;
    return wrap(impl, creationContext, isolate);
}

inline v8::Handle<v8::Value> toV8ForMainWorld(${nativeType}* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(worldType(isolate) == MainWorld);
    if (UNLIKELY(!impl))
        return v8NullWithCheck(isolate);
    v8::Handle<v8::Value> wrapper = DOMDataStore::getWrapperForMainWorld(impl);
    if (!wrapper.IsEmpty())
        return wrapper;
    return wrap(impl, creationContext, isolate);
}

template<class HolderContainer, class Wrappable>
inline v8::Handle<v8::Value> toV8Fast(${nativeType}* impl, const HolderContainer& container, Wrappable* wrappable)
{
    if (UNLIKELY(!impl))
        return v8Null(container.GetIsolate());
    v8::Handle<v8::Object> wrapper = DOMDataStore::getWrapperFast(impl, container, wrappable);
    if (!wrapper.IsEmpty())
        return wrapper;
    return wrap(impl, container.Holder(), container.GetIsolate());
}

template<class HolderContainer, class Wrappable>
inline v8::Handle<v8::Value> toV8FastForMainWorld(${nativeType}* impl, const HolderContainer& container, Wrappable* wrappable)
{
    ASSERT(worldType(container.GetIsolate()) == MainWorld);
    if (UNLIKELY(!impl))
        return v8Null(container.GetIsolate());
    v8::Handle<v8::Object> wrapper = DOMDataStore::getWrapperForMainWorld(impl);
    if (!wrapper.IsEmpty())
        return wrapper;
    return wrap(impl, container.Holder(), container.GetIsolate());
}

template<class HolderContainer, class Wrappable>
inline v8::Handle<v8::Value> toV8FastForMainWorld(PassRefPtr< ${nativeType} > impl, const HolderContainer& container, Wrappable* wrappable)
{
    return toV8FastForMainWorld(impl.get(), container, wrappable);
}

END
    }

    $header{nameSpaceWebCore}->add(<<END);

template<class HolderContainer, class Wrappable>
inline v8::Handle<v8::Value> toV8Fast(PassRefPtr< ${nativeType} > impl, const HolderContainer& container, Wrappable* wrappable)
{
    return toV8Fast(impl.get(), container, wrappable);
}

inline v8::Handle<v8::Value> toV8(PassRefPtr< ${nativeType} > impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    return toV8(impl.get(), creationContext, isolate);
}

END

    if (IsConstructorTemplate($interface, "Event")) {
        $header{nameSpaceWebCore}->add("bool fill${implClassName}Init(${implClassName}Init&, const Dictionary&);\n\n");
    }
}

sub GetInternalFields
{
    my $interface = shift;

    my @customInternalFields = ();
    # Event listeners on DOM nodes are explicitly supported in the GC controller.
    if (!InheritsInterface($interface, "Node") &&
        InheritsExtendedAttribute($interface, "EventTarget")) {
        push(@customInternalFields, "eventListenerCacheIndex");
    }
    return @customInternalFields;
}

sub GenerateHeaderCustomInternalFieldIndices
{
    my $interface = shift;
    my @customInternalFields = GetInternalFields($interface);
    my $customFieldCounter = 0;
    foreach my $customInternalField (@customInternalFields) {
        $header{classPublic}->add(<<END);
    static const int ${customInternalField} = v8DefaultWrapperInternalFieldCount + ${customFieldCounter};
END
        $customFieldCounter++;
    }
    $header{classPublic}->add(<<END);
    static const int internalFieldCount = v8DefaultWrapperInternalFieldCount + ${customFieldCounter};
END
}

sub GenerateHeaderNamedAndIndexedPropertyAccessors
{
    my $interface = shift;

    my $indexedGetterFunction = GetIndexedGetterFunction($interface);
    my $hasCustomIndexedGetter = $indexedGetterFunction ? $indexedGetterFunction->extendedAttributes->{"Custom"} : 0;

    my $indexedSetterFunction = GetIndexedSetterFunction($interface);
    my $hasCustomIndexedSetter = $indexedSetterFunction ? $indexedSetterFunction->extendedAttributes->{"Custom"} : 0;

    my $indexedDeleterFunction = GetIndexedDeleterFunction($interface);
    my $hasCustomIndexedDeleters = $indexedDeleterFunction ? $indexedDeleterFunction->extendedAttributes->{"Custom"} : 0;

    my $namedGetterFunction = GetNamedGetterFunction($interface);
    my $hasCustomNamedGetter = $namedGetterFunction ? $namedGetterFunction->extendedAttributes->{"Custom"} : 0;

    my $namedSetterFunction = GetNamedSetterFunction($interface);
    my $hasCustomNamedSetter = $namedSetterFunction ? $namedSetterFunction->extendedAttributes->{"Custom"} : 0;

    my $namedDeleterFunction = GetNamedDeleterFunction($interface);
    my $hasCustomNamedDeleter = $namedDeleterFunction ? $namedDeleterFunction->extendedAttributes->{"Custom"} : 0;

    my $namedEnumeratorFunction = $namedGetterFunction;
    $namedEnumeratorFunction = 0 if $namedGetterFunction && $namedGetterFunction->extendedAttributes->{"NotEnumerable"};
    my $hasCustomNamedEnumerator = 1 if $namedGetterFunction && $namedGetterFunction->extendedAttributes->{"CustomEnumerateProperty"};

    if ($indexedGetterFunction) {
        $header{classPublic}->add(<<END);
    static void indexedPropertyGetter(uint32_t, const v8::PropertyCallbackInfo<v8::Value>&);
END
    }

    if ($indexedSetterFunction) {
        $header{classPublic}->add(<<END);
    static void indexedPropertySetter(uint32_t, v8::Local<v8::Value>, const v8::PropertyCallbackInfo<v8::Value>&);
END
    }
    if ($indexedDeleterFunction) {
        $header{classPublic}->add(<<END);
    static void indexedPropertyDeleter(uint32_t, const v8::PropertyCallbackInfo<v8::Boolean>&);
END
    }
    if ($namedGetterFunction) {
        $header{classPublic}->add(<<END);
    static void namedPropertyGetter(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);
END
    }
    if ($namedSetterFunction) {
        $header{classPublic}->add(<<END);
    static void namedPropertySetter(v8::Local<v8::String>, v8::Local<v8::Value>, const v8::PropertyCallbackInfo<v8::Value>&);
END
    }
    if ($namedDeleterFunction) {
        $header{classPublic}->add(<<END);
    static void namedPropertyDeleter(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Boolean>&);
END
    }
    if ($namedEnumeratorFunction) {
        $header{classPublic}->add(<<END);
    static void namedPropertyEnumerator(const v8::PropertyCallbackInfo<v8::Array>&);
    static void namedPropertyQuery(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Integer>&);
END
    }
}

sub GenerateHeaderLegacyCall
{
    my $interface = shift;

    if ($interface->extendedAttributes->{"CustomLegacyCall"}) {
        $header{classPublic}->add("    static void legacyCallCustom(const v8::FunctionCallbackInfo<v8::Value>&);\n");
    }
}

sub HasActivityLogging
{
    my $forMainWorldSuffix = shift;
    my $attrExt = shift;
    my $access = shift;

    if (!$attrExt->{"ActivityLog"}) {
        return 0;
    }
    my $logAllAccess = ($attrExt->{"ActivityLog"} =~ /^Access/);
    my $logGetter = ($attrExt->{"ActivityLog"} =~ /^Getter/);
    my $logSetter = ($attrExt->{"ActivityLog"} =~ /^Setter/);
    my $logOnlyIsolatedWorlds = ($attrExt->{"ActivityLog"} =~ /ForIsolatedWorlds$/);

    if ($logOnlyIsolatedWorlds && $forMainWorldSuffix eq "ForMainWorld") {
        return 0;
    }
    return $logAllAccess || ($logGetter && $access eq "Getter") || ($logSetter && $access eq "Setter");
}

sub IsConstructable
{
    my $interface = shift;

    return $interface->extendedAttributes->{"CustomConstructor"} || $interface->extendedAttributes->{"Constructor"} || $interface->extendedAttributes->{"ConstructorTemplate"};
}

sub HasCustomConstructor
{
    my $interface = shift;

    return $interface->extendedAttributes->{"CustomConstructor"};
}

sub HasCustomGetter
{
    my $attrExt = shift;
    return $attrExt->{"Custom"} || $attrExt->{"CustomGetter"};
}

sub HasCustomSetter
{
    my $attrExt = shift;
    return $attrExt->{"Custom"} || $attrExt->{"CustomSetter"};
}

sub HasCustomMethod
{
    my $attrExt = shift;
    return $attrExt->{"Custom"};
}

sub IsReadonly
{
    my $attribute = shift;
    my $attrExt = $attribute->extendedAttributes;
    return $attribute->isReadOnly && !$attrExt->{"Replaceable"};
}

sub GetV8ClassName
{
    my $interface = shift;
    return "V8" . $interface->name;
}

sub GetImplName
{
    my $interfaceOrAttributeOrFunction = shift;
    return $interfaceOrAttributeOrFunction->extendedAttributes->{"ImplementedAs"} || $interfaceOrAttributeOrFunction->name;
}

sub GetImplNameFromImplementedBy
{
    my $implementedBy = shift;

    my $interface = ParseInterface($implementedBy);
    return $interface->extendedAttributes->{"ImplementedAs"} || $implementedBy;
}

sub GenerateDomainSafeFunctionGetter
{
    my $function = shift;
    my $interface = shift;

    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $funcName = $function->name;

    my $functionLength = GetFunctionLength($function);
    my $signature = "v8::Signature::New(V8PerIsolateData::from(info.GetIsolate())->rawTemplate(&" . $v8ClassName . "::info, currentWorldType))";
    if ($function->extendedAttributes->{"DoNotCheckSignature"}) {
        $signature = "v8::Local<v8::Signature>()";
    }

    my $newTemplateParams = "${implClassName}V8Internal::${funcName}MethodCallback, v8Undefined(), $signature";

    AddToImplIncludes("core/page/Frame.h");
    AddToImplIncludes("bindings/v8/BindingSecurity.h");
    $implementation{nameSpaceInternal}->add(<<END);
static void ${funcName}AttrGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    // This is only for getting a unique pointer which we can pass to privateTemplate.
    static const char* privateTemplateUniqueKey = "${funcName}PrivateTemplate";
    WrapperWorldType currentWorldType = worldType(info.GetIsolate());
    V8PerIsolateData* data = V8PerIsolateData::from(info.GetIsolate());
    v8::Handle<v8::FunctionTemplate> privateTemplate = data->privateTemplate(currentWorldType, &privateTemplateUniqueKey, $newTemplateParams, $functionLength);

    v8::Handle<v8::Object> holder = info.This()->FindInstanceInPrototypeChain(${v8ClassName}::GetTemplate(info.GetIsolate(), currentWorldType));
    if (holder.IsEmpty()) {
        // can only reach here by 'object.__proto__.func', and it should passed
        // domain security check already
        v8SetReturnValue(info, privateTemplate->GetFunction());
        return;
    }
    ${implClassName}* imp = ${v8ClassName}::toNative(holder);
    if (!BindingSecurity::shouldAllowAccessToFrame(imp->frame(), DoNotReportSecurityError)) {
        static const char* sharedTemplateUniqueKey = "${funcName}SharedTemplate";
        v8::Handle<v8::FunctionTemplate> sharedTemplate = data->privateTemplate(currentWorldType, &sharedTemplateUniqueKey, $newTemplateParams, $functionLength);
        v8SetReturnValue(info, sharedTemplate->GetFunction());
        return;
    }

    v8::Local<v8::Value> hiddenValue = info.This()->GetHiddenValue(name);
    if (!hiddenValue.IsEmpty()) {
        v8SetReturnValue(info, hiddenValue);
        return;
    }

    v8SetReturnValue(info, privateTemplate->GetFunction());
}

END
    $implementation{nameSpaceInternal}->add(<<END);
static void ${funcName}AttrGetterCallback(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    ${implClassName}V8Internal::${funcName}AttrGetter(name, info);
}

END
}

sub GenerateDomainSafeFunctionSetter
{
    my $interface = shift;

    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);

    AddToImplIncludes("bindings/v8/BindingSecurity.h");
    $implementation{nameSpaceInternal}->add(<<END);
static void ${implClassName}DomainSafeFunctionSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.This()->FindInstanceInPrototypeChain(${v8ClassName}::GetTemplate(info.GetIsolate(), worldType(info.GetIsolate())));
    if (holder.IsEmpty())
        return;
    ${implClassName}* imp = ${v8ClassName}::toNative(holder);
    if (!BindingSecurity::shouldAllowAccessToFrame(imp->frame()))
        return;

    info.This()->SetHiddenValue(name, value);
}

END
}

sub GenerateConstructorGetter
{
    my $interface = shift;
    my $implClassName = GetImplName($interface);

    $implementation{nameSpaceInternal}->add(<<END);
static void ${implClassName}ConstructorGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Value> data = info.Data();
    ASSERT(data->IsExternal());
    V8PerContextData* perContextData = V8PerContextData::from(info.Holder()->CreationContext());
    if (!perContextData)
        return;
    v8SetReturnValue(info, perContextData->constructorForType(WrapperTypeInfo::unwrap(data)));
}
END
}

sub GenerateFeatureObservation
{
    my $measureAs = shift;

    if ($measureAs) {
        AddToImplIncludes("core/page/UseCounter.h");
        return "    UseCounter::count(activeDOMWindow(), UseCounter::${measureAs});\n";
    }

    return "";
}

sub GenerateDeprecationNotification
{
    my $deprecateAs = shift;
    if ($deprecateAs) {
        AddToImplIncludes("core/page/PageConsole.h");
        AddToImplIncludes("core/page/UseCounter.h");
        return "    UseCounter::countDeprecation(activeDOMWindow(), UseCounter::${deprecateAs});\n";
    }
    return "";
}

sub GenerateActivityLogging
{
    my $accessType = shift;
    my $interface = shift;
    my $propertyName = shift;

    my $interfaceName = $interface->name;

    AddToImplIncludes("bindings/v8/V8Binding.h");
    AddToImplIncludes("bindings/v8/V8DOMActivityLogger.h");
    AddToImplIncludes("wtf/Vector.h");

    my $code = "";
    if ($accessType eq "Method") {
        $code .= <<END;
    V8PerContextData* contextData = V8PerContextData::from(args.GetIsolate()->GetCurrentContext());
    if (contextData && contextData->activityLogger()) {
        Vector<v8::Handle<v8::Value> > loggerArgs = toVectorOfArguments(args);
        contextData->activityLogger()->log("${interfaceName}.${propertyName}", args.Length(), loggerArgs.data(), "${accessType}");
    }
END
    } elsif ($accessType eq "Setter") {
        $code .= <<END;
    V8PerContextData* contextData = V8PerContextData::from(info.GetIsolate()->GetCurrentContext());
    if (contextData && contextData->activityLogger()) {
        v8::Handle<v8::Value> loggerArg[] = { value };
        contextData->activityLogger()->log("${interfaceName}.${propertyName}", 1, &loggerArg[0], "${accessType}");
    }
END
    } elsif ($accessType eq "Getter") {
        $code .= <<END;
    V8PerContextData* contextData = V8PerContextData::from(info.GetIsolate()->GetCurrentContext());
    if (contextData && contextData->activityLogger())
        contextData->activityLogger()->log("${interfaceName}.${propertyName}", 0, 0, "${accessType}");
END
    } else {
        die "Unrecognized activity logging access type";
    }

    return $code;
}

sub GenerateNormalAttrGetterCallback
{
    my $attribute = shift;
    my $interface = shift;
    my $forMainWorldSuffix = shift;

    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $attrExt = $attribute->extendedAttributes;
    my $attrName = $attribute->name;

    my $conditionalString = GenerateConditionalString($attribute);
    my $code = "";
    $code .= "#if ${conditionalString}\n\n" if $conditionalString;

    $code .= "static void ${attrName}AttrGetterCallback${forMainWorldSuffix}(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)\n";
    $code .= "{\n";
    $code .= GenerateFeatureObservation($attrExt->{"MeasureAs"});
    $code .= GenerateDeprecationNotification($attrExt->{"DeprecateAs"});
    if (HasActivityLogging($forMainWorldSuffix, $attrExt, "Getter")) {
        $code .= GenerateActivityLogging("Getter", $interface, "${attrName}");
    }
    if (HasCustomGetter($attrExt)) {
        $code .= "    ${v8ClassName}::${attrName}AttrGetterCustom(name, info);\n";
    } else {
        $code .= "    ${implClassName}V8Internal::${attrName}AttrGetter${forMainWorldSuffix}(name, info);\n";
    }
    $code .= "}\n\n";
    $code .= "#endif // ${conditionalString}\n\n" if $conditionalString;

    $implementation{nameSpaceInternal}->add($code);
}

sub GenerateNormalAttrGetter
{
    my $attribute = shift;
    my $interface = shift;
    my $forMainWorldSuffix = shift;

    my $interfaceName = $interface->name;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $attrExt = $attribute->extendedAttributes;
    my $attrName = $attribute->name;
    my $attrType = $attribute->type;

    if (HasCustomGetter($attrExt)) {
        return;
    }

    AssertNotSequenceType($attrType);
    my $getterStringUsesImp = $interfaceName ne "SVGNumber";
    my $nativeType = GetNativeTypeFromAttributeOrParameter($attribute, -1);
    my $svgNativeType = GetSVGTypeNeedingTearOff($interfaceName);

    my $conditionalString = GenerateConditionalString($attribute);
    my $code = "";
    $code .= "#if ${conditionalString}\n\n" if $conditionalString;
    $code .= <<END;
static void ${attrName}AttrGetter${forMainWorldSuffix}(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
END
    if ($svgNativeType) {
        my $svgWrappedNativeType = GetSVGWrappedTypeNeedingTearOff($interfaceName);
        if ($svgWrappedNativeType =~ /List/) {
            $code .= <<END;
    $svgNativeType* imp = ${v8ClassName}::toNative(info.Holder());
END
        } else {
            $code .= <<END;
    $svgNativeType* wrapper = ${v8ClassName}::toNative(info.Holder());
    $svgWrappedNativeType& impInstance = wrapper->propertyReference();
END
            if ($getterStringUsesImp) {
                $code .= <<END;
    $svgWrappedNativeType* imp = &impInstance;
END
            }
        }
    } elsif ($attrExt->{"OnProto"} || $attrExt->{"Unforgeable"}) {
        if ($interfaceName eq "Window") {
            $code .= <<END;
    v8::Handle<v8::Object> holder = info.Holder();
END
        } else {
            # perform lookup first
            $code .= <<END;
    v8::Handle<v8::Object> holder = info.This()->FindInstanceInPrototypeChain(${v8ClassName}::GetTemplate(info.GetIsolate(), worldType(info.GetIsolate())));
    if (holder.IsEmpty())
        return;
END
        }
        $code .= <<END;
    ${implClassName}* imp = ${v8ClassName}::toNative(holder);
END
    } else {
        my $reflect = $attribute->extendedAttributes->{"Reflect"};
        my $url = $attribute->extendedAttributes->{"URL"};
        if ($getterStringUsesImp && $reflect && !$url && InheritsInterface($interface, "Node") && $attrType eq "DOMString") {
            # Generate super-compact call for regular attribute getter:
            my ($functionName, @arguments) = GetterExpression($interfaceName, $attribute);
            $code .= "    Element* imp = V8Element::toNative(info.Holder());\n";
            $code .= "    v8SetReturnValue(info, v8String(imp->${functionName}(" . join(", ", @arguments) . "), info.GetIsolate(), ReturnUnsafeHandle));\n";
            $code .= "    return;\n";
            $code .= "}\n\n";
            $code .= "#endif // ${conditionalString}\n\n" if $conditionalString;
            $implementation{nameSpaceInternal}->add($code);
            return;
            # Skip the rest of the function!
        }
        if ($attribute->type eq "SerializedScriptValue" && $attrExt->{"CachedAttribute"}) {
            $code .= <<END;
    v8::Handle<v8::String> propertyName = v8::String::NewSymbol("${attrName}");
    v8::Handle<v8::Value> value = info.Holder()->GetHiddenValue(propertyName);
    if (!value.IsEmpty()) {
        v8SetReturnValue(info, value);
        return;
    }
END
        }
        if (!$attribute->isStatic) {
            $code .= <<END;
    ${implClassName}* imp = ${v8ClassName}::toNative(info.Holder());
END
        }
    }

    # Generate security checks if necessary
    if ($attribute->extendedAttributes->{"CheckSecurityForNode"}) {
        AddToImplIncludes("bindings/v8/BindingSecurity.h");
        $code .= "    if (!BindingSecurity::shouldAllowAccessToNode(imp->" . GetImplName($attribute) . "())) {\n";
        $code .= "        v8SetReturnValueNull(info);\n";
        $code .= "        return;\n";
        $code .= "    }\n";
    }

    my $useExceptions = 1 if $attribute->extendedAttributes->{"GetterRaisesException"} ||  $attribute->extendedAttributes->{"RaisesException"};
    my $isNullable = $attribute->isNullable;
    if ($useExceptions) {
        AddToImplIncludes("core/dom/ExceptionCode.h");
        $code .= "    ExceptionCode ec = 0;\n";
    }

    if ($isNullable) {
        $code .= "    bool isNull = false;\n";
    }

    my $returnType = $attribute->type;
    my $getterString;

    if ($getterStringUsesImp) {
        my ($functionName, @arguments) = GetterExpression($interfaceName, $attribute);
        push(@arguments, "isNull") if $isNullable;
        push(@arguments, "ec") if $useExceptions;
        if ($attribute->extendedAttributes->{"ImplementedBy"}) {
            my $implementedBy = $attribute->extendedAttributes->{"ImplementedBy"};
            my $implementedByImplName = GetImplNameFromImplementedBy($implementedBy);
            AddToImplIncludes(HeaderFilesForInterface($implementedBy, $implementedByImplName));
            unshift(@arguments, "imp") if !$attribute->isStatic;
            $functionName = "${implementedByImplName}::${functionName}";
        } elsif ($attribute->isStatic) {
            $functionName = "${implClassName}::${functionName}";
        } else {
            $functionName = "imp->${functionName}";
        }
        my ($arg, $subCode) = GenerateCallWith($attribute->extendedAttributes->{"CallWith"}, "    ", 0);
        $code .= $subCode;
        unshift(@arguments, @$arg);
        $getterString = "${functionName}(" . join(", ", @arguments) . ")";
    } else {
        $getterString = "impInstance";
    }

    my $expression;
    if ($attribute->type eq "EventListener" && $interface->name eq "Window") {
        $code .= "    if (!imp->document())\n";
        $code .= "        return;\n";
    }

    if ($useExceptions || $isNullable) {
        if ($nativeType =~ /^V8StringResource/) {
            $code .= "    " . ConvertToV8StringResource($attribute, $nativeType, "v", $getterString) . ";\n";
        } else {
            $code .= "    $nativeType v = $getterString;\n";
        }

        if ($isNullable) {
            $code .= "    if (isNull) {\n";
            $code .= "        v8SetReturnValueNull(info);\n";
            $code .= "        return;\n";
            $code .= "    }\n";
        }

        if ($useExceptions) {
            $code .= "    if (UNLIKELY(ec)) {\n";
            $code .= "        setDOMException(ec, info.GetIsolate());\n";
            $code .= "        return;\n";
            $code .= "    };\n";

            if (ExtendedAttributeContains($attribute->extendedAttributes->{"CallWith"}, "ScriptState")) {
                $code .= "    if (state.hadException()) {\n";
                $code .= "        throwError(state.exception(), info.GetIsolate());\n";
                $code .= "        return;\n";
                $code .= "    }\n";
            }
        }

        $expression = "v";
        $expression .= ".release()" if (IsRefPtrType($returnType));
    } else {
        # Can inline the function call into the return statement to avoid overhead of using a Ref<> temporary
        $expression = $getterString;
        # Fix amigious conversion problem, by casting to the base type first ($getterString returns a type that inherits from SVGAnimatedEnumeration, not the base class directly).
        $expression = "static_pointer_cast<SVGAnimatedEnumeration>($expression)" if $returnType eq "SVGAnimatedEnumeration";
    }

    if (ShouldKeepAttributeAlive($interface, $attribute, $returnType)) {
        my $arrayType = GetArrayType($returnType);
        if ($arrayType) {
            AddIncludeForType("V8$arrayType.h");
            $code .= "    v8SetReturnValue(info, v8Array(${getterString}, info.GetIsolate()));\n";
            $code .= "    return;\n";
            $code .= "}\n\n";
            $implementation{nameSpaceInternal}->add($code);
            return;
        }

        AddIncludesForType($returnType);
        AddToImplIncludes("bindings/v8/V8HiddenPropertyName.h");
        # Check for a wrapper in the wrapper cache. If there is one, we know that a hidden reference has already
        # been created. If we don't find a wrapper, we create both a wrapper and a hidden reference.
        my $nativeReturnType = GetNativeType($returnType);
        $code .= "    $nativeReturnType result = ${getterString};\n";
        if ($forMainWorldSuffix) {
          $code .= "    v8::Handle<v8::Value> wrapper = result.get() ? v8::Handle<v8::Value>(DOMDataStore::getWrapper${forMainWorldSuffix}(result.get())) : v8Undefined();\n";
        } else {
          $code .= "    v8::Handle<v8::Value> wrapper = result.get() ? v8::Handle<v8::Value>(DOMDataStore::getWrapper(result.get(), info.GetIsolate())) : v8Undefined();\n";
        }
        $code .= "    if (wrapper.IsEmpty()) {\n";
        $code .= "        wrapper = toV8(result.get(), info.Holder(), info.GetIsolate());\n"; # FIXME: Could use wrap here since the wrapper is empty.
        $code .= "        if (!wrapper.IsEmpty())\n";
        $code .= "            V8HiddenPropertyName::setNamedHiddenReference(info.Holder(), \"${attrName}\", wrapper);\n";
        $code .= "    }\n";
        $code .= "    v8SetReturnValue(info, wrapper);\n";
        $code .= "    return;\n";
        $code .= "}\n\n";
        $code .= "#endif // ${conditionalString}\n\n" if $conditionalString;
        $implementation{nameSpaceInternal}->add($code);
        return;
    }

    if ((IsSVGAnimatedType($interfaceName) or $interfaceName eq "SVGViewSpec") and IsSVGTypeNeedingTearOff($attrType)) {
        AddToImplIncludes("V8$attrType.h");
        my $svgNativeType = GetSVGTypeNeedingTearOff($attrType);
        # Convert from abstract SVGProperty to real type, so the right toJS() method can be invoked.
        $code .= "    v8SetReturnValue(info, toV8Fast$forMainWorldSuffix(static_cast<$svgNativeType*>($expression), info, imp));\n";
        $code .= "    return;\n";
    } elsif (IsSVGTypeNeedingTearOff($attrType) and not $interfaceName =~ /List$/) {
        AddToImplIncludes("V8$attrType.h");
        AddToImplIncludes("core/svg/properties/SVGPropertyTearOff.h");
        my $tearOffType = GetSVGTypeNeedingTearOff($attrType);
        my $wrappedValue;
        if (IsSVGTypeWithWritablePropertiesNeedingTearOff($attrType) and not defined $attribute->extendedAttributes->{"Immutable"}) {
            my $getter = $expression;
            $getter =~ s/imp->//;
            $getter =~ s/\(\)//;

            my $updateMethod = "&${implClassName}::update" . FirstLetterToUpperCase($getter);

            my $selfIsTearOffType = IsSVGTypeNeedingTearOff($interfaceName);
            if ($selfIsTearOffType) {
                AddToImplIncludes("core/svg/properties/SVGStaticPropertyWithParentTearOff.h");
                $tearOffType =~ s/SVGPropertyTearOff</SVGStaticPropertyWithParentTearOff<$implClassName, /;

                if ($expression =~ /matrix/ and $interfaceName eq "SVGTransform") {
                    # SVGTransform offers a matrix() method for internal usage that returns an AffineTransform
                    # and a svgMatrix() method returning a SVGMatrix, used for the bindings.
                    $expression =~ s/matrix/svgMatrix/;
                }

                $wrappedValue = "WTF::getPtr(${tearOffType}::create(wrapper, $expression, $updateMethod))";
            } else {
                AddToImplIncludes("core/svg/properties/SVGStaticPropertyTearOff.h");
                $tearOffType =~ s/SVGPropertyTearOff</SVGStaticPropertyTearOff<$implClassName, /;

                $wrappedValue = "WTF::getPtr(${tearOffType}::create(imp, $expression, $updateMethod))";
            }
        } elsif ($tearOffType =~ /SVGStaticListPropertyTearOff/) {
                $wrappedValue = "WTF::getPtr(${tearOffType}::create(imp, $expression))";
        } elsif ($tearOffType =~ /SVG(Point|PathSeg)List/) {
                $wrappedValue = "WTF::getPtr($expression)";
        } else {
                $wrappedValue = "WTF::getPtr(${tearOffType}::create($expression))";
        }
        $code .= "    v8SetReturnValue(info, toV8Fast$forMainWorldSuffix($wrappedValue, info, imp));\n";
        $code .= "    return;\n";
    } elsif ($attribute->type eq "SerializedScriptValue" && $attrExt->{"CachedAttribute"}) {
        my $getterFunc = ToMethodName($attribute->name);
        $code .= <<END;
    RefPtr<SerializedScriptValue> serialized = imp->${getterFunc}();
    value = serialized ? serialized->deserialize() : v8::Handle<v8::Value>(v8Null(info.GetIsolate()));
    info.Holder()->SetHiddenValue(propertyName, value);
    v8SetReturnValue(info, value);
    return;
END
    } elsif ($attribute->type eq "EventListener") {
        AddToImplIncludes("bindings/v8/V8AbstractEventListener.h");
        my $getterFunc = ToMethodName($attribute->name);
        # FIXME: Pass the main world ID for main-world-only getters.
        $code .= "    EventListener* listener = imp->${getterFunc}(isolatedWorldForIsolate(info.GetIsolate()));\n";
        $code .= "    v8SetReturnValue(info, listener ? v8::Handle<v8::Value>(V8AbstractEventListener::cast(listener)->getListenerObject(imp->scriptExecutionContext())) : v8::Handle<v8::Value>(v8Null(info.GetIsolate())));\n";
        $code .= "    return;\n";
    } else {
        my $nativeValue = NativeToJSValue($attribute->type, $attribute->extendedAttributes, $expression, "    ", "", "info.Holder()", "info.GetIsolate()", "info", "imp", "ReturnUnsafeHandle", $forMainWorldSuffix, "return");
        $code .= "${nativeValue}\n";
        $code .= "    return;\n";
    }

    $code .= "}\n\n";  # end of getter
    $code .= "#endif // ${conditionalString}\n\n" if $conditionalString;
    $implementation{nameSpaceInternal}->add($code);
}

sub ShouldKeepAttributeAlive
{
    my ($interface, $attribute, $returnType) = @_;
    my $attrName = $attribute->name;

    return 1 if $attribute->extendedAttributes->{"KeepAttributeAliveForGC"};

    # Basically, for readonly or replaceable attributes, we have to guarantee
    # that JS wrappers don't get garbage-collected prematually when their
    # lifetime is strongly tied to their owner.
    return 0 if !IsWrapperType($returnType);
    return 0 if !IsReadonly($attribute) && !$attribute->extendedAttributes->{"Replaceable"};

    # However, there are a couple of exceptions.

    # Node lifetime is managed by object grouping.
    return 0 if InheritsInterface($interface, "Node");
    return 0 if IsDOMNodeType($returnType);

    # To avoid adding a reference to itself.
    # FIXME: Introduce [DoNotKeepAttributeAliveForGC] and remove this hack
    # depending on the attribute name.
    return 0 if $attrName eq "self";

    # FIXME: Remove these hard-coded hacks.
    return 0 if $returnType eq "EventTarget";
    return 0 if $returnType eq "SerializedScriptValue";
    return 0 if $returnType eq "Window";
    return 0 if $returnType =~ /SVG/;
    return 0 if $returnType =~ /HTML/;

    return 1;
}

sub GenerateReplaceableAttrSetterCallback
{
    my $interface = shift;
    my $implClassName = GetImplName($interface);

    my $code = "";
    $code .= "static void ${implClassName}ReplaceableAttrSetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)\n";
    $code .= "{\n";
    $code .= GenerateFeatureObservation($interface->extendedAttributes->{"MeasureAs"});
    $code .= GenerateDeprecationNotification($interface->extendedAttributes->{"DeprecateAs"});
    if (HasActivityLogging("", $interface->extendedAttributes, "Setter")) {
         die "IDL error: ActivityLog attribute cannot exist on a ReplacableAttrSetterCallback";
    }
    $code .= "    ${implClassName}V8Internal::${implClassName}ReplaceableAttrSetter(name, value, info);\n";
    $code .= "}\n\n";
    $implementation{nameSpaceInternal}->add($code);
}

sub GenerateReplaceableAttrSetter
{
    my $interface = shift;

    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);

    my $code = "";
    $code .= <<END;
static void ${implClassName}ReplaceableAttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)
{
END
    if ($interface->extendedAttributes->{"CheckSecurity"}) {
        AddToImplIncludes("core/page/Frame.h");
        AddToImplIncludes("bindings/v8/BindingSecurity.h");
        $code .= <<END;
    ${implClassName}* imp = ${v8ClassName}::toNative(info.Holder());
    if (!BindingSecurity::shouldAllowAccessToFrame(imp->frame()))
        return;
END
    }

    $code .= <<END;
    info.This()->ForceSet(name, value);
}

END
    $implementation{nameSpaceInternal}->add($code);
}

sub GenerateCustomElementInvocationScopeIfNeeded
{
    my $code = "";
    my $ext = shift;

    if ($ext->{"DeliverCustomElementCallbacks"}) {
        if ($ext->{"Reflect"}) {
            die "IDL error: [Reflect] and [DeliverCustomElementCallbacks] cannot coexist yet";
        }

        AddToImplIncludes("core/dom/CustomElementRegistry.h");
        $code .= <<END;
    CustomElementRegistry::CallbackDeliveryScope deliveryScope;
END
    }
    return $code;
}

sub GenerateNormalAttrSetterCallback
{
    my $attribute = shift;
    my $interface = shift;
    my $forMainWorldSuffix = shift;

    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $attrExt = $attribute->extendedAttributes;
    my $attrName = $attribute->name;

    my $conditionalString = GenerateConditionalString($attribute);
    my $code = "";
    $code .= "#if ${conditionalString}\n\n" if $conditionalString;

    $code .= "static void ${attrName}AttrSetterCallback${forMainWorldSuffix}(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)\n";
    $code .= "{\n";
    $code .= GenerateFeatureObservation($attrExt->{"MeasureAs"});
    $code .= GenerateDeprecationNotification($attrExt->{"DeprecateAs"});
    if (HasActivityLogging($forMainWorldSuffix, $attrExt, "Setter")) {
        $code .= GenerateActivityLogging("Setter", $interface, "${attrName}");
    }
    if (HasCustomSetter($attrExt)) {
        $code .= "    ${v8ClassName}::${attrName}AttrSetterCustom(name, value, info);\n";
    } else {
        $code .= "    ${implClassName}V8Internal::${attrName}AttrSetter${forMainWorldSuffix}(name, value, info);\n";
    }
    $code .= "}\n\n";
    $code .= "#endif // ${conditionalString}\n\n" if $conditionalString;
    $implementation{nameSpaceInternal}->add($code);
}

sub GenerateNormalAttrSetter
{
    my $attribute = shift;
    my $interface = shift;
    my $forMainWorldSuffix = shift;

    my $interfaceName = $interface->name;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $attrName = $attribute->name;
    my $attrExt = $attribute->extendedAttributes;
    my $attrType = $attribute->type;

    if (HasCustomSetter($attrExt)) {
        return;
    }

    my $conditionalString = GenerateConditionalString($attribute);
    my $code = "";
    $code .= "#if ${conditionalString}\n\n" if $conditionalString;
    $code .= "static void ${attrName}AttrSetter${forMainWorldSuffix}(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info)\n";
    $code .= "{\n";

    # If the "StrictTypeChecking" extended attribute is present, and the attribute's type is an
    # interface type, then if the incoming value does not implement that interface, a TypeError is
    # thrown rather than silently passing NULL to the C++ code.
    # Per the Web IDL and ECMAScript specifications, incoming values can always be converted to both
    # strings and numbers, so do not throw TypeError if the attribute is of these types.
    if ($attribute->extendedAttributes->{"StrictTypeChecking"}) {
        my $argType = $attribute->type;
        if (IsWrapperType($argType)) {
            $code .= "    if (!isUndefinedOrNull(value) && !V8${argType}::HasInstance(value, info.GetIsolate(), worldType(info.GetIsolate()))) {\n";
            $code .= "        throwTypeError(0, info.GetIsolate());\n";
            $code .= "        return;\n";
            $code .= "    }\n";
        }
    }

    my $svgNativeType = GetSVGTypeNeedingTearOff($interfaceName);
    if ($svgNativeType) {
        my $svgWrappedNativeType = GetSVGWrappedTypeNeedingTearOff($interfaceName);
        if ($svgWrappedNativeType =~ /List$/) {
            $code .= <<END;
    $svgNativeType* imp = ${v8ClassName}::toNative(info.Holder());
END
        } else {
            AddToImplIncludes("core/dom/ExceptionCode.h");
            $code .= "    $svgNativeType* wrapper = ${v8ClassName}::toNative(info.Holder());\n";
            $code .= "    if (wrapper->isReadOnly()) {\n";
            $code .= "        setDOMException(NO_MODIFICATION_ALLOWED_ERR, info.GetIsolate());\n";
            $code .= "        return;\n";
            $code .= "    }\n";
            $code .= "    $svgWrappedNativeType& impInstance = wrapper->propertyReference();\n";
            $code .= "    $svgWrappedNativeType* imp = &impInstance;\n";
        }
    } elsif ($attrExt->{"OnProto"}) {
        $code .= <<END;
    ${implClassName}* imp = ${v8ClassName}::toNative(info.Holder());
END
    } else {
        my $reflect = $attribute->extendedAttributes->{"Reflect"};
        if ($reflect && InheritsInterface($interface, "Node") && $attrType eq "DOMString") {
            # Generate super-compact call for regular attribute setter:
            my $contentAttributeName = $reflect eq "VALUE_IS_MISSING" ? lc $attrName : $reflect;
            my $namespace = NamespaceForAttributeName($interfaceName, $contentAttributeName);
            AddToImplIncludes("${namespace}.h");
            $code .= "    Element* imp = V8Element::toNative(info.Holder());\n";
            $code .= "    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<WithNullCheck>, stringResource, value);\n";
            $code .= "    imp->setAttribute(${namespace}::${contentAttributeName}Attr, stringResource);\n";
            $code .= "}\n\n";
            $code .= "#endif // ${conditionalString}\n\n" if $conditionalString;
            $implementation{nameSpaceInternal}->add($code);
            return;
            # Skip the rest of the function!
        }

        if (!$attribute->isStatic) {
            $code .= <<END;
    ${implClassName}* imp = ${v8ClassName}::toNative(info.Holder());
END
        }
    }

    my $nativeType = GetNativeTypeFromAttributeOrParameter($attribute, 0);
    if ($attribute->type eq "EventListener") {
        if ($interface->name eq "Window") {
            $code .= "    if (!imp->document())\n";
            $code .= "        return;\n";
        }
    } else {
        my $value = JSValueToNative($attribute->type, $attribute->extendedAttributes, "value", "info.GetIsolate()");
        my $arrayType = GetArrayType($nativeType);

        if ($nativeType =~ /^V8StringResource/) {
            $code .= "    " . ConvertToV8StringResource($attribute, $nativeType, "v", $value) . "\n";
        } elsif ($arrayType) {
            $code .= "    Vector<$arrayType> v = $value;\n";
        } elsif ($attribute->extendedAttributes->{"EnforceRange"}) {
            $code .= "    V8TRYCATCH_WITH_TYPECHECK_VOID($nativeType, v, $value, info.GetIsolate());\n";
        } else {
            $code .= "    V8TRYCATCH_VOID($nativeType, v, $value);\n";
        }
    }

    if (IsEnumType($attrType)) {
        # setter ignores invalid enumeration values
        my @enumValues = ValidEnumValues($attrType);
        my @validEqualities = ();
        foreach my $enumValue (@enumValues) {
            push(@validEqualities, "string == \"$enumValue\"");
        }
        my $enumValidationExpression = join(" || ", @validEqualities);
        $code .= <<END;
    String string = v;
    if (!($enumValidationExpression))
        return;
END
    }

    my $expression = "v";
    my $returnType = $attribute->type;
    if (IsRefPtrType($returnType) && !GetArrayType($returnType)) {
        $expression = "WTF::getPtr(" . $expression . ")";
    }

    $code .= GenerateCustomElementInvocationScopeIfNeeded($attribute->extendedAttributes);

    my $useExceptions = 1 if $attribute->extendedAttributes->{"SetterRaisesException"} ||  $attribute->extendedAttributes->{"RaisesException"};

    if ($useExceptions) {
        AddToImplIncludes("core/dom/ExceptionCode.h");
        $code .= "    ExceptionCode ec = 0;\n";
    }

    if ($interfaceName eq "SVGNumber") {
        $code .= "    *imp = $expression;\n";
    } else {
        if ($attribute->type eq "EventListener") {
            my $implSetterFunctionName = FirstLetterToUpperCase($attrName);
            AddToImplIncludes("bindings/v8/V8AbstractEventListener.h");
            if (!InheritsInterface($interface, "Node")) {
                my $attrImplName = GetImplName($attribute);
                $code .= "    transferHiddenDependency(info.Holder(), imp->${attrImplName}(isolatedWorldForIsolate(info.GetIsolate())), value, ${v8ClassName}::eventListenerCacheIndex, info.GetIsolate());\n";
            }
            AddToImplIncludes("bindings/v8/V8EventListenerList.h");
            if (($interfaceName eq "Window" or $interfaceName eq "WorkerContext") and $attribute->name eq "onerror") {
                AddToImplIncludes("bindings/v8/V8ErrorHandler.h");
                $code .= "    imp->set$implSetterFunctionName(V8EventListenerList::findOrCreateWrapper<V8ErrorHandler>(value, true), isolatedWorldForIsolate(info.GetIsolate()));\n";
            } else {
                $code .= "    imp->set$implSetterFunctionName(V8EventListenerList::getEventListener(value, true, ListenerFindOrCreate), isolatedWorldForIsolate(info.GetIsolate()));\n";
            }
        } else {
            my ($functionName, @arguments) = SetterExpression($interfaceName, $attribute);
            push(@arguments, $expression);
            push(@arguments, "ec") if $useExceptions;
            if ($attribute->extendedAttributes->{"ImplementedBy"}) {
                my $implementedBy = $attribute->extendedAttributes->{"ImplementedBy"};
                my $implementedByImplName = GetImplNameFromImplementedBy($implementedBy);
                AddToImplIncludes(HeaderFilesForInterface($implementedBy, $implementedByImplName));
                unshift(@arguments, "imp") if !$attribute->isStatic;
                $functionName = "${implementedByImplName}::${functionName}";
            } elsif ($attribute->isStatic) {
                $functionName = "${implClassName}::${functionName}";
            } else {
                $functionName = "imp->${functionName}";
            }
            my ($arg, $subCode) = GenerateCallWith($attribute->extendedAttributes->{"SetterCallWith"} || $attribute->extendedAttributes->{"CallWith"}, "    ", 1);
            $code .= $subCode;
            unshift(@arguments, @$arg);
            $code .= "    ${functionName}(" . join(", ", @arguments) . ");\n";
        }
    }

    if ($useExceptions) {
        $code .= "    if (UNLIKELY(ec))\n";
        $code .= "        setDOMException(ec, info.GetIsolate());\n";
    }

    if (ExtendedAttributeContains($attribute->extendedAttributes->{"CallWith"}, "ScriptState")) {
        $code .= "    if (state.hadException())\n";
        $code .= "        throwError(state.exception(), info.GetIsolate());\n";
    }

    if ($svgNativeType) {
        if ($useExceptions) {
            $code .= "    if (!ec)\n";
            $code .= "        wrapper->commitChange();\n";
        } else {
            $code .= "    wrapper->commitChange();\n";
        }
    }

    if ($attribute->type eq "SerializedScriptValue" && $attribute->extendedAttributes->{"CachedAttribute"}) {
        $code .= <<END;
    info.Holder()->DeleteHiddenValue(v8::String::NewSymbol("${attrName}")); // Invalidate the cached value.
END
    }

    $code .= "    return;\n";
    $code .= "}\n\n";  # end of setter
    $code .= "#endif // ${conditionalString}\n\n" if $conditionalString;
    $implementation{nameSpaceInternal}->add($code);
}

sub GenerateParametersCheckExpression
{
    my $numParameters = shift;
    my $function = shift;

    my @andExpression = ();
    push(@andExpression, "args.Length() == $numParameters");
    my $parameterIndex = 0;
    foreach my $parameter (@{$function->parameters}) {
        last if $parameterIndex >= $numParameters;
        my $value = "args[$parameterIndex]";
        my $type = $parameter->type;

        # Only DOMString or wrapper types are checked.
        # For DOMString with StrictTypeChecking only Null, Undefined and Object
        # are accepted for compatibility. Otherwise, no restrictions are made to
        # match the non-overloaded behavior.
        # FIXME: Implement WebIDL overload resolution algorithm.
        if ($type eq "DOMString") {
            if ($parameter->extendedAttributes->{"StrictTypeChecking"}) {
                push(@andExpression, "(${value}->IsNull() || ${value}->IsUndefined() || ${value}->IsString() || ${value}->IsObject())");
            }
        } elsif (IsCallbackInterface($parameter->type)) {
            # For Callbacks only checks if the value is null or object.
            push(@andExpression, "(${value}->IsNull() || ${value}->IsFunction())");
        } elsif (GetArrayOrSequenceType($type)) {
            if ($parameter->isNullable) {
                push(@andExpression, "(${value}->IsNull() || ${value}->IsArray())");
            } else {
                push(@andExpression, "(${value}->IsArray())");
            }
        } elsif (IsWrapperType($type)) {
            if ($parameter->isNullable) {
                push(@andExpression, "(${value}->IsNull() || V8${type}::HasInstance($value, args.GetIsolate(), worldType(args.GetIsolate())))");
            } else {
                push(@andExpression, "(V8${type}::HasInstance($value, args.GetIsolate(), worldType(args.GetIsolate())))");
            }
        }

        $parameterIndex++;
    }
    my $res = join(" && ", @andExpression);
    $res = "($res)" if @andExpression > 1;
    return $res;
}

# As per Web IDL specification, the length of a function Object is
# its number of mandatory parameters.
sub GetFunctionLength
{
    my $function = shift;

    my $numMandatoryParams = 0;
    foreach my $parameter (@{$function->parameters}) {
        # Abort as soon as we find the first optional parameter as no mandatory
        # parameter can follow an optional one.
        last if $parameter->isOptional;
        $numMandatoryParams++;
    }
    return $numMandatoryParams;
}

sub GenerateFunctionParametersCheck
{
    my $function = shift;

    my @orExpression = ();
    my $numParameters = 0;
    my $hasVariadic = 0;
    my $numMandatoryParams = @{$function->parameters};
    foreach my $parameter (@{$function->parameters}) {
        if ($parameter->isOptional) {
            push(@orExpression, GenerateParametersCheckExpression($numParameters, $function));
            $numMandatoryParams--;
        }
        if ($parameter->isVariadic) {
            $hasVariadic = 1;
            last;
        }
        $numParameters++;
    }
    if (!$hasVariadic) {
        push(@orExpression, GenerateParametersCheckExpression($numParameters, $function));
    }
    return ($numMandatoryParams, join(" || ", @orExpression));
}

sub GenerateOverloadedFunction
{
    my $function = shift;
    my $interface = shift;
    my $forMainWorldSuffix = shift;

    # Generate code for choosing the correct overload to call. Overloads are
    # chosen based on the total number of arguments passed and the type of
    # values passed in non-primitive argument slots. When more than a single
    # overload is applicable, precedence is given according to the order of
    # declaration in the IDL.

    my $name = $function->name;

    my $conditionalString = GenerateConditionalString($function);
    my $leastNumMandatoryParams = 255;
    my $code = "";
    $code .= "#if ${conditionalString}\n\n" if $conditionalString;
    $code .= <<END;
static void ${name}Method${forMainWorldSuffix}(const v8::FunctionCallbackInfo<v8::Value>& args)
{
END
    $code .= GenerateFeatureObservation($function->extendedAttributes->{"MeasureAs"});
    $code .= GenerateDeprecationNotification($function->extendedAttributes->{"DeprecateAs"});

    foreach my $overload (@{$function->{overloads}}) {
        my ($numMandatoryParams, $parametersCheck) = GenerateFunctionParametersCheck($overload);
        $leastNumMandatoryParams = $numMandatoryParams if ($numMandatoryParams < $leastNumMandatoryParams);
        $code .= "    if ($parametersCheck) {\n";
        my $overloadedIndexString = $overload->{overloadIndex};
        $code .= "        ${name}${overloadedIndexString}Method${forMainWorldSuffix}(args);\n";
        $code .= "        return;\n";
        $code .= "    }\n";
    }
    if ($leastNumMandatoryParams >= 1) {
        $code .= "    if (args.Length() < $leastNumMandatoryParams) {\n";
        $code .= "        throwNotEnoughArgumentsError(args.GetIsolate());\n";
        $code .= "        return;\n";
        $code .= "    }\n";
    }
    $code .= <<END;
    throwTypeError(0, args.GetIsolate());
END
    $code .= "}\n\n";
    $code .= "#endif // ${conditionalString}\n\n" if $conditionalString;
    $implementation{nameSpaceInternal}->add($code);
}

sub GenerateFunctionCallback
{
    my $function = shift;
    my $interface = shift;
    my $forMainWorldSuffix = shift;

    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $name = $function->name;

    if ($name eq "") {
        return;
    }

    my $conditionalString = GenerateConditionalString($function);
    my $code = "";
    $code .= "#if ${conditionalString}\n\n" if $conditionalString;
    $code .= <<END;
static void ${name}MethodCallback${forMainWorldSuffix}(const v8::FunctionCallbackInfo<v8::Value>& args)
{
END
    $code .= GenerateFeatureObservation($function->extendedAttributes->{"MeasureAs"});
    $code .= GenerateDeprecationNotification($function->extendedAttributes->{"DeprecateAs"});
    if (HasActivityLogging($forMainWorldSuffix, $function->extendedAttributes, "Access")) {
        $code .= GenerateActivityLogging("Method", $interface, "${name}");
    }
    if (HasCustomMethod($function->extendedAttributes)) {
        $code .= "    ${v8ClassName}::${name}MethodCustom(args);\n";
    } else {
        $code .= "    ${implClassName}V8Internal::${name}Method${forMainWorldSuffix}(args);\n";
    }
    $code .= "}\n\n";
    $code .= "#endif // ${conditionalString}\n\n" if $conditionalString;
    $implementation{nameSpaceInternal}->add($code);
}

sub GenerateFunction
{
    my $function = shift;
    my $interface = shift;
    my $forMainWorldSuffix = shift;

    my $interfaceName = $interface->name;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $name = $function->name;
    my $implName = GetImplName($function);
    my $funcExt = $function->extendedAttributes;

    if (HasCustomMethod($funcExt) || $name eq "") {
        return;
    }

    if (@{$function->{overloads}} > 1) {
        # Append a number to an overloaded method's name to make it unique:
        $name = $name . $function->{overloadIndex};
    }

    my $conditionalString = GenerateConditionalString($function);
    my $code = "";
    $code .= "#if ${conditionalString}\n\n" if $conditionalString;
    $code .= "static void ${name}Method${forMainWorldSuffix}(const v8::FunctionCallbackInfo<v8::Value>& args)\n";
    $code .= "{\n";

    if ($name eq "addEventListener" || $name eq "removeEventListener") {
        my $lookupType = ($name eq "addEventListener") ? "OrCreate" : "Only";
        my $passRefPtrHandling = ($name eq "addEventListener") ? "" : ".get()";
        my $hiddenDependencyAction = ($name eq "addEventListener") ? "create" : "remove";

        AddToImplIncludes("bindings/v8/V8EventListenerList.h");
        $code .= <<END;
    RefPtr<EventListener> listener = V8EventListenerList::getEventListener(args[1], false, ListenerFind${lookupType});
    if (listener) {
        V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<WithNullCheck>, stringResource, args[0]);
        ${v8ClassName}::toNative(args.Holder())->${implName}(stringResource, listener${passRefPtrHandling}, args[2]->BooleanValue());
END
        if (!InheritsInterface($interface, "Node")) {
            $code .= <<END;
        ${hiddenDependencyAction}HiddenDependency(args.Holder(), args[1], ${v8ClassName}::eventListenerCacheIndex, args.GetIsolate());
END
        }
        $code .= <<END;
    }
}

END
        $code .= "#endif // ${conditionalString}\n\n" if $conditionalString;
        $implementation{nameSpaceInternal}->add($code);
        return;
    }

    $code .= GenerateArgumentsCountCheck($function, $interface);

    if ($name eq "set" and IsConstructorTemplate($interface, "TypedArray")) {
        AddToImplIncludes("bindings/v8/custom/V8ArrayBufferViewCustom.h");
        $code .= <<END;
    setWebGLArrayHelper<$implClassName, ${v8ClassName}>(args);
}

END
        $implementation{nameSpaceInternal}->add($code);
        return;
    }

    my ($svgPropertyType, $svgListPropertyType, $svgNativeType) = GetSVGPropertyTypes($interfaceName);

    if ($svgNativeType) {
        my $nativeClassName = GetNativeType($interfaceName);
        if ($interfaceName =~ /List$/) {
            $code .= "    $nativeClassName imp = ${v8ClassName}::toNative(args.Holder());\n";
        } else {
            AddToImplIncludes("core/dom/ExceptionCode.h");
            $code .= "    $nativeClassName wrapper = ${v8ClassName}::toNative(args.Holder());\n";
            $code .= "    if (wrapper->isReadOnly()) {\n";
            $code .= "        setDOMException(NO_MODIFICATION_ALLOWED_ERR, args.GetIsolate());\n";
            $code .= "        return;\n";
            $code .= "    }\n";
            my $svgWrappedNativeType = GetSVGWrappedTypeNeedingTearOff($interfaceName);
            $code .= "    $svgWrappedNativeType& impInstance = wrapper->propertyReference();\n";
            $code .= "    $svgWrappedNativeType* imp = &impInstance;\n";
        }
    } elsif (!$function->isStatic) {
        $code .= <<END;
    ${implClassName}* imp = ${v8ClassName}::toNative(args.Holder());
END
    }

    $code .= GenerateCustomElementInvocationScopeIfNeeded($funcExt);

    # Check domain security if needed
    if ($interface->extendedAttributes->{"CheckSecurity"} && !$function->extendedAttributes->{"DoNotCheckSecurity"}) {
        # We have not find real use cases yet.
        AddToImplIncludes("core/page/Frame.h");
        AddToImplIncludes("bindings/v8/BindingSecurity.h");
        $code .= <<END;
    if (!BindingSecurity::shouldAllowAccessToFrame(imp->frame()))
        return;
END
    }

    my $raisesExceptions = $function->extendedAttributes->{"RaisesException"};
    if (!$raisesExceptions) {
        foreach my $parameter (@{$function->parameters}) {
            if ($parameter->extendedAttributes->{"IsIndex"}) {
                $raisesExceptions = 1;
            }
        }
    }

    if ($raisesExceptions) {
        AddToImplIncludes("core/dom/ExceptionCode.h");
        $code .= "    ExceptionCode ec = 0;\n";
    }

    if ($function->extendedAttributes->{"CheckSecurityForNode"}) {
        AddToImplIncludes("bindings/v8/BindingSecurity.h");
        $code .= "    if (!BindingSecurity::shouldAllowAccessToNode(imp->" . GetImplName($function) . "(ec))) {\n";
        $code .= "        v8SetReturnValueNull(args);\n";
        $code .= "        return;\n";
        $code .= "    }\n";
END
    }

    my ($parameterCheckString, $paramIndex, %replacements) = GenerateParametersCheck($function, $interface, $forMainWorldSuffix);
    $code .= $parameterCheckString;

    # Build the function call string.
    $code .= GenerateFunctionCallString($function, $paramIndex, "    ", $interface, $forMainWorldSuffix, %replacements);
    $code .= "}\n\n";
    $code .= "#endif // ${conditionalString}\n\n" if $conditionalString;
    $implementation{nameSpaceInternal}->add($code);
}

sub GenerateCallWith
{
    my $callWith = shift;
    return ([], "") unless $callWith;
    my $indent = shift;
    my $returnVoid = shift;
    my $function = shift;
    my $code = "";

    my @callWithArgs;
    if (ExtendedAttributeContains($callWith, "ScriptState")) {
        $code .= $indent . "ScriptState* currentState = ScriptState::current();\n";
        $code .= $indent . "if (!currentState)\n";
        $code .= $indent . "    return" . ($returnVoid ? "" : " v8Undefined()") . ";\n";
        $code .= $indent . "ScriptState& state = *currentState;\n";
        push(@callWithArgs, "&state");
    }
    if (ExtendedAttributeContains($callWith, "ScriptExecutionContext")) {
        $code .= $indent . "ScriptExecutionContext* scriptContext = getScriptExecutionContext();\n";
        push(@callWithArgs, "scriptContext");
    }
    if ($function and ExtendedAttributeContains($callWith, "ScriptArguments")) {
        $code .= $indent . "RefPtr<ScriptArguments> scriptArguments(createScriptArguments(args, " . @{$function->parameters} . "));\n";
        push(@callWithArgs, "scriptArguments.release()");
        AddToImplIncludes("bindings/v8/ScriptCallStackFactory.h");
        AddToImplIncludes("core/inspector/ScriptArguments.h");
    }
    if (ExtendedAttributeContains($callWith, "ActiveWindow")) {
        push(@callWithArgs, "activeDOMWindow()");
    }
    if (ExtendedAttributeContains($callWith, "FirstWindow")) {
        push(@callWithArgs, "firstDOMWindow()");
    }
    return ([@callWithArgs], $code);
}

sub GenerateArgumentsCountCheck
{
    my $function = shift;
    my $interface = shift;

    my $numMandatoryParams = 0;
    my $allowNonOptional = 1;
    foreach my $param (@{$function->parameters}) {
        if ($param->isOptional or $param->isVariadic) {
            $allowNonOptional = 0;
        } else {
            die "An argument must not be declared to be optional unless all subsequent arguments to the operation are also optional." if !$allowNonOptional;
            $numMandatoryParams++;
        }
    }

    my $argumentsCountCheckString = "";
    if ($numMandatoryParams >= 1) {
        $argumentsCountCheckString .= "    if (args.Length() < $numMandatoryParams) {\n";
        $argumentsCountCheckString .= "        throwNotEnoughArgumentsError(args.GetIsolate());\n";
        $argumentsCountCheckString .= "        return;\n";
        $argumentsCountCheckString .= "    }\n";
    }
    return $argumentsCountCheckString;
}

sub GenerateParametersCheck
{
    my $function = shift;
    my $interface = shift;
    my $forMainWorldSuffix = shift;
    my $style = shift || "new";

    my $parameterCheckString = "";
    my $paramIndex = 0;
    my %replacements = ();

    foreach my $parameter (@{$function->parameters}) {
        my $nativeType = GetNativeTypeFromAttributeOrParameter($parameter, $paramIndex);

        # Optional arguments without [Default=...] should generate an early call with fewer arguments.
        # Optional arguments with [Optional=...] should not generate the early call.
        # Optional Dictionary arguments always considered to have default of empty dictionary.
        if ($parameter->isOptional && !$parameter->extendedAttributes->{"Default"} && $nativeType ne "Dictionary" && !IsCallbackInterface($parameter->type)) {
            $parameterCheckString .= "    if (args.Length() <= $paramIndex)";
            my $functionCall = GenerateFunctionCallString($function, $paramIndex, "    " x 2, $interface, $forMainWorldSuffix, %replacements);
            my $multiLine = ($functionCall =~ tr/\n//) > 1;
            $parameterCheckString .= $multiLine ? " {\n" : "\n";
            $parameterCheckString .= $functionCall;
            $parameterCheckString .= $multiLine ? "    }\n" : "\n";
        }

        my $parameterDefaultPolicy = "DefaultIsUndefined";
        my $default = defined $parameter->extendedAttributes->{"Default"} ? $parameter->extendedAttributes->{"Default"} : "";
        if ($parameter->isOptional and $default eq "NullString") {
            $parameterDefaultPolicy = "DefaultIsNullString";
        }

        my $parameterName = $parameter->name;
        AddToImplIncludes("core/dom/ExceptionCode.h");
        if (IsCallbackInterface($parameter->type)) {
            my $v8ClassName = "V8" . $parameter->type;
            AddToImplIncludes("$v8ClassName.h");
            if ($parameter->isOptional) {
                $parameterCheckString .= "    RefPtr<" . $parameter->type . "> $parameterName;\n";
                $parameterCheckString .= "    if (args.Length() > $paramIndex && !args[$paramIndex]->IsNull() && !args[$paramIndex]->IsUndefined()) {\n";
                $parameterCheckString .= "        if (!args[$paramIndex]->IsFunction()) {\n";
                $parameterCheckString .= "            throwTypeError(0, args.GetIsolate());\n";
                $parameterCheckString .= "            return;\n";
                $parameterCheckString .= "        }\n";
                $parameterCheckString .= "        $parameterName = ${v8ClassName}::create(args[$paramIndex], getScriptExecutionContext());\n";
                $parameterCheckString .= "    }\n";
            } else {
                $parameterCheckString .= "    if (args.Length() <= $paramIndex || !args[$paramIndex]->IsFunction()) {\n";
                $parameterCheckString .= "        throwTypeError(0, args.GetIsolate());\n";
                $parameterCheckString .= "        return;\n";
                $parameterCheckString .= "    }\n";
                $parameterCheckString .= "    RefPtr<" . $parameter->type . "> $parameterName = ${v8ClassName}::create(args[$paramIndex], getScriptExecutionContext());\n";
            }
        } elsif ($parameter->extendedAttributes->{"Clamp"}) {
                my $nativeValue = "${parameterName}NativeValue";
                my $paramType = $parameter->type;
                $parameterCheckString .= "    $paramType $parameterName = 0;\n";
                $parameterCheckString .= "    V8TRYCATCH_VOID(double, $nativeValue, args[$paramIndex]->NumberValue());\n";
                $parameterCheckString .= "    if (!std::isnan($nativeValue))\n";
                $parameterCheckString .= "        $parameterName = clampTo<$paramType>($nativeValue);\n";
        } elsif ($parameter->type eq "SerializedScriptValue") {
            AddToImplIncludes("bindings/v8/SerializedScriptValue.h");
            $parameterCheckString .= "    bool ${parameterName}DidThrow = false;\n";
            $parameterCheckString .= "    $nativeType $parameterName = SerializedScriptValue::create(args[$paramIndex], 0, 0, ${parameterName}DidThrow, args.GetIsolate());\n";
            $parameterCheckString .= "    if (${parameterName}DidThrow)\n";
            $parameterCheckString .= "        return;\n";
        } elsif ($parameter->isVariadic) {
            my $nativeElementType = GetNativeType($parameter->type);
            if ($nativeElementType =~ />$/) {
                $nativeElementType .= " ";
            }

            my $argType = $parameter->type;
            if (IsWrapperType($argType)) {
                $parameterCheckString .= "    Vector<$nativeElementType> $parameterName;\n";
                $parameterCheckString .= "    for (int i = $paramIndex; i < args.Length(); ++i) {\n";
                $parameterCheckString .= "        if (!V8${argType}::HasInstance(args[i], args.GetIsolate(), worldType(args.GetIsolate()))) {\n";
                $parameterCheckString .= "            throwTypeError(0, args.GetIsolate());\n";
                $parameterCheckString .= "            return;\n";
                $parameterCheckString .= "        }\n";
                $parameterCheckString .= "        $parameterName.append(V8${argType}::toNative(v8::Handle<v8::Object>::Cast(args[i])));\n";
                $parameterCheckString .= "    }\n";
            } else {
                $parameterCheckString .= "    V8TRYCATCH_VOID(Vector<$nativeElementType>, $parameterName, toNativeArguments<$nativeElementType>(args, $paramIndex));\n";
            }
        } elsif ($nativeType =~ /^V8StringResource/) {
            my $default = defined $parameter->extendedAttributes->{"Default"} ? $parameter->extendedAttributes->{"Default"} : "";
            my $value = JSValueToNative($parameter->type, $parameter->extendedAttributes, $parameter->isOptional && $default eq "NullString" ? "argumentOrNull(args, $paramIndex)" : "args[$paramIndex]", "args.GetIsolate()");
            $parameterCheckString .= "    " . ConvertToV8StringResource($parameter, $nativeType, $parameterName, $value) . "\n";
            if (IsEnumType($parameter->type)) {
                my @enumValues = ValidEnumValues($parameter->type);
                my @validEqualities = ();
                foreach my $enumValue (@enumValues) {
                    push(@validEqualities, "string == \"$enumValue\"");
                }
                my $enumValidationExpression = join(" || ", @validEqualities);
                $parameterCheckString .=  "    String string = $parameterName;\n";
                $parameterCheckString .= "    if (!($enumValidationExpression)) {\n";
                $parameterCheckString .= "        throwTypeError(0, args.GetIsolate());\n";
                $parameterCheckString .= "        return;\n";
                $parameterCheckString .= "    }\n";
            }
        } else {
            # If the "StrictTypeChecking" extended attribute is present, and the argument's type is an
            # interface type, then if the incoming value does not implement that interface, a TypeError
            # is thrown rather than silently passing NULL to the C++ code.
            # Per the Web IDL and ECMAScript specifications, incoming values can always be converted
            # to both strings and numbers, so do not throw TypeError if the argument is of these
            # types.
            if ($function->extendedAttributes->{"StrictTypeChecking"}) {
                my $argValue = "args[$paramIndex]";
                my $argType = $parameter->type;
                if (IsWrapperType($argType)) {
                    $parameterCheckString .= "    if (args.Length() > $paramIndex && !isUndefinedOrNull($argValue) && !V8${argType}::HasInstance($argValue, args.GetIsolate(), worldType(args.GetIsolate()))) {\n";
                    $parameterCheckString .= "        throwTypeError(0, args.GetIsolate());\n";
                    $parameterCheckString .= "        return;\n";
                    $parameterCheckString .= "    }\n";
                }
            }
            my $default = defined $parameter->extendedAttributes->{"Default"} ? $parameter->extendedAttributes->{"Default"} : "";
            my $value = JSValueToNative($parameter->type, $parameter->extendedAttributes, $parameter->isOptional && $default eq "NullString" ? "argumentOrNull(args, $paramIndex)" : "args[$paramIndex]", "args.GetIsolate()");
            if ($parameter->extendedAttributes->{"EnforceRange"}) {
                $parameterCheckString .= "    V8TRYCATCH_WITH_TYPECHECK_VOID($nativeType, $parameterName, $value, args.GetIsolate());\n";
            } else {
                $parameterCheckString .= "    V8TRYCATCH_VOID($nativeType, $parameterName, $value);\n";
            }
            if ($nativeType eq 'Dictionary') {
                $parameterCheckString .= "    if (!$parameterName.isUndefinedOrNull() && !$parameterName.isObject()) {\n";
                $parameterCheckString .= "        throwTypeError(\"Not an object.\", args.GetIsolate());\n";
                $parameterCheckString .= "        return;\n";
                $parameterCheckString .= "    }\n";
            }
        }

        if ($parameter->extendedAttributes->{"IsIndex"}) {
            $parameterCheckString .= "    if (UNLIKELY($parameterName < 0)) {\n";
            $parameterCheckString .= "        setDOMException(INDEX_SIZE_ERR, args.GetIsolate());\n";
            $parameterCheckString .= "        return;\n";
            $parameterCheckString .= "    }\n";
        }

        $paramIndex++;
    }
    return ($parameterCheckString, $paramIndex, %replacements);
}

sub GenerateOverloadedConstructorCallback
{
    my $interface = shift;
    my $implClassName = GetImplName($interface);

    my $code = "";
    $code .= <<END;
static void constructor(const v8::FunctionCallbackInfo<v8::Value>& args)
{
END
    my $leastNumMandatoryParams = 255;
    foreach my $constructor (@{$interface->constructors}) {
        my $name = "constructor" . $constructor->overloadedIndex;
        my ($numMandatoryParams, $parametersCheck) = GenerateFunctionParametersCheck($constructor);
        $leastNumMandatoryParams = $numMandatoryParams if ($numMandatoryParams < $leastNumMandatoryParams);
        $code .= "    if ($parametersCheck) {\n";
        $code .= "        ${implClassName}V8Internal::${name}(args);\n";
        $code .= "        return;\n";
        $code .= "    }\n";
    }
    if ($leastNumMandatoryParams >= 1) {
        $code .= "    if (args.Length() < $leastNumMandatoryParams) {\n";
        $code .= "        throwNotEnoughArgumentsError(args.GetIsolate());\n";
        $code .= "        return;\n";
        $code .= "    }\n";
    }
    $code .= <<END;
    throwTypeError(0, args.GetIsolate());
    return;
END
    $code .= "}\n\n";
    $implementation{nameSpaceInternal}->add($code);
}

sub GenerateSingleConstructorCallback
{
    my $interface = shift;
    my $function = shift;

    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $overloadedIndexString = "";
    if ($function->overloadedIndex > 0) {
        $overloadedIndexString .= $function->overloadedIndex;
    }

    my $raisesExceptions = $function->extendedAttributes->{"RaisesException"};
    if ($interface->extendedAttributes->{"ConstructorRaisesException"}) {
        $raisesExceptions = 1;
    }
    if (!$raisesExceptions) {
        foreach my $parameter (@{$function->parameters}) {
            if ($parameter->extendedAttributes->{"IsIndex"}) {
                $raisesExceptions = 1;
            }
        }
    }

    my @beforeArgumentList;
    my @afterArgumentList;
    my $code = "";
    $code .= <<END;
static void constructor${overloadedIndexString}(const v8::FunctionCallbackInfo<v8::Value>& args)
{
END

    if ($function->overloadedIndex == 0) {
        $code .= GenerateArgumentsCountCheck($function, $interface);
    }

    if ($raisesExceptions) {
        AddToImplIncludes("core/dom/ExceptionCode.h");
        $code .= "    ExceptionCode ec = 0;\n";
    }

    # FIXME: Currently [Constructor(...)] does not yet support optional arguments without [Default=...]
    my ($parameterCheckString, $paramIndex, %replacements) = GenerateParametersCheck($function, $interface, "");
    $code .= $parameterCheckString;

    if ($interface->extendedAttributes->{"ConstructorCallWith"} && $interface->extendedAttributes->{"ConstructorCallWith"} eq "ScriptExecutionContext") {
        push(@beforeArgumentList, "context");
        $code .= <<END;

    ScriptExecutionContext* context = getScriptExecutionContext();
END
    }

    if ($interface->extendedAttributes->{"ConstructorRaisesException"}) {
        push(@afterArgumentList, "ec");
    }

    my @argumentList;
    my $index = 0;
    foreach my $parameter (@{$function->parameters}) {
        last if $index eq $paramIndex;
        if ($replacements{$parameter->name}) {
            push(@argumentList, $replacements{$parameter->name});
        } else {
            push(@argumentList, $parameter->name);
        }
        $index++;
    }

    my $argumentString = join(", ", @beforeArgumentList, @argumentList, @afterArgumentList);
    $code .= "\n";
    $code .= "    RefPtr<${implClassName}> impl = ${implClassName}::create(${argumentString});\n";
    $code .= "    v8::Handle<v8::Object> wrapper = args.Holder();\n";

    if ($interface->extendedAttributes->{"ConstructorRaisesException"}) {
        $code .= "    if (ec) {\n";
        $code .= "        setDOMException(ec, args.GetIsolate());\n";
        $code .= "        return;\n";
        $code .= "    }\n";
    }

    $code .= <<END;

    V8DOMWrapper::associateObjectWithWrapper(impl.release(), &${v8ClassName}::info, wrapper, args.GetIsolate(), WrapperConfiguration::Dependent);
    args.GetReturnValue().Set(wrapper);
}

END
    $implementation{nameSpaceInternal}->add($code);
}

# The Web IDL specification states that Interface objects for interfaces MUST have a property named
# "length" that returns the length of the shortest argument list of the entries in the effective
# overload set for constructors. In other words, use the lowest number of mandatory arguments among
# all constructors.
sub GetInterfaceLength
{
    my $interface = shift;

    my $leastConstructorLength = 0;
    if (IsConstructorTemplate($interface, "Event") || IsConstructorTemplate($interface, "TypedArray")) {
        $leastConstructorLength = 1;
    } elsif ($interface->extendedAttributes->{"Constructor"} || $interface->extendedAttributes->{"CustomConstructor"}) {
        my @constructors = @{$interface->constructors};
        my @customConstructors = @{$interface->customConstructors};
        $leastConstructorLength = 255;
        foreach my $constructor (@constructors, @customConstructors) {
            my $constructorLength = GetFunctionLength($constructor);
            $leastConstructorLength = $constructorLength if ($constructorLength < $leastConstructorLength);
        }
    }

    return $leastConstructorLength;
}

sub GenerateConstructorCallback
{
    my $interface = shift;

    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $code = "";
    $code .= "void ${v8ClassName}::constructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args)\n";
    $code .= "{\n";
    $code .= GenerateFeatureObservation($interface->extendedAttributes->{"MeasureAs"});
    $code .= GenerateDeprecationNotification($interface->extendedAttributes->{"DeprecateAs"});
    $code .= GenerateConstructorHeader();
    if (HasCustomConstructor($interface)) {
        $code .= "    ${v8ClassName}::constructorCustom(args);\n";
    } else {
        $code .= "    ${implClassName}V8Internal::constructor(args);\n";
    }
    $code .= "}\n\n";
    $implementation{nameSpaceWebCore}->add($code);
}

sub GenerateConstructor
{
    my $interface = shift;

    if (@{$interface->constructors} == 1) {
        GenerateSingleConstructorCallback($interface, @{$interface->constructors}[0]);
    } else {
        foreach my $constructor (@{$interface->constructors}) {
            GenerateSingleConstructorCallback($interface, $constructor);
        }
        GenerateOverloadedConstructorCallback($interface);
    }
}

sub GenerateEventConstructor
{
    my $interface = shift;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);

    AddToImplIncludes("bindings/v8/Dictionary.h");
    $implementation{nameSpaceInternal}->add(<<END);
static void constructor(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() < 1) {
        throwNotEnoughArgumentsError(args.GetIsolate());
        return;
    }

    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, type, args[0]);
    ${implClassName}Init eventInit;
    if (args.Length() >= 2) {
        V8TRYCATCH_VOID(Dictionary, options, Dictionary(args[1], args.GetIsolate()));
        if (!fill${implClassName}Init(eventInit, options))
            return;
    }

    RefPtr<${implClassName}> event = ${implClassName}::create(type, eventInit);

    v8::Handle<v8::Object> wrapper = args.Holder();
    V8DOMWrapper::associateObjectWithWrapper(event.release(), &${v8ClassName}::info, wrapper, args.GetIsolate(), WrapperConfiguration::Dependent);
    args.GetReturnValue().Set(wrapper);
}
END

    my $code = "";
    $code .= <<END;
bool fill${implClassName}Init(${implClassName}Init& eventInit, const Dictionary& options)
{
END

    foreach my $interfaceBase (@{$interface->parents}) {
        $code .= <<END;
    if (!fill${interfaceBase}Init(eventInit, options))
        return false;

END
    }

    for (my $index = 0; $index < @{$interface->attributes}; $index++) {
        my $attribute = @{$interface->attributes}[$index];
        if ($attribute->extendedAttributes->{"InitializedByEventConstructor"}) {
            my $attributeName = $attribute->name;
            $code .= "    options.get(\"$attributeName\", eventInit.$attributeName);\n";
        }
    }

    $code .= <<END;
    return true;
}

END
    $implementation{nameSpaceWebCore}->add($code);
}

sub GenerateTypedArrayConstructor
{
    my $interface = shift;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);

    my ($nativeType, $arrayType) = GetNativeTypeOfTypedArray($interface);
    AddToImplIncludes("bindings/v8/custom/V8ArrayBufferViewCustom.h");

    $implementation{nameSpaceInternal}->add(<<END);
static void constructor(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    return constructWebGLArray<$implClassName, ${v8ClassName}, $nativeType>(args, &${v8ClassName}::info, $arrayType);
}

END
}

sub GenerateNamedConstructor
{
    my $function = shift;
    my $interface = shift;

    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $raisesExceptions = $function->extendedAttributes->{"RaisesException"};
    if ($interface->extendedAttributes->{"ConstructorRaisesException"}) {
        $raisesExceptions = 1;
    }
    if (!$raisesExceptions) {
        foreach my $parameter (@{$function->parameters}) {
            if ($parameter->extendedAttributes->{"IsIndex"}) {
                $raisesExceptions = 1;
            }
        }
    }

    my $maybeObserveFeature = GenerateFeatureObservation($function->extendedAttributes->{"MeasureAs"});
    my $maybeDeprecateFeature = GenerateDeprecationNotification($function->extendedAttributes->{"DeprecateAs"});

    my @beforeArgumentList;
    my @afterArgumentList;

    my $toActiveDOMObject = "0";
    if (InheritsExtendedAttribute($interface, "ActiveDOMObject")) {
        $toActiveDOMObject = "${v8ClassName}::toActiveDOMObject";
    }

    my $toEventTarget = "0";
    if (InheritsExtendedAttribute($interface, "EventTarget")) {
        $toEventTarget = "${v8ClassName}::toEventTarget";
    }

    AddToImplIncludes("core/page/Frame.h");
    $implementation{nameSpaceWebCore}->add(<<END);
WrapperTypeInfo ${v8ClassName}Constructor::info = { ${v8ClassName}Constructor::GetTemplate, ${v8ClassName}::derefObject, $toActiveDOMObject, $toEventTarget, 0, ${v8ClassName}::installPerContextPrototypeProperties, 0, WrapperTypeObjectPrototype };

END

    my $code = "";
    $code .= <<END;
static void ${v8ClassName}ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
END
    $code .= $maybeObserveFeature if $maybeObserveFeature;
    $code .= $maybeDeprecateFeature if $maybeDeprecateFeature;
    $code .= GenerateConstructorHeader();
    AddToImplIncludes("V8Document.h");
    $code .= <<END;
    Document* document = currentDocument();

    // Make sure the document is added to the DOM Node map. Otherwise, the ${implClassName} instance
    // may end up being the only node in the map and get garbage-collected prematurely.
    toV8(document, args.Holder(), args.GetIsolate());

END

    $code .= GenerateArgumentsCountCheck($function, $interface);

    if ($raisesExceptions) {
        AddToImplIncludes("core/dom/ExceptionCode.h");
        $code .= "    ExceptionCode ec = 0;\n";
    }

    my ($parameterCheckString, $paramIndex, %replacements) = GenerateParametersCheck($function, $interface);
    $code .= $parameterCheckString;

    push(@beforeArgumentList, "document");

    if ($interface->extendedAttributes->{"ConstructorRaisesException"}) {
        push(@afterArgumentList, "ec");
    }

    my @argumentList;
    my $index = 0;
    foreach my $parameter (@{$function->parameters}) {
        last if $index eq $paramIndex;
        if ($replacements{$parameter->name}) {
            push(@argumentList, $replacements{$parameter->name});
        } else {
            push(@argumentList, $parameter->name);
        }
        $index++;
    }

    my $argumentString = join(", ", @beforeArgumentList, @argumentList, @afterArgumentList);
    $code .= "\n";
    $code .= "    RefPtr<${implClassName}> impl = ${implClassName}::createForJSConstructor(${argumentString});\n";
    $code .= "    v8::Handle<v8::Object> wrapper = args.Holder();\n";

    if ($interface->extendedAttributes->{"ConstructorRaisesException"}) {
        $code .= "    if (ec) {\n";
        $code .= "        setDOMException(ec, args.GetIsolate());\n";
        $code .= "        return;\n";
        $code .= "    }\n";
    }

    $code .= <<END;

    V8DOMWrapper::associateObjectWithWrapper(impl.release(), &${v8ClassName}Constructor::info, wrapper, args.GetIsolate(), WrapperConfiguration::Dependent);
    args.GetReturnValue().Set(wrapper);
}

END
    $implementation{nameSpaceWebCore}->add($code);

    $code = <<END;
v8::Handle<v8::FunctionTemplate> ${v8ClassName}Constructor::GetTemplate(v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    static v8::Persistent<v8::FunctionTemplate> cachedTemplate;
    if (!cachedTemplate.IsEmpty())
        return v8::Local<v8::FunctionTemplate>::New(isolate, cachedTemplate);

    v8::HandleScope scope(isolate);
    v8::Local<v8::FunctionTemplate> result = v8::FunctionTemplate::New(${v8ClassName}ConstructorCallback);

    v8::Local<v8::ObjectTemplate> instance = result->InstanceTemplate();
    instance->SetInternalFieldCount(${v8ClassName}::internalFieldCount);
    result->SetClassName(v8::String::NewSymbol("${implClassName}"));
    result->Inherit(${v8ClassName}::GetTemplate(isolate, currentWorldType));

    cachedTemplate.Reset(isolate, result);
    return scope.Close(v8::Local<v8::FunctionTemplate>::New(isolate, cachedTemplate));
}

END
    $implementation{nameSpaceWebCore}->add($code);
}

sub GenerateConstructorHeader
{
    AddToImplIncludes("bindings/v8/V8ObjectConstructor.h");
    my $content = <<END;
    if (!args.IsConstructCall()) {
        throwTypeError("DOM object constructor cannot be called as a function.", args.GetIsolate());
        return;
    }

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject) {
        args.GetReturnValue().Set(args.Holder());
        return;
    }

END
    return $content;
}

sub GenerateBatchedAttributeData
{
    my $interface = shift;
    my $attributes = shift;
    my $code = "";

    foreach my $attribute (@$attributes) {
        my $conditionalString = GenerateConditionalString($attribute);
        my $subCode = "";
        $subCode .= "#if ${conditionalString}\n" if $conditionalString;
        $subCode .= GenerateSingleBatchedAttribute($interface, $attribute, ",", "");
        $subCode .= "#endif // ${conditionalString}\n" if $conditionalString;
        $code .= $subCode;
    }
    return $code;
}

sub GenerateSingleBatchedAttribute
{
    my $interface = shift;
    my $attribute = shift;
    my $delimiter = shift;
    my $indent = shift;
    my $code = "";
    my $attrName = $attribute->name;
    my $attrExt = $attribute->extendedAttributes;
    my $implClassName = GetImplName($interface);

    my $accessControl = "v8::DEFAULT";
    if ($attrExt->{"DoNotCheckSecurityOnGetter"}) {
        $accessControl = "v8::ALL_CAN_READ";
    } elsif ($attrExt->{"DoNotCheckSecurityOnSetter"}) {
        $accessControl = "v8::ALL_CAN_WRITE";
    } elsif ($attrExt->{"DoNotCheckSecurity"}) {
        $accessControl = "v8::ALL_CAN_READ";
        if (!IsReadonly($attribute)) {
            $accessControl .= " | v8::ALL_CAN_WRITE";
        }
    }
    if ($attrExt->{"Unforgeable"}) {
        $accessControl .= " | v8::PROHIBITS_OVERWRITING";
    }
    $accessControl = "static_cast<v8::AccessControl>(" . $accessControl . ")";

    my $customAccessor = HasCustomGetter($attrExt) || HasCustomSetter($attrExt) || "";
    if ($customAccessor eq "VALUE_IS_MISSING") {
        # use the naming convension, interface + (capitalize) attr name
        $customAccessor = $implClassName . "::" . $attrName;
    }

    my $getter;
    my $setter;
    my $getterForMainWorld;
    my $setterForMainWorld;
    my $propAttr = "v8::None";

    my $isConstructor = ($attribute->type =~ /Constructor$/);

    # Check attributes.
    # As per Web IDL specification, constructor properties on the ECMAScript global object should be
    # configurable and should not be enumerable.
    if ($attrExt->{"NotEnumerable"} || $isConstructor) {
        $propAttr .= " | v8::DontEnum";
    }
    if ($attrExt->{"Unforgeable"} && !$isConstructor) {
        $propAttr .= " | v8::DontDelete";
    }

    my $on_proto = "0 /* on instance */";
    my $data = "0 /* no data */";

    # Constructor
    if ($isConstructor) {
        my $constructorType = $attribute->type;
        $constructorType =~ s/Constructor$//;
        # $constructorType ~= /Constructor$/ indicates that it is NamedConstructor.
        # We do not generate the header file for NamedConstructor of class XXXX,
        # since we generate the NamedConstructor declaration into the header file of class XXXX.
        if ($constructorType !~ /Constructor$/ || $attribute->extendedAttributes->{"CustomConstructor"}) {
            AddToImplIncludes("V8${constructorType}.h");
        }
        $data = "&V8${constructorType}::info";
        $getter = "${implClassName}V8Internal::${implClassName}ConstructorGetter";
        $setter = "${implClassName}V8Internal::${implClassName}ReplaceableAttrSetterCallback";
        $getterForMainWorld = "0";
        $setterForMainWorld = "0";
    } else {
        # Default Getter and Setter
        $getter = "${implClassName}V8Internal::${attrName}AttrGetterCallback";
        $setter = "${implClassName}V8Internal::${attrName}AttrSetterCallback";
        $getterForMainWorld = "${getter}ForMainWorld";
        $setterForMainWorld = "${setter}ForMainWorld";

        if (!HasCustomSetter($attrExt) && $attrExt->{"Replaceable"}) {
            $setter = "${implClassName}V8Internal::${implClassName}ReplaceableAttrSetterCallback";
            $setterForMainWorld = "0";
        }
    }

    # Read only attributes
    if (IsReadonly($attribute)) {
        $setter = "0";
        $setterForMainWorld = "0";
    }

    # An accessor can be installed on the proto
    if ($attrExt->{"OnProto"}) {
        $on_proto = "1 /* on proto */";
    }

    if (!$attrExt->{"PerWorldBindings"}) {
      $getterForMainWorld = "0";
      $setterForMainWorld = "0";
    }

    my $commentInfo = "Attribute '$attrName'";

    $code .= $indent . "    \/\/ $commentInfo\n";
    $code .= $indent . "    {\"$attrName\", $getter, $setter, $getterForMainWorld, $setterForMainWorld, $data, $accessControl, static_cast<v8::PropertyAttribute>($propAttr), $on_proto}" . $delimiter . "\n";
    return $code;
}

sub IsStandardFunction
{
    my $interface = shift;
    my $function = shift;

    my $interfaceName = $interface->name;
    my $attrExt = $function->extendedAttributes;
    return 0 if $attrExt->{"Unforgeable"};
    return 0 if $function->isStatic;
    return 0 if $attrExt->{"EnabledAtRuntime"};
    return 0 if $attrExt->{"EnabledPerContext"};
    return 0 if RequiresCustomSignature($function);
    return 0 if $attrExt->{"DoNotCheckSignature"};
    return 0 if ($attrExt->{"DoNotCheckSecurity"} && ($interface->extendedAttributes->{"CheckSecurity"} || $interfaceName eq "Window"));
    return 0 if $attrExt->{"NotEnumerable"};
    return 0 if $attrExt->{"ReadOnly"};
    return 1;
}

sub GenerateNonStandardFunction
{
    my $interface = shift;
    my $function = shift;
    my $code = "";

    my $implClassName = GetImplName($interface);
    my $attrExt = $function->extendedAttributes;
    my $name = $function->name;

    my $property_attributes = "v8::DontDelete";
    if ($attrExt->{"NotEnumerable"}) {
        $property_attributes .= " | v8::DontEnum";
    }
    if ($attrExt->{"ReadOnly"}) {
        $property_attributes .= " | v8::ReadOnly";
    }

    my $commentInfo = "Function '$name' (ExtAttr: '" . join(' ', keys(%{$attrExt})) . "')";

    my $template = "proto";
    if ($attrExt->{"Unforgeable"}) {
        $template = "instance";
    }
    if ($function->isStatic) {
        $template = "desc";
    }

    my $conditional = "";
    if ($attrExt->{"EnabledAtRuntime"}) {
        # Only call Set()/SetAccessor() if this method should be enabled
        my $enable_function = GetRuntimeEnableFunctionName($function);
        $conditional = "if (${enable_function}())\n        ";
    }
    if ($attrExt->{"EnabledPerContext"}) {
        # Only call Set()/SetAccessor() if this method should be enabled
        my $enable_function = GetContextEnableFunction($function);
        $conditional = "if (${enable_function}(impl->document()))\n        ";
    }

    if ($interface->extendedAttributes->{"CheckSecurity"} && $attrExt->{"DoNotCheckSecurity"}) {
        my $setter = $attrExt->{"ReadOnly"} ? "0" : "${implClassName}V8Internal::${implClassName}DomainSafeFunctionSetter";
        # Functions that are marked DoNotCheckSecurity are always readable but if they are changed
        # and then accessed on a different domain we do not return the underlying value but instead
        # return a new copy of the original function. This is achieved by storing the changed value
        # as hidden property.
        $code .= <<END;

    // $commentInfo
    ${conditional}$template->SetAccessor(v8::String::NewSymbol("$name"), ${implClassName}V8Internal::${name}AttrGetterCallback, ${setter}, v8Undefined(), v8::ALL_CAN_READ, static_cast<v8::PropertyAttribute>($property_attributes));
END
        return $code;
    }

    my $signature = "defaultSignature";
    if ($attrExt->{"DoNotCheckSignature"} || $function->isStatic) {
       $signature = "v8::Local<v8::Signature>()";
    }

    if (RequiresCustomSignature($function)) {
        $signature = "${name}Signature";
        $code .= "\n    // Custom Signature '$name'\n" . CreateCustomSignature($function);
    }

    if ($property_attributes eq "v8::DontDelete") {
        $property_attributes = "";
    } else {
        $property_attributes = ", static_cast<v8::PropertyAttribute>($property_attributes)";
    }

    if ($template eq "proto" && $conditional eq "" && $signature eq "defaultSignature" && $property_attributes eq "") {
        die "This shouldn't happen: Class '$implClassName' $commentInfo\n";
    }

    my $functionLength = GetFunctionLength($function);

    my $conditionalString = GenerateConditionalString($function);
    $code .= "#if ${conditionalString}\n" if $conditionalString;
    if ($function->extendedAttributes->{"PerWorldBindings"}) {
        $code .= "    if (currentWorldType == MainWorld) {\n";
        $code .= "        ${conditional}$template->Set(v8::String::NewSymbol(\"$name\"), v8::FunctionTemplate::New(${implClassName}V8Internal::${name}MethodCallbackForMainWorld, v8Undefined(), ${signature}, $functionLength)$property_attributes);\n";
        $code .= "    } else {\n";
        $code .= "        ${conditional}$template->Set(v8::String::NewSymbol(\"$name\"), v8::FunctionTemplate::New(${implClassName}V8Internal::${name}MethodCallback, v8Undefined(), ${signature}, $functionLength)$property_attributes);\n";
        $code .= "    }\n";
    } else {
        $code .= "    ${conditional}$template->Set(v8::String::NewSymbol(\"$name\"), v8::FunctionTemplate::New(${implClassName}V8Internal::${name}MethodCallback, v8Undefined(), ${signature}, $functionLength)$property_attributes);\n";
    }
    $code .= "#endif // ${conditionalString}\n" if $conditionalString;
    return $code;
}

sub GenerateIsNullExpression
{
    my $type = shift;
    my $variableName = shift;
    if (IsUnionType($type)) {
        my $types = $type->unionMemberTypes;
        my @expression = ();
        for my $i (0 .. scalar(@$types)-1) {
            my $unionMemberType = $types->[$i];
            my $unionMemberVariable = $variableName . $i;
            my $isNull = GenerateIsNullExpression($unionMemberType, $unionMemberVariable);
            push @expression, $isNull;
        }
        return join " && ", @expression;
    }
    if (IsRefPtrType($type)) {
        return "!${variableName}";
    } elsif ($type eq "DOMString") {
        return "${variableName}.isNull()";
    } else {
        return "";
    }
}

sub GenerateIfElseStatement
{
    my $type = shift;
    my $outputVariableName = shift;
    my $conditions = shift;
    my $statements = shift;

    my $code = "";
    if (@$conditions == 1) {
        $code .= "    ${type} ${outputVariableName} = " . $statements->[0] . "\n";
    } else {
        $code .= "    ${type} ${outputVariableName};\n";
        for my $i (0 .. @$conditions - 1) {
            my $token = "else if";
            $token = "if" if $i == 0;
            $token = "else" if $i == @$conditions - 1;
            $code .= "    ${token}";
            $code .= " (" . $conditions->[$i] . ")" if $conditions->[$i];
            $code .= "\n";
            $code .= "        ${outputVariableName} = " . $statements->[$i] . "\n";
        }
    }
    return $code;
}

sub GenerateImplementationIndexedPropertyAccessors
{
    my $interface = shift;
    my $interfaceName = $interface->name;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);

    my $indexedGetterFunction = GetIndexedGetterFunction($interface);
    my $hasCustomIndexedGetter = $indexedGetterFunction ? $indexedGetterFunction->extendedAttributes->{"Custom"} : 0;
    if ($indexedGetterFunction && !$hasCustomIndexedGetter) {
        GenerateImplementationIndexedPropertyGetter($interface, $indexedGetterFunction);
    }

    my $indexedSetterFunction = GetIndexedSetterFunction($interface);
    my $hasCustomIndexedSetter = $indexedSetterFunction ? $indexedSetterFunction->extendedAttributes->{"Custom"} : 0;
    if ($indexedSetterFunction && !$hasCustomIndexedSetter) {
        GenerateImplementationIndexedPropertySetter($interface, $indexedSetterFunction);
    }

    my $indexedDeleterFunction = GetIndexedDeleterFunction($interface);
    my $hasCustomIndexedDeleter = $indexedDeleterFunction ? $indexedDeleterFunction->extendedAttributes->{"Custom"} : 0;
    if ($indexedDeleterFunction && !$hasCustomIndexedDeleter) {
        GenerateImplementationIndexedPropertyDeleter($interface, $indexedDeleterFunction);
    }

    my $indexedEnumeratorFunction = $indexedGetterFunction;
    $indexedEnumeratorFunction = 0 if $indexedGetterFunction && $indexedGetterFunction->extendedAttributes->{"NotEnumerable"};

    my $indexedQueryFunction = 0;
    # If there is an enumerator, there MUST be a query method to properly communicate property attributes.
    my $hasQuery = $indexedQueryFunction || $indexedEnumeratorFunction;

    my $setOn = "Instance";

    # V8 has access-check callback API (see ObjectTemplate::SetAccessCheckCallbacks) and it's used on Window
    # instead of deleters or enumerators. In addition, the getter should be set on prototype template, to
    # get implementation straight out of the Window prototype regardless of what prototype is actually set
    # on the object.
    if ($interfaceName eq "Window") {
        $setOn = "Prototype";
    }

    my $code = "";
    if ($indexedGetterFunction || $indexedSetterFunction || $indexedDeleterFunction || $indexedEnumeratorFunction || $hasQuery) {
        $code .= "    desc->${setOn}Template()->SetIndexedPropertyHandler(${v8ClassName}::indexedPropertyGetter";
        $code .= $indexedSetterFunction ? ", ${v8ClassName}::indexedPropertySetter" : ", 0";
        $code .= ", 0"; # IndexedPropertyQuery -- not being used at the moment.
        $code .= $indexedDeleterFunction ? ", ${v8ClassName}::indexedPropertyDeleter" : ", 0";
        $code .= ", nodeCollectionIndexedPropertyEnumerator<${implClassName}>" if $indexedEnumeratorFunction;
        $code .= ");\n";
    }

    return $code;
}

sub GenerateImplementationIndexedPropertyGetter
{
    my $interface = shift;
    my $indexedGetterFunction = shift;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $methodName = GetImplName($indexedGetterFunction);

    AddToImplIncludes("bindings/v8/V8Collection.h");
    my $returnType = $indexedGetterFunction->type;
    my $nativeType = GetNativeType($returnType);
    my $nativeValue = "element";
    $nativeValue .= ".release()" if (IsRefPtrType($returnType));
    my $isNull = GenerateIsNullExpression($returnType, "element");
    my $returnJSValueCode = NativeToJSValue($indexedGetterFunction->type, $indexedGetterFunction->extendedAttributes, $nativeValue, "    ", "", "info.Holder()", "info.GetIsolate()", "info", "collection", "", "", "return");
    my $raisesExceptions = $indexedGetterFunction->extendedAttributes->{"RaisesException"};
    my $methodCallCode = GenerateMethodCall($returnType, "element", "collection->${methodName}", "index", $raisesExceptions);
    my $getterCode = "void ${v8ClassName}::indexedPropertyGetter(uint32_t index, const v8::PropertyCallbackInfo<v8::Value>& info)\n";
    $getterCode .= "{\n";
    $getterCode .= "    ASSERT(V8DOMWrapper::maybeDOMWrapper(info.Holder()));\n";
    $getterCode .= "    ${implClassName}* collection = toNative(info.Holder());\n";
    if ($raisesExceptions) {
        $getterCode .= "    ExceptionCode ec = 0;\n";
    }
    $getterCode .= $methodCallCode . "\n";
    if ($raisesExceptions) {
        $getterCode .= "    if (ec) {\n";
        $getterCode .= "        setDOMException(ec, info.GetIsolate());\n";
        $getterCode .= "        return;\n";
        $getterCode .= "    }\n";
    }
    if (IsUnionType($returnType)) {
        $getterCode .= "${returnJSValueCode}\n";
        $getterCode .= "    return;\n";
    } else {
        $getterCode .= "    if (${isNull})\n";
        $getterCode .= "        return;\n";
        $getterCode .= $returnJSValueCode . "\n";
    }
    $getterCode .= "}\n\n";
    $implementation{nameSpaceWebCore}->add($getterCode);
}

sub GenerateImplementationIndexedPropertySetter
{
    my $interface = shift;
    my $indexedSetterFunction = shift;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $methodName = GetImplName($indexedSetterFunction);

    AddToImplIncludes("bindings/v8/V8Collection.h");
    my $type = $indexedSetterFunction->parameters->[1]->type;
    my $raisesExceptions = $indexedSetterFunction->extendedAttributes->{"RaisesException"};
    my $treatNullAs = $indexedSetterFunction->parameters->[1]->extendedAttributes->{"TreatNullAs"};
    my $treatUndefinedAs = $indexedSetterFunction->parameters->[1]->extendedAttributes->{"TreatUndefinedAs"};
    my $code = "void ${v8ClassName}::indexedPropertySetter(uint32_t index, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info)\n";
    $code .= "{\n";
    $code .= "    ${implClassName}* collection = toNative(info.Holder());\n";
    $code .= GenerateNativeValueDefinition($indexedSetterFunction, $indexedSetterFunction->parameters->[1], "value", "propertyValue", "info.GetIsolate()");

    my $extraArguments = "";
    if ($raisesExceptions) {
        $code .= "    ExceptionCode ec = 0;\n";
        $extraArguments = ", ec";
    }
    my $passNativeValue = "propertyValue";
    $passNativeValue .= ".release()" if (IsRefPtrType($type));

    my @conditions = ();
    my @statements = ();
    if ($treatNullAs && $treatNullAs ne "NullString") {
        push @conditions, "value->IsNull()";
        push @statements, "collection->${treatNullAs}(index$extraArguments);";
    }
    if ($treatUndefinedAs && $treatUndefinedAs ne "NullString") {
        push @conditions, "value->IsUndefined()";
        push @statements, "collection->${treatUndefinedAs}(index$extraArguments);";
    }
    push @conditions, "";
    push @statements, "collection->${methodName}(index, $passNativeValue$extraArguments);";
    $code .= GenerateIfElseStatement("bool", "result", \@conditions, \@statements);

    $code .= "    if (!result)\n";
    $code .= "        return;\n";
    if ($raisesExceptions) {
        $code .= "    if (ec) {\n";
        $code .= "        setDOMException(ec, info.GetIsolate());\n";
        $code .= "        return;\n";
        $code .= "    }\n";
    }
    $code .= "    v8SetReturnValue(info, value);\n";
    $code .= "}\n\n";
    $implementation{nameSpaceWebCore}->add($code);
}

sub GenerateImplementationNamedPropertyAccessors
{
    my $interface = shift;

    my $interfaceName = $interface->name;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);

    my $namedGetterFunction = GetNamedGetterFunction($interface);
    my $hasCustomNamedGetter = $namedGetterFunction ? $namedGetterFunction->extendedAttributes->{"Custom"} : 0;
    if ($namedGetterFunction && !$hasCustomNamedGetter) {
        GenerateImplementationNamedPropertyGetter($interface, $namedGetterFunction);
    }

    my $namedSetterFunction = GetNamedSetterFunction($interface);
    my $hasCustomNamedSetter = $namedSetterFunction ? $namedSetterFunction->extendedAttributes->{"Custom"} : 0;
    if ($namedSetterFunction && !$hasCustomNamedSetter) {
        GenerateImplementationNamedPropertySetter($interface, $namedSetterFunction);
    }

    my $namedDeleterFunction = GetNamedDeleterFunction($interface);
    my $hasCustomNamedDeleter = $namedDeleterFunction ? $namedDeleterFunction->extendedAttributes->{"Custom"} : 0;
    if ($namedDeleterFunction && !$hasCustomNamedDeleter) {
        GenerateImplementationNamedPropertyDeleter($interface, $namedDeleterFunction);
    }
    my $hasDeleter = $namedDeleterFunction;

    my $namedEnumeratorFunction = $namedGetterFunction;
    $namedEnumeratorFunction = 0 if $namedGetterFunction && $namedGetterFunction->extendedAttributes->{"NotEnumerable"};
    my $hasCustomNamedEnumerator = 1 if $namedGetterFunction && $namedGetterFunction->extendedAttributes->{"CustomEnumerateProperty"};
    if ($namedEnumeratorFunction && !$hasCustomNamedEnumerator) {
        GenerateImplementationNamedPropertyEnumerator($interface);
    }

    # If there is an enumerator, there MUST be a query method to properly communicate property attributes.
    my $hasQuery = $namedEnumeratorFunction;
    my $hasCustomNamedQuery = $hasCustomNamedEnumerator;
    if ($hasQuery && !$hasCustomNamedQuery) {
        GenerateImplementationNamedPropertyQuery($interface);
    }

    my $subCode = "";
    if ($namedGetterFunction || $namedSetterFunction || $namedDeleterFunction || $namedEnumeratorFunction || $hasQuery) {
        my $setOn = "Instance";

        # V8 has access-check callback API (see ObjectTemplate::SetAccessCheckCallbacks) and it's used on Window
        # instead of deleters or enumerators. In addition, the getter should be set on prototype template, to
        # get implementation straight out of the Window prototype regardless of what prototype is actually set
        # on the object.
        if ($interfaceName eq "Window") {
            $setOn = "Prototype";
        }

        $subCode .= "    desc->${setOn}Template()->SetNamedPropertyHandler(";
        $subCode .= $namedGetterFunction ? "${v8ClassName}::namedPropertyGetter, " : "0, ";
        $subCode .= $namedSetterFunction ? "${v8ClassName}::namedPropertySetter, " : "0, ";
        $subCode .= $hasQuery ? "${v8ClassName}::namedPropertyQuery, " : "0, ";
        $subCode .= $hasDeleter ? "${v8ClassName}::namedPropertyDeleter, " : "0, ";
        $subCode .= $namedEnumeratorFunction ? "${v8ClassName}::namedPropertyEnumerator" : "0";
        $subCode .= ");\n";
    }

    return $subCode;
}

sub GenerateMethodCall
{
    my $returnType = shift; # string or UnionType
    my $returnName = shift;
    my $functionExpression = shift;
    my $firstArgument = shift;
    my $raisesExceptions = shift;

    my @arguments = ();
    push @arguments, $firstArgument;
    if ($raisesExceptions) {
        push @arguments, "ec";
    }

    if (IsUnionType($returnType)) {
        my $code = "";
        my @extraArguments = ();
        for my $i (0..scalar(@{$returnType->unionMemberTypes})-1) {
            my $unionMemberType = $returnType->unionMemberTypes->[$i];
            my $nativeType = GetNativeType($unionMemberType);
            my $unionMemberVariable = $returnName . $i;
            my $unionMemberEnabledVariable = $returnName . $i . "Enabled";
            $code .= "    bool ${unionMemberEnabledVariable} = false;\n";
            $code .= "    ${nativeType} ${unionMemberVariable};\n";
            push @extraArguments, $unionMemberEnabledVariable;
            push @extraArguments, $unionMemberVariable;
        }
        push @arguments, @extraArguments;
        $code .= "    ${functionExpression}(" . (join ", ", @arguments) . ");";
        return $code;
    } else {
        my $nativeType = GetNativeType($returnType);
        return "    ${nativeType} element = ${functionExpression}(" . (join ", ", @arguments) . ");"
    }
}

sub GenerateImplementationNamedPropertyGetter
{
    my $interface = shift;
    my $namedGetterFunction = shift;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $methodName = GetImplName($namedGetterFunction);

    AddToImplIncludes("bindings/v8/V8Collection.h");
    my $returnType = $namedGetterFunction->type;
    my $isNull = GenerateIsNullExpression($returnType, "element");
    my $nativeValue = "element";
    $nativeValue .= ".release()" if (IsRefPtrType($returnType));
    my $returnJSValueCode = NativeToJSValue($namedGetterFunction->type, $namedGetterFunction->extendedAttributes, $nativeValue, "    ", "", "info.Holder()", "info.GetIsolate()", "info", "collection", "", "", "return");
    my $raisesExceptions = $namedGetterFunction->extendedAttributes->{"RaisesException"};
    my $methodCallCode = GenerateMethodCall($returnType, "element", "collection->${methodName}", "propertyName", $raisesExceptions);

    my $code = "void ${v8ClassName}::namedPropertyGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)\n";
    $code .= "{\n";
    if (!$namedGetterFunction->extendedAttributes->{"OverrideBuiltins"}) {
        $code .= "    if (!info.Holder()->GetRealNamedPropertyInPrototypeChain(name).IsEmpty())\n";
        $code .= "        return;\n";
        $code .= "    if (info.Holder()->HasRealNamedCallbackProperty(name))\n";
        $code .= "        return;\n";
        $code .= "    if (info.Holder()->HasRealNamedProperty(name))\n";
        $code .= "        return;\n";
    }
    $code .= "\n";
    $code .= "    ASSERT(V8DOMWrapper::maybeDOMWrapper(info.Holder()));\n";
    $code .= "    ${implClassName}* collection = toNative(info.Holder());\n";
    $code .= "    AtomicString propertyName = toWebCoreAtomicString(name);\n";
    if ($raisesExceptions) {
        $code .= "    ExceptionCode ec = 0;\n";
    }
    $code .= $methodCallCode . "\n";
    if ($raisesExceptions) {
        $code .= "    if (ec) {\n";
        $code .= "        setDOMException(ec, info.GetIsolate());\n";
        $code .= "        return;\n";
        $code .= "    }\n";
    }
    if (IsUnionType($returnType)) {
        $code .= "${returnJSValueCode}\n";
        $code .= "    return;\n";
    } else {
        $code .= "    if (${isNull})\n";
        $code .= "        return;\n";
        $code .= $returnJSValueCode . "\n";
    }
    $code .= "}\n\n";
    $implementation{nameSpaceWebCore}->add($code);
}

sub GenerateNativeValueDefinition
{
    my $function = shift;
    my $parameter = shift;
    my $jsValue = shift;
    my $nativeValueName = shift;
    my $getIsolate = shift;

    my $treatNullAs = $parameter->extendedAttributes->{"TreatNullAs"} || "";
    my $treatUndefinedAs = $parameter->extendedAttributes->{"TreatUndefinedAs"} || "";
    my $code = "";
    my $nativeType = GetNativeType($parameter->type);
    my $nativeValue = JSValueToNative($parameter->type, $function->extendedAttributes, $jsValue, $getIsolate);
    if ($parameter->type eq "DOMString") {
        my $nullCheck = "";
        if ($treatNullAs eq "NullString") {
            $nullCheck = "WithUndefinedOrNullCheck";
            if ($treatUndefinedAs eq "NullString") {
                $nullCheck = "WithNullCheck";
            }
        }
        $code .= "    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<${nullCheck}>, ${nativeValueName}, ${jsValue});\n";
    } else {
        $code .= "    ${nativeType} ${nativeValueName} = ${nativeValue};\n";
    }
    return $code;
}

sub GenerateImplementationNamedPropertySetter
{
    my $interface = shift;
    my $namedSetterFunction = shift;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $methodName = GetImplName($namedSetterFunction);

    AddToImplIncludes("bindings/v8/V8Collection.h");
    my $raisesExceptions = $namedSetterFunction->extendedAttributes->{"RaisesException"};
    my $treatNullAs = $namedSetterFunction->parameters->[1]->extendedAttributes->{"TreatNullAs"};
    my $treatUndefinedAs = $namedSetterFunction->parameters->[1]->extendedAttributes->{"TreatUndefinedAs"};

    my $code = "void ${v8ClassName}::namedPropertySetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info)\n";
    $code .= "{\n";
    if (!$namedSetterFunction->extendedAttributes->{"OverrideBuiltins"}) {
        $code .= "    if (!info.Holder()->GetRealNamedPropertyInPrototypeChain(name).IsEmpty())\n";
        $code .= "        return;\n";
        $code .= "    if (info.Holder()->HasRealNamedCallbackProperty(name))\n";
        $code .= "        return;\n";
        $code .= "    if (info.Holder()->HasRealNamedProperty(name))\n";
        $code .= "        return;\n";
    }
    $code .= "    ${implClassName}* collection = toNative(info.Holder());\n";
    $code .= GenerateNativeValueDefinition($namedSetterFunction, $namedSetterFunction->parameters->[0], "name", "propertyName", "info.GetIsolate()");
    $code .= GenerateNativeValueDefinition($namedSetterFunction, $namedSetterFunction->parameters->[1], "value", "propertyValue", "info.GetIsolate()");
    my $extraArguments = "";
    if ($raisesExceptions) {
        $code .= "    ExceptionCode ec = 0;\n";
        $extraArguments = ", ec";
    }

    my @conditions = ();
    my @statements = ();
    if ($treatNullAs && $treatNullAs ne "NullString") {
        push @conditions, "value->IsNull()";
        push @statements, "collection->${treatNullAs}(propertyName$extraArguments);";
    }
    if ($treatUndefinedAs && $treatUndefinedAs ne "NullString") {
        push @conditions, "value->IsUndefined()";
        push @statements, "collection->${treatUndefinedAs}(propertyName$extraArguments);";
    }
    push @conditions, "";
    push @statements, "collection->${methodName}(propertyName, propertyValue$extraArguments);";
    $code .= GenerateIfElseStatement("bool", "result", \@conditions, \@statements);

    $code .= "    if (!result)\n";
    $code .= "        return;\n";
    if ($raisesExceptions) {
        $code .= "    if (ec) {\n";
        $code .= "        setDOMException(ec, info.GetIsolate());\n";
        $code .= "        return;\n";
        $code .= "    }\n";
    }
    $code .= "    v8SetReturnValue(info, value);\n";
    $code .= "}\n\n";
    $implementation{nameSpaceWebCore}->add($code);
}

sub GenerateImplementationIndexedPropertyDeleter
{
    my $interface = shift;
    my $indexedDeleterFunction = shift;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $methodName = GetImplName($indexedDeleterFunction);

    my $raisesExceptions = $indexedDeleterFunction->extendedAttributes->{"RaisesException"};

    my $code = "void ${v8ClassName}::indexedPropertyDeleter(unsigned index, const v8::PropertyCallbackInfo<v8::Boolean>& info)\n";
    $code .= "{\n";
    $code .= "    ${implClassName}* collection = toNative(info.Holder());\n";
    my $extraArguments = "";
    if ($raisesExceptions) {
        $code .= "    ExceptionCode ec = 0;\n";
        $extraArguments = ", ec";
    }
    $code .= "    bool result = collection->${methodName}(index$extraArguments);\n";
    if ($raisesExceptions) {
        $code .= "    if (ec) {\n";
        $code .= "        setDOMException(ec, info.GetIsolate());\n";
        $code .= "        return;\n";
        $code .= "    }\n";
    }
    $code .= "    return v8SetReturnValueBool(info, result);\n";
    $code .= "}\n\n";
    $implementation{nameSpaceWebCore}->add($code);
}

sub GenerateImplementationNamedPropertyDeleter
{
    my $interface = shift;
    my $namedDeleterFunction = shift;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $methodName = GetImplName($namedDeleterFunction);

    my $raisesExceptions = $namedDeleterFunction->extendedAttributes->{"RaisesException"};

    my $code = "void ${v8ClassName}::namedPropertyDeleter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Boolean>& info)\n";
    $code .= "{\n";
    $code .= "    ${implClassName}* collection = toNative(info.Holder());\n";
    $code .= "    AtomicString propertyName = toWebCoreAtomicString(name);\n";
    my $extraArguments = "";
    if ($raisesExceptions) {
        $code .= "    ExceptionCode ec = 0;\n";
        $extraArguments = ", ec";
    }
    $code .= "    bool result = collection->${methodName}(propertyName$extraArguments);\n";
    if ($raisesExceptions) {
        $code .= "    if (ec) {\n";
        $code .= "        setDOMException(ec, info.GetIsolate());\n";
        $code .= "        return;\n";
        $code .= "    }\n";
    }
    $code .= "    return v8SetReturnValueBool(info, result);\n";
    $code .= "}\n\n";
    $implementation{nameSpaceWebCore}->add($code);
}

sub GenerateImplementationNamedPropertyEnumerator
{
    my $interface = shift;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);

    $implementation{nameSpaceWebCore}->add(<<END);
void ${v8ClassName}::namedPropertyEnumerator(const v8::PropertyCallbackInfo<v8::Array>& info)
{
    ExceptionCode ec = 0;
    ${implClassName}* collection = toNative(info.Holder());
    Vector<String> names;
    collection->namedPropertyEnumerator(names, ec);
    if (ec) {
        setDOMException(ec, info.GetIsolate());
        return;
    }
    v8::Handle<v8::Array> v8names = v8::Array::New(names.size());
    for (size_t i = 0; i < names.size(); ++i)
        v8names->Set(v8Integer(i, info.GetIsolate()), v8String(names[i], info.GetIsolate()));
    v8SetReturnValue(info, v8names);
}

END
}

sub GenerateImplementationNamedPropertyQuery
{
    my $interface = shift;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);

    $implementation{nameSpaceWebCore}->add(<<END);
void ${v8ClassName}::namedPropertyQuery(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Integer>& info)
{
    ${implClassName}* collection = toNative(info.Holder());
    AtomicString propertyName = toWebCoreAtomicString(name);
    ExceptionCode ec = 0;
    bool result = collection->namedPropertyQuery(propertyName, ec);
    if (ec) {
        setDOMException(ec, info.GetIsolate());
        return;
    }
    if (!result)
        return;
    v8SetReturnValueInt(info, v8::None);
}

END
}

sub GenerateImplementationLegacyCall
{
    my $interface = shift;
    my $code = "";

    my $v8ClassName = GetV8ClassName($interface);

    if ($interface->extendedAttributes->{"CustomLegacyCall"}) {
        $code .= "    desc->InstanceTemplate()->SetCallAsFunctionHandler(${v8ClassName}::legacyCallCustom);\n";
    }
    return $code;
}

sub GenerateImplementationMasqueradesAsUndefined
{
    my $interface = shift;
    my $code = "";

    if ($interface->extendedAttributes->{"MasqueradesAsUndefined"})
    {
        $code .= "    desc->InstanceTemplate()->MarkAsUndetectable();\n";
    }
    return $code;
}

sub GenerateImplementation
{
    my $object = shift;
    my $interface = shift;
    my $interfaceName = $interface->name;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);
    my $nativeType = GetNativeTypeForConversions($interface);

    AddToImplIncludes("bindings/v8/V8Binding.h");
    AddToImplIncludes("bindings/v8/V8DOMWrapper.h");
    AddToImplIncludes("core/dom/ContextFeatures.h");
    AddToImplIncludes("core/dom/Document.h");
    AddToImplIncludes("RuntimeEnabledFeatures.h");

    AddIncludesForType($interfaceName);

    my $toActiveDOMObject = InheritsExtendedAttribute($interface, "ActiveDOMObject") ? "${v8ClassName}::toActiveDOMObject" : "0";
    my $toEventTarget = InheritsExtendedAttribute($interface, "EventTarget") ? "${v8ClassName}::toEventTarget" : "0";
    my $rootForGC = NeedsOpaqueRootForGC($interface) ? "${v8ClassName}::opaqueRootForGC" : "0";

    # Find the super descriptor.
    my $parentClass = "";
    my $parentClassTemplate = "";
    foreach (@{$interface->parents}) {
        my $parent = $_;
        AddToImplIncludes("V8${parent}.h");
        $parentClass = "V8" . $parent;
        $parentClassTemplate = $parentClass . "::GetTemplate(isolate, currentWorldType)";
        last;
    }

    my $parentClassInfo = $parentClass ? "&${parentClass}::info" : "0";
    my $WrapperTypePrototype = $interface->isException ? "WrapperTypeErrorPrototype" : "WrapperTypeObjectPrototype";

    if (!IsSVGTypeNeedingTearOff($interfaceName)) {
        my $code = <<END;
static void initializeScriptWrappableForInterface(${implClassName}* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::setTypeInfoInObject(object, &${v8ClassName}::info);
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

// In ScriptWrappable::init, the use of a local function declaration has an issue on Windows:
// the local declaration does not pick up the surrounding namespace. Therefore, we provide this function
// in the global namespace.
// (More info on the MSVC bug here: http://connect.microsoft.com/VisualStudio/feedback/details/664619/the-namespace-of-local-function-declarations-in-c)
END

    if (GetNamespaceForInterface($interface) eq "WebCore") {
        $code .= "void webCoreInitializeScriptWrappableForInterface(WebCore::${implClassName}* object)\n";
    } else {
        $code .= "void webCoreInitializeScriptWrappableForInterface(${implClassName}* object)\n";
    }

    $code .= <<END;
{
    WebCore::initializeScriptWrappableForInterface(object);
}

namespace WebCore {
END
        $implementation{nameSpaceWebCore}->addHeader($code);
    }

    my $code = "WrapperTypeInfo ${v8ClassName}::info = { ${v8ClassName}::GetTemplate, ${v8ClassName}::derefObject, $toActiveDOMObject, $toEventTarget, ";
    $code .= "$rootForGC, ${v8ClassName}::installPerContextPrototypeProperties, $parentClassInfo, $WrapperTypePrototype };\n";
    $implementation{nameSpaceWebCore}->addHeader($code);

    $implementation{nameSpaceInternal}->add("template <typename T> void V8_USE(T) { }\n\n");

    my $hasConstructors = 0;
    my $hasReplaceable = 0;

    # Generate property accessors for attributes.
    for (my $index = 0; $index < @{$interface->attributes}; $index++) {
        my $attribute = @{$interface->attributes}[$index];
        my $attrType = $attribute->type;
        my $attrExt = $attribute->extendedAttributes;

        # Generate special code for the constructor attributes.
        if ($attrType =~ /Constructor$/) {
            if (!HasCustomGetter($attrExt)) {
                $hasConstructors = 1;
            }
            next;
        }

        if ($attrType eq "EventListener" && $interfaceName eq "Window") {
            $attrExt->{"OnProto"} = 1;
        }

        if ($attrType eq "SerializedScriptValue") {
            AddToImplIncludes("bindings/v8/SerializedScriptValue.h");
        }

        GenerateNormalAttrGetter($attribute, $interface, "");
        GenerateNormalAttrGetterCallback($attribute, $interface, "");
        if ($attrExt->{"PerWorldBindings"}) {
            GenerateNormalAttrGetter($attribute, $interface, "ForMainWorld");
            GenerateNormalAttrGetterCallback($attribute, $interface, "ForMainWorld");
        }
        if (!HasCustomSetter($attrExt) && $attrExt->{"Replaceable"}) {
            $hasReplaceable = 1;
        } elsif (!IsReadonly($attribute)) {
            GenerateNormalAttrSetter($attribute, $interface, "");
            GenerateNormalAttrSetterCallback($attribute, $interface, "");
            if ($attrExt->{"PerWorldBindings"}) {
              GenerateNormalAttrSetter($attribute, $interface, "ForMainWorld");
              GenerateNormalAttrSetterCallback($attribute, $interface, "ForMainWorld");
            }
        }
    }

    if ($hasConstructors) {
        GenerateConstructorGetter($interface);
    }

    if ($hasConstructors || $hasReplaceable) {
        GenerateReplaceableAttrSetter($interface);
        GenerateReplaceableAttrSetterCallback($interface);
    }

    if (NeedsOpaqueRootForGC($interface)) {
        GenerateOpaqueRootForGC($interface);
    }

    if ($interface->extendedAttributes->{"CheckSecurity"} && $interface->name ne "Window") {
        GenerateSecurityCheckFunctions($interface);
    }

    if (IsConstructorTemplate($interface, "TypedArray")) {
        my ($nativeType, $arrayType) = GetNativeTypeOfTypedArray($interface);
        $implementation{nameSpaceWebCore}->add(<<END);
v8::Handle<v8::Object> wrap($implClassName* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    v8::Handle<v8::Object> wrapper = ${v8ClassName}::createWrapper(impl, creationContext, isolate);
    if (!wrapper.IsEmpty())
        wrapper->SetIndexedPropertiesToExternalArrayData(impl->baseAddress(), $arrayType, impl->length());
    return wrapper;
}

END
    }

    my @enabledPerContextFunctions;
    my @normalFunctions;
    my $needsDomainSafeFunctionSetter = 0;
    # Generate methods for functions.
    foreach my $function (@{$interface->functions}) {
        next if $function->name eq "";
        GenerateFunction($function, $interface, "");
        if ($function->extendedAttributes->{"PerWorldBindings"}) {
            GenerateFunction($function, $interface, "ForMainWorld");
        }
        if ($function->{overloadIndex} == @{$function->{overloads}}) {
            if ($function->{overloadIndex} > 1) {
                GenerateOverloadedFunction($function, $interface, "");
                if ($function->extendedAttributes->{"PerWorldBindings"}) {
                    GenerateOverloadedFunction($function, $interface, "ForMainWorld");
                }
            }
            GenerateFunctionCallback($function, $interface, "");
            if ($function->extendedAttributes->{"PerWorldBindings"}) {
                GenerateFunctionCallback($function, $interface, "ForMainWorld");
            }
        }

        # If the function does not need domain security check, we need to
        # generate an access getter that returns different function objects
        # for different calling context.
        if ($interface->extendedAttributes->{"CheckSecurity"} && $function->extendedAttributes->{"DoNotCheckSecurity"}) {
            if (!HasCustomMethod($function->extendedAttributes) || $function->{overloadIndex} == 1) {
                GenerateDomainSafeFunctionGetter($function, $interface);
                if (!$function->extendedAttributes->{"ReadOnly"}) {
                    $needsDomainSafeFunctionSetter = 1;
                }
            }
        }

        # Separate out functions that are enabled per context so we can process them specially.
        if ($function->extendedAttributes->{"EnabledPerContext"}) {
            push(@enabledPerContextFunctions, $function);
        } else {
            push(@normalFunctions, $function);
        }
    }

    if ($needsDomainSafeFunctionSetter) {
        GenerateDomainSafeFunctionSetter($interface);
    }

    # Attributes
    my $attributes = $interface->attributes;

    # For the Window interface we partition the attributes into the
    # ones that disallows shadowing and the rest.
    my @disallowsShadowing;
    # Also separate out attributes that are enabled at runtime so we can process them specially.
    my @enabledAtRuntimeAttributes;
    my @enabledPerContextAttributes;
    my @normalAttributes;
    foreach my $attribute (@$attributes) {

        if ($interfaceName eq "Window" && $attribute->extendedAttributes->{"Unforgeable"}) {
            push(@disallowsShadowing, $attribute);
        } elsif ($attribute->extendedAttributes->{"EnabledAtRuntime"} || $attribute->extendedAttributes->{"EnabledPerContext"}) {
            if ($attribute->extendedAttributes->{"EnabledPerContext"}) {
                push(@enabledPerContextAttributes, $attribute);
            }
            if ($attribute->extendedAttributes->{"EnabledAtRuntime"}) {
                push(@enabledAtRuntimeAttributes, $attribute);
            }
        } else {
            push(@normalAttributes, $attribute);
        }
    }
    AddToImplIncludes("bindings/v8/V8DOMConfiguration.h");
    $attributes = \@normalAttributes;
    # Put the attributes that disallow shadowing on the shadow object.
    if (@disallowsShadowing) {
        my $code = "";
        $code .= "static const V8DOMConfiguration::BatchedAttribute shadowAttrs[] = {\n";
        $code .= GenerateBatchedAttributeData($interface, \@disallowsShadowing);
        $code .= "};\n\n";
        $implementation{nameSpaceWebCore}->add($code);
    }

    my $has_attributes = 0;
    if (@$attributes) {
        $has_attributes = 1;
        my $code = "";
        $code .= "static const V8DOMConfiguration::BatchedAttribute ${v8ClassName}Attrs[] = {\n";
        $code .= GenerateBatchedAttributeData($interface, $attributes);
        $code .= "};\n\n";
        $implementation{nameSpaceWebCore}->add($code);
    }

    # Setup table of standard callback functions
    my $num_callbacks = 0;
    my $has_callbacks = 0;
    $code = "";
    foreach my $function (@normalFunctions) {
        # Only one table entry is needed for overloaded methods:
        next if $function->{overloadIndex} > 1;
        # Don't put any nonstandard functions into this table:
        next if !IsStandardFunction($interface, $function);
        next if $function->name eq "";
        if (!$has_callbacks) {
            $has_callbacks = 1;
            $code .= "static const V8DOMConfiguration::BatchedMethod ${v8ClassName}Methods[] = {\n";
        }
        my $name = $function->name;
        my $methodForMainWorld = "0";
        if ($function->extendedAttributes->{"PerWorldBindings"}) {
            $methodForMainWorld = "${implClassName}V8Internal::${name}MethodCallbackForMainWorld";
        }
        my $functionLength = GetFunctionLength($function);
        my $conditionalString = GenerateConditionalString($function);
        $code .= "#if ${conditionalString}\n" if $conditionalString;
        $code .= <<END;
    {"$name", ${implClassName}V8Internal::${name}MethodCallback, ${methodForMainWorld}, ${functionLength}},
END
        $code .= "#endif\n" if $conditionalString;
        $num_callbacks++;
    }
    $code .= "};\n\n"  if $has_callbacks;
    $implementation{nameSpaceWebCore}->add($code);

    # Setup constants
    my $has_constants = 0;
    my @constantsEnabledAtRuntime;
    $code = "";
    if (@{$interface->constants}) {
        $has_constants = 1;
        $code .= "static const V8DOMConfiguration::BatchedConstant ${v8ClassName}Consts[] = {\n";
    }
    foreach my $constant (@{$interface->constants}) {
        my $name = $constant->name;
        my $value = $constant->value;
        my $attrExt = $constant->extendedAttributes;
        my $implementedBy = $attrExt->{"ImplementedBy"};
        if ($implementedBy) {
            my $implementedByImplName = GetImplNameFromImplementedBy($implementedBy);
            AddToImplIncludes(HeaderFilesForInterface($implementedBy, $implementedByImplName));
        }
        if ($attrExt->{"EnabledAtRuntime"}) {
            push(@constantsEnabledAtRuntime, $constant);
        } else {
            my $conditionalString = GenerateConditionalString($constant);
            $code .= "#if ${conditionalString}\n" if $conditionalString;
            # If the value we're dealing with is a hex number, preprocess it into a signed integer
            # here, rather than running static_cast<signed int> in the generated code.
            if (substr($value, 0, 2) eq "0x") {
              $value = unpack('i', pack('I', hex($value)));
            }
            $code .= <<END;
    {"${name}", $value},
END
            $code .= "#endif\n" if $conditionalString;
        }
    }
    if ($has_constants) {
        $code .= "};\n\n";
        $code .= join "", GenerateCompileTimeCheckForEnumsIfNeeded($interface);
        $implementation{nameSpaceWebCore}->add($code);
    }

    if (!HasCustomConstructor($interface)) {
        if ($interface->extendedAttributes->{"NamedConstructor"}) {
            GenerateNamedConstructor(@{$interface->constructors}[0], $interface);
        } elsif ($interface->extendedAttributes->{"Constructor"}) {
            GenerateConstructor($interface);
        } elsif (IsConstructorTemplate($interface, "Event")) {
            GenerateEventConstructor($interface);
        } elsif (IsConstructorTemplate($interface, "TypedArray")) {
            GenerateTypedArrayConstructor($interface);
        }
    }
    if (IsConstructable($interface)) {
        GenerateConstructorCallback($interface);
    }

    my $access_check = "";
    if ($interface->extendedAttributes->{"CheckSecurity"} && $interfaceName ne "Window") {
        $access_check = "instance->SetAccessCheckCallbacks(${implClassName}V8Internal::namedSecurityCheck, ${implClassName}V8Internal::indexedSecurityCheck, v8::External::New(&${v8ClassName}::info));";
    }

    # For the Window interface, generate the shadow object template
    # configuration method.
    if ($interfaceName eq "Window") {
        $implementation{nameSpaceWebCore}->add(<<END);
static void ConfigureShadowObjectTemplate(v8::Handle<v8::ObjectTemplate> templ, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    V8DOMConfiguration::batchConfigureAttributes(templ, v8::Handle<v8::ObjectTemplate>(), shadowAttrs, WTF_ARRAY_LENGTH(shadowAttrs), isolate, currentWorldType);

    // Install a security handler with V8.
    templ->SetAccessCheckCallbacks(V8Window::namedSecurityCheckCustom, V8Window::indexedSecurityCheckCustom, v8::External::New(&V8Window::info));
    templ->SetInternalFieldCount(V8Window::internalFieldCount);
}
END
    }

    if (!$parentClassTemplate) {
        $parentClassTemplate = "v8::Local<v8::FunctionTemplate>()";
    }

    # Generate the template configuration method
    $code =  <<END;
static v8::Handle<v8::FunctionTemplate> Configure${v8ClassName}Template(v8::Handle<v8::FunctionTemplate> desc, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    desc->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
END
    if ($interface->extendedAttributes->{"EnabledAtRuntime"}) {
        my $enable_function = GetRuntimeEnableFunctionName($interface);
        $code .= <<END;
    if (!${enable_function}())
        defaultSignature = V8DOMConfiguration::configureTemplate(desc, \"\", $parentClassTemplate, ${v8ClassName}::internalFieldCount, 0, 0, 0, 0, isolate, currentWorldType);
    else
END
    }
    $code .=  <<END;
    defaultSignature = V8DOMConfiguration::configureTemplate(desc, \"${interfaceName}\", $parentClassTemplate, ${v8ClassName}::internalFieldCount,
END
    # Set up our attributes if we have them
    if ($has_attributes) {
        $code .= <<END;
        ${v8ClassName}Attrs, WTF_ARRAY_LENGTH(${v8ClassName}Attrs),
END
    } else {
        $code .= <<END;
        0, 0,
END
    }

    if ($has_callbacks) {
        $code .= <<END;
        ${v8ClassName}Methods, WTF_ARRAY_LENGTH(${v8ClassName}Methods), isolate, currentWorldType);
END
    } else {
        $code .= <<END;
        0, 0, isolate, currentWorldType);
END
    }

    AddToImplIncludes("wtf/UnusedParam.h");
    $code .= <<END;
    UNUSED_PARAM(defaultSignature); // In some cases, it will not be used.
END

    if (IsConstructable($interface)) {
        $code .= "    desc->SetCallHandler(${v8ClassName}::constructorCallback);\n";
        my $interfaceLength = GetInterfaceLength($interface);
        $code .= "    desc->SetLength(${interfaceLength});\n";
    }

    if ($access_check or @enabledAtRuntimeAttributes or @normalFunctions or $has_constants) {
        $code .=  <<END;
    v8::Local<v8::ObjectTemplate> instance = desc->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> proto = desc->PrototypeTemplate();
    UNUSED_PARAM(instance); // In some cases, it will not be used.
    UNUSED_PARAM(proto); // In some cases, it will not be used.
END
    }

    if ($access_check) {
        $code .=  "    $access_check\n";
    }

    # Setup the enable-at-runtime attrs if we have them
    foreach my $runtime_attr (@enabledAtRuntimeAttributes) {
        next if grep { $_ eq $runtime_attr } @enabledPerContextAttributes;
        my $enable_function = GetRuntimeEnableFunctionName($runtime_attr);
        my $conditionalString = GenerateConditionalString($runtime_attr);
        $code .= "\n#if ${conditionalString}\n" if $conditionalString;
        $code .= "    if (${enable_function}()) {\n";
        $code .= "        static const V8DOMConfiguration::BatchedAttribute attrData =\\\n";
        $code .= GenerateSingleBatchedAttribute($interface, $runtime_attr, ";", "    ");
        $code .= <<END;
        V8DOMConfiguration::configureAttribute(instance, proto, attrData, isolate, currentWorldType);
    }
END
        $code .= "\n#endif // ${conditionalString}\n" if $conditionalString;
    }

    # Setup the enable-at-runtime constants if we have them
    foreach my $runtime_const (@constantsEnabledAtRuntime) {
        my $enable_function = GetRuntimeEnableFunctionName($runtime_const);
        my $conditionalString = GenerateConditionalString($runtime_const);
        my $name = $runtime_const->name;
        my $value = $runtime_const->value;
        $code .= "\n#if ${conditionalString}\n" if $conditionalString;
        $code .= "    if (${enable_function}()) {\n";
        $code .= <<END;
        static const V8DOMConfiguration::BatchedConstant constData = {"${name}", static_cast<signed int>(${value})};
        V8DOMConfiguration::batchConfigureConstants(desc, proto, &constData, 1, isolate);
END
        $code .= "    }\n";
        $code .= "\n#endif // ${conditionalString}\n" if $conditionalString;
    }

    $code .= GenerateImplementationIndexedPropertyAccessors($interface);
    $code .= GenerateImplementationNamedPropertyAccessors($interface);
    $code .= GenerateImplementationLegacyCall($interface);
    $code .= GenerateImplementationMasqueradesAsUndefined($interface);

    # Define our functions with Set() or SetAccessor()
    my $total_functions = 0;
    foreach my $function (@normalFunctions) {
        # Only one accessor is needed for overloaded methods:
        next if $function->{overloadIndex} > 1;
        next if $function->name eq "";

        $total_functions++;
        next if IsStandardFunction($interface, $function);
        $code .= GenerateNonStandardFunction($interface, $function);
        $num_callbacks++;
    }

    die "Wrong number of callbacks generated for $interfaceName ($num_callbacks, should be $total_functions)" if $num_callbacks != $total_functions;

    if ($has_constants) {
        $code .= <<END;
    V8DOMConfiguration::batchConfigureConstants(desc, proto, ${v8ClassName}Consts, WTF_ARRAY_LENGTH(${v8ClassName}Consts), isolate);
END
    }

    # Special cases
    if ($interfaceName eq "Window") {
        $code .= <<END;

    proto->SetInternalFieldCount(V8Window::internalFieldCount);
    desc->SetHiddenPrototype(true);
    instance->SetInternalFieldCount(V8Window::internalFieldCount);
    // Set access check callbacks, but turned off initially.
    // When a context is detached from a frame, turn on the access check.
    // Turning on checks also invalidates inline caches of the object.
    instance->SetAccessCheckCallbacks(V8Window::namedSecurityCheckCustom, V8Window::indexedSecurityCheckCustom, v8::External::New(&V8Window::info), false);
END
    }
    if ($interfaceName eq "HTMLDocument" or $interfaceName eq "DedicatedWorkerContext" or $interfaceName eq "SharedWorkerContext") {
        $code .= <<END;
    desc->SetHiddenPrototype(true);
END
    }

    $code .= <<END;

    // Custom toString template
    desc->Set(v8::String::NewSymbol("toString"), V8PerIsolateData::current()->toStringTemplate());
    return desc;
}

END
    $implementation{nameSpaceWebCore}->add($code);

    $implementation{nameSpaceWebCore}->add(<<END);
v8::Handle<v8::FunctionTemplate> ${v8ClassName}::GetTemplate(v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    V8PerIsolateData* data = V8PerIsolateData::from(isolate);
    V8PerIsolateData::TemplateMap::iterator result = data->templateMap(currentWorldType).find(&info);
    if (result != data->templateMap(currentWorldType).end())
        return result->value.newLocal(isolate);

    v8::HandleScope handleScope(isolate);
    v8::Handle<v8::FunctionTemplate> templ =
        Configure${v8ClassName}Template(data->rawTemplate(&info, currentWorldType), isolate, currentWorldType);
    data->templateMap(currentWorldType).add(&info, UnsafePersistent<v8::FunctionTemplate>(isolate, templ));
    return handleScope.Close(templ);
}

END
    $implementation{nameSpaceWebCore}->add(<<END);
bool ${v8ClassName}::HasInstance(v8::Handle<v8::Value> value, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&info, value, currentWorldType);
}

END
    $implementation{nameSpaceWebCore}->add(<<END);
bool ${v8ClassName}::HasInstanceInAnyWorld(v8::Handle<v8::Value> value, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&info, value, MainWorld)
        || V8PerIsolateData::from(isolate)->hasInstance(&info, value, IsolatedWorld)
        || V8PerIsolateData::from(isolate)->hasInstance(&info, value, WorkerWorld);
}

END

    if (@enabledPerContextAttributes) {
        my $code = "";
        $code .= <<END;
void ${v8ClassName}::installPerContextProperties(v8::Handle<v8::Object> instance, ${nativeType}* impl, v8::Isolate* isolate)
{
    v8::Local<v8::Object> proto = v8::Local<v8::Object>::Cast(instance->GetPrototype());
END

        # Setup the enable-by-settings attrs if we have them
        foreach my $runtimeAttr (@enabledPerContextAttributes) {
            my $enableFunction = GetContextEnableFunction($runtimeAttr);
            my $conditionalString = GenerateConditionalString($runtimeAttr);
            $code .= "\n#if ${conditionalString}\n" if $conditionalString;
            if (grep { $_ eq $runtimeAttr } @enabledAtRuntimeAttributes) {
                my $runtimeEnableFunction = GetRuntimeEnableFunctionName($runtimeAttr);
                $code .= "    if (${enableFunction}(impl->document()) && ${runtimeEnableFunction}()) {\n";
            } else {
                $code .= "    if (${enableFunction}(impl->document())) {\n";
            }

            $code .= "        static const V8DOMConfiguration::BatchedAttribute attrData =\\\n";
            $code .= GenerateSingleBatchedAttribute($interface, $runtimeAttr, ";", "    ");
            $code .= <<END;
        V8DOMConfiguration::configureAttribute(instance, proto, attrData, isolate);
END
            $code .= "    }\n";
            $code .= "#endif // ${conditionalString}\n" if $conditionalString;
        }
        $code .= <<END;
}

END
        $implementation{nameSpaceWebCore}->add($code);
    }

    if (@enabledPerContextFunctions) {
        my $code = "";
        $code .= <<END;
void ${v8ClassName}::installPerContextPrototypeProperties(v8::Handle<v8::Object> proto, v8::Isolate* isolate)
{
    UNUSED_PARAM(proto);
END
        # Setup the enable-by-settings functions if we have them
        $code .=  <<END;
    v8::Local<v8::Signature> defaultSignature = v8::Signature::New(GetTemplate(isolate, worldType(isolate)));
    UNUSED_PARAM(defaultSignature); // In some cases, it will not be used.

    ScriptExecutionContext* context = toScriptExecutionContext(proto->CreationContext());
END

        foreach my $runtimeFunc (@enabledPerContextFunctions) {
            my $enableFunction = GetContextEnableFunction($runtimeFunc);
            my $functionLength = GetFunctionLength($runtimeFunc);
            my $conditionalString = GenerateConditionalString($runtimeFunc);
            $code .= "\n#if ${conditionalString}\n" if $conditionalString;
            $code .= "    if (context && context->isDocument() && ${enableFunction}(toDocument(context)))\n";
            my $name = $runtimeFunc->name;
            $code .= <<END;
        proto->Set(v8::String::NewSymbol("${name}"), v8::FunctionTemplate::New(${implClassName}V8Internal::${name}MethodCallback, v8Undefined(), defaultSignature, $functionLength)->GetFunction());
END
            $code .= "#endif // ${conditionalString}\n" if $conditionalString;
        }

        $code .= <<END;
}

END
        $implementation{nameSpaceWebCore}->add($code);
    }

    if (InheritsExtendedAttribute($interface, "ActiveDOMObject")) {
        # MessagePort is handled like an active dom object even though it doesn't inherit
        # from ActiveDOMObject, so don't try to cast it to ActiveDOMObject.
        my $returnValue = $interfaceName eq "MessagePort" ? "0" : "toNative(object)";
        $implementation{nameSpaceWebCore}->add(<<END);
ActiveDOMObject* ${v8ClassName}::toActiveDOMObject(v8::Handle<v8::Object> object)
{
    return $returnValue;
}

END
    }

    if (InheritsExtendedAttribute($interface, "EventTarget")) {
        $implementation{nameSpaceWebCore}->add(<<END);
EventTarget* ${v8ClassName}::toEventTarget(v8::Handle<v8::Object> object)
{
    return toNative(object);
}

END
    }

    if ($interfaceName eq "Window") {
        $implementation{nameSpaceWebCore}->add(<<END);
v8::Handle<v8::ObjectTemplate> V8Window::GetShadowObjectTemplate(v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    if (currentWorldType == MainWorld) {
        static v8::Persistent<v8::ObjectTemplate> V8WindowShadowObjectCacheForMainWorld;
        if (V8WindowShadowObjectCacheForMainWorld.IsEmpty()) {
            v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
            ConfigureShadowObjectTemplate(templ, isolate, currentWorldType);
            V8WindowShadowObjectCacheForMainWorld.Reset(isolate, templ);
            return templ;
        }
        return v8::Local<v8::ObjectTemplate>::New(isolate, V8WindowShadowObjectCacheForMainWorld);
    } else {
        static v8::Persistent<v8::ObjectTemplate> V8WindowShadowObjectCacheForNonMainWorld;
        if (V8WindowShadowObjectCacheForNonMainWorld.IsEmpty()) {
            v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
            ConfigureShadowObjectTemplate(templ, isolate, currentWorldType);
            V8WindowShadowObjectCacheForNonMainWorld.Reset(isolate, templ);
            return templ;
        }
        return v8::Local<v8::ObjectTemplate>::New(isolate, V8WindowShadowObjectCacheForNonMainWorld);
    }
}

END
    }

    GenerateToV8Converters($interface, $v8ClassName, $nativeType);

    $implementation{nameSpaceWebCore}->add(<<END);
void ${v8ClassName}::derefObject(void* object)
{
    static_cast<${nativeType}*>(object)->deref();
}

END
}

sub GenerateHeaderContentHeader
{
    my $interface = shift;
    my $v8ClassName = GetV8ClassName($interface);
    my $conditionalString = GenerateConditionalString($interface);

    my @headerContentHeader = split("\r", $headerTemplate);

    push(@headerContentHeader, "\n#ifndef ${v8ClassName}" . "_h\n");
    push(@headerContentHeader, "#define ${v8ClassName}" . "_h\n\n");
    push(@headerContentHeader, "#if ${conditionalString}\n") if $conditionalString;
    return join "", @headerContentHeader;
}

sub GenerateCallbackHeader
{
    my $object = shift;
    my $interface = shift;

    my $interfaceName = $interface->name;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);

    $header{root}->addFooter("\n");

    my @includes = ();
    push(@includes, "bindings/v8/ActiveDOMCallback.h");
    push(@includes, "bindings/v8/DOMWrapperWorld.h");
    push(@includes, "bindings/v8/ScopedPersistent.h");
    push(@includes, HeaderFilesForInterface($interfaceName, $implClassName));
    for my $include (sort @includes) {
        $header{includes}->add("#include \"$include\"\n");
    }
    $header{nameSpaceWebCore}->addHeader("\nclass ScriptExecutionContext;\n\n");
    $header{class}->addHeader("class $v8ClassName : public $implClassName, public ActiveDOMCallback {");
    $header{class}->addFooter("};\n");

    $header{classPublic}->add(<<END);
    static PassRefPtr<${v8ClassName}> create(v8::Handle<v8::Value> value, ScriptExecutionContext* context)
    {
        ASSERT(value->IsObject());
        ASSERT(context);
        return adoptRef(new ${v8ClassName}(v8::Handle<v8::Object>::Cast(value), context));
    }

    virtual ~${v8ClassName}();

END

    # Functions
    my $numFunctions = @{$interface->functions};
    if ($numFunctions > 0) {
        $header{classPublic}->add("    // Functions\n");
        foreach my $function (@{$interface->functions}) {
            my $code = "    virtual " . GetNativeTypeForCallbacks($function->type) . " " . $function->name . "(";

            my @args = ();
            my @params = @{$function->parameters};
            foreach my $param (@params) {
                push(@args, GetNativeTypeForCallbacks($param->type) . " " . $param->name);
            }
            $code .= join(", ", @args);
            $code .= ");\n";
            $header{classPublic}->add($code);
        }
    }

    $header{classPublic}->add(<<END);

    virtual ScriptExecutionContext* scriptExecutionContext() const { return ContextDestructionObserver::scriptExecutionContext(); }

END
    $header{classPrivate}->add(<<END);
    ${v8ClassName}(v8::Handle<v8::Object>, ScriptExecutionContext*);

    ScopedPersistent<v8::Object> m_callback;
    RefPtr<DOMWrapperWorld> m_world;
END
}

sub GenerateCallbackImplementation
{
    my $object = shift;
    my $interface = shift;
    my $v8ClassName = GetV8ClassName($interface);

    AddToImplIncludes("core/dom/ScriptExecutionContext.h");
    AddToImplIncludes("bindings/v8/V8Binding.h");
    AddToImplIncludes("bindings/v8/V8Callback.h");
    AddToImplIncludes("wtf/Assertions.h");

    $implementation{nameSpaceWebCore}->add(<<END);
${v8ClassName}::${v8ClassName}(v8::Handle<v8::Object> callback, ScriptExecutionContext* context)
    : ActiveDOMCallback(context)
    , m_callback(callback)
    , m_world(DOMWrapperWorld::current())
{
}

END

    $implementation{nameSpaceWebCore}->add(<<END);
${v8ClassName}::~${v8ClassName}()
{
}

END

    # Functions
    my $numFunctions = @{$interface->functions};
    if ($numFunctions > 0) {
        $implementation{nameSpaceWebCore}->add("// Functions\n");
        foreach my $function (@{$interface->functions}) {
            my $code = "";
            my @params = @{$function->parameters};
            next if $function->extendedAttributes->{"Custom"};

            AddIncludesForType($function->type);
            die "We don't yet support callbacks that return non-boolean values.\n" if $function->type ne "boolean";
            $code .= "\n" . GetNativeTypeForCallbacks($function->type) . " ${v8ClassName}::" . $function->name . "(";

            my @args = ();
            my @argsCheck = ();
            foreach my $param (@params) {
                my $paramName = $param->name;
                my $type = $param->type;
                my $arrayOrSequenceType = GetArrayOrSequenceType($type);

                if ($arrayOrSequenceType) {
                    if (IsRefPtrType($arrayOrSequenceType)) {
                        AddIncludesForType($arrayOrSequenceType);
                     }
                } else {
                    AddIncludesForType($type);
                }

                push(@args, GetNativeTypeForCallbacks($type) . " " . $paramName);
            }
            $code .= join(", ", @args);

            $code .= ")\n";
            $code .= "{\n";
            $code .= join "", @argsCheck if @argsCheck;
            $code .= "    if (!canInvokeCallback())\n";
            $code .= "        return true;\n\n";
            $code .= "    v8::HandleScope handleScope;\n\n";
            $code .= "    v8::Handle<v8::Context> v8Context = toV8Context(scriptExecutionContext(), m_world.get());\n";
            $code .= "    if (v8Context.IsEmpty())\n";
            $code .= "        return true;\n\n";
            $code .= "    v8::Context::Scope scope(v8Context);\n\n";

            @args = ();
            foreach my $param (@params) {
                my $paramName = $param->name;
                $code .= NativeToJSValue($param->type, $param->extendedAttributes, $paramName, "    ", "v8::Handle<v8::Value> ${paramName}Handle =", "v8::Handle<v8::Object>()", "v8Context->GetIsolate()", "") . "\n";
                $code .= "    if (${paramName}Handle.IsEmpty()) {\n";
                $code .= "        if (!isScriptControllerTerminating())\n";
                $code .= "            CRASH();\n";
                $code .= "        return true;\n";
                $code .= "    }\n";
                push(@args, "        ${paramName}Handle");
            }

            if (scalar(@args) > 0) {
                $code .= "\n    v8::Handle<v8::Value> argv[] = {\n";
                $code .= join(",\n", @args);
                $code .= "\n    };\n\n";
            } else {
                $code .= "\n    v8::Handle<v8::Value> *argv = 0;\n\n";
            }
            $code .= "    bool callbackReturnValue = false;\n";
            $code .= "    return !invokeCallback(m_callback.get(), " . scalar(@params) . ", argv, callbackReturnValue, scriptExecutionContext());\n";
            $code .= "}\n";
            $implementation{nameSpaceWebCore}->add($code);
        }
    }
}

sub BaseInterfaceName
{
    my $interface = shift;

    while (@{$interface->parents}) {
        $interface = ParseInterface(@{$interface->parents}[0]);
    }

    return $interface->name;
}

sub GenerateToV8Converters
{
    my $interface = shift;
    my $v8ClassName = shift;
    my $nativeType = shift;
    my $interfaceName = $interface->name;

    if ($interface->extendedAttributes->{"DoNotGenerateWrap"} || $interface->extendedAttributes->{"DoNotGenerateToV8"}) {
        return;
    }

    AddToImplIncludes("bindings/v8/ScriptController.h");
    AddToImplIncludes("core/page/Frame.h");

    my $createWrapperArgumentType = GetPassRefPtrType($nativeType);
    my $baseType = BaseInterfaceName($interface);

    # FIXME: Do we really need to treat "GenerateIsReachable", "CustomIsReachable" and /SVG/
    # as dependent DOM objects?
    my $wrapperConfiguration = "WrapperConfiguration::Independent";
    if (InheritsExtendedAttribute($interface, "ActiveDOMObject")
        || InheritsExtendedAttribute($interface, "DependentLifetime")
        || InheritsExtendedAttribute($interface, "GenerateIsReachable")
        || InheritsExtendedAttribute($interface, "CustomIsReachable")
        || $v8ClassName =~ /SVG/) {
        $wrapperConfiguration = "WrapperConfiguration::Dependent";
    }

    my $code = "";
    $code .= <<END;

v8::Handle<v8::Object> ${v8ClassName}::createWrapper(${createWrapperArgumentType} impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl.get());
    ASSERT(DOMDataStore::getWrapper(impl.get(), isolate).IsEmpty());
END

    $code .= <<END if ($baseType ne $interfaceName);
    ASSERT(static_cast<void*>(static_cast<${baseType}*>(impl.get())) == static_cast<void*>(impl.get()));
END

    if (InheritsInterface($interface, "Document")) {
        $code .= <<END;
    if (Frame* frame = impl->frame()) {
        if (frame->script()->initializeMainWorld()) {
            // initializeMainWorld may have created a wrapper for the object, retry from the start.
            v8::Handle<v8::Object> wrapper = DOMDataStore::getWrapper(impl.get(), isolate);
            if (!wrapper.IsEmpty())
                return wrapper;
        }
    }
END
    }

    $code .= <<END;

    v8::Handle<v8::Object> wrapper = V8DOMWrapper::createWrapper(creationContext, &info, impl.get(), isolate);
    if (UNLIKELY(wrapper.IsEmpty()))
        return wrapper;
END
    if ($interface->name eq "ArrayBuffer") {
      AddToImplIncludes("bindings/v8/custom/V8ArrayBufferCustom.h");
      $code .= <<END;
    if (!impl->hasDeallocationObserver()) {
        v8::V8::AdjustAmountOfExternalAllocatedMemory(impl->byteLength());
        impl->setDeallocationObserver(V8ArrayBufferDeallocationObserver::instance());
    }
END
    } elsif (IsTypedArrayType($interface->name)) {
      AddToImplIncludes("bindings/v8/custom/V8ArrayBufferCustom.h");
      $code .= <<END;
    if (!impl->buffer()->hasDeallocationObserver()) {
        v8::V8::AdjustAmountOfExternalAllocatedMemory(impl->buffer()->byteLength());
        impl->buffer()->setDeallocationObserver(V8ArrayBufferDeallocationObserver::instance());
    }
END
    }

    $code .= <<END;
    installPerContextProperties(wrapper, impl.get(), isolate);
    V8DOMWrapper::associateObjectWithWrapper(impl, &info, wrapper, isolate, $wrapperConfiguration);
    return wrapper;
}
END
    $implementation{nameSpaceWebCore}->add($code);
}

sub GenerateSecurityCheckFunctions
{
    my $interface = shift;
    my $implClassName = GetImplName($interface);
    my $v8ClassName = GetV8ClassName($interface);

    AddToImplIncludes("bindings/v8/BindingSecurity.h");
    $implementation{nameSpaceInternal}->add(<<END);
bool indexedSecurityCheck(v8::Local<v8::Object> host, uint32_t index, v8::AccessType type, v8::Local<v8::Value>)
{
    $implClassName* imp =  ${v8ClassName}::toNative(host);
    return BindingSecurity::shouldAllowAccessToFrame(imp->frame(), DoNotReportSecurityError);
}

END
    $implementation{nameSpaceInternal}->add(<<END);
bool namedSecurityCheck(v8::Local<v8::Object> host, v8::Local<v8::Value> key, v8::AccessType type, v8::Local<v8::Value>)
{
    $implClassName* imp =  ${v8ClassName}::toNative(host);
    return BindingSecurity::shouldAllowAccessToFrame(imp->frame(), DoNotReportSecurityError);
}

END
}

sub GetNativeTypeForConversions
{
    my $interface = shift;
    my $implClassName = GetImplName($interface);
    $implClassName = GetSVGTypeNeedingTearOff($interface->name) if IsSVGTypeNeedingTearOff($interface->name);
    return $implClassName;
}

sub GetNamespaceForInterface
{
    my $interface = shift;
    return "WTF" if IsTypedArrayType($interface->name);
    return "WebCore";
}

sub GenerateFunctionCallString
{
    my $function = shift;
    my $numberOfParameters = shift;
    my $indent = shift;
    my $interface = shift;
    my $forMainWorldSuffix = shift;
    my %replacements = @_;

    my $interfaceName = $interface->name;
    my $implClassName = GetImplName($interface);
    my $name = GetImplName($function);
    my $returnType = $function->type;
    my $nativeReturnType = GetNativeType($returnType, 0);
    my $code = "";

    my $isSVGTearOffType = (IsSVGTypeNeedingTearOff($returnType) and not $interfaceName =~ /List$/);
    $nativeReturnType = GetSVGWrappedTypeNeedingTearOff($returnType) if $isSVGTearOffType;

    my $index = 0;

    my @arguments;
    my $functionName;
    my $implementedBy = $function->extendedAttributes->{"ImplementedBy"};
    if ($implementedBy) {
        my $implementedByImplName = GetImplNameFromImplementedBy($implementedBy);
        AddToImplIncludes(HeaderFilesForInterface($implementedBy, $implementedByImplName));
        unshift(@arguments, "imp") if !$function->isStatic;
        $functionName = "${implementedByImplName}::${name}";
    } elsif ($function->isStatic) {
        $functionName = "${implClassName}::${name}";
    } else {
        $functionName = "imp->${name}";
    }

    my $callWith = $function->extendedAttributes->{"CallWith"};
    my ($callWithArgs, $subCode) = GenerateCallWith($callWith, $indent, 1, $function);
    $code .= $subCode;
    unshift(@arguments, @$callWithArgs);
    $index += @$callWithArgs;
    $numberOfParameters += @$callWithArgs;

    foreach my $parameter (@{$function->parameters}) {
        if ($index eq $numberOfParameters) {
            last;
        }
        my $paramName = $parameter->name;
        my $paramType = $parameter->type;

        if ($replacements{$paramName}) {
            push @arguments, $replacements{$paramName};
        } elsif ($parameter->type eq "NodeFilter" || $parameter->type eq "XPathNSResolver") {
            push @arguments, "$paramName.get()";
        } elsif (IsSVGTypeNeedingTearOff($parameter->type) and not $interfaceName =~ /List$/) {
            push @arguments, "$paramName->propertyReference()";
            $code .= $indent . "if (!$paramName) {\n";
            $code .= $indent . "    setDOMException(WebCore::TYPE_MISMATCH_ERR, args.GetIsolate());\n";
            $code .= $indent . "    return;\n";
            $code .= $indent . "}\n";
        } elsif ($parameter->type eq "SVGMatrix" and $interfaceName eq "SVGTransformList") {
            push @arguments, "$paramName.get()";
        } else {
            push @arguments, $paramName;
        }
        $index++;
    }

    if ($function->extendedAttributes->{"RaisesException"}) {
        push @arguments, "ec";
    }

    my $functionString = "$functionName(" . join(", ", @arguments) . ")";

    my $return = "result";
    my $returnIsRef = IsRefPtrType($returnType);

    if ($returnType eq "void") {
        $code .= $indent . "$functionString;\n";
    } elsif (ExtendedAttributeContains($callWith, "ScriptState") or $function->extendedAttributes->{"RaisesException"}) {
        $code .= $indent . $nativeReturnType . " result = $functionString;\n";
    } else {
        # Can inline the function call into the return statement to avoid overhead of using a Ref<> temporary
        $return = $functionString;
        $returnIsRef = 0;

        if ($interfaceName eq "SVGTransformList" and IsRefPtrType($returnType)) {
            $return = "WTF::getPtr(" . $return . ")";
        }
    }

    if ($function->extendedAttributes->{"RaisesException"}) {
        $code .= $indent . "if (UNLIKELY(ec)) {\n";
        $code .= $indent . "    setDOMException(ec, args.GetIsolate());\n";
        $code .= $indent . "    return;\n";
        $code .= $indent . "}\n";
    }

    if (ExtendedAttributeContains($callWith, "ScriptState")) {
        $code .= $indent . "if (state.hadException()) {\n";
        $code .= $indent . "    v8::Local<v8::Value> exception = state.exception();\n";
        $code .= $indent . "    state.clearException();\n";
        $code .= $indent . "    throwError(exception, args.GetIsolate());\n";
        $code .= $indent . "    return;\n";
        $code .= $indent . "}\n";
    }

    if ($isSVGTearOffType) {
        AddToImplIncludes("V8$returnType.h");
        AddToImplIncludes("core/svg/properties/SVGPropertyTearOff.h");
        my $svgNativeType = GetSVGTypeNeedingTearOff($returnType);
        # FIXME: Update for all ScriptWrappables.
        if (IsDOMNodeType($interfaceName)) {
            $code .= $indent . "v8SetReturnValue(args, toV8Fast${forMainWorldSuffix}(WTF::getPtr(${svgNativeType}::create($return)), args, imp));\n";
        } else {
            $code .= $indent . "v8SetReturnValue(args, toV8${forMainWorldSuffix}(WTF::getPtr(${svgNativeType}::create($return)), args.Holder(), args.GetIsolate()));\n";
        }
        $code .= $indent . "return;\n";
        return $code;
    }

    # If the implementing class is a POD type, commit changes
    if (IsSVGTypeNeedingTearOff($interfaceName) and not $interfaceName =~ /List$/) {
        $code .= $indent . "wrapper->commitChange();\n";
    }

    $return .= ".release()" if ($returnIsRef);

    my $nativeValue;
    # FIXME: Update for all ScriptWrappables.
    if (IsDOMNodeType($interfaceName)) {
        $nativeValue = NativeToJSValue($function->type, $function->extendedAttributes, $return, $indent, "", "args.Holder()", "args.GetIsolate()", "args", "imp", "ReturnUnsafeHandle", $forMainWorldSuffix, "return");
    } else {
        $nativeValue = NativeToJSValue($function->type, $function->extendedAttributes, $return, $indent, "", "args.Holder()", "args.GetIsolate()", "args", 0, "ReturnUnsafeHandle", $forMainWorldSuffix, "return");
    }

    $code .= $nativeValue . "\n";
    $code .= $indent . "return;\n";

    return $code;
}

sub GetNativeTypeFromAttributeOrParameter
{
    my $attributeOrParameter = shift;
    my $parameterIndex = shift;

    my $type = $attributeOrParameter->type;

    if ($type eq "unsigned long" and $attributeOrParameter->extendedAttributes->{"IsIndex"}) {
        # Special-case index arguments because we need to check that they aren't < 0.
        return "int";
    }

    $type = GetNativeType($type, $parameterIndex >= 0 ? 1 : 0);

    if ($parameterIndex >= 0 && $type eq "V8StringResource") {
        # FIXME: This implements [TreatNullAs=NullString] and [TreatUndefinedAs=NullString],
        # but the Web IDL spec requires [TreatNullAs=EmptyString] and [TreatUndefinedAs=EmptyString].
        my $mode = "";
        if (($attributeOrParameter->extendedAttributes->{"TreatNullAs"} and $attributeOrParameter->extendedAttributes->{"TreatNullAs"} eq "NullString") and ($attributeOrParameter->extendedAttributes->{"TreatUndefinedAs"} and $attributeOrParameter->extendedAttributes->{"TreatUndefinedAs"} eq "NullString")) {
            $mode = "WithUndefinedOrNullCheck";
        } elsif (($attributeOrParameter->extendedAttributes->{"TreatNullAs"} and $attributeOrParameter->extendedAttributes->{"TreatNullAs"} eq "NullString") or $attributeOrParameter->extendedAttributes->{"Reflect"}) {
            $mode = "WithNullCheck";
        }
        # FIXME: Add the case for 'elsif ($attributeOrParameter->extendedAttributes->{"TreatUndefinedAs"} and $attributeOrParameter->extendedAttributes->{"TreatUndefinedAs"} eq "NullString"))'.
        $type .= "<$mode>";
    }

    return $type;
}

sub GetNativeType
{
    my $type = shift;
    my $isParameter = shift;

    my $svgNativeType = GetSVGTypeNeedingTearOff($type);
    if ($svgNativeType) {
        if ($svgNativeType =~ /List$/) {
            return "${svgNativeType}*";
        } else {
            return "RefPtr<${svgNativeType} >";
        }
    }

    return "float" if $type eq "float";
    return "double" if $type eq "double";
    return "int" if $type eq "long" or $type eq "int" or $type eq "short" or $type eq "byte";
    return "unsigned" if $type eq "unsigned long" or $type eq "unsigned int" or $type eq "unsigned short" or $type eq "octet";
    return "long long" if $type eq "long long";
    return "unsigned long long" if $type eq "unsigned long long";
    return "bool" if $type eq "boolean";

    return "V8StringResource" if ($type eq "DOMString" or IsEnumType($type)) and $isParameter;
    return "String" if $type eq "DOMString" or IsEnumType($type);

    return "Range::CompareHow" if $type eq "CompareHow";
    return "DOMTimeStamp" if $type eq "DOMTimeStamp";
    return "double" if $type eq "Date";
    return "ScriptValue" if $type eq "any" or IsCallbackFunctionType($type);
    return "Dictionary" if $type eq "Dictionary";

    return "RefPtr<DOMStringList>" if $type eq "DOMStringList";
    return "RefPtr<MediaQueryListListener>" if $type eq "MediaQueryListListener";
    return "RefPtr<NodeFilter>" if $type eq "NodeFilter";
    return "RefPtr<SerializedScriptValue>" if $type eq "SerializedScriptValue";
    return "RefPtr<XPathNSResolver>" if $type eq "XPathNSResolver";

    die "UnionType is not supported" if IsUnionType($type);

    # We need to check [ImplementedAs] extended attribute for wrapper types.
    if (IsWrapperType($type)) {
        my $interface = ParseInterface($type);
        my $implClassName = GetImplName($interface);
        return $isParameter ? "${implClassName}*" : "RefPtr<${implClassName}>";
    }
    return "RefPtr<${type}>" if IsRefPtrType($type) and not $isParameter;

    my $arrayOrSequenceType = GetArrayOrSequenceType($type);

    if ($arrayOrSequenceType) {
        my $nativeType = GetNativeType($arrayOrSequenceType);
        $nativeType .= " " if ($nativeType =~ />$/);
        return "Vector<${nativeType}>";
    }

    # Default, assume native type is a pointer with same type name as idl type
    return "${type}*";
}

sub GetNativeTypeForCallbacks
{
    my $type = shift;
    return "const String&" if $type eq "DOMString";
    return "PassRefPtr<SerializedScriptValue>" if $type eq "SerializedScriptValue";

    # Callbacks use raw pointers, so pass isParameter = 1
    return GetNativeType($type, 1);
}

sub JSValueToNative
{
    my $type = shift;
    my $extendedAttributes = shift;
    my $value = shift;
    my $getIsolate = shift;

    my $intConversion = $extendedAttributes->{"EnforceRange"} ? "EnforceRange" : "NormalConversion";

    return "$value->BooleanValue()" if $type eq "boolean";
    return "static_cast<$type>($value->NumberValue())" if $type eq "float" or $type eq "double";

    if ($intConversion ne "NormalConversion") {
        return "toInt8($value, $intConversion, ok)" if $type eq "byte";
        return "toUInt8($value, $intConversion, ok)" if $type eq "octet";
        return "toInt32($value, $intConversion, ok)" if $type eq "long" or $type eq "short";
        return "toUInt32($value, $intConversion, ok)" if $type eq "unsigned long" or $type eq "unsigned short";
        return "toInt64($value, $intConversion, ok)" if $type eq "long long";
        return "toUInt64($value, $intConversion, ok)" if $type eq "unsigned long long";
    } else {
        return "toInt8($value)" if $type eq "byte";
        return "toUInt8($value)" if $type eq "octet";
        return "toInt32($value)" if $type eq "long" or $type eq "short";
        return "toUInt32($value)" if $type eq "unsigned long" or $type eq "unsigned short";
        return "toInt64($value)" if $type eq "long long";
        return "toUInt64($value)" if $type eq "unsigned long long";
    }
    return "static_cast<Range::CompareHow>($value->Int32Value())" if $type eq "CompareHow";
    return "toWebCoreDate($value)" if $type eq "Date";
    return "toDOMStringList($value, $getIsolate)" if $type eq "DOMStringList";

    if ($type eq "DOMString" or IsEnumType($type)) {
        return $value;
    }

    if ($type eq "SerializedScriptValue") {
        AddToImplIncludes("bindings/v8/SerializedScriptValue.h");
        return "SerializedScriptValue::create($value, $getIsolate)";
    }

    if ($type eq "Dictionary") {
        AddToImplIncludes("bindings/v8/Dictionary.h");
        return "Dictionary($value, $getIsolate)";
    }

    if ($type eq "any" || IsCallbackFunctionType($type)) {
        AddToImplIncludes("bindings/v8/ScriptValue.h");
        return "ScriptValue($value)";
    }

    if ($type eq "NodeFilter") {
        return "toNodeFilter($value)";
    }

    if ($type eq "MediaQueryListListener") {
        AddToImplIncludes("core/css/MediaQueryListListener.h");
        return "MediaQueryListListener::create(" . $value . ")";
    }

    if ($type eq "EventTarget") {
        return "V8DOMWrapper::isDOMWrapper($value) ? toWrapperTypeInfo(v8::Handle<v8::Object>::Cast($value))->toEventTarget(v8::Handle<v8::Object>::Cast($value)) : 0";
    }

    if ($type eq "XPathNSResolver") {
        return "toXPathNSResolver($value, $getIsolate)";
    }

    my $arrayOrSequenceType = GetArrayOrSequenceType($type);

    if ($arrayOrSequenceType) {
        if (IsRefPtrType($arrayOrSequenceType)) {
            AddToImplIncludes("V8${arrayOrSequenceType}.h");
            return "(toRefPtrNativeArray<${arrayOrSequenceType}, V8${arrayOrSequenceType}>($value, $getIsolate))";
        }
        return "toNativeArray<" . GetNativeType($arrayOrSequenceType) . ">($value)";
    }

    AddIncludesForType($type);

    AddToImplIncludes("V8${type}.h");
    return "V8${type}::HasInstance($value, $getIsolate, worldType($getIsolate)) ? V8${type}::toNative(v8::Handle<v8::Object>::Cast($value)) : 0";
}

sub CreateCustomSignature
{
    my $function = shift;
    my $count = @{$function->parameters};
    my $name = $function->name;
    my $code = "    const int ${name}Argc = ${count};\n" .
      "    v8::Handle<v8::FunctionTemplate> ${name}Argv[${name}Argc] = { ";
    my $first = 1;
    foreach my $parameter (@{$function->parameters}) {
        if ($first) { $first = 0; }
        else { $code .= ", "; }
        if (IsWrapperType($parameter->type)) {
            if ($parameter->type eq "XPathNSResolver") {
                # Special case for XPathNSResolver.  All other browsers accepts a callable,
                # so, even though it's against IDL, accept objects here.
                $code .= "v8::Handle<v8::FunctionTemplate>()";
            } else {
                my $type = $parameter->type;
                my $arrayOrSequenceType = GetArrayOrSequenceType($type);

                if ($arrayOrSequenceType) {
                    if (IsRefPtrType($arrayOrSequenceType)) {
                        AddIncludesForType($arrayOrSequenceType);
                    } else {
                        $code .= "v8::Handle<v8::FunctionTemplate>()";
                        next;
                    }
                } else {
                    AddIncludesForType($type);
                }
                $code .= "V8PerIsolateData::from(isolate)->rawTemplate(&V8${type}::info, currentWorldType)";
            }
        } else {
            $code .= "v8::Handle<v8::FunctionTemplate>()";
        }
    }
    $code .= " };\n";
    $code .= "    v8::Handle<v8::Signature> ${name}Signature = v8::Signature::New(desc, ${name}Argc, ${name}Argv);\n";
    return $code;
}


sub RequiresCustomSignature
{
    my $function = shift;
    # No signature needed for Custom function
    if (HasCustomMethod($function->extendedAttributes)) {
        return 0;
    }
    # No signature needed for overloaded function
    if (@{$function->{overloads}} > 1) {
        return 0;
    }
    if ($function->isStatic) {
        return 0;
    }
    # Type checking is performed in the generated code
    if ($function->extendedAttributes->{"StrictTypeChecking"}) {
      return 0;
    }
    foreach my $parameter (@{$function->parameters}) {
        if (($parameter->isOptional && !$parameter->extendedAttributes->{"Default"}) || IsCallbackInterface($parameter->type)) {
            return 0;
        }
    }

    foreach my $parameter (@{$function->parameters}) {
        if (IsWrapperType($parameter->type)) {
            return 1;
        }
    }
    return 0;
}

sub IsUnionType
{
    my $type = shift; # string or UnionType
    if(ref($type) eq "UnionType") {
        die "Currently only 2 values of non-union type is supported as union type.\n" unless @{$type->unionMemberTypes} == 2;
        return 1;
    }
    return 0;
}

sub IsWrapperType
{
    my $type = shift;
    return 0 if GetArrayType($type);
    return 0 if GetSequenceType($type);
    return 0 if IsCallbackFunctionType($type);
    return 0 if IsEnumType($type);
    return 0 if IsPrimitiveType($type);
    return 0 if $type eq "DOMString";
    return !$nonWrapperTypes{$type};
}

sub IsCallbackInterface
{
    my $type = shift;
    return 0 unless IsWrapperType($type);

    my $idlFile = IDLFileForInterface($type)
        or die("Could NOT find IDL file for interface \"$type\"!\n");

    open FILE, "<", $idlFile;
    my @lines = <FILE>;
    close FILE;

    my $fileContents = join('', @lines);
    return ($fileContents =~ /callback\s+interface\s+(\w+)/gs);
}

sub GetNativeTypeOfTypedArray
{
    my $interface = shift;
    my $interfaceName = $interface->name;
    die "TypedArray of unknown type is found" unless $typedArrayHash{$interface->name};
    return @{$typedArrayHash{$interface->name}};
}

sub IsDOMNodeType
{
    my $type = shift;

    return 1 if $type eq 'Attr';
    return 1 if $type eq 'CDATASection';
    return 1 if $type eq 'CharacterData';
    return 1 if $type eq 'Comment';
    return 1 if $type eq 'Document';
    return 1 if $type eq 'DocumentFragment';
    return 1 if $type eq 'DocumentType';
    return 1 if $type eq 'Element';
    return 1 if $type eq 'Entity';
    return 1 if $type eq 'HTMLDocument';
    return 1 if $type eq 'Node';
    return 1 if $type eq 'Notation';
    return 1 if $type eq 'ProcessingInstruction';
    return 1 if $type eq 'ShadowRoot';
    return 1 if $type eq 'SVGDocument';
    return 1 if $type eq 'Text';

    return 1 if $type =~ /^HTML.*Element$/;
    return 1 if $type =~ /^SVG.*Element$/;

    return 1 if $type eq 'TestNode';

    return 0;
}


sub NativeToJSValue
{
    my $type = shift;
    my $extendedAttributes = shift;
    my $nativeValue = shift;
    my $indent = shift;  # added before every line
    my $receiver = shift;  # "return" or "<variableName> ="
    my $getCreationContext = shift;
    my $getIsolate = shift;
    die "An Isolate is mandatory for native value => JS value conversion." unless $getIsolate;
    my $getHolderContainer = shift || "";
    my $getHolderContainerArg = $getHolderContainer ? ", $getHolderContainer" : "";
    my $getScriptWrappable = shift || "";
    my $getScriptWrappableArg = $getScriptWrappable ? ", $getScriptWrappable" : "";
    my $returnHandleType = shift || "";
    my $returnHandleTypeArg = $returnHandleType ? ", $returnHandleType" : "";
    my $forMainWorldSuffix = shift || "";
    my $returnValueArg = shift || 0;
    my $isReturnValue = $returnValueArg eq "return";

    if (IsUnionType($type)) {
        my $types = $type->unionMemberTypes;
        my @codes = ();
        for my $i (0 .. scalar(@$types)-1) {
            my $unionMemberType = $types->[$i];
            my $unionMemberNumber = $i + 1;
            my $unionMemberVariable = $nativeValue . $i;
            my $unionMemberEnabledVariable = $nativeValue . $i . "Enabled";
            my $unionMemberNativeValue = $unionMemberVariable;
            $unionMemberNativeValue .= ".release()" if (IsRefPtrType($unionMemberType));
            my $returnJSValueCode = NativeToJSValue($unionMemberType, $extendedAttributes, $unionMemberNativeValue, $indent . "    ", $receiver, $getCreationContext, $getIsolate, $getHolderContainer, $getScriptWrappable, $returnHandleType, $forMainWorldSuffix, $returnValueArg);
            my $code = "";
            if ($isReturnValue) {
              $code .= "${indent}if (${unionMemberEnabledVariable}) {\n";
              $code .= "${returnJSValueCode}\n";
              $code .= "${indent}    return;\n";
              $code .= "${indent}}\n";
            } else {
              $code .= "${indent}if (${unionMemberEnabledVariable})\n";
              $code .= "${returnJSValueCode}";
            }
            push @codes, $code;
        }
        return join "\n", @codes;
    }

    if ($type eq "boolean") {
        return "${indent}v8SetReturnValueBool(${getHolderContainer}, ${nativeValue});" if $isReturnValue;
        return "$indent$receiver v8Boolean($nativeValue, $getIsolate);";
    }

    if ($type eq "void") { # equivalent to v8Undefined()
        return "" if $isReturnValue;
        return "$indent$receiver v8Undefined();"
    }

    # HTML5 says that unsigned reflected attributes should be in the range
    # [0, 2^31). When a value isn't in this range, a default value (or 0)
    # should be returned instead.
    if ($extendedAttributes->{"Reflect"} and ($type eq "unsigned long" or $type eq "unsigned short")) {
        $nativeValue =~ s/getUnsignedIntegralAttribute/getIntegralAttribute/g;
        return "${indent}v8SetReturnValueUnsigned(${getHolderContainer}, std::max(0, ${nativeValue}));" if $isReturnValue;
        return "$indent$receiver v8UnsignedInteger(std::max(0, " . $nativeValue . "), $getIsolate);";
    }

    # For all the types where we use 'int' as the representation type,
    # we use v8Integer() which has a fast small integer conversion check.
    my $nativeType = GetNativeType($type);
    if ($nativeType eq "int") {
        return "${indent}v8SetReturnValueInt(${getHolderContainer}, ${nativeValue});" if $isReturnValue;
        return "$indent$receiver v8Integer($nativeValue, $getIsolate);";
    }

    if ($nativeType eq "unsigned") {
        return "${indent}v8SetReturnValueUnsigned(${getHolderContainer}, ${nativeValue});" if $isReturnValue;
        return "$indent$receiver v8UnsignedInteger($nativeValue, $getIsolate);";
    }

    if ($type eq "Date") {
        return "${indent}v8SetReturnValue(${getHolderContainer}, v8DateOrNull($nativeValue, $getIsolate));" if $isReturnValue;
        return "$indent$receiver v8DateOrNull($nativeValue, $getIsolate);"
    }

    # long long and unsigned long long are not representable in ECMAScript.
    if ($type eq "long long" or $type eq "unsigned long long" or $type eq "DOMTimeStamp") {
        return "${indent}v8SetReturnValue(${getHolderContainer}, static_cast<double>($nativeValue));" if $isReturnValue;
        return "$indent$receiver v8::Number::New(static_cast<double>($nativeValue));";
    }

    if (IsPrimitiveType($type)) {
        die "unexpected type $type" if not ($type eq "float" or $type eq "double");
        return "${indent}v8SetReturnValue(${getHolderContainer}, ${nativeValue});" if $isReturnValue;
        return "$indent$receiver v8::Number::New($nativeValue);";
    }

    if ($nativeType eq "ScriptValue") {
        return "${indent}v8SetReturnValue(${getHolderContainer}, ${nativeValue}.v8Value());" if $isReturnValue;
        return "$indent$receiver $nativeValue.v8Value();";
    }

    if ($type eq "DOMString" or IsEnumType($type)) {
        my $conv = $extendedAttributes->{"TreatReturnedNullStringAs"};
        my $returnValue = "";
        if (defined $conv) {
            if ($conv eq "Null") {
                $returnValue = "v8StringOrNull($nativeValue, $getIsolate$returnHandleTypeArg)";
            } elsif ($conv eq "Undefined") {
                $returnValue = "v8StringOrUndefined($nativeValue, $getIsolate$returnHandleTypeArg)";
            } else {
                die "Unknown value for TreatReturnedNullStringAs extended attribute";
            }
        } else {
            $returnValue = "v8String($nativeValue, $getIsolate$returnHandleTypeArg)";
        }
        # FIXME: Use safe handles
        return "${indent}v8SetReturnValue(${getHolderContainer}, $returnValue);" if $isReturnValue;
        return "$indent$receiver $returnValue;";
    }

    my $arrayOrSequenceType = GetArrayOrSequenceType($type);

    if ($arrayOrSequenceType) {
        if (IsRefPtrType($arrayOrSequenceType)) {
            AddIncludesForType($arrayOrSequenceType);
        }
        return "${indent}v8SetReturnValue(${getHolderContainer}, v8Array($nativeValue, $getIsolate));" if $isReturnValue;
        return "$indent$receiver v8Array($nativeValue, $getIsolate);";
    }

    AddIncludesForType($type);

    if (IsDOMNodeType($type) || $type eq "EventTarget") {
      if ($getScriptWrappable) {
          # FIXME: Use safe handles
          return "${indent}v8SetReturnValue(${getHolderContainer}, toV8Fast${forMainWorldSuffix}($nativeValue$getHolderContainerArg$getScriptWrappableArg));" if $isReturnValue;
          return "$indent$receiver toV8Fast${forMainWorldSuffix}($nativeValue$getHolderContainerArg$getScriptWrappableArg);";
      }
      # FIXME: Use safe handles
      return "${indent}v8SetReturnValue(${getHolderContainer}, toV8($nativeValue, $getCreationContext, $getIsolate));" if $isReturnValue;
      return "$indent$receiver toV8($nativeValue, $getCreationContext, $getIsolate);";
    }

    if ($type eq "SerializedScriptValue") {
        AddToImplIncludes("$type.h");
        my $returnValue = "$nativeValue ? $nativeValue->deserialize() : v8::Handle<v8::Value>(v8Null($getIsolate))";
        return "${indent}v8SetReturnValue(${getHolderContainer}, $returnValue);" if $isReturnValue;
        return "$indent$receiver $returnValue;";
    }

    AddToImplIncludes("wtf/RefCounted.h");
    AddToImplIncludes("wtf/RefPtr.h");
    AddToImplIncludes("wtf/GetPtr.h");

    if ($getScriptWrappable) {
        # FIXME: Use safe handles
        return "${indent}v8SetReturnValue(${getHolderContainer}, toV8Fast${forMainWorldSuffix}($nativeValue$getHolderContainerArg$getScriptWrappableArg));" if $isReturnValue;
        return "$indent$receiver toV8Fast${forMainWorldSuffix}($nativeValue$getHolderContainerArg$getScriptWrappableArg);";
    }
    # FIXME: Use safe handles
    return "${indent}v8SetReturnValue(${getHolderContainer}, toV8($nativeValue, $getCreationContext, $getIsolate));" if $isReturnValue;
    return "$indent$receiver toV8($nativeValue, $getCreationContext, $getIsolate);";
}

sub WriteData
{
    my $object = shift;
    my $interface = shift;
    my $outputDirectory = shift;
    my $outputHeadersDirectory = shift;

    my $name = $interface->name;
    my $headerFileName = "$outputHeadersDirectory/V8$name.h";
    my $implFileName = "$outputDirectory/V8$name.cpp";

    my @includes = ();
    foreach my $include (keys %implIncludes) {
        push @includes, "\"$include\"";
    }

    #FIXME: do not treat main header special
    my $mainInclude = "\"V8$name.h\"";
    foreach my $include (sort @includes) {
        $implementation{includes}->add("#include $include\n") unless $include eq $mainInclude;
    }
    $implementation{includes}->add("\n") unless $interface->isCallback;
    WriteFileIfChanged($implFileName, $implementation{root}->toString());

    %implIncludes = ();

    WriteFileIfChanged($headerFileName, $header{root}->toString());
}

sub ConvertToV8StringResource
{
    my $attributeOrParameter = shift;
    my $nativeType = shift;
    my $variableName = shift;
    my $value = shift;

    die "Wrong native type passed: $nativeType" unless $nativeType =~ /^V8StringResource/;
    if ($attributeOrParameter->type eq "DOMString" or IsEnumType($attributeOrParameter->type)) {
        return "V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID($nativeType, $variableName, $value);"
    } else {
        return "$nativeType $variableName($value, true);";
    }
}

# Returns the RuntimeEnabledFeatures function name that is hooked up to check if a method/attribute is enabled.
sub GetRuntimeEnableFunctionName
{
    my $signature = shift;

    # If a parameter is given (e.g. "EnabledAtRuntime=FeatureName") return the RuntimeEnabledFeatures::{FeatureName}Enabled() method.
    return "RuntimeEnabledFeatures::" . ToMethodName($signature->extendedAttributes->{"EnabledAtRuntime"}) . "Enabled" if ($signature->extendedAttributes->{"EnabledAtRuntime"} && $signature->extendedAttributes->{"EnabledAtRuntime"} ne "VALUE_IS_MISSING");

    # Otherwise return a function named RuntimeEnabledFeatures::{methodName}Enabled().
    return "RuntimeEnabledFeatures::" . ToMethodName($signature->name) . "Enabled";
}

sub GetContextEnableFunction
{
    my $signature = shift;

    # If a parameter is given (e.g. "EnabledPerContext=FeatureName") return the {FeatureName}Allowed() method.
    if ($signature->extendedAttributes->{"EnabledPerContext"} && $signature->extendedAttributes->{"EnabledPerContext"} ne "VALUE_IS_MISSING") {
        return "ContextFeatures::" . ToMethodName($signature->extendedAttributes->{"EnabledPerContext"}) . "Enabled";
    }

    # Or it fallbacks to the attribute name if the parameter value is missing.
    return "ContextFeatures::" . ToMethodName($signature->name) . "Enabled";
}

sub GetPassRefPtrType
{
    my $v8ClassName = shift;

    my $angleBracketSpace = $v8ClassName =~ />$/ ? " " : "";
    return "PassRefPtr<${v8ClassName}${angleBracketSpace}>";
}

sub WriteFileIfChanged
{
    my $fileName = shift;
    my $contents = shift;

    if (-f $fileName && $writeFileOnlyIfChanged) {
        open FH, "<", $fileName or die "Couldn't open $fileName: $!\n";
        my @lines = <FH>;
        my $oldContents = join "", @lines;
        close FH;
        return if $contents eq $oldContents;
    }
    open FH, ">", $fileName or die "Couldn't open $fileName: $!\n";
    print FH $contents;
    close FH;
}

sub ForAllParents
{
    my $interface = shift;
    my $beforeRecursion = shift;
    my $afterRecursion = shift;

    my $recurse;
    $recurse = sub {
        my $currentInterface = shift;

        for (@{$currentInterface->parents}) {
            my $interfaceName = $_;
            my $parentInterface = ParseInterface($interfaceName);

            if ($beforeRecursion) {
                &$beforeRecursion($parentInterface) eq 'prune' and next;
            }
            &$recurse($parentInterface);
            &$afterRecursion($parentInterface) if $afterRecursion;
        }
    };

    &$recurse($interface);
}

sub AddMethodsConstantsAndAttributesFromParentInterfaces
{
    # Add to $interface all of its inherited interface members, except for those
    # inherited through $interface's first listed parent.  If an array reference
    # is passed in as $parents, the names of all ancestor interfaces visited
    # will be appended to the array. The names of $interface's first listed parent
    # and its ancestors will also be appended to $parents.

    my $interface = shift;
    my $parents = shift;

    my $first = 1;
    ForAllParents($interface, sub {
        my $currentInterface = shift;

        if ($first) {
            # Ignore first parent class, already handled by the generation itself.
            $first = 0;

            # Just collect the names of the direct ancestor interfaces,
            # if necessary.
            push(@$parents, $currentInterface->name);
            ForAllParents($currentInterface, sub {
                my $currentInterface = shift;
                push(@$parents, $currentInterface->name);
            });
            return 'prune';
        }

        # Collect the name of this additional parent.
        push(@$parents, $currentInterface->name) if $parents;

        # Add this parent's members to $interface.
        push(@{$interface->constants}, @{$currentInterface->constants});
        push(@{$interface->functions}, @{$currentInterface->functions});
        push(@{$interface->attributes}, @{$currentInterface->attributes});
    });
}

sub FindSuperMethod
{
    my ($interface, $functionName) = @_;
    my $indexer;
    ForAllParents($interface, undef, sub {
        my $currentInterface = shift;
        foreach my $function (@{$currentInterface->functions}) {
            if ($function->name eq $functionName) {
                $indexer = $function;
                return 'prune';
            }
        }
    });
    return $indexer;
}

sub IsConstructorTemplate
{
    my $interface = shift;
    my $template = shift;

    return $interface->extendedAttributes->{"ConstructorTemplate"} && $interface->extendedAttributes->{"ConstructorTemplate"} eq $template;
}

sub IsPrimitiveType
{
    my $type = shift;

    return 1 if $primitiveTypeHash{$type};
    return 0;
}

sub IsCallbackFunctionType
{
    my $type = shift;

    return 1 if $callbackFunctionTypeHash{$type};
    return 0;
}

sub IsEnumType
{
    my $type = shift;

    return 1 if $enumTypeHash{$type};
    return 0;
}

sub ValidEnumValues
{
    my $type = shift;

    return @{$enumTypeHash{$type}};
}

sub IsSVGTypeNeedingTearOff
{
    my $type = shift;

    return 1 if $svgTypeNeedingTearOff{$type};
    return 0;
}

sub IsSVGTypeWithWritablePropertiesNeedingTearOff
{
    my $type = shift;

    return 1 if $svgTypeWithWritablePropertiesNeedingTearOff{$type};
    return 0;
}

sub IsTypedArrayType
{
    my $type = shift;
    return 1 if $typedArrayHash{$type};
    return 0;
}

sub IsRefPtrType
{
    my $type = shift;

    return 0 if $type eq "any";
    return 0 if IsPrimitiveType($type);
    return 0 if GetArrayType($type);
    return 0 if GetSequenceType($type);
    return 0 if $type eq "DOMString";
    return 0 if IsCallbackFunctionType($type);
    return 0 if IsEnumType($type);
    return 0 if IsUnionType($type);

    return 1;
}

sub GetSVGTypeNeedingTearOff
{
    my $type = shift;

    return $svgTypeNeedingTearOff{$type} if exists $svgTypeNeedingTearOff{$type};
    return undef;
}

sub GetSVGWrappedTypeNeedingTearOff
{
    my $type = shift;

    my $svgTypeNeedingTearOff = GetSVGTypeNeedingTearOff($type);
    return $svgTypeNeedingTearOff if not $svgTypeNeedingTearOff;

    if ($svgTypeNeedingTearOff =~ /SVGPropertyTearOff/) {
        $svgTypeNeedingTearOff =~ s/SVGPropertyTearOff<//;
    } elsif ($svgTypeNeedingTearOff =~ /SVGListPropertyTearOff/) {
        $svgTypeNeedingTearOff =~ s/SVGListPropertyTearOff<//;
    } elsif ($svgTypeNeedingTearOff =~ /SVGStaticListPropertyTearOff/) {
        $svgTypeNeedingTearOff =~ s/SVGStaticListPropertyTearOff<//;
    }  elsif ($svgTypeNeedingTearOff =~ /SVGTransformListPropertyTearOff/) {
        $svgTypeNeedingTearOff =~ s/SVGTransformListPropertyTearOff<//;
    }

    $svgTypeNeedingTearOff =~ s/>//;
    return $svgTypeNeedingTearOff;
}

sub IsSVGAnimatedType
{
    my $type = shift;

    return 1 if $svgAnimatedTypeHash{$type};
    return 0;
}

sub GetSequenceType
{
    my $type = shift;

    return $1 if $type =~ /^sequence<([\w\d_\s]+)>.*/;
    return "";
}

sub GetArrayType
{
    my $type = shift;

    return $1 if $type =~ /^([\w\d_\s]+)\[\]/;
    return "";
}

sub GetArrayOrSequenceType
{
    my $type = shift;

    return GetArrayType($type) || GetSequenceType($type);
}

sub AssertNotSequenceType
{
    my $type = shift;
    die "Sequences must not be used as the type of an attribute, constant or exception field." if GetSequenceType($type);
}

sub FirstLetterToUpperCase
{
    my $param = shift;
    my $ret = ucfirst($param);
    # xmlEncoding becomes XMLEncoding, but xmlllang becomes Xmllang.
    $ret =~ s/Xml/XML/ if $ret =~ /^Xml[^a-z]/;
    $ret =~ s/Css/CSS/ if $ret =~ /^Css[^T]/;  # css -> setCSS, except setCssText.
    $ret =~ s/Ime/IME/ if $ret =~ /^Ime/;  # ime -> setIME
    return $ret;
}

# URL becomes url, but SetURL becomes setURL.
sub ToMethodName
{
    my $param = shift;
    my $ret = lcfirst($param);
    $ret =~ s/hTML/html/ if $ret =~ /^hTML/;
    $ret =~ s/uRL/url/ if $ret =~ /^uRL/;
    $ret =~ s/jS/js/ if $ret =~ /^jS/;
    $ret =~ s/xML/xml/ if $ret =~ /^xML/;
    $ret =~ s/xSLT/xslt/ if $ret =~ /^xSLT/;
    $ret =~ s/cSS/css/ if $ret =~ /^cSS/;

    # For HTML5 FileSystem API Flags attributes.
    # (create is widely used to instantiate an object and must be avoided.)
    $ret =~ s/^create/isCreate/ if $ret =~ /^create$/;
    $ret =~ s/^exclusive/isExclusive/ if $ret =~ /^exclusive$/;

    return $ret;
}

sub NamespaceForAttributeName
{
    my ($interfaceName, $attributeName) = @_;
    return "SVGNames" if $interfaceName =~ /^SVG/ && !$svgAttributesInHTMLHash{$attributeName};
    return "HTMLNames";
}

# Identifies overloaded functions and for each function adds an array with
# links to its respective overloads (including itself).
sub LinkOverloadedFunctions
{
    my $interface = shift;

    my %nameToFunctionsMap = ();
    foreach my $function (@{$interface->functions}) {
        my $name = $function->name;
        $nameToFunctionsMap{$name} = [] if !exists $nameToFunctionsMap{$name} or !$name;  # Nameless functions cannot be overloaded
        push(@{$nameToFunctionsMap{$name}}, $function);
        $function->{overloads} = $nameToFunctionsMap{$name};
        $function->{overloadIndex} = @{$nameToFunctionsMap{$name}};
    }
}

sub AttributeNameForGetterAndSetter
{
    my $attribute = shift;

    my $attributeName = GetImplName($attribute);
    if ($attribute->extendedAttributes->{"ImplementedAs"}) {
        $attributeName = $attribute->extendedAttributes->{"ImplementedAs"};
    }
    my $attributeType = $attribute->type;

    # Avoid clash with C++ keyword.
    $attributeName = "_operator" if $attributeName eq "operator";

    # SVGAElement defines a non-virtual "String& target() const" method which clashes with "virtual String target() const" in Element.
    # To solve this issue the SVGAElement method was renamed to "svgTarget", take care of that when calling this method.
    $attributeName = "svgTarget" if $attributeName eq "target" and $attributeType eq "SVGAnimatedString";

    # SVG animated types need to use a special attribute name.
    # The rest of the special casing for SVG animated types is handled in the language-specific code generators.
    $attributeName .= "Animated" if IsSVGAnimatedType($attributeType);

    return $attributeName;
}

sub ContentAttributeName
{
    my ($interfaceName, $attribute) = @_;

    my $contentAttributeName = $attribute->extendedAttributes->{"Reflect"};
    return undef if !$contentAttributeName;

    $contentAttributeName = lc AttributeNameForGetterAndSetter($attribute) if $contentAttributeName eq "VALUE_IS_MISSING";

    my $namespace = NamespaceForAttributeName($interfaceName, $contentAttributeName);

    AddToImplIncludes("${namespace}.h");
    return "WebCore::${namespace}::${contentAttributeName}Attr";
}

sub CanUseFastAttribute
{
    my $attribute = shift;
    return !IsSVGAnimatedType($attribute->type);
}

sub GetterExpression
{
    my ($interfaceName, $attribute) = @_;

    my $contentAttributeName = ContentAttributeName($interfaceName, $attribute);

    if (!$contentAttributeName) {
        return (ToMethodName(AttributeNameForGetterAndSetter($attribute)));
    }

    my $functionName;
    if ($attribute->extendedAttributes->{"URL"}) {
        $functionName = "getURLAttribute";
    } elsif ($attribute->type eq "boolean") {
        if (CanUseFastAttribute($attribute)) {
            $functionName = "fastHasAttribute";
        } else {
            $functionName = "hasAttribute";
        }
    } elsif ($attribute->type eq "long") {
        $functionName = "getIntegralAttribute";
    } elsif ($attribute->type eq "unsigned long") {
        $functionName = "getUnsignedIntegralAttribute";
    } else {
        if ($contentAttributeName eq "WebCore::HTMLNames::idAttr") {
            $functionName = "getIdAttribute";
            $contentAttributeName = "";
        } elsif ($contentAttributeName eq "WebCore::HTMLNames::nameAttr") {
            $functionName = "getNameAttribute";
            $contentAttributeName = "";
        } elsif ($contentAttributeName eq "WebCore::HTMLNames::classAttr") {
            $functionName = "getClassAttribute";
            $contentAttributeName = "";
        } elsif (CanUseFastAttribute($attribute)) {
            $functionName = "fastGetAttribute";
        } else {
            $functionName = "getAttribute";
        }
    }

    return ($functionName, $contentAttributeName);
}

sub SetterExpression
{
    my ($interfaceName, $attribute) = @_;

    my $contentAttributeName = ContentAttributeName($interfaceName, $attribute);

    if (!$contentAttributeName) {
        return ("set" . FirstLetterToUpperCase(AttributeNameForGetterAndSetter($attribute)));
    }

    my $functionName;
    if ($attribute->type eq "boolean") {
        $functionName = "setBooleanAttribute";
    } elsif ($attribute->type eq "long") {
        $functionName = "setIntegralAttribute";
    } elsif ($attribute->type eq "unsigned long") {
        $functionName = "setUnsignedIntegralAttribute";
    } else {
        $functionName = "setAttribute";
    }

    return ($functionName, $contentAttributeName);
}

sub GenerateConditionalString
{
    my $node = shift;

    my $conditional = $node->extendedAttributes->{"Conditional"};
    if ($conditional) {
        return GenerateConditionalStringFromAttributeValue($conditional);
    } else {
        return "";
    }
}

sub GenerateConditionalStringFromAttributeValue
{
    my $conditional = shift;

    my $operator = ($conditional =~ /&/ ? '&' : ($conditional =~ /\|/ ? '|' : ''));
    if ($operator) {
        # Avoid duplicated conditions.
        my %conditions;
        map { $conditions{$_} = 1 } split('\\' . $operator, $conditional);
        return "ENABLE(" . join(") $operator$operator ENABLE(", sort keys %conditions) . ")";
    } else {
        return "ENABLE(" . $conditional . ")";
    }
}

sub GenerateCompileTimeCheckForEnumsIfNeeded
{
    my $interface = shift;
    my $implClassName = GetImplName($interface);
    my @checks = ();
    # If necessary, check that all constants are available as enums with the same value.
    if (!$interface->extendedAttributes->{"DoNotCheckConstants"} && @{$interface->constants}) {
        push(@checks, "\n");
        foreach my $constant (@{$interface->constants}) {
            my $reflect = $constant->extendedAttributes->{"Reflect"};
            my $name = $reflect ? $reflect : $constant->name;
            my $value = $constant->value;
            my $conditionalString = GenerateConditionalString($constant);
            push(@checks, "#if ${conditionalString}\n") if $conditionalString;

            if ($constant->extendedAttributes->{"ImplementedBy"}) {
                my $implementedByImplName = GetImplNameFromImplementedBy($constant->extendedAttributes->{"ImplementedBy"});
                push(@checks, "COMPILE_ASSERT($value == " . $implementedByImplName . "::$name, ${implClassName}Enum${name}IsWrongUseDoNotCheckConstants);\n");
            } else {
                push(@checks, "COMPILE_ASSERT($value == ${implClassName}::$name, ${implClassName}Enum${name}IsWrongUseDoNotCheckConstants);\n");
            }

            push(@checks, "#endif\n") if $conditionalString;
        }
        push(@checks, "\n");
    }
    return @checks;
}

sub ExtendedAttributeContains
{
    my $callWith = shift;
    return 0 unless $callWith;
    my $keyword = shift;

    my @callWithKeywords = split /\s*\|\s*/, $callWith;
    return grep { $_ eq $keyword } @callWithKeywords;
}

sub InheritsInterface
{
    my $interface = shift;
    my $interfaceName = shift;
    my $found = 0;

    return 1 if $interfaceName eq $interface->name;
    ForAllParents($interface, sub {
        my $currentInterface = shift;
        if ($currentInterface->name eq $interfaceName) {
            $found = 1;
        }
        return 1 if $found;
    }, 0);

    return $found;
}

sub InheritsExtendedAttribute
{
    my $interface = shift;
    my $extendedAttribute = shift;
    my $found = 0;

    return 1 if $interface->extendedAttributes->{$extendedAttribute};
    ForAllParents($interface, sub {
        my $currentInterface = shift;
        if ($currentInterface->extendedAttributes->{$extendedAttribute}) {
            $found = 1;
        }
        return 1 if $found;
    }, 0);

    return $found;
}

1;
