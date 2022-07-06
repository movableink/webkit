#include "config.h"

#include <QString>
#include <wtf/text/WTFString.h>

namespace WTF {

AtomString::AtomString(const QString& qstr)
    : m_string(AtomStringImpl::add(reinterpret_cast_ptr<const UChar*>(qstr.constData()), qstr.length()))
{
    if (qstr.isNull())
        return;
}

AtomString::AtomString(const QStringRef& ref)
    : m_string(AtomStringImpl::add(reinterpret_cast_ptr<const UChar*>(ref.unicode()), ref.length()))
{
    if (!ref.string())
        return;
}

}
