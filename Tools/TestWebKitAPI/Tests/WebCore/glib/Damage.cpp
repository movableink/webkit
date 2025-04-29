/*
 * Copyright (C) 2025 Igalia, S.L.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if PLATFORM(GTK) || PLATFORM(WPE)
#include "Test.h"
#include <WebCore/Damage.h>

namespace TestWebKitAPI {
using namespace WebCore;

TEST(Damage, Basics)
{
    Damage damage(IntSize { 2048, 1024 });
    EXPECT_TRUE(damage.isEmpty());
    EXPECT_EQ(damage.size(), 0);
}

TEST(Damage, Mode)
{
    // Rectangles is the default mode.
    Damage rectsDamage(IntSize { 1024, 768 });
    EXPECT_TRUE(rectsDamage.mode() == Damage::Mode::Rectangles);
    EXPECT_TRUE(rectsDamage.add(IntRect { 100, 100, 200, 200 }));
    EXPECT_TRUE(rectsDamage.add(IntRect { 300, 300, 200, 200 }));
    EXPECT_FALSE(rectsDamage.isEmpty());
    EXPECT_EQ(rectsDamage.size(), 2);
    EXPECT_EQ(rectsDamage.bounds().x(), 100);
    EXPECT_EQ(rectsDamage.bounds().y(), 100);
    EXPECT_EQ(rectsDamage.bounds().width(), 400);
    EXPECT_EQ(rectsDamage.bounds().height(), 400);

    // BoundingBox always unite damage in bounds.
    Damage bboxDamage(IntSize { 1024, 768 }, Damage::Mode::BoundingBox);
    EXPECT_TRUE(bboxDamage.mode() == Damage::Mode::BoundingBox);
    EXPECT_TRUE(bboxDamage.add(IntRect { 100, 100, 200, 200 }));
    EXPECT_TRUE(bboxDamage.add(IntRect { 300, 300, 200, 200 }));
    EXPECT_FALSE(bboxDamage.isEmpty());
    EXPECT_EQ(bboxDamage.size(), 1);
    EXPECT_EQ(bboxDamage[0], bboxDamage.bounds());
    EXPECT_EQ(bboxDamage.bounds().x(), 100);
    EXPECT_EQ(bboxDamage.bounds().y(), 100);
    EXPECT_EQ(bboxDamage.bounds().width(), 400);
    EXPECT_EQ(bboxDamage.bounds().height(), 400);

    // Full ignores any adds and always reports the whole area.
    Damage fullDamage(IntSize { 1024, 768 }, Damage::Mode::Full);
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Full);
    EXPECT_FALSE(fullDamage.add(IntRect { 100, 100, 200, 200 }));
    EXPECT_FALSE(fullDamage.add(IntRect { 300, 300, 200, 200 }));
    EXPECT_FALSE(fullDamage.isEmpty());
    EXPECT_EQ(fullDamage.size(), 1);
    EXPECT_EQ(fullDamage[0], fullDamage.bounds());
    EXPECT_EQ(fullDamage.bounds().x(), 0);
    EXPECT_EQ(fullDamage.bounds().y(), 0);
    EXPECT_EQ(fullDamage.bounds().width(), 1024);
    EXPECT_EQ(fullDamage.bounds().height(), 768);

    // We can make a Damage full.
    fullDamage = rectsDamage;
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Rectangles);
    fullDamage.makeFull();
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Full);
    EXPECT_FALSE(fullDamage.isEmpty());
    EXPECT_EQ(fullDamage.size(), 1);
    EXPECT_EQ(fullDamage[0], fullDamage.bounds());
    EXPECT_EQ(fullDamage.bounds().x(), 0);
    EXPECT_EQ(fullDamage.bounds().y(), 0);
    EXPECT_EQ(fullDamage.bounds().width(), 1024);
    EXPECT_EQ(fullDamage.bounds().height(), 768);

    // Adding a rectangle with the size of the Damage rect makes it full.
    fullDamage = rectsDamage;
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Rectangles);
    EXPECT_TRUE(fullDamage.add(IntRect { 0, 0, 1024, 768 }));
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Full);
    EXPECT_FALSE(fullDamage.isEmpty());
    EXPECT_EQ(fullDamage.size(), 1);
    EXPECT_EQ(fullDamage.bounds().x(), 0);
    EXPECT_EQ(fullDamage.bounds().y(), 0);
    EXPECT_EQ(fullDamage.bounds().width(), 1024);
    EXPECT_EQ(fullDamage.bounds().height(), 768);

    // Adding a rectangle containing the Damage rect makes it full.
    fullDamage = rectsDamage;
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Rectangles);
    EXPECT_TRUE(fullDamage.add(IntRect { 0, 0, 2048, 1024 }));
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Full);
    EXPECT_FALSE(fullDamage.isEmpty());
    EXPECT_EQ(fullDamage.size(), 1);
    EXPECT_EQ(fullDamage.bounds().x(), 0);
    EXPECT_EQ(fullDamage.bounds().y(), 0);
    EXPECT_EQ(fullDamage.bounds().width(), 1024);
    EXPECT_EQ(fullDamage.bounds().height(), 768);

    // Adding vector of rects with any value containing the Damage rect makes it full.
    fullDamage = rectsDamage;
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Rectangles);
    EXPECT_TRUE(fullDamage.add(Vector<IntRect, 1> {
        { 200, 200, 200, 200 },
        { 0, 0, 1024, 768 },
        { 400, 400, 200, 200 }
    }));
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Full);
    EXPECT_FALSE(fullDamage.isEmpty());
    EXPECT_EQ(fullDamage.size(), 1);
    EXPECT_EQ(fullDamage.bounds().x(), 0);
    EXPECT_EQ(fullDamage.bounds().y(), 0);
    EXPECT_EQ(fullDamage.bounds().width(), 1024);
    EXPECT_EQ(fullDamage.bounds().height(), 768);

    // It should be the same if the vector is long enough to unite.
    fullDamage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Rectangles);
    EXPECT_TRUE(fullDamage.add(Vector<IntRect, 1> {
        { 0, 0, 4, 4 },
        { 200, 0, 4, 4 },
        { 0, 200, 4, 4 },
        { 200, 200, 4, 4 },
        { 0, 0, 512, 512 },
        { 128, 128, 4, 4 }
    }));
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Full);
    EXPECT_FALSE(fullDamage.isEmpty());
    EXPECT_EQ(fullDamage.size(), 1);
    EXPECT_EQ(fullDamage.bounds().x(), 0);
    EXPECT_EQ(fullDamage.bounds().y(), 0);
    EXPECT_EQ(fullDamage.bounds().width(), 512);
    EXPECT_EQ(fullDamage.bounds().height(), 512);

    // Adding a full Damage to another with the same rect makes it full.
    fullDamage = rectsDamage;
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Rectangles);
    Damage fullDamage2(IntSize { 1024, 768 }, Damage::Mode::Full);
    EXPECT_TRUE(fullDamage.add(fullDamage2));
    EXPECT_TRUE(fullDamage.mode() == Damage::Mode::Full);
    EXPECT_FALSE(fullDamage.isEmpty());
    EXPECT_EQ(fullDamage.size(), 1);
    EXPECT_EQ(fullDamage.bounds().x(), 0);
    EXPECT_EQ(fullDamage.bounds().y(), 0);
    EXPECT_EQ(fullDamage.bounds().width(), 1024);
    EXPECT_EQ(fullDamage.bounds().height(), 768);
}

TEST(Damage, Move)
{
    Damage damage(IntSize { 2048, 1024 });
    EXPECT_TRUE(damage.add(IntRect { 100, 100, 200, 200 }));
    EXPECT_TRUE(damage.add(IntRect { 300, 300, 200, 200 }));
    EXPECT_FALSE(damage.isEmpty());
    EXPECT_EQ(damage.size(), 2);
    EXPECT_EQ(damage.bounds().x(), 100);
    EXPECT_EQ(damage.bounds().y(), 100);
    EXPECT_EQ(damage.bounds().width(), 400);
    EXPECT_EQ(damage.bounds().height(), 400);

    Damage other = WTFMove(damage);
    EXPECT_FALSE(other.isEmpty());
    EXPECT_EQ(other.size(), 2);
    EXPECT_EQ(other.bounds().x(), 100);
    EXPECT_EQ(other.bounds().y(), 100);
    EXPECT_EQ(other.bounds().width(), 400);
    EXPECT_EQ(other.bounds().height(), 400);
    EXPECT_TRUE(damage.isEmpty());
    EXPECT_EQ(damage.size(), 0);
    EXPECT_EQ(damage.bounds().x(), 100);
    EXPECT_EQ(damage.bounds().y(), 100);
    EXPECT_EQ(damage.bounds().width(), 400);
    EXPECT_EQ(damage.bounds().height(), 400);
}

TEST(Damage, AddRect)
{
    Damage damage(IntSize { 2048, 1024 });
    EXPECT_TRUE(damage.add(IntRect { 100, 100, 200, 200 }));
    EXPECT_EQ(damage.size(), 1);

    // When there's only one rect, that should be the bounds.
    EXPECT_EQ(damage.bounds().x(), 100);
    EXPECT_EQ(damage.bounds().y(), 100);
    EXPECT_EQ(damage.bounds().width(), 200);
    EXPECT_EQ(damage.bounds().height(), 200);

    // When there's only one rect, adding a rect already contained
    // by the bounding box does nothing.
    EXPECT_FALSE(damage.add(IntRect { 150, 150, 100, 100 }));
    EXPECT_EQ(damage.size(), 1);

    // Adding an empty rect does nothing.
    EXPECT_FALSE(damage.add(IntRect { }));
    EXPECT_EQ(damage.size(), 1);

    // Adding a new rect not contained by previous one adds it to the list.
    EXPECT_TRUE(damage.add(IntRect { 300, 300, 200, 200 }));
    EXPECT_EQ(damage.size(), 2);

    // Now the bounding box contains the two rectangles.
    EXPECT_EQ(damage.bounds().x(), 100);
    EXPECT_EQ(damage.bounds().y(), 100);
    EXPECT_EQ(damage.bounds().width(), 400);
    EXPECT_EQ(damage.bounds().height(), 400);

    // Adding a rect containing the bounds makes it the only rect.
    EXPECT_TRUE(damage.add(IntRect { 50, 50, 500, 500 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_EQ(damage.bounds().x(), 50);
    EXPECT_EQ(damage.bounds().y(), 50);
    EXPECT_EQ(damage.bounds().width(), 500);
    EXPECT_EQ(damage.bounds().height(), 500);

    // Adding FloatRect takes the enclosingIntRect
    EXPECT_TRUE(damage.add(FloatRect { 1024.50, 1024.25, 50.32, 25.75 }));
    EXPECT_EQ(damage.size(), 2);
    EXPECT_EQ(damage[1].x(), 1024);
    EXPECT_EQ(damage[1].y(), 1024);
    EXPECT_EQ(damage[1].width(), 51);
    EXPECT_EQ(damage[1].height(), 26);

    // Adding an empty FloatRect does nothing.
    EXPECT_FALSE(damage.add(FloatRect { 1024.50, 1024.25, 0, 0 }));
    EXPECT_EQ(damage.size(), 2);
}

TEST(Damage, AddRects)
{
    Damage damage(IntSize { 2048, 1024 });
    EXPECT_TRUE(damage.add(Vector<IntRect, 1> { { 100, 100, 200, 200 } }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_EQ(damage.bounds(), IntRect(100, 100, 200, 200));

    // Adding an empty Vector does nothing.
    EXPECT_FALSE(damage.add(Vector<IntRect, 1> { }));
    EXPECT_EQ(damage.size(), 1);

    // Adding a Vector with empty rets does nothing.
    EXPECT_FALSE(damage.add(Vector<IntRect, 1> { { }, { } }));
    EXPECT_EQ(damage.size(), 1);

    // Adding more than 4 rectangles will unite.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(Vector<IntRect, 1> {
        { 0, 0, 4, 4 },
        { 200, 0, 4, 4 },
        { 0, 200, 4, 4 },
        { 200, 200, 4, 4 },
        { 128, 128, 4, 4 }
    }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_EQ(damage[0], damage.bounds());

    // Adding more than 4 rectangles to a non empty damage should unite too.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 300, 0, 4, 4 }));
    EXPECT_TRUE(damage.add(Vector<IntRect, 1> {
        { 500, 0, 4, 4 },
        { 300, 200, 4, 4 },
        { 500, 200, 4, 4 },
        { 384, 128, 4, 4 }
    }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_EQ(damage[0], damage.bounds());

    // Adding more than 4 empty rectangles does nothing.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_FALSE(damage.add(Vector<IntRect, 1> { { }, { }, { }, { }, { } }));
    EXPECT_EQ(damage.size(), 0);

    // Adding more than 4 empty rectangles to a non empty damage does nothing too.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 300, 0, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_FALSE(damage.add(Vector<IntRect, 1> { { }, { }, { }, { }, { } }));
    EXPECT_EQ(damage.size(), 1);

    // Adding a Vector to damage in BoundingBox mode always unite damage in bounds.
    damage = Damage(IntSize { 1024, 768 }, Damage::Mode::BoundingBox);
    EXPECT_TRUE(damage.add(Vector<IntRect, 1> {
        { 100, 100, 200, 200 },
        { 300, 300, 200, 200 }
    }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_EQ(damage[0], damage.bounds());
    EXPECT_EQ(damage.bounds(), IntRect(100, 100, 400, 400));

    // Adding a Vector to damage in Full mode does nothing.
    damage = Damage(IntSize { 1024, 768 }, Damage::Mode::Full);
    EXPECT_FALSE(damage.add(Vector<IntRect, 1> {
        { 100, 100, 200, 200 },
        { 300, 300, 200, 200 }
    }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_EQ(damage[0], damage.bounds());
    EXPECT_EQ(damage.bounds(), IntRect(0, 0, 1024, 768));
}

TEST(Damage, AddDamage)
{
    Damage damage(IntSize { 2048, 1024 });
    EXPECT_TRUE(damage.add(IntRect { 100, 100, 200, 200 }));
    EXPECT_EQ(damage.size(), 1);

    // Adding empty Damage does nothing.
    Damage other(IntSize { 2048, 1024 });
    EXPECT_FALSE(damage.add(other));
    EXPECT_EQ(damage.size(), 1);

    // Adding a valid Damage adds its rectangles.
    EXPECT_TRUE(other.add(IntRect { 300, 300, 200, 200 }));
    EXPECT_EQ(other.size(), 1);
    EXPECT_TRUE(damage.add(other));
    EXPECT_EQ(damage.size(), 2);
    EXPECT_EQ(damage.bounds().x(), 100);
    EXPECT_EQ(damage.bounds().y(), 100);
    EXPECT_EQ(damage.bounds().width(), 400);
    EXPECT_EQ(damage.bounds().height(), 400);

    // It's possible to add one Damage to another with different rectangle.
    damage = Damage(IntSize { 1024, 768 });
    EXPECT_TRUE(damage.add(IntRect { 100, 100, 200, 200 }));
    EXPECT_EQ(damage.size(), 1);
    other = Damage(IntSize { 800, 600 });
    EXPECT_TRUE(other.add(IntRect { 300, 300, 200, 200 }));
    EXPECT_EQ(other.size(), 1);
    EXPECT_TRUE(damage.add(other));
    EXPECT_EQ(damage.size(), 2);
    EXPECT_EQ(damage[0], IntRect(100, 100, 200, 200));
    EXPECT_EQ(damage[1], IntRect(300, 300, 200, 200));

    // Adding a Damage already united with the same rectangle, just unites every rectangle in the grid.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(Vector<IntRect, 1> {
        { 0, 0, 4, 4 },
        { 300, 0, 4, 4 },
        { 0, 300, 4, 4 },
        { 300, 300, 4, 4 },
        { 128, 128, 4, 4 }
    }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_EQ(damage[0], IntRect(0, 0, 132, 132));
    EXPECT_EQ(damage[1], IntRect(300, 0, 4, 4));
    EXPECT_EQ(damage[2], IntRect(0, 300, 4, 4));
    EXPECT_EQ(damage[3], IntRect(300, 300, 4, 4));
    other = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(other.add(Vector<IntRect, 1> {
        { 10, 10, 4, 4 },
        { 310, 10, 4, 4 },
        { 10, 310, 4, 4 },
        { 310, 310, 4, 4 },
        { 384, 384, 4, 4 }
    }));
    EXPECT_EQ(other.size(), 4);
    EXPECT_EQ(other[0], IntRect(10, 10, 4, 4));
    EXPECT_EQ(other[1], IntRect(310, 10, 4, 4));
    EXPECT_EQ(other[2], IntRect(10, 310, 4, 4));
    EXPECT_EQ(other[3], IntRect(310, 310, 78, 78));
    EXPECT_TRUE(damage.add(other));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_EQ(damage[0], IntRect(0, 0, 132, 132));
    EXPECT_EQ(damage[1], IntRect(300, 0, 14, 14));
    EXPECT_EQ(damage[2], IntRect(0, 300, 14, 14));
    EXPECT_EQ(damage[3], IntRect(300, 300, 88, 88));
}

TEST(Damage, Unite)
{
    // Add several rects to the first tile.
    Damage damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 0, 0, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_TRUE(damage.add(IntRect { 200, 0, 4, 4 }));
    EXPECT_EQ(damage.size(), 2);
    EXPECT_TRUE(damage.add(IntRect { 0, 200, 4, 4 }));
    EXPECT_EQ(damage.size(), 3);
    EXPECT_TRUE(damage.add(IntRect { 200, 200, 4, 4 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_TRUE(damage.add(IntRect { 128, 128, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_FALSE(damage[0].isEmpty());
    EXPECT_EQ(damage[0], damage.bounds());

    // Add several rects to the second tile.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 300, 0, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_TRUE(damage.add(IntRect { 500, 0, 4, 4 }));
    EXPECT_EQ(damage.size(), 2);
    EXPECT_TRUE(damage.add(IntRect { 300, 200, 4, 4 }));
    EXPECT_EQ(damage.size(), 3);
    EXPECT_TRUE(damage.add(IntRect { 500, 200, 4, 4 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_TRUE(damage.add(IntRect { 384, 128, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_FALSE(damage[0].isEmpty());
    EXPECT_EQ(damage[0], damage.bounds());

    // Add several rects to the third tile.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 0, 300, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_TRUE(damage.add(IntRect { 200, 300, 4, 4 }));
    EXPECT_EQ(damage.size(), 2);
    EXPECT_TRUE(damage.add(IntRect { 0, 500, 4, 4 }));
    EXPECT_EQ(damage.size(), 3);
    EXPECT_TRUE(damage.add(IntRect { 200, 500, 4, 4 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_TRUE(damage.add(IntRect { 128, 384, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_FALSE(damage[0].isEmpty());
    EXPECT_EQ(damage[0], damage.bounds());

    // Add several rects to the fourth tile.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 300, 300, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_TRUE(damage.add(IntRect { 500, 300, 4, 4 }));
    EXPECT_EQ(damage.size(), 2);
    EXPECT_TRUE(damage.add(IntRect { 300, 500, 4, 4 }));
    EXPECT_EQ(damage.size(), 3);
    EXPECT_TRUE(damage.add(IntRect { 500, 500, 4, 4 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_TRUE(damage.add(IntRect { 384, 384, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_FALSE(damage[0].isEmpty());
    EXPECT_EQ(damage[0], damage.bounds());

    // Add one rect per tile.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 0, 0, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_TRUE(damage.add(IntRect { 300, 0, 4, 4 }));
    EXPECT_EQ(damage.size(), 2);
    EXPECT_TRUE(damage.add(IntRect { 0, 300, 4, 4 }));
    EXPECT_EQ(damage.size(), 3);
    EXPECT_TRUE(damage.add(IntRect { 300, 300, 4, 4 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_TRUE(damage.add(IntRect { 384, 384, 4, 4 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_FALSE(damage[0].isEmpty());
    EXPECT_EQ(damage[0].x(), 0);
    EXPECT_EQ(damage[0].y(), 0);
    EXPECT_EQ(damage[0].width(), 4);
    EXPECT_EQ(damage[0].height(), 4);
    EXPECT_FALSE(damage[1].isEmpty());
    EXPECT_EQ(damage[1].x(), 300);
    EXPECT_EQ(damage[1].y(), 0);
    EXPECT_EQ(damage[1].width(), 4);
    EXPECT_EQ(damage[1].height(), 4);
    EXPECT_FALSE(damage[2].isEmpty());
    EXPECT_EQ(damage[2].x(), 0);
    EXPECT_EQ(damage[2].y(), 300);
    EXPECT_EQ(damage[2].width(), 4);
    EXPECT_EQ(damage[2].height(), 4);
    EXPECT_FALSE(damage[3].isEmpty());
    EXPECT_EQ(damage[3].x(), 300);
    EXPECT_EQ(damage[3].y(), 300);
    EXPECT_EQ(damage[3].width(), 88);
    EXPECT_EQ(damage[3].height(), 88);

    // Add rects with points off the grid area.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { -2, 0, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_TRUE(damage.add(IntRect { 50, -2, 4, 4 }));
    EXPECT_EQ(damage.size(), 2);
    EXPECT_TRUE(damage.add(IntRect { 550, 0, 4, 4 }));
    EXPECT_EQ(damage.size(), 3);
    EXPECT_TRUE(damage.add(IntRect { 300, -2, 4, 4 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_TRUE(damage.add(IntRect { -2, 300, 4, 4 }));
    EXPECT_EQ(damage.size(), 3);
    EXPECT_TRUE(damage.add(IntRect { 50, 550, 4, 4 }));
    EXPECT_EQ(damage.size(), 3);
    EXPECT_TRUE(damage.add(IntRect { 300, 550, 4, 4 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_TRUE(damage.add(IntRect { 550, 300, 4, 4 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_FALSE(damage[0].isEmpty());
    EXPECT_EQ(damage[0].x(), -2);
    EXPECT_EQ(damage[0].y(), -2);
    EXPECT_EQ(damage[0].width(), 56);
    EXPECT_EQ(damage[0].height(), 6);
    EXPECT_FALSE(damage[1].isEmpty());
    EXPECT_EQ(damage[1].x(), 300);
    EXPECT_EQ(damage[1].y(), -2);
    EXPECT_EQ(damage[1].width(), 254);
    EXPECT_EQ(damage[1].height(), 6);
    EXPECT_FALSE(damage[2].isEmpty());
    EXPECT_EQ(damage[2].x(), -2);
    EXPECT_EQ(damage[2].y(), 300);
    EXPECT_EQ(damage[2].width(), 56);
    EXPECT_EQ(damage[2].height(), 254);
    EXPECT_FALSE(damage[3].isEmpty());
    EXPECT_EQ(damage[3].x(), 300);
    EXPECT_EQ(damage[3].y(), 300);
    EXPECT_EQ(damage[3].width(), 254);
    EXPECT_EQ(damage[3].height(), 254);

    // Add several rects and check that unite works for single tile grid.
    damage = Damage(IntSize { 128, 128 });
    EXPECT_TRUE(damage.add(IntRect { 10, 10, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_TRUE(damage.add(IntRect { 60, 60, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_TRUE(damage.add(IntRect { 70, 10, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_TRUE(damage.add(IntRect { 120, 60, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_TRUE(damage.add(IntRect { 10, 70, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_TRUE(damage.add(IntRect { 120, 120, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);

    // Grid size should be ceiled.
    damage = Damage(IntSize { 512, 333 });
    EXPECT_TRUE(damage.add(IntRect { 0, 0, 1, 1 }));
    EXPECT_TRUE(damage.add(IntRect { 1, 1, 1, 1 }));
    EXPECT_TRUE(damage.add(IntRect { 2, 2, 1, 1 }));
    EXPECT_TRUE(damage.add(IntRect { 3, 3, 1, 1 }));
    EXPECT_EQ(damage.size(), 4);

    // Grid size should be ceiled with high precision.
    damage = Damage(IntSize { 257, 50 });
    EXPECT_TRUE(damage.add(IntRect { 0, 0, 1, 1 }));
    EXPECT_TRUE(damage.add(IntRect { 1, 1, 1, 1 }));
    EXPECT_EQ(damage.size(), 2);

    // Unification should work correctly when grid does not start and { 0, 0 }.
    damage = Damage(IntRect { 256, 256, 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 300, 300, 1, 1 }));
    EXPECT_TRUE(damage.add(IntRect { 600, 300, 1, 1 }));
    EXPECT_TRUE(damage.add(IntRect { 300, 600, 1, 1 }));
    EXPECT_TRUE(damage.add(IntRect { 600, 600, 1, 1 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_TRUE(damage.add(IntRect { 301, 301, 1, 1 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_EQ(damage[0], IntRect(300, 300, 2, 2));
    EXPECT_EQ(damage[1], IntRect(600, 300, 1, 1));
    EXPECT_EQ(damage[2], IntRect(300, 600, 1, 1));
    EXPECT_EQ(damage[3], IntRect(600, 600, 1, 1));

    // Adding a rect covering the current bounding box makes the Damage no longer unified.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 0, 0, 4, 4 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_TRUE(damage.add(IntRect { 300, 0, 4, 4 }));
    EXPECT_EQ(damage.size(), 2);
    EXPECT_TRUE(damage.add(IntRect { 0, 300, 4, 4 }));
    EXPECT_EQ(damage.size(), 3);
    EXPECT_TRUE(damage.add(IntRect { 300, 300, 4, 4 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_TRUE(damage.add(IntRect { 384, 384, 4, 4 }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_TRUE(damage.add(IntRect { 0, 0, 400, 400 }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_EQ(damage.bounds(), IntRect(0, 0, 400, 400));
}

TEST(Damage, RectsForPainting)
{
    // The function should return the original rect when theres only a single one.
    Damage damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 250, 250, 12, 12 }));
    ASSERT_EQ(damage.rectsForPainting().size(), 1);
    EXPECT_EQ(damage.rectsForPainting()[0], IntRect(250, 250, 12, 12));

    // The function should remove overlaps.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 0, 0, 100, 100 }));
    EXPECT_TRUE(damage.add(IntRect { 50, 50, 100, 100 }));
    ASSERT_EQ(damage.rectsForPainting().size(), 1);
    EXPECT_EQ(damage.rectsForPainting()[0], IntRect(0, 0, 150, 150));

    // The function should clip the rects.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { -2, -2, 10, 10 }));
    EXPECT_TRUE(damage.add(IntRect { 504, 504, 10, 10 }));
    ASSERT_EQ(damage.rectsForPainting().size(), 2);
    EXPECT_EQ(damage.rectsForPainting()[0], IntRect(0, 0, 8, 8));
    EXPECT_EQ(damage.rectsForPainting()[1], IntRect(504, 504, 8, 8));

    // The function should preserve the layout of cells when unification is enabled.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 0, 0, 10, 10 }));
    EXPECT_TRUE(damage.add(IntRect { 10, 10, 10, 10 }));
    EXPECT_TRUE(damage.add(IntRect { 0, 256, 10, 10 }));
    EXPECT_TRUE(damage.add(IntRect { 256, 0, 10, 10 }));
    EXPECT_TRUE(damage.add(IntRect { 256, 256, 10, 10 }));
    EXPECT_EQ(damage.size(), 4);
    auto rectsForPainting = damage.rectsForPainting();
    EXPECT_EQ(rectsForPainting.size(), damage.size());
    EXPECT_EQ(rectsForPainting[0], damage[0]);
    EXPECT_EQ(rectsForPainting[1], damage[1]);
    EXPECT_EQ(rectsForPainting[2], damage[2]);
    EXPECT_EQ(rectsForPainting[3], damage[3]);

    // The function should preserve the layout of cells when unification is enabled
    // and the grid does not start at { 0, 0 }.
    damage = Damage(IntRect { 256, 256, 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 256, 256, 10, 10 }));
    EXPECT_TRUE(damage.add(IntRect { 256 + 10, 256 + 10, 10, 10 }));
    EXPECT_TRUE(damage.add(IntRect { 256, 256 + 256, 10, 10 }));
    EXPECT_TRUE(damage.add(IntRect { 256 + 256, 256, 10, 10 }));
    EXPECT_TRUE(damage.add(IntRect { 256 + 256, 256 + 256, 10, 10 }));
    EXPECT_EQ(damage.size(), 4);
    rectsForPainting = damage.rectsForPainting();
    EXPECT_EQ(rectsForPainting.size(), damage.size());
    EXPECT_EQ(rectsForPainting[0], damage[0]);
    EXPECT_EQ(rectsForPainting[1], damage[1]);
    EXPECT_EQ(rectsForPainting[2], damage[2]);
    EXPECT_EQ(rectsForPainting[3], damage[3]);

    // The function should split a rect spanning multiple cells.
    damage = Damage(IntSize { 512, 512 });
    EXPECT_TRUE(damage.add(IntRect { 250, 250, 12, 12 }));
    EXPECT_TRUE(damage.add(IntRect { 249, 249, 1, 1 }));
    rectsForPainting = damage.rectsForPainting();
    ASSERT_EQ(rectsForPainting.size(), 4);
    EXPECT_EQ(rectsForPainting[0], IntRect(249, 249, 7, 7));
    EXPECT_EQ(rectsForPainting[1], IntRect(256, 250, 6, 6));
    EXPECT_EQ(rectsForPainting[2], IntRect(250, 256, 6, 6));
    EXPECT_EQ(rectsForPainting[3], IntRect(256, 256, 6, 6));

    // The function should just return original rects when mode != Mode::Rectangles.
    damage = Damage(IntRect { 1024, 512, 512, 512 }, Damage::Mode::BoundingBox);
    EXPECT_TRUE(damage.add(IntRect { 1278, 678, 9, 341 }));
    EXPECT_TRUE(damage.add(IntRect { 1285, 678, 5, 341 }));
    EXPECT_FALSE(damage.add(IntRect { 1279, 678, 9, 341 }));
    EXPECT_TRUE(damage.add(IntRect { 1286, 678, 5, 341 }));
    rectsForPainting = damage.rectsForPainting();
    ASSERT_EQ(rectsForPainting.size(), 1);
    EXPECT_EQ(rectsForPainting[0], damage.bounds());
    damage = Damage(IntRect { 1024, 512, 512, 512 }, Damage::Mode::Full);
    rectsForPainting = damage.rectsForPainting();
    ASSERT_EQ(rectsForPainting.size(), 1);
    EXPECT_EQ(rectsForPainting[0], IntRect(1024, 512, 512, 512));
}

TEST(Damage, MaxRectangles)
{
    Damage damage(IntSize { 512, 512 }, Damage::Mode::Rectangles, 2);
    EXPECT_TRUE(damage.add(Vector<IntRect, 1> {
        { 0, 0, 4, 4 },
        { 300, 0, 4, 4 },
        { 0, 300, 4, 4 },
        { 300, 300, 4, 4 }
    }));
    EXPECT_EQ(damage.size(), 2);
    EXPECT_EQ(damage[0], IntRect(0, 0, 4, 304));
    EXPECT_EQ(damage[1], IntRect(300, 0, 4, 304));

    damage = Damage(IntSize { 2048, 1024 }, Damage::Mode::Rectangles, 3);
    EXPECT_TRUE(damage.add(Vector<IntRect, 1> {
        { 100, 100, 200, 200 },
        { 700, 500, 200, 200 },
        { 1400, 700, 200, 200 },
        { 1800, 800, 200, 200 }
    }));
    EXPECT_EQ(damage.size(), 3);
    EXPECT_EQ(damage[0], IntRect(100, 100, 200, 200));
    EXPECT_EQ(damage[1], IntRect(700, 500, 200, 200));
    EXPECT_EQ(damage[2], IntRect(1400, 700, 600, 300));

    // Using MaxRectangles = 0 means no maximum so default fixed cell size is used
    damage = Damage(IntSize { 512, 512 }, Damage::Mode::Rectangles, 0);
    EXPECT_TRUE(damage.add(Vector<IntRect, 1> {
        { 0, 0, 4, 4 },
        { 300, 0, 4, 4 },
        { 0, 300, 4, 4 },
        { 300, 300, 4, 4 },
        { 384, 384, 4, 4 }
    }));
    EXPECT_EQ(damage.size(), 4);
    EXPECT_EQ(damage[0], IntRect(0, 0, 4, 4));
    EXPECT_EQ(damage[1], IntRect(300, 0, 4, 4));
    EXPECT_EQ(damage[2], IntRect(0, 300, 4, 4));
    EXPECT_EQ(damage[3], IntRect(300, 300, 88, 88));

    // Passing MaxRectangles = 1 always unifies.
    damage = Damage(IntSize { 512, 512 }, Damage::Mode::Rectangles, 1);
    EXPECT_TRUE(damage.add(Vector<IntRect, 1> {
        { 0, 0, 4, 4 },
        { 300, 0, 4, 4 },
        { 0, 300, 4, 4 },
        { 300, 300, 4, 4 },
        { 384, 384, 4, 4 }
    }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_EQ(damage[0], damage.bounds());
    EXPECT_EQ(damage.bounds(), IntRect(0, 0, 388, 388));

    // Passing MaxRectangles with BoundingBode mode ignores it.
    damage = Damage(IntSize { 1024, 768 }, Damage::Mode::BoundingBox, 2);
    EXPECT_TRUE(damage.add(Vector<IntRect, 1> {
        { 100, 100, 200, 200 },
        { 200, 200, 200, 200 },
        { 300, 300, 200, 200 }
    }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_EQ(damage[0], damage.bounds());
    EXPECT_EQ(damage.bounds(), IntRect(100, 100, 400, 400));

    // Passing MaxRectangles with Full mode ignores it.
    damage = Damage(IntSize { 1024, 768 }, Damage::Mode::Full, 3);
    EXPECT_FALSE(damage.add(Vector<IntRect, 1> {
        { 100, 100, 200, 200 },
        { 300, 300, 200, 200 }
    }));
    EXPECT_EQ(damage.size(), 1);
    EXPECT_EQ(damage[0], damage.bounds());
    EXPECT_EQ(damage.bounds(), IntRect(0, 0, 1024, 768));
}

} // namespace TestWebKitAPI

#endif // PLATFORM(GTK) || PLATFORM(WPE)
