#include "CryptoClientQt.h"
#include "WebCore/SerializedCryptoKeyWrap.h"

std::optional<Vector<uint8_t>> CryptoClientQt::wrapCryptoKey(const Vector<uint8_t>& key) const
{
    // This is no-op for Qt port, see GCrypt implementation for more details
    // This means that we don't need master key
    Vector<uint8_t> wrappedKey;
    if (!WebCore::wrapSerializedCryptoKey({ }, key, wrappedKey))
        return std::nullopt;
    return wrappedKey;
}

std::optional<Vector<uint8_t>> CryptoClientQt::unwrapCryptoKey(const Vector<uint8_t>& wrappedKey) const
{
    // This is no-op so we don't need master key
    Vector<uint8_t> key;
    if (!WebCore::unwrapSerializedCryptoKey({ }, wrappedKey, key))
        return std::nullopt;
    return key;
}
