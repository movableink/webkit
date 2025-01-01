/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 * Copyright (C) 2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <wtf/StdLibExtras.h>
#include <wtf/text/StringCommon.h>
#include <wtf/text/StringParsingBuffer.h>

WTF_ALLOW_UNSAFE_BUFFER_USAGE_BEGIN

namespace WTF {

template<typename CharacterType> inline bool isNotASCIISpace(CharacterType c)
{
    return !isUnicodeCompatibleASCIIWhitespace(c);
}

template<typename CharacterType, typename DelimiterType> bool skipExactly(const CharacterType*& position, const CharacterType* end, DelimiterType delimiter)
{
    if (position < end && *position == delimiter) {
        ++position;
        return true;
    }
    return false;
}

template<typename CharacterType, typename DelimiterType> bool skipExactly(std::span<const CharacterType>& data, DelimiterType delimiter)
{
    if (!data.empty() && data.front() == delimiter) {
        data = data.subspan(1);
        return true;
    }
    return false;
}

template<typename CharacterType, typename DelimiterType> bool skipExactly(StringParsingBuffer<CharacterType>& buffer, DelimiterType delimiter)
{
    if (buffer.hasCharactersRemaining() && *buffer == delimiter) {
        ++buffer;
        return true;
    }
    return false;
}

template<bool characterPredicate(LChar)> bool skipExactly(const LChar*& position, const LChar* end)
{
    if (position < end && characterPredicate(*position)) {
        ++position;
        return true;
    }
    return false;
}

template<bool characterPredicate(UChar)> bool skipExactly(const UChar*& position, const UChar* end)
{
    if (position < end && characterPredicate(*position)) {
        ++position;
        return true;
    }
    return false;
}

template<bool characterPredicate(LChar)> bool skipExactly(StringParsingBuffer<LChar>& buffer)
{
    if (buffer.hasCharactersRemaining() && characterPredicate(*buffer)) {
        ++buffer;
        return true;
    }
    return false;
}

template<bool characterPredicate(UChar)> bool skipExactly(StringParsingBuffer<UChar>& buffer)
{
    if (buffer.hasCharactersRemaining() && characterPredicate(*buffer)) {
        ++buffer;
        return true;
    }
    return false;
}

template<bool characterPredicate(LChar)> bool skipExactly(std::span<const LChar>& buffer)
{
    if (!buffer.empty() && characterPredicate(buffer[0])) {
        buffer = buffer.subspan(1);
        return true;
    }
    return false;
}

template<bool characterPredicate(UChar)> bool skipExactly(std::span<const UChar>& buffer)
{
    if (!buffer.empty() && characterPredicate(buffer[0])) {
        buffer = buffer.subspan(1);
        return true;
    }
    return false;
}

template<typename CharacterType, typename DelimiterType> void skipUntil(const CharacterType*& position, const CharacterType* end, DelimiterType delimiter)
{
    while (position < end && *position != delimiter)
        ++position;
}

template<typename CharacterType, typename DelimiterType> void skipUntil(StringParsingBuffer<CharacterType>& buffer, DelimiterType delimiter)
{
    while (buffer.hasCharactersRemaining() && *buffer != delimiter)
        ++buffer;
}

template<bool characterPredicate(LChar)> void skipUntil(const LChar*& position, const LChar* end)
{
    while (position < end && !characterPredicate(*position))
        ++position;
}

template<bool characterPredicate(UChar)> void skipUntil(const UChar*& position, const UChar* end)
{
    while (position < end && !characterPredicate(*position))
        ++position;
}

template<bool characterPredicate(LChar)> void skipUntil(std::span<const LChar>& data)
{
    size_t index = 0;
    while (index < data.size() && !characterPredicate(data[index]))
        ++index;
    data = data.subspan(index);
}

template<bool characterPredicate(UChar)> void skipUntil(std::span<const UChar>& data)
{
    size_t index = 0;
    while (index < data.size() && !characterPredicate(data[index]))
        ++index;
    data = data.subspan(index);
}

template<bool characterPredicate(LChar)> void skipUntil(StringParsingBuffer<LChar>& buffer)
{
    while (buffer.hasCharactersRemaining() && !characterPredicate(*buffer))
        ++buffer;
}

template<bool characterPredicate(UChar)> void skipUntil(StringParsingBuffer<UChar>& buffer)
{
    while (buffer.hasCharactersRemaining() && !characterPredicate(*buffer))
        ++buffer;
}

template<typename CharacterType, typename DelimiterType> void skipWhile(StringParsingBuffer<CharacterType>& buffer, DelimiterType delimiter)
{
    while (buffer.hasCharactersRemaining() && *buffer == delimiter)
        ++buffer;
}

template<bool characterPredicate(LChar)> void skipWhile(const LChar*& position, const LChar* end)
{
    while (position < end && characterPredicate(*position))
        ++position;
}

template<bool characterPredicate(UChar)> void skipWhile(const UChar*& position, const UChar* end)
{
    while (position < end && characterPredicate(*position))
        ++position;
}

template<bool characterPredicate(LChar)> void skipWhile(std::span<const LChar>& data)
{
    size_t index = 0;
    while (index < data.size() && characterPredicate(data[index]))
        ++index;
    data = data.subspan(index);
}

template<bool characterPredicate(UChar)> void skipWhile(std::span<const UChar>& data)
{
    size_t index = 0;
    while (index < data.size() && characterPredicate(data[index]))
        ++index;
    data = data.subspan(index);
}

template<bool characterPredicate(LChar)> void skipWhile(StringParsingBuffer<LChar>& buffer)
{
    while (buffer.hasCharactersRemaining() && characterPredicate(*buffer))
        ++buffer;
}

template<bool characterPredicate(UChar)> void skipWhile(StringParsingBuffer<UChar>& buffer)
{
    while (buffer.hasCharactersRemaining() && characterPredicate(*buffer))
        ++buffer;
}

template<bool characterPredicate(LChar)> void reverseSkipWhile(const LChar*& position, const LChar* start)
{
    while (position >= start && characterPredicate(*position))
        --position;
}

template<bool characterPredicate(UChar)> void reverseSkipWhile(const UChar*& position, const UChar* start)
{
    while (position >= start && characterPredicate(*position))
        --position;
}

template<typename CharacterType> bool skipExactlyIgnoringASCIICase(const CharacterType*& position, const CharacterType* end, ASCIILiteral literal)
{
    auto literalLength = literal.length();

    if (position + literalLength > end)
        return false;
    if (!equalLettersIgnoringASCIICaseWithLength(std::span { position, literalLength }, literal.span8(), literalLength))
        return false;
    position += literalLength;
    return true;
}

template<typename CharacterType> bool skipExactlyIgnoringASCIICase(StringParsingBuffer<CharacterType>& buffer, ASCIILiteral literal)
{
    auto literalLength = literal.length();

    if (buffer.lengthRemaining() < literalLength)
        return false;
    if (!equalLettersIgnoringASCIICaseWithLength(buffer.span(), literal.span8(), literalLength))
        return false;
    buffer += literalLength;
    return true;
}

template<typename CharacterType, std::size_t Extent> bool skipLettersExactlyIgnoringASCIICase(StringParsingBuffer<CharacterType>& buffer, std::span<const CharacterType, Extent> letters)
{
    if (buffer.lengthRemaining() < letters.size())
        return false;
    for (unsigned i = 0; i < letters.size(); ++i) {
        ASSERT(isASCIIAlpha(letters[i]));
        if (!isASCIIAlphaCaselessEqual(buffer.position()[i], static_cast<char>(letters[i])))
            return false;
    }
    buffer += letters.size();
    return true;
}

template<typename CharacterType, std::size_t Extent> bool skipLettersExactlyIgnoringASCIICase(std::span<const CharacterType>& buffer, std::span<const CharacterType, Extent> letters)
{
    if (buffer.size() < letters.size())
        return false;
    if (!equalLettersIgnoringASCIICaseWithLength(buffer, letters, letters.size()))
        return false;
    buffer = buffer.subspan(letters.size());
    return true;
}

template<typename CharacterType, std::size_t Extent> constexpr bool skipCharactersExactly(StringParsingBuffer<CharacterType>& buffer, std::span<const CharacterType, Extent> string)
{
    if (!spanHasPrefix(buffer.span(), string))
        return false;
    buffer += string.size();
    return true;
}

template<typename T> T& consumeSingleElement(std::span<T>& data)
{
    T& value = data[0];
    data = data.subspan(1);
    return value;
}

template<typename T> std::span<T> consumeSpan(std::span<T>& data, size_t amountToConsume)
{
    auto consumed = data.first(amountToConsume);
    data = data.subspan(amountToConsume);
    return consumed;
}

// Adapt a UChar-predicate to an LChar-predicate.
template<bool characterPredicate(UChar)>
static inline bool LCharPredicateAdapter(LChar c) { return characterPredicate(c); }

} // namespace WTF

using WTF::LCharPredicateAdapter;
using WTF::consumeSingleElement;
using WTF::consumeSpan;
using WTF::isNotASCIISpace;
using WTF::reverseSkipWhile;
using WTF::skipCharactersExactly;
using WTF::skipExactly;
using WTF::skipExactlyIgnoringASCIICase;
using WTF::skipLettersExactlyIgnoringASCIICase;
using WTF::skipUntil;
using WTF::skipWhile;

WTF_ALLOW_UNSAFE_BUFFER_USAGE_END

