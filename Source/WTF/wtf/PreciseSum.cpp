/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This is a C++ port of the xsum implementation, 
 * originally invented by Radford M. Neal, and ported by Keita Nonaka.
 * 
 * The original C implementation is from https://gitlab.com/radfordneal/xsum
 * The C++ implementation is from https://github.com/Gumichocopengin8/xsum.cpp
 *
 * The LICENSE is as follows:
 *
 * This software for exact summation of floating-point values is licensed
 * under the "MIT license", included here.
 * 
 * Copyright 2015, 2018, 2021, 2024 Radford M. Neal
 * Copyright 2025 Keita Nonaka
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "config.h"
#include <wtf/PreciseSum.h>

#include <array>
#include <cmath>

namespace WTF {

namespace {

constexpr int64_t XSUM_MANTISSA_BITS = 52; // Bits in fp mantissa, excludes implict 1
constexpr int64_t XSUM_EXP_BITS = 11; // Bits in fp exponent
constexpr int64_t XSUM_MANTISSA_MASK = (static_cast<int64_t>(1) << XSUM_MANTISSA_BITS) - 1; // Mask for mantissa bits
constexpr int64_t XSUM_EXP_MASK = (1 << XSUM_EXP_BITS) - 1; // Mask for exponent
constexpr int64_t XSUM_EXP_BIAS = (1 << (XSUM_EXP_BITS - 1)) - 1; // Bias added to signed exponent
constexpr int64_t XSUM_SIGN_BIT = XSUM_MANTISSA_BITS + XSUM_EXP_BITS; // Position of sign bit
constexpr uint64_t XSUM_SIGN_MASK = static_cast<uint64_t>(1) << XSUM_SIGN_BIT; // Mask for sign bit
constexpr int64_t XSUM_SCHUNK_BITS = 64; // Bits in chunk of the small accumulator
constexpr int64_t XSUM_LOW_EXP_BITS = 5; // # of low bits of exponent, in one chunk
constexpr int64_t XSUM_LOW_EXP_MASK = (1 << XSUM_LOW_EXP_BITS) - 1; // Mask for low-order exponent bits
constexpr int64_t XSUM_HIGH_EXP_BITS = XSUM_EXP_BITS - XSUM_LOW_EXP_BITS; // # of high exponent bits for index
constexpr int64_t XSUM_SCHUNKS = (1 << XSUM_HIGH_EXP_BITS) + 3; // # of chunks in small accumulator
constexpr int64_t XSUM_LOW_MANTISSA_BITS = 1 << XSUM_LOW_EXP_BITS; // Bits in low part of mantissa
constexpr int64_t XSUM_LOW_MANTISSA_MASK = (static_cast<int64_t>(1) << XSUM_LOW_MANTISSA_BITS) - 1; // Mask for low bits
constexpr int64_t XSUM_SMALL_CARRY_BITS = (XSUM_SCHUNK_BITS - 1) - XSUM_MANTISSA_BITS; // Bits sums can carry into
constexpr int64_t XSUM_SMALL_CARRY_TERMS = (1 << XSUM_SMALL_CARRY_BITS) - 1; // # terms can add before need prop.

} // anonymous namespace

namespace Xsum {

SmallAccumulator::SmallAccumulator(
    int addsUntilPropagate,
    int64_t inf,
    int64_t nan
) : chunk(XSUM_SCHUNKS, 0LL), addsUntilPropagate { addsUntilPropagate }, inf { inf }, nan { nan } { }

} // namespace Xsum

PreciseSum::PreciseSum()
: m_smallAccumulator { XSUM_SMALL_CARRY_TERMS, 0, 0 }, m_sizeCount { 0 }, m_hasPosNumber { false } { }

/*
ADD A VECTOR OF FLOATING-POINT NUMBERS TO A SMALL ACCUMULATOR. Mixes
calls of xsumCarryPropagate with calls of xsumAdd1NoCarry.
*/
void PreciseSum::addList(const std::span<const double> vec)
{
    size_t offset = 0;
    size_t n = vec.size();

    while (0 < n) {
        if (!m_smallAccumulator.addsUntilPropagate)
            xsumCarryPropagate();
        size_t m = std::min(static_cast<int>(n), m_smallAccumulator.addsUntilPropagate);
        for (size_t i = 0; i < m; i++) {
            const double value = vec[offset + i];
            incrementWhenValueAdded(value);
            xsumAdd1NoCarry(value);
        }
        m_smallAccumulator.addsUntilPropagate -= m;
        offset += m;
        n -= m;
    }
}

/*
ADD ONE DOUBLE TO A SMALL ACCUMULATOR. This is equivalent to, but
somewhat faster than, calling xsum_small_addv with a vector of one
value.
*/
void PreciseSum::add(double value)
{
    incrementWhenValueAdded(value);
    if (!m_smallAccumulator.addsUntilPropagate)
        xsumCarryPropagate();
    xsumAdd1NoCarry(value);
    m_smallAccumulator.addsUntilPropagate -= 1;
}

/*
RETURN THE RESULT OF ROUNDING A SMALL ACCUMULATOR. The rounding mode
is to nearest, with ties to even. The small accumulator may be modified
by this operation (by carry propagation being done), but the value it
represents should not change.
*/
double PreciseSum::compute()
{
    if (m_smallAccumulator.nan)
        return std::bit_cast<double>(m_smallAccumulator.nan);
    if (m_smallAccumulator.inf)
        return std::bit_cast<double>(m_smallAccumulator.inf);
    if (!m_sizeCount)
        return -0.0;

    const int i = xsumCarryPropagate();
    int64_t ivalue = m_smallAccumulator.chunk[i];
    int64_t intv = 0;
    if (i <= 1) {
        if (!ivalue)
            return !m_hasPosNumber ? -0.0 : 0.0;
        if (!i) {
            intv = 0 <= ivalue ? ivalue : -ivalue;
            intv >>= 1;
            if (ivalue < 0)
                intv |= XSUM_SIGN_MASK;
            return std::bit_cast<double>(intv);
        }
        int64_t intv = ivalue * (static_cast<int64_t>(1) << (XSUM_LOW_MANTISSA_BITS - 1)) + (m_smallAccumulator.chunk[0] >> 1);
        if (intv < 0) {
            if (intv > -(static_cast<int64_t>(1) << XSUM_MANTISSA_BITS)) {
                intv = (-intv) | XSUM_SIGN_MASK;
                return std::bit_cast<double>(intv);
            }
        } else if (static_cast<uint64_t>(intv) < static_cast<uint64_t>(1) << XSUM_MANTISSA_BITS)
            return std::bit_cast<double>(intv);
    }

    const double fltv = static_cast<double>(ivalue);
    intv = std::bit_cast<int64_t>(fltv);
    int e = (intv >> XSUM_MANTISSA_BITS) & XSUM_EXP_MASK;
    int more = 2 + XSUM_MANTISSA_BITS + XSUM_EXP_BIAS - e;

    ivalue *= static_cast<int64_t>(1) << more;
    int j = i - 1;
    int64_t lower = m_smallAccumulator.chunk[j];
    if (more >= XSUM_LOW_MANTISSA_BITS) {
        more -= XSUM_LOW_MANTISSA_BITS;
        ivalue += lower << more;
        j -= 1;
        lower = j < 0 ? 0 : m_smallAccumulator.chunk[j];
    }
    ivalue += lower >> (XSUM_LOW_MANTISSA_BITS - more);
    lower &= (static_cast<int64_t>(1) << (XSUM_LOW_MANTISSA_BITS - more)) - 1;

    bool shouldRoundAwayFromZero = false;
    if (0 <= ivalue) {
        intv = 0;
        if (!(ivalue & 2)) {
            // this is not required,
            // but removing the branch would change the logic
            // leave it as it is
            shouldRoundAwayFromZero = false;
        } else if ((ivalue & 1))
            shouldRoundAwayFromZero = true;
        else if ((ivalue & 4))
            shouldRoundAwayFromZero = true;
        else {
            if (!lower) {
                while (j > 0) {
                    j -= 1;
                    if (m_smallAccumulator.chunk[j]) {
                        lower = 1;
                        break;
                    }
                }
            }
            if (lower)
                shouldRoundAwayFromZero = true;
        }
    } else {
        if (!((-ivalue) & (static_cast<int64_t>(1) << (XSUM_MANTISSA_BITS + 2)))) {
            int pos = (int64_t)1 << (XSUM_LOW_MANTISSA_BITS - 1 - more);
            ivalue *= 2;
            if (lower & pos) {
                ivalue += 1;
                lower &= ~pos;
            }
            e -= 1;
        }

        intv = XSUM_SIGN_MASK;
        ivalue = -ivalue;
        if ((ivalue & 3) == 3)
            shouldRoundAwayFromZero = true;
        if (!lower) {
            while (j > 0) {
                j -= 1;
                if (m_smallAccumulator.chunk[j]) {
                    lower = 1;
                    break;
                }
            }
        }
        if (!lower)
            shouldRoundAwayFromZero = true;
    }

    if (shouldRoundAwayFromZero) {
        ivalue += 4;
        if (ivalue & (static_cast<int64_t>(1) << (XSUM_MANTISSA_BITS + 3))) {
            ivalue >>= 1;
            e += 1;
        }
    }

    ivalue >>= 2;
    e += (i << XSUM_LOW_EXP_BITS) - XSUM_EXP_BIAS - XSUM_MANTISSA_BITS;
    if (e >= XSUM_EXP_MASK) {
        intv |= static_cast<int64_t>(XSUM_EXP_MASK) << XSUM_MANTISSA_BITS;
        return std::bit_cast<double>(intv);
    }
    intv += (static_cast<int64_t>(e) << XSUM_MANTISSA_BITS) + (ivalue & XSUM_MANTISSA_MASK);
    return std::bit_cast<double>(intv);
}

/*
ADD AN INF OR NAN TO A SMALL ACCUMULATOR. This only changes the flags,
not the chunks in the accumulator, which retains the sum of the finite
terms (which is perhaps sometimes useful to access, though no function
to do so is defined at present). A nan with larger payload (seen as a
52-bit unsigned integer) takes precedence, with the sign of the nan always
being positive. This ensures that the order of summing nan values doesn't
matter.
*/
void PreciseSum::xsumSmallAddInfNan(int64_t ivalue)
{
    const int64_t mantissa = ivalue & XSUM_MANTISSA_MASK;
    if (!mantissa) {
        if (!m_smallAccumulator.inf)
            m_smallAccumulator.inf = ivalue;
        else if (m_smallAccumulator.inf != ivalue) {
            double fltv = std::bit_cast<double>(ivalue);
            fltv = fltv - fltv;
            m_smallAccumulator.inf = std::bit_cast<int64_t>(fltv);
        }
    } else {
        if ((m_smallAccumulator.nan & XSUM_MANTISSA_MASK) <= mantissa)
            m_smallAccumulator.nan = ivalue & ~XSUM_SIGN_MASK;
    }
}

/*
ADD ONE NUMBER TO A SMALL ACCUMULATOR ASSUMING NO CARRY PROPAGATION REQ'D.
This function is declared INLINE regardless of the setting of INLINE_SMALL
and for good performance it must be inlined by the compiler (otherwise the
procedure call overhead will result in substantial inefficiency).
*/
inline void PreciseSum::xsumAdd1NoCarry(double value)
{
    const int64_t ivalue = std::bit_cast<int64_t>(value);
    int_fast16_t exp = (ivalue >> XSUM_MANTISSA_BITS) & XSUM_EXP_MASK;
    int64_t mantissa = ivalue & XSUM_MANTISSA_MASK;
    const int_fast16_t highExp = exp >> XSUM_LOW_EXP_BITS;
    int_fast16_t lowExp = exp & XSUM_LOW_EXP_MASK;

    if (!exp) {
        if (!mantissa)
            return;
        exp = lowExp = 1;
    } else if (exp == XSUM_EXP_MASK) {
        xsumSmallAddInfNan(ivalue);
        return;
    } else
        mantissa |= static_cast<int64_t>(1) << XSUM_MANTISSA_BITS;

    const std::array<int64_t, 2> splitMantissa {
        static_cast<int64_t>((static_cast<uint64_t>(mantissa) << lowExp) & XSUM_LOW_MANTISSA_MASK),
        mantissa >> (XSUM_LOW_MANTISSA_BITS - lowExp)
    };

    if (ivalue < 0) {
        m_smallAccumulator.chunk[highExp] -= splitMantissa[0];
        m_smallAccumulator.chunk[highExp + 1] -= splitMantissa[1];
    } else {
        m_smallAccumulator.chunk[highExp] += splitMantissa[0];
        m_smallAccumulator.chunk[highExp + 1] += splitMantissa[1];
    }
}

/*
PROPAGATE CARRIES TO NEXT CHUNK IN A SMALL ACCUMULATOR. Needs to
be called often enough that accumulated carries don't overflow out
the top, as indicated by m_smallAccumulator.addsUntilPropagate.  
Returns the index of the uppermost non-zero chunk (0 if number is zero).

After carry propagation, the uppermost non-zero chunk will indicate
the sign of the number, and will not be -1 (all 1s). It will be in
the range -2^XSUM_LOW_MANTISSA_BITS to 2^XSUM_LOW_MANTISSA_BITS - 1.
Lower chunks will be non-negative, and in the range from 0 up to
2^XSUM_LOW_MANTISSA_BITS - 1.
*/
int PreciseSum::xsumCarryPropagate()
{
    int u = XSUM_SCHUNKS - 1;
    while (0 <= u && !m_smallAccumulator.chunk[u]) {
        if (!u) {
            m_smallAccumulator.addsUntilPropagate = XSUM_SMALL_CARRY_TERMS - 1;
            return 0;
        }
        --u;
    }

    int i = 0;
    int uix = -1;
    do {
        int64_t c;
        int64_t clow;
        int64_t chigh;
        do {
            c = m_smallAccumulator.chunk[i];
            if (c)
                break;
            i += 1;
        } while (i <= u);

        if (i > u)
            break;

        chigh = c >> XSUM_LOW_MANTISSA_BITS;
        if (!chigh) {
            uix = i;
            i += 1;
            continue;
        }

        if (u == i) {
            if (chigh == -1) {
                uix = i;
                break;
            }
            u = i + 1;
        }

        clow = c & XSUM_LOW_MANTISSA_MASK;
        if (clow)
            uix = i;

        m_smallAccumulator.chunk[i] = clow;
        if (i + 1 >= XSUM_SCHUNKS) {
            xsumSmallAddInfNan((static_cast<int64_t>(XSUM_EXP_MASK) << XSUM_MANTISSA_BITS) | XSUM_MANTISSA_MASK);
            u = i;
        } else
            m_smallAccumulator.chunk[i + 1] += chigh;
        i += 1;
    } while (i <= u);

    if (uix < 0) {
        uix = 0;
        m_smallAccumulator.addsUntilPropagate = XSUM_SMALL_CARRY_TERMS - 1;
        return uix;
    }

    while (m_smallAccumulator.chunk[uix] == -1 && uix > 0) {
        m_smallAccumulator.chunk[uix - 1] += static_cast<int64_t>(-1) * (static_cast<int64_t>(1) << XSUM_LOW_MANTISSA_BITS);
        m_smallAccumulator.chunk[uix] = 0;
        uix -= 1;
    }
    m_smallAccumulator.addsUntilPropagate = XSUM_SMALL_CARRY_TERMS - 1;
    return uix;
}

/*
Increment m_sizeCount and check positive value every time when value is added.
This is needed to return -0 (negative zero) if applicable.
*/
ALWAYS_INLINE void PreciseSum::incrementWhenValueAdded(double value)
{
    m_sizeCount++;
    m_hasPosNumber = m_hasPosNumber || !std::signbit(value);
}

} // namespace WTF
