/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "LifecycleLogger.h"
#include "MoveOnly.h"
#include "RefLogger.h"
#include <wtf/CompactVariant.h>
#include <wtf/GetPtr.h>

namespace TestWebKitAPI {

struct EmptyStruct {
    constexpr bool operator==(const EmptyStruct&) const = default;
};

struct SmallEnoughStruct {
    float value { 0 };

    constexpr bool operator==(const SmallEnoughStruct&) const = default;
    constexpr bool operator==(float other) const { return value == other; };
};

struct TooBigStruct {
    double value { 0 };

    constexpr bool operator==(const TooBigStruct&) const = default;
    constexpr bool operator==(double other) const { return value == other; };
};

} // namespace TestWebKitAPI

namespace WTF {

// Treat LifecycleLogger as smart pointer to allow its use in CompactVariant.
template<> struct IsSmartPtr<TestWebKitAPI::LifecycleLogger> {
    static constexpr bool value = true;
};

template<> struct CompactVariantTraits<TestWebKitAPI::TooBigStruct> {
   static constexpr bool hasAlternativeRepresentation = true;

   static constexpr uint64_t encodeFromArguments(double value)
   {
       return static_cast<uint64_t>(std::bit_cast<uint32_t>(static_cast<float>(value)));
   }

   static constexpr uint64_t encode(TestWebKitAPI::TooBigStruct&& value)
   {
       return static_cast<uint64_t>(std::bit_cast<uint32_t>(static_cast<float>(value.value)));
   }

   static constexpr TestWebKitAPI::TooBigStruct decode(uint64_t value)
   {
       return { std::bit_cast<float>(static_cast<uint32_t>(value)) };
   }
};

} // namespace WTF

