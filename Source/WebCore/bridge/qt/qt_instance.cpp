/*
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "config.h"
#include "qt_instance.h"

#include <JavaScriptCore/APICast.h>
#include "CommonVM.h"
#include <JavaScriptCore/Error.h>
#include "JSDOMBinding.h"
#include "JSDOMWindowBase.h"
#include <JavaScriptCore/FunctionPrototype.h>
#include <JavaScriptCore/JSGlobalObject.h>
#include <JavaScriptCore/JSLock.h>
#include <JavaScriptCore/PropertyNameArray.h>
#include "qt_class.h"
#include "qt_runtime.h"
#include "runtime_object.h"

#include <qdebug.h>
#include <qhash.h>
#include <qmetaobject.h>
#include <qmetatype.h>

namespace JSC {
namespace Bindings {

// Cache QtInstances
typedef QMultiHash<void*, QtInstance*> QObjectInstanceMap;
static QObjectInstanceMap cachedInstances;

// Derived RuntimeObject
class QtRuntimeObject : public RuntimeObject {
public:
    typedef RuntimeObject Base;

    static QtRuntimeObject* create(VM& vm, Structure* structure, RefPtr<Instance>&& instance)
    {
        QtRuntimeObject* object = new (allocateCell<QtRuntimeObject>(vm)) QtRuntimeObject(vm, structure, WTFMove(instance));
        object->finishCreation(vm);
        return object;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType,  StructureFlags), info());
    }

protected:
    // FIXME
    // static const unsigned StructureFlags = RuntimeObject::StructureFlags | OverridesVisitChildren;

private:
    QtRuntimeObject(VM&, Structure*, RefPtr<Instance>&&);
};

const ClassInfo QtRuntimeObject::s_info = { "QtRuntimeObject"_s, &RuntimeObject::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(QtRuntimeObject) };

QtRuntimeObject::QtRuntimeObject(VM& vm, Structure* structure, RefPtr<Instance>&& instance)
    : RuntimeObject(vm, structure, WTFMove(instance))
{
}

// QtInstance
QtInstance::QtInstance(QObject* o, RefPtr<RootObject>&& rootObject, ValueOwnership ownership)
    : Instance(WTFMove(rootObject))
    , m_class(0)
    , m_object(o)
    , m_hashkey(o)
    , m_ownership(ownership)
{
}

QtInstance::~QtInstance()
{
    JSLockHolder lock(WebCore::commonVM());

    cachedInstances.remove(m_hashkey);

    qDeleteAll(m_methods);
    m_methods.clear();

    qDeleteAll(m_fields);
    m_fields.clear();

    if (m_object) {
        switch (m_ownership) {
        case QtOwnership:
            break;
        case AutoOwnership:
            if (m_object.data()->parent())
                break;
            // fall through!
        case ScriptOwnership:
            delete m_object.data();
            break;
        }
    }
}

RefPtr<QtInstance> QtInstance::getQtInstance(QObject* o, RootObject* rootObject, ValueOwnership ownership)
{
    JSLockHolder lock(WebCore::commonVM());

    Q_FOREACH (QtInstance* instance, cachedInstances.values(o))
        if (instance->rootObject() == rootObject) {
            // The garbage collector removes instances, but it may happen that the wrapped
            // QObject dies before the gc kicks in. To handle that case we have to do an additional
            // check if to see if the instance's wrapped object is still alive. If it isn't, then
            // we have to create a new wrapper.
            if (!instance->getObject())
                cachedInstances.remove(instance->hashKey());
            else
                return instance;
        }

    RefPtr<QtInstance> ret = QtInstance::create(o, rootObject, ownership);
    cachedInstances.insert(o, ret.get());

    return ret;
}

bool QtInstance::getOwnPropertySlot(JSObject* object, JSGlobalObject* lexicalGlobalObject, PropertyName propertyName, PropertySlot& slot)
{
    return JSObject::getOwnPropertySlot(object, lexicalGlobalObject, propertyName, slot);
}

bool QtInstance::put(JSObject* object, JSGlobalObject* lexicalGlobalObject, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    return JSObject::put(object, lexicalGlobalObject, propertyName, value, slot);
}

QtInstance* QtInstance::getInstance(JSGlobalObject* lexicalGlobalObject, JSObject* object)
{
    if (!object)
        return 0;
    if (!object->inherits(QtRuntimeObject::info()))
        return 0;
    return static_cast<QtInstance*>(static_cast<RuntimeObject*>(object)->getInternalInstance());
}

Class* QtInstance::getClass() const
{
    if (!m_class) {
        if (!m_object)
            return 0;
        m_class = QtClass::classForObject(m_object.data());
    }
    return m_class;
}

RuntimeObject* QtInstance::newRuntimeObject(JSGlobalObject* lexicalGlobalObject)
{
    JSLockHolder lock(lexicalGlobalObject);
    qDeleteAll(m_methods);
    m_methods.clear();

    // FIXME: deprecatedGetDOMStructure uses the prototype off of the wrong global object.
    return QtRuntimeObject::create(lexicalGlobalObject->vm(), WebCore::deprecatedGetDOMStructure<QtRuntimeObject>(lexicalGlobalObject), this);
}

void QtInstance::getPropertyNames(JSGlobalObject* lexicalGlobalObject, PropertyNameArray& array)
{
    // This is the enumerable properties, so put:
    // properties
    // dynamic properties
    // slots
    QObject* obj = getObject();
    if (obj) {
        VM& vm = lexicalGlobalObject->vm();
        const QMetaObject* meta = obj->metaObject();

        int i;
        for (i = 0; i < meta->propertyCount(); i++) {
            QMetaProperty prop = meta->property(i);
            if (prop.isScriptable())
                array.add(Identifier::fromString(vm, String::fromUTF8(prop.name())));
        }

#ifndef QT_NO_PROPERTIES
        QList<QByteArray> dynProps = obj->dynamicPropertyNames();
        Q_FOREACH (const QByteArray& ba, dynProps)
            array.add(Identifier::fromString(vm, String::fromUTF8(ba.constData())));
#endif

        const int methodCount = meta->methodCount();
        for (i = 0; i < methodCount; i++) {
            QMetaMethod method = meta->method(i);
            if (method.access() != QMetaMethod::Private) {
                QByteArray sig = method.methodSignature();
                array.add(Identifier::fromString(vm, String(sig.constData(), sig.length())));
            }
        }
    }
}

JSValue QtInstance::getMethod(JSGlobalObject* lexicalGlobalObject, PropertyName propertyName)
{
    if (!getClass())
        return jsNull();
    Method* method = m_class->methodNamed(propertyName, this);
    return RuntimeMethod::create(lexicalGlobalObject, lexicalGlobalObject, WebCore::deprecatedGetDOMStructure<RuntimeMethod>(lexicalGlobalObject), propertyName.publicName(), method);
}

JSValue QtInstance::invokeMethod(JSGlobalObject*, CallFrame*, RuntimeMethod*)
{
    // Implemented via fallbackMethod & QtRuntimeMetaMethod::callAsFunction
    return jsUndefined();
}

JSValue QtInstance::defaultValue(JSGlobalObject* lexicalGlobalObject, PreferredPrimitiveType hint) const
{
    if (hint == PreferString)
        return stringValue(lexicalGlobalObject);
    if (hint == PreferNumber)
        return numberValue(lexicalGlobalObject);
    return valueOf(lexicalGlobalObject);
}

JSValue QtInstance::stringValue(JSGlobalObject* lexicalGlobalObject) const
{
    QObject* obj = getObject();
    if (!obj)
        return jsNull();

    // Hmm.. see if there is a toString defined
    QByteArray buf;
    bool useDefault = true;
    getClass();
    if (m_class) {
        // Cheat and don't use the full name resolution
        int index = obj->metaObject()->indexOfMethod("toString()");
        if (index >= 0) {
            QMetaMethod m = obj->metaObject()->method(index);
            // Check to see how much we can call it
            if (m.access() != QMetaMethod::Private
                && m.methodType() != QMetaMethod::Signal
                && m.parameterCount() == 0
                && m.returnType() != QMetaType::Void) {
                QVariant ret(m.returnMetaType(), (void*)0);
                void * qargs[1];
                qargs[0] = ret.data();

                if (QMetaObject::metacall(obj, QMetaObject::InvokeMetaMethod, index, qargs) < 0) {
                    if (ret.isValid() && ret.canConvert(QVariant::String)) {
                        buf = ret.toString().toLatin1().constData(); // ### Latin 1? Ascii?
                        useDefault = false;
                    }
                }
            }
        }
    }

    if (useDefault) {
        const QMetaObject* meta = obj ? obj->metaObject() : &QObject::staticMetaObject;
        QString name = obj ? obj->objectName() : QString::fromUtf8("unnamed");
        QString str = QString::fromUtf8("%0(name = \"%1\")")
                      .arg(QLatin1String(meta->className())).arg(name);

        buf = str.toLatin1();
    }
    return jsString(lexicalGlobalObject->vm(), String::fromUTF8(buf.constData()));
}

JSValue QtInstance::numberValue(JSGlobalObject*) const
{
    return jsNumber(0);
}

JSValue QtInstance::booleanValue() const
{
    // ECMA 9.2
    return jsBoolean(getObject());
}

JSValue QtInstance::valueOf(JSGlobalObject* lexicalGlobalObject) const
{
    return stringValue(lexicalGlobalObject);
}

QByteArray QtField::name() const
{
    if (m_type == MetaProperty)
        return m_property.name();
    if (m_type == ChildObject && m_childObject)
        return m_childObject.data()->objectName().toLatin1();
#ifndef QT_NO_PROPERTIES
    if (m_type == DynamicProperty)
        return m_dynamicProperty;
#endif
    return QByteArray(); // deleted child object
}

JSValue QtField::valueFromInstance(JSGlobalObject* lexicalGlobalObject, const Instance* inst) const
{
    VM& vm = lexicalGlobalObject->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    const QtInstance* instance = static_cast<const QtInstance*>(inst);
    QObject* obj = instance->getObject();

    if (obj) {
        QVariant val;
        if (m_type == MetaProperty) {
            if (m_property.isReadable())
                val = m_property.read(obj);
            else
                return jsUndefined();
        } else if (m_type == ChildObject)
            val = QVariant::fromValue((QObject*) m_childObject.data());
#ifndef QT_NO_PROPERTIES
        else if (m_type == DynamicProperty)
            val = obj->property(m_dynamicProperty);
#endif
        JSValueRef exception = 0;
        JSValueRef jsValue = convertQVariantToValue(toRef(lexicalGlobalObject), inst->rootObject(), val, &exception);
        if (exception)
            return throwException(lexicalGlobalObject, scope, toJS(lexicalGlobalObject, exception));
        return toJS(lexicalGlobalObject, jsValue);
    }
    QString msg = QString(QLatin1String("cannot access member `%1' of deleted QObject")).arg(QLatin1String(name()));
    return throwException(lexicalGlobalObject, scope, createError(lexicalGlobalObject, String(msg)));
}

bool QtField::setValueToInstance(JSGlobalObject* lexicalGlobalObject, const Instance* inst, JSValue aValue) const
{
    if (m_type == ChildObject) // QtScript doesn't allow setting to a named child
        return false;

    VM& vm = lexicalGlobalObject->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    const QtInstance* instance = static_cast<const QtInstance*>(inst);
    QObject* obj = instance->getObject();
    if (obj) {
        QMetaType::Type argtype = QMetaType::Void;
        if (m_type == MetaProperty)
            argtype = (QMetaType::Type) m_property.userType();

        // dynamic properties just get any QVariant
        JSValueRef exception = 0;
        QVariant val = convertValueToQVariant(toRef(lexicalGlobalObject), toRef(lexicalGlobalObject, aValue), argtype, 0, &exception);
        if (exception) {
            throwException(lexicalGlobalObject, scope, toJS(lexicalGlobalObject, exception));
            return false;
        }
        if (m_type == MetaProperty) {
            if (m_property.isWritable())
                return m_property.write(obj, val);
        }
#ifndef QT_NO_PROPERTIES
        else if (m_type == DynamicProperty)
            return obj->setProperty(m_dynamicProperty.constData(), val);
#endif
    } else {
        QString msg = QLatin1String("cannot access member `%1' of deleted QObject").arg(QLatin1String(name()));
        throwException(lexicalGlobalObject, scope, createError(lexicalGlobalObject, String(msg)));
    }
    return false;
}


}
}
