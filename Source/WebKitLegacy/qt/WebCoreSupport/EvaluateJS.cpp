/*
    Copyright (C) 2023 Movable, Inc

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "EvaluateJS.h"
#include <JavaScriptCore/APICast.h>
#include <JavaScriptCore/JSGlobalObject.h>
#include <JavaScriptCore/JSValueRef.h>
#include <JavaScriptCore/Completion.h>
#include <wtf/NakedPtr.h>
#include <WebCore/ElementInlines.h>
#include <WebCore/ScriptSourceCode.h>
#include <WebCore/ScriptController.h>
#include <WebCore/JSElement.h>
#include <WebCore/qt_runtime.h>
#include <QVariant>

using namespace WebCore;

static bool setupScriptContext(Element* element, JSC::JSGlobalObject*& lexicalGlobalObject)
{
    if (!element)
        return false;

    LocalFrame* frame = element->document().frame();
    if (!frame)
        return false;

    lexicalGlobalObject = frame->script().globalObject(mainThreadNormalWorld())->globalObject();
    if (!lexicalGlobalObject)
        return false;

    return true;
}

// Extracted from qwebelement.cpp because that file needs rtti and WebCore::ScriptSourceCode has rtti disabled
QVariant evaluateJavaScriptString(const String& scriptSource, Element* element)
{
    JSC::JSGlobalObject* lexicalGlobalObject = nullptr;

    if (!setupScriptContext(element, lexicalGlobalObject))
	return QVariant();

    JSC::JSLockHolder lock(lexicalGlobalObject);

    JSC::JSValue thisValue = toJS(lexicalGlobalObject, toJSLocalDOMWindow(element->document().frame(), currentWorld(*lexicalGlobalObject)), element);
    if (!thisValue)
	return QVariant();

    ScriptSourceCode sourceCode(scriptSource);
 
    NakedPtr<JSC::Exception> evaluationException;
    JSC::JSValue evaluationResult = JSC::evaluate(lexicalGlobalObject, sourceCode.jsSourceCode(), thisValue, evaluationException);   
    if (evaluationException)
	return QVariant();

    JSValueRef evaluationResultRef = toRef(lexicalGlobalObject, evaluationResult);

    int distance = 0;
    JSValueRef* ignoredException = 0;
    return JSC::Bindings::convertValueToQVariant(toRef(lexicalGlobalObject), evaluationResultRef, QMetaType::Void, &distance, ignoredException);
}
