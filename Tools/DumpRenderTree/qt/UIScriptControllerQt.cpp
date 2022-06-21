#include "config.h"
#include "UIScriptController.h"
#include "UIScriptContext.h"

namespace WTR {

Ref<UIScriptController> UIScriptController::create(UIScriptContext& context)
{
    return adoptRef(*new UIScriptController(context));
}

}
