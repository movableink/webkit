#include "CryptoClientQt.h"
#include <WebCore/WrappedCryptoKey.h>
#include <WebCore/SerializedCryptoKeyWrap.h>
#include <wtf/TZoneMallocInlines.h>

WTF_MAKE_TZONE_ALLOCATED_IMPL(CryptoClientQt);

std::optional<Vector<uint8_t>> CryptoClientQt::wrapCryptoKey(const Vector<uint8_t>& key) const
{
    // This is no-op for Qt port, see GCrypt implementation for more details
    // This means that we don't need master key
    Vector<uint8_t> wrappedKey;
    if (!WebCore::wrapSerializedCryptoKey({ }, key, wrappedKey))
        return std::nullopt;
    return wrappedKey;
}

std::optional<Vector<uint8_t>> CryptoClientQt::unwrapCryptoKey(const Vector<uint8_t>& serializedKey) const
{
    auto wrappedKey = WebCore::readSerializedCryptoKey(serializedKey);
    if (!wrappedKey)
        return std::nullopt;

    return WebCore::unwrapCryptoKey({ }, *wrappedKey);
}
