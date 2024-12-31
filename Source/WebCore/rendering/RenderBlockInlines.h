/**
 * Copyright (C) 2003-2023 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "RenderBlock.h"
#include "RenderStyleInlines.h"

namespace WebCore {

inline LayoutUnit RenderBlock::endOffsetForContent() const { return !writingMode().isLogicalLeftInlineStart() ? logicalLeftOffsetForContent() : logicalWidth() - logicalRightOffsetForContent(); }
inline LayoutUnit RenderBlock::endOffsetForContent(LayoutUnit blockOffset) const { return endOffsetForContent(fragmentAtBlockOffset(blockOffset)); }
inline LayoutUnit RenderBlock::logicalLeftOffsetForContent() const { return isHorizontalWritingMode() ? borderLeft() + paddingLeft() : borderTop() + paddingTop(); }
inline LayoutUnit RenderBlock::logicalMarginBoxHeightForChild(const RenderBox& child) const { return isHorizontalWritingMode() ? child.marginBoxRect().height() : child.marginBoxRect().width(); }
inline LayoutUnit RenderBlock::logicalRightOffsetForContent() const { return logicalLeftOffsetForContent() + availableLogicalWidth(); }
inline LayoutUnit RenderBlock::startOffsetForContent() const { return writingMode().isLogicalLeftInlineStart() ? logicalLeftOffsetForContent() : logicalWidth() - logicalRightOffsetForContent(); }
inline LayoutUnit RenderBlock::startOffsetForContent(LayoutUnit blockOffset) const { return startOffsetForContent(fragmentAtBlockOffset(blockOffset)); }
inline LayoutUnit RenderBlock::availableLogicalWidthForLine(LayoutUnit position, LayoutUnit logicalHeight) const { return availableLogicalWidthForLineInFragment({ }, position, logicalHeight); }
inline LayoutUnit RenderBlock::logicalRightOffsetForLine(LayoutUnit position, LayoutUnit logicalHeight) const { return adjustLogicalRightOffsetForLine(logicalRightFloatOffsetForLine(position, logicalRightOffsetForContent(position), logicalHeight)); }
inline LayoutUnit RenderBlock::logicalLeftOffsetForLine(LayoutUnit position, LayoutUnit logicalHeight) const { return adjustLogicalLeftOffsetForLine(logicalLeftFloatOffsetForLine(position, logicalLeftOffsetForContent(position), logicalHeight)); }

inline LayoutUnit RenderBlock::endOffsetForContent(RenderFragmentContainer* fragment) const
{
    return !writingMode().isLogicalLeftInlineStart() ? logicalLeftOffsetForContent(fragment) : logicalWidth() - logicalRightOffsetForContent(fragment);
}

inline LayoutUnit RenderBlock::endOffsetForLine(LayoutUnit position, LayoutUnit logicalHeight) const
{
    return !writingMode().isLogicalLeftInlineStart() ? logicalLeftOffsetForLine(position, logicalHeight) : logicalWidth() - logicalRightOffsetForLine(position, logicalHeight);
}

inline bool RenderBlock::shouldSkipCreatingRunsForObject(RenderObject& object)
{
    return object.isFloating() || (object.isOutOfFlowPositioned() && !object.style().isOriginalDisplayInlineType() && !object.container()->isRenderInline());
}

inline LayoutUnit RenderBlock::startOffsetForContent(RenderFragmentContainer* fragment) const
{
    return writingMode().isLogicalLeftInlineStart() ? logicalLeftOffsetForContent(fragment) : logicalWidth() - logicalRightOffsetForContent(fragment);
}

inline LayoutUnit RenderBlock::startOffsetForLine(LayoutUnit position, LayoutUnit logicalHeight) const
{
    return writingMode().isLogicalLeftInlineStart() ? logicalLeftOffsetForLine(position, logicalHeight)
        : logicalWidth() - logicalRightOffsetForLine(position, logicalHeight);
}

// Versions that can compute line offsets with the fragment and page offset passed in. Used for speed to avoid having to
// compute the fragment all over again when you already know it.
inline LayoutUnit RenderBlock::availableLogicalWidthForLineInFragment(RenderFragmentContainer* fragment, LayoutUnit position, LayoutUnit logicalHeight) const
{
    auto logicalRightOffsetForLineInFragment = adjustLogicalRightOffsetForLine(logicalRightFloatOffsetForLine(position, logicalRightOffsetForContent(fragment), logicalHeight));
    auto logicalLeftOffsetForLineInFragment = adjustLogicalLeftOffsetForLine(logicalLeftFloatOffsetForLine(position, logicalLeftOffsetForContent(fragment), logicalHeight));
    return std::max(0_lu, logicalRightOffsetForLineInFragment - logicalLeftOffsetForLineInFragment);
}

} // namespace WebCore
