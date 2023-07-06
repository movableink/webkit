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
#include <WebCore/ScriptSourceCode.h>

// Extracted from qwebelement.cpp because that file needs rtti and WebCore::ScriptSourceCode has rtti disabled
JSValueRef evaluateJavaScriptString(JSC::JSGlobalObject* lexicalGlobalObject, const String& scriptSource, const JSC::JSValue& thisValue)
{
    WebCore::ScriptSourceCode sourceCode(scriptSource);
 
    NakedPtr<JSC::Exception> evaluationException;
    JSC::JSValue evaluationResult = JSC::evaluate(lexicalGlobalObject, sourceCode.jsSourceCode(), thisValue, evaluationException);   
    if (evaluationException)
	return JSValueMakeUndefined(toRef(lexicalGlobalObject));

    return toRef(lexicalGlobalObject, evaluationResult);
}
