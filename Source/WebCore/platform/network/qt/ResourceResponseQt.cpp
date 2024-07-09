#include "config.h"

#include "ResourceResponse.h"

#include "HTTPHeaderNames.h"
#include "HTTPParsers.h"
#include "MIMETypeRegistry.h"
#include <QMimeDatabase>

namespace WebCore {

String ResourceResponse::platformSuggestedFilename() const
{
    // FIXME: Move to base class
    String contentDisposition(httpHeaderField(HTTPHeaderName::ContentDisposition));

    if (contentDisposition.isEmpty())
        return { };

    if (contentDisposition.is8Bit())
        contentDisposition = String::fromUTF8WithLatin1Fallback(contentDisposition.span8());

    StringView suggestedFilename = filenameFromHTTPContentDisposition(contentDisposition);
    if (!suggestedFilename.isEmpty())
        return suggestedFilename.toString();

    Vector<String> extensions = MIMETypeRegistry::extensionsForMIMEType(mimeType());
    if (extensions.isEmpty())
        return url().lastPathComponent().toString();

    // If the suffix doesn't match the MIME type, correct the suffix.
    QString filename = url().lastPathComponent().toString();
    const String suffix = QMimeDatabase().suffixForFileName(filename);
    if (!extensions.contains(suffix)) {
        filename.chop(suffix.length());
        filename += MIMETypeRegistry::preferredExtensionForMIMEType(mimeType());
    }
    return filename;
}

}