namespace TestWebKitAPI {

TEST(WTF_CompactVariant, Pointers)
{
    int testInt = 1;
    float testFloat = 2.0f;

    WTF::CompactVariant<std::variant<int*, float*>> variant = &testInt;
    EXPECT_TRUE(variant.holds_alternative<int*>());
    EXPECT_FALSE(variant.holds_alternative<float*>());

    WTF::switchOn(variant,
        [&](int* const& value)   { EXPECT_EQ(*value, 1); },
        [&](float* const& value) { FAIL(); }
    );

    variant = &testFloat;
    EXPECT_FALSE(variant.holds_alternative<int*>());
    EXPECT_TRUE(variant.holds_alternative<float*>());

    WTF::switchOn(variant,
        [&](int* const& value)   { FAIL(); },
        [&](float* const& value) { EXPECT_EQ(*value, 2.0f); }
    );
}

TEST(WTF_CompactVariant, SmartPointers)
{
    {
        RefLogger testRefLogger("testRefLogger");
        Ref<RefLogger> ref(testRefLogger);

        WTF::CompactVariant<std::variant<Ref<RefLogger>, std::unique_ptr<double>>> variant { std::in_place_type<Ref<RefLogger>>, WTFMove(ref) };

        EXPECT_TRUE(variant.holds_alternative<Ref<RefLogger>>());
        EXPECT_FALSE(variant.holds_alternative<std::unique_ptr<double>>());

        WTF::switchOn(variant,
            [&](const Ref<RefLogger>&)          { SUCCEED(); },
            [&](const std::unique_ptr<double>&) { FAIL(); }
        );

        variant = std::make_unique<double>(2.0);
        EXPECT_FALSE(variant.holds_alternative<Ref<RefLogger>>());
        EXPECT_TRUE(variant.holds_alternative<std::unique_ptr<double>>());

        WTF::switchOn(variant,
            [&](const Ref<RefLogger>&)                { FAIL(); },
            [&](const std::unique_ptr<double>& value) { EXPECT_EQ(*value, 2.0);  }
        );
    }
    ASSERT_STREQ("ref(testRefLogger) deref(testRefLogger) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, SmallScalars)
{
    float testFloat = 2.0f;

    WTF::CompactVariant<std::variant<int*, float*, float>> variant = 3.0f;
    EXPECT_FALSE(variant.holds_alternative<int*>());
    EXPECT_FALSE(variant.holds_alternative<float*>());
    EXPECT_TRUE(variant.holds_alternative<float>());

    WTF::switchOn(variant,
        [&](int* const& value)   { FAIL(); },
        [&](float* const& value) { FAIL(); },
        [&](const float& value)  { EXPECT_EQ(value, 3.0f); }
    );

    variant = &testFloat;
    EXPECT_FALSE(variant.holds_alternative<int*>());
    EXPECT_TRUE(variant.holds_alternative<float*>());
    EXPECT_FALSE(variant.holds_alternative<float>());

    WTF::switchOn(variant,
        [&](int* const& value)   { FAIL(); },
        [&](float* const& value) { EXPECT_EQ(*value, 2.0f); },
        [&](const float& value)  { FAIL(); }
    );
}

TEST(WTF_CompactVariant, EmptyStruct)
{
    float testFloat = 2.0f;

    WTF::CompactVariant<std::variant<int*, float*, EmptyStruct>> variant = EmptyStruct { };
    EXPECT_FALSE(variant.holds_alternative<int*>());
    EXPECT_FALSE(variant.holds_alternative<float*>());
    EXPECT_TRUE(variant.holds_alternative<EmptyStruct>());

    WTF::switchOn(variant,
        [&](int* const& value)   { FAIL(); },
        [&](float* const& value) { FAIL(); },
        [&](const EmptyStruct&)  { SUCCEED(); }
    );

    variant = &testFloat;
    EXPECT_FALSE(variant.holds_alternative<int*>());
    EXPECT_TRUE(variant.holds_alternative<float*>());
    EXPECT_FALSE(variant.holds_alternative<EmptyStruct>());

    WTF::switchOn(variant,
        [&](int* const& value)   { FAIL(); },
        [&](float* const& value) { EXPECT_EQ(*value, 2.0f); },
        [&](const EmptyStruct&)  { FAIL(); }
    );
}

TEST(WTF_CompactVariant, SmallEnoughStruct)
{
    float testFloat = 2.0f;

    WTF::CompactVariant<std::variant<int*, float*, SmallEnoughStruct>> variant = SmallEnoughStruct { 3.0f };
    EXPECT_FALSE(variant.holds_alternative<int*>());
    EXPECT_FALSE(variant.holds_alternative<float*>());
    EXPECT_TRUE(variant.holds_alternative<SmallEnoughStruct>());

    WTF::switchOn(variant,
        [&](int* const& value)              { FAIL(); },
        [&](float* const& value)            { FAIL(); },
        [&](const SmallEnoughStruct& value) { EXPECT_EQ(value.value, 3.0f); }
    );

    variant = &testFloat;
    EXPECT_FALSE(variant.holds_alternative<int*>());
    EXPECT_TRUE(variant.holds_alternative<float*>());
    EXPECT_FALSE(variant.holds_alternative<SmallEnoughStruct>());

    WTF::switchOn(variant,
        [&](int* const& value)        { FAIL(); },
        [&](float* const& value)      { EXPECT_EQ(*value, 2.0f); },
        [&](const SmallEnoughStruct&) { FAIL(); }
    );
}

TEST(WTF_CompactVariant, TooBigStruct)
{
    float testFloat = 2.0f;

    WTF::CompactVariant<std::variant<int*, float*, TooBigStruct>> variant = TooBigStruct { 4.0 };
    EXPECT_FALSE(variant.holds_alternative<int*>());
    EXPECT_FALSE(variant.holds_alternative<float*>());
    EXPECT_TRUE(variant.holds_alternative<TooBigStruct>());

    WTF::switchOn(variant,
        [&](int* const& value)        { FAIL(); },
        [&](float* const& value)      { FAIL(); },
        [&](const TooBigStruct value) { EXPECT_EQ(value.value, 4.0); }
    );

    variant = &testFloat;
    EXPECT_FALSE(variant.holds_alternative<int*>());
    EXPECT_TRUE(variant.holds_alternative<float*>());
    EXPECT_FALSE(variant.holds_alternative<TooBigStruct>());

    WTF::switchOn(variant,
        [&](int* const& value)   { FAIL(); },
        [&](float* const& value) { EXPECT_EQ(*value, 2.0f); },
        [&](const TooBigStruct)  { FAIL(); }
    );
}

TEST(WTF_CompactVariant, MoveOnlyStruct)
{
    float testFloat = 2.0f;

    WTF::CompactVariant<std::variant<int*, float*, MoveOnly>> variant = MoveOnly { 5u };
    EXPECT_FALSE(variant.holds_alternative<int*>());
    EXPECT_FALSE(variant.holds_alternative<float*>());
    EXPECT_TRUE(variant.holds_alternative<MoveOnly>());

    WTF::switchOn(variant,
        [&](int* const& value)      { FAIL(); },
        [&](float* const& value)    { FAIL(); },
        [&](const MoveOnly& value)  { EXPECT_EQ(value.value(), 5u); }
    );

    variant = &testFloat;
    EXPECT_FALSE(variant.holds_alternative<int*>());
    EXPECT_TRUE(variant.holds_alternative<float*>());
    EXPECT_FALSE(variant.holds_alternative<MoveOnly>());

    WTF::switchOn(variant,
        [&](int* const& value)   { FAIL(); },
        [&](float* const& value) { EXPECT_EQ(*value, 2.0f); },
        [&](const MoveOnly&)     { FAIL(); }
    );

    variant = MoveOnly { 6u };
    EXPECT_FALSE(variant.holds_alternative<int*>());
    EXPECT_FALSE(variant.holds_alternative<float*>());
    EXPECT_TRUE(variant.holds_alternative<MoveOnly>());

    WTF::switchOn(variant,
        [&](int* const& value)     { FAIL(); },
        [&](float* const& value)   { FAIL(); },
        [&](const MoveOnly& value) { EXPECT_EQ(value.value(), 6u); }
    );
}

TEST(WTF_CompactVariant, ValuelessByMove)
{
    int testInt = 1;
    WTF::CompactVariant<std::variant<int*, float*>> variant = &testInt;
    EXPECT_FALSE(variant.valueless_by_move());

    WTF::CompactVariant<std::variant<int*, float*>> other = WTFMove(variant);
    EXPECT_FALSE(other.valueless_by_move());
    EXPECT_TRUE(variant.valueless_by_move());

    // Test copying the "valueless_by_move" variant.
    WTF::CompactVariant<std::variant<int*, float*>> copy = variant;
    EXPECT_TRUE(variant.valueless_by_move());
    EXPECT_TRUE(copy.valueless_by_move());

    // Test re-moving the "valueless_by_move" variant.
    WTF::CompactVariant<std::variant<int*, float*>> moved = WTFMove(variant);
    EXPECT_TRUE(variant.valueless_by_move());
    EXPECT_TRUE(moved.valueless_by_move());
}

TEST(WTF_CompactVariant, ArgumentAssignment)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant = LifecycleLogger("compact");

        ASSERT_STREQ("construct(compact) move-construct(compact) destruct(<default>) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) ", takeLogStr().c_str());
}


TEST(WTF_CompactVariant, ArgumentConstruct)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { LifecycleLogger("compact") };

        ASSERT_STREQ("construct(compact) move-construct(compact) destruct(<default>) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, ArgumentConstructInPlaceType)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { std::in_place_type<LifecycleLogger>, "compact" };

        ASSERT_STREQ("construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, ArgumentConstructInPlaceIndex)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { std::in_place_index<1>, "compact" };

        ASSERT_STREQ("construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, ArgumentMoveConstruct)
{
    {
        LifecycleLogger lifecycleLogger("compact");
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { WTFMove(lifecycleLogger) };

        ASSERT_STREQ("construct(compact) move-construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) destruct(<default>) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, ArgumentCopyConstruct)
{
    {
        LifecycleLogger lifecycleLogger("compact");
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { lifecycleLogger };

        ASSERT_STREQ("construct(compact) copy-construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, ArgumentMoveAssignment)
{
    {
        LifecycleLogger lifecycleLogger("compact");
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant = WTFMove(lifecycleLogger);

        ASSERT_STREQ("construct(compact) move-construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) destruct(<default>) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, ArgumentCopyAssignment)
{
    {
        LifecycleLogger lifecycleLogger("compact");
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant = lifecycleLogger;

        ASSERT_STREQ("construct(compact) copy-construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, CopyConstruct)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { std::in_place_type<LifecycleLogger>, "compact" };

        WTF::CompactVariant<std::variant<float, LifecycleLogger>> other { variant };

        ASSERT_STREQ("construct(compact) copy-construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, CopyAssignment)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { std::in_place_type<LifecycleLogger>, "compact" };

        WTF::CompactVariant<std::variant<float, LifecycleLogger>> other = variant;

        ASSERT_STREQ("construct(compact) copy-construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, MoveConstruct)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { std::in_place_type<LifecycleLogger>, "compact" };

        WTF::CompactVariant<std::variant<float, LifecycleLogger>> other { WTFMove(variant) };

        ASSERT_STREQ("construct(compact) move-construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, MoveAssignment)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { std::in_place_type<LifecycleLogger>, "compact" };

        WTF::CompactVariant<std::variant<float, LifecycleLogger>> other = WTFMove(variant);

        ASSERT_STREQ("construct(compact) move-construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, ConstructThenReassign)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { std::in_place_type<LifecycleLogger>, "compact" };

        variant = 1.0f;

        ASSERT_STREQ("construct(compact) destruct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, ArgumentReassignment)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { 1.0f };

        variant = LifecycleLogger { "compact" };

        ASSERT_STREQ("construct(compact) move-construct(compact) destruct(<default>) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, ArgumentCopyReassignment)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { 1.0f };

        LifecycleLogger lifecycleLogger { "compact" };
        variant = lifecycleLogger;

        ASSERT_STREQ("construct(compact) copy-construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, ArgumentMoveReassignment)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { 1.0f };

        LifecycleLogger lifecycleLogger { "compact" };
        variant = WTFMove(lifecycleLogger);

        ASSERT_STREQ("construct(compact) move-construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(<default>) destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, EmplaceType)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { 1.0f };

        variant.emplace<LifecycleLogger>("compact");

        ASSERT_STREQ("construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, EmplaceIndex)
{
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { 1.0f };

        variant.emplace<1>("compact");

        ASSERT_STREQ("construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) ", takeLogStr().c_str());
}

TEST(WTF_CompactVariant, SwitchOn)
{
    // `switchOn` should not cause any lifecycle events.
    {
        WTF::CompactVariant<std::variant<float, LifecycleLogger>> variant { std::in_place_type<LifecycleLogger>, "compact" };

        WTF::switchOn(variant,
            [&](const float&) { },
            [&](const LifecycleLogger&) { }
        );

        ASSERT_STREQ("construct(compact) ", takeLogStr().c_str());
    }
    ASSERT_STREQ("destruct(compact) ", takeLogStr().c_str());
}

} // namespace TestWebKitAPI
