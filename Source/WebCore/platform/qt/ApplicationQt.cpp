#include "config.h"

#include <QCoreApplication>

namespace WebCore {

const char* getApplicationName() {
  return QCoreApplication::applicationName().toUtf8().data();
}

} // namespace WebCore
