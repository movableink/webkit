#include "config.h"
#include "SystemFontDatabase.h"

#include "NotImplemented.h"
#include "WebKitFontFamilyNames.h"
#include <wtf/NeverDestroyed.h>

namespace WebCore {

SystemFontDatabase& SystemFontDatabase::singleton()
{
    static NeverDestroyed<SystemFontDatabase> database = SystemFontDatabase();
    return database.get();
}

auto SystemFontDatabase::platformSystemFontShorthandInfo(FontShorthand) -> SystemFontShorthandInfo
{
    notImplemented();
    return { WebKitFontFamilyNames::standardFamily, 16, normalWeightValue() };
}

void SystemFontDatabase::platformInvalidate()
{
}

}
