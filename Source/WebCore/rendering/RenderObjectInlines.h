/**
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Document.h"
#include "LocalFrame.h"
#include "RenderElement.h"
#include "RenderObject.h"
#include "RenderStyleInlines.h"
#include "RenderView.h"

namespace WebCore {

inline bool RenderObject::hasTransformOrPerspective() const { return hasTransformRelatedProperty() && (isTransformed() || style().hasPerspective()); }
inline bool RenderObject::isAtomicInlineLevelBox() const { return style().isDisplayInlineType() && !(style().display() == DisplayType::Inline && !isReplacedOrAtomicInline()); }
inline bool RenderObject::isTransformed() const { return hasTransformRelatedProperty() && (style().affectsTransform() || hasSVGTransform()); }
inline bool RenderObject::preservesNewline() const { return !isRenderSVGInlineText() && style().preserveNewline(); }
inline Document& RenderObject::document() const { return m_node.get().document(); }
inline Ref<Document> RenderObject::protectedDocument() const { return document(); }
inline const LocalFrameViewLayoutContext& RenderObject::layoutContext() const { return view().frameView().layoutContext(); }

inline CheckedRef<const RenderStyle> RenderObject::checkedStyle() const
{
    return style();
}

inline Ref<TreeScope> RenderObject::protectedTreeScopeForSVGReferences() const
{
    return treeScopeForSVGReferences();
}

inline bool RenderObject::isDocumentElementRenderer() const
{
    return document().documentElement() == m_node.ptr();
}

inline RenderView& RenderObject::view() const
{
    return *document().renderView();
}

inline LocalFrame& RenderObject::frame() const
{
    return *document().frame();
}

inline Ref<LocalFrame> RenderObject::protectedFrame() const
{
    return frame();
}

inline Page& RenderObject::page() const
{
    // The render tree will always be torn down before Frame is disconnected from Page,
    // so it's safe to assume Frame::page() is non-null as long as there are live RenderObjects.
    ASSERT(frame().page());
    return *frame().page();
}

inline Ref<Page> RenderObject::protectedPage() const
{
    return page();
}

inline Settings& RenderObject::settings() const
{
    return page().settings();
}

inline bool RenderObject::renderTreeBeingDestroyed() const
{
    return document().renderTreeBeingDestroyed();
}

} // namespace WebCore
