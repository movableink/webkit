/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "config.h"
#import "JavaScriptEvaluationResult.h"

#include "APIArray.h"
#include "APIDictionary.h"
#include "APINumber.h"
#include "APISerializedScriptValue.h"
#include "APIString.h"
#include "CoreIPCNumber.h"
#include "WKSharedAPICast.h"
#include <JavaScriptCore/JSContext.h>
#include <JavaScriptCore/JSValue.h>
#include <WebCore/ExceptionDetails.h>
#include <WebCore/SerializedScriptValue.h>
#include <wtf/RunLoop.h>

namespace WebKit {

JavaScriptEvaluationResult::JavaScriptEvaluationResult(JSObjectID root, HashMap<JSObjectID, Variant>&& map)
    : m_map(WTFMove(map))
    , m_root(root) { }

RetainPtr<id> JavaScriptEvaluationResult::toID(Variant&& root)
{
    return WTF::switchOn(WTFMove(root), [] (NullType nullType) -> RetainPtr<id> {
        switch (nullType) {
        case NullType::NSNull:
            return NSNull.null;
        case NullType::NullPointer:
            break;
        }
        return nullptr;
    }, [] (bool value) -> RetainPtr<id> {
        return value ? @YES : @NO;
    }, [] (CoreIPCNumber&& value) -> RetainPtr<id> {
        return value.toID();
    }, [] (String&& value) -> RetainPtr<id> {
        return (NSString *)value;
    }, [] (Seconds value) -> RetainPtr<id> {
        return [NSDate dateWithTimeIntervalSince1970:value.seconds()];
    }, [&] (Vector<JSObjectID>&& vector) -> RetainPtr<id> {
        RetainPtr array = adoptNS([[NSMutableArray alloc] initWithCapacity:vector.size()]);
        m_nsArrays.append({ WTFMove(vector), array });
        return array;
    }, [&] (HashMap<JSObjectID, JSObjectID>&& map) -> RetainPtr<id> {
        RetainPtr dictionary = adoptNS([[NSMutableDictionary alloc] initWithCapacity:map.size()]);
        m_nsDictionaries.append({ WTFMove(map), dictionary });
        return dictionary;
    });
}

RefPtr<API::Object> JavaScriptEvaluationResult::toAPI(Variant&& root)
{
    return WTF::switchOn(WTFMove(root), [] (NullType) -> RefPtr<API::Object> {
        return nullptr;
    }, [] (bool value) -> RefPtr<API::Object> {
        return API::Boolean::create(value);
    }, [] (CoreIPCNumber&& value) -> RefPtr<API::Object> {
        return API::Double::create([value.toID() doubleValue]);
    }, [] (String&& value) -> RefPtr<API::Object> {
        return API::String::create(value);
    }, [] (Seconds value) -> RefPtr<API::Object> {
        return API::Double::create(value.seconds());
    }, [&] (Vector<JSObjectID>&& vector) -> RefPtr<API::Object> {
        Ref array = API::Array::create();
        m_arrays.append({ WTFMove(vector), array });
        return { WTFMove(array) };
    }, [&] (HashMap<JSObjectID, JSObjectID>&& map) -> RefPtr<API::Object> {
        Ref dictionary = API::Dictionary::create();
        m_dictionaries.append({ WTFMove(map), dictionary });
        return { WTFMove(dictionary) };
    });
}

RetainPtr<id> JavaScriptEvaluationResult::toID()
{
    for (auto [identifier, variant] : std::exchange(m_map, { }))
        m_instantiatedNSObjects.add(identifier, toID(WTFMove(variant)));
    for (auto [vector, array] : std::exchange(m_nsArrays, { })) {
        for (auto identifier : vector) {
            if (RetainPtr element = m_instantiatedNSObjects.get(identifier))
                [array addObject:element.get()];
        }
    }
    for (auto [map, dictionary] : std::exchange(m_nsDictionaries, { })) {
        for (auto [keyIdentifier, valueIdentifier] : map) {
            RetainPtr key = m_instantiatedNSObjects.get(keyIdentifier);
            if (!key)
                continue;
            RetainPtr value = m_instantiatedNSObjects.get(valueIdentifier);
            if (!value)
                continue;
            [dictionary setObject:value.get() forKey:key.get()];
        }
    }
    return std::exchange(m_instantiatedNSObjects, { }).take(m_root);
}

WKRetainPtr<WKTypeRef> JavaScriptEvaluationResult::toWK()
{
    for (auto [identifier, variant] : std::exchange(m_map, { }))
        m_instantiatedObjects.add(identifier, toAPI(WTFMove(variant)));
    for (auto [vector, array] : std::exchange(m_arrays, { })) {
        for (auto identifier : vector) {
            if (RefPtr object = m_instantiatedObjects.get(identifier))
                Ref { array }->append(object.releaseNonNull());
        }
    }
    for (auto [map, dictionary] : std::exchange(m_dictionaries, { })) {
        for (auto [keyIdentifier, valueIdentifier] : map) {
            RefPtr key = dynamicDowncast<API::String>(m_instantiatedObjects.get(keyIdentifier));
            if (!key)
                continue;
            RefPtr value = m_instantiatedObjects.get(valueIdentifier);
            if (!value)
                continue;
            Ref { dictionary }->add(key->string(), WTFMove(value));
        }
    }
    return WebKit::toAPI(std::exchange(m_instantiatedObjects, { }).take(m_root).get());
}

auto JavaScriptEvaluationResult::toVariant(id object) -> Variant
{
    if (!object)
        return NullType::NullPointer;

    if ([object isKindOfClass:NSNull.class])
        return NullType::NSNull;

    if ([object isKindOfClass:NSNumber.class]) {
        if (CFNumberGetType((CFNumberRef)object) == kCFNumberCharType) {
            if ([object isEqual:@YES])
                return true;
            if ([object isEqual:@NO])
                return false;
        }
        return CoreIPCNumber((NSNumber *)object);
    }

    if ([object isKindOfClass:NSString.class])
        return String((NSString *)object);

    if ([object isKindOfClass:NSDate.class])
        return Seconds([(NSDate *)object timeIntervalSince1970]);

    if ([object isKindOfClass:NSArray.class]) {
        Vector<JSObjectID> vector;
        for (id element : (NSArray *)object)
            vector.append(addObjectToMap(element));
        return { WTFMove(vector) };
    }

    if ([object isKindOfClass:NSDictionary.class]) {
        HashMap<JSObjectID, JSObjectID> map;
        [(NSDictionary *)object enumerateKeysAndObjectsUsingBlock:[&](id key, id value, BOOL *) {
            map.add(addObjectToMap(key), addObjectToMap(value));
        }];
        return { WTFMove(map) };
    }

    // This object has been null checked and came from JSC::valueToObject which only supports these types.
    ASSERT_NOT_REACHED();
    return NullType::NullPointer;
}

JSObjectID JavaScriptEvaluationResult::addObjectToMap(id object)
{
    if (!object) {
        if (!m_nullObjectID) {
            m_nullObjectID = JSObjectID::generate();
            m_map.add(*m_nullObjectID, Variant { NullType::NullPointer });
        }
        return *m_nullObjectID;
    }

    auto it = m_objectsInMap.find(object);
    if (it != m_objectsInMap.end())
        return it->value;

    auto identifier = JSObjectID::generate();
    m_objectsInMap.set(object, identifier);
    m_map.add(identifier, toVariant(object));
    return identifier;
}

static std::optional<JSValueRef> roundTripThroughSerializedScriptValue(JSGlobalContextRef serializationContext, JSGlobalContextRef deserializationContext, JSValueRef value)
{
    if (RefPtr serialized = WebCore::SerializedScriptValue::create(serializationContext, value, nullptr))
        return serialized->deserialize(deserializationContext, nullptr);
    return std::nullopt;
}

static RetainPtr<id> convertToObjC(JSGlobalContextRef context, JSValueRef value)
{
    return [JSValue valueWithJSValueRef:value inContext:[JSContext contextWithJSGlobalContextRef:context]].toObject;
}

Expected<JavaScriptEvaluationResult, std::optional<WebCore::ExceptionDetails>> JavaScriptEvaluationResult::extract(JSGlobalContextRef context, JSValueRef value)
{
    JSRetainPtr deserializationContext = API::SerializedScriptValue::deserializationContext();

    auto result = roundTripThroughSerializedScriptValue(context, deserializationContext.get(), value);
    if (!result)
        return makeUnexpected(std::nullopt);
    return { JavaScriptEvaluationResult { deserializationContext.get(), *result } };
}

static bool isSerializable(id argument)
{
    if (!argument)
        return true;

    if ([argument isKindOfClass:[NSString class]] || [argument isKindOfClass:[NSNumber class]] || [argument isKindOfClass:[NSDate class]] || [argument isKindOfClass:[NSNull class]])
        return true;

    if ([argument isKindOfClass:[NSArray class]]) {
        __block BOOL valid = true;

        [argument enumerateObjectsUsingBlock:^(id object, NSUInteger, BOOL *stop) {
            if (!isSerializable(object)) {
                valid = false;
                *stop = YES;
            }
        }];

        return valid;
    }

    if ([argument isKindOfClass:[NSDictionary class]]) {
        __block bool valid = true;

        [argument enumerateKeysAndObjectsUsingBlock:^(id key, id value, BOOL *stop) {
            if (!isSerializable(key) || !isSerializable(value)) {
                valid = false;
                *stop = YES;
            }
        }];

        return valid;
    }

    return false;
}

std::optional<JavaScriptEvaluationResult> JavaScriptEvaluationResult::extract(id object)
{
    if (object && !isSerializable(object))
        return std::nullopt;
    return JavaScriptEvaluationResult { object };
}

JavaScriptEvaluationResult::JavaScriptEvaluationResult(id object)
    : m_root(addObjectToMap(object))
{
    m_objectsInMap.clear();
    m_nullObjectID = std::nullopt;
}

JavaScriptEvaluationResult::JavaScriptEvaluationResult(JSGlobalContextRef context, JSValueRef value)
    : JavaScriptEvaluationResult(convertToObjC(context, value).get())
{
    // FIXME: This does not need to roundtrip through ObjC.
    // As a performance improvement we could make a converter directly from JS.
}

JSValueRef JavaScriptEvaluationResult::toJS(JSGlobalContextRef context)
{
    // FIXME: This does not need to roundtrip through ObjC.
    // As a performance improvement we could make a converter directly to JS.
    if (JSValueRef result = [[JSValue valueWithObject:toID().get() inContext:[JSContext contextWithJSGlobalContextRef:context]] JSValueRef])
        return result;
    return JSValueMakeUndefined(context);
}

}
