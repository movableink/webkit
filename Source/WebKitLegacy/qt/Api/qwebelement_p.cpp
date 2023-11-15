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

#include "qwebelement_p.h"
#include "qwebelement.h"
#include <JavaScriptCore/APICast.h>
#include <JavaScriptCore/JSGlobalObject.h>
#include <JavaScriptCore/JSValueRef.h>
#include <JavaScriptCore/Completion.h>
#include <wtf/NakedPtr.h>
#include <WebCore/ElementInlines.h>
#include <WebCore/ScriptSourceCode.h>
#include <WebCore/ScriptController.h>
#include <WebCore/JSDocument.h>
#include <WebCore/JSElement.h>
#include <WebCore/qt_runtime.h>
#include <QVariant>
#include <QString>

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
QVariant QWebElementPrivate::evaluateJavaScriptString(const QString& scriptSource, Element* element)
{
    JSC::JSGlobalObject* lexicalGlobalObject = nullptr;

    if (!setupScriptContext(element, lexicalGlobalObject))
	return QVariant();

    JSC::JSLockHolder lock(lexicalGlobalObject);

    JSC::JSValue thisValue = toJS(lexicalGlobalObject, toJSLocalDOMWindow(element->document().frame(), currentWorld(*lexicalGlobalObject)), element);
    if (!thisValue)
	return QVariant();

    ScriptSourceCode sourceCode(scriptSource, JSC::SourceTaintedOrigin::Untainted);

    NakedPtr<JSC::Exception> evaluationException;
    JSC::JSValue evaluationResult = JSC::evaluate(lexicalGlobalObject, sourceCode.jsSourceCode(), thisValue, evaluationException);   
    if (evaluationException)
	return QVariant();

    JSValueRef evaluationResultRef = toRef(lexicalGlobalObject, evaluationResult);

    int distance = 0;
    JSValueRef* ignoredException = 0;
    return JSC::Bindings::convertValueToQVariant(toRef(lexicalGlobalObject), evaluationResultRef, QMetaType::Void, &distance, ignoredException);
}

static QVariant convertJSValueToWebElementVariant(JSC::JSGlobalObject* lexicalGlobalObject, JSC::JSObject* object, int *distance, HashSet<JSObjectRef>* visitedObjects)
{
    JSC::VM& vm = lexicalGlobalObject->vm();
    Element* element = 0;
    QVariant ret;
    if (object && object->inherits<JSElement>()) {
        element = JSElement::toWrapped(vm, object);
        *distance = 0;
        // Allow other objects to reach this one. This won't cause our algorithm to
        // loop since when we find an Element we do not recurse.
        visitedObjects->remove(toRef(object));
    } else if (object && object->inherits<JSElement>()) {
        // To support TestRunnerQt::nodesFromRect(), used in DRT, we do an implicit
        // conversion from 'document' to the QWebElement representing the 'document.documentElement'.
        // We can't simply use a QVariantMap in nodesFromRect() because it currently times out
        // when serializing DOMMimeType and DOMPlugin, even if we limit the recursion.
        element = JSDocument::toWrapped(vm, object)->documentElement();
    }

    return QVariant::fromValue<QWebElement>(QtWebElementRuntime::create(element));
}

static JSC::JSValue convertWebElementVariantToJSValue(JSC::JSGlobalObject* lexicalGlobalObject, WebCore::JSDOMGlobalObject* globalObject, const QVariant& variant)
{
    return WebCore::toJS(lexicalGlobalObject, globalObject, QtWebElementRuntime::get(variant.value<QWebElement>()));
}

void QtWebElementRuntime::initialize()
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;
    int id = qRegisterMetaType<QWebElement>();
    JSC::Bindings::registerCustomType(id, convertJSValueToWebElementVariant, convertWebElementVariantToJSValue);
}
