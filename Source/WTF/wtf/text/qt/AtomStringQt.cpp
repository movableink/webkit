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

AtomString::AtomString(QStringView view)
    : m_string(AtomStringImpl::add(reinterpret_cast_ptr<const UChar*>(view.constData()), view.length()))
{
    if (view.isNull())
        return;
}

}
