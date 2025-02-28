// Copyright (C) 2025 Apple Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.

internal import SwiftUI
@_spi(Private) @_spi(CrossImportOverlay) internal import WebKit

struct ContextMenuContext {
#if os(macOS)
    let menu: (WebPage.ElementInfo) -> NSMenu
#endif
}

// FIXME: (rdar://145869526) Improve efficiency and memory usage of `OnScrollGeometryChangeContext`.
class OnScrollGeometryChangeContext {
    private struct Change {
        var transform: (ScrollGeometry) -> AnyHashable
        var action: (AnyHashable, AnyHashable) -> Void
    }

    private var changes: [Namespace.ID : Change] = [:]

    func register<T>(
        changeID: Namespace.ID,
        transform: @escaping (ScrollGeometry) -> T,
        action: @escaping (T, T) -> Void
    ) where T: Hashable {
        let erasedTransform = { (geometry: ScrollGeometry) in
            AnyHashable(transform(geometry))
        }

        let erasedAction = { (old: AnyHashable, new: AnyHashable) in
            action(old.base as! T, new.base as! T)
        }

        if changes[changeID] == nil {
            changes[changeID] = .init(transform: erasedTransform, action: erasedAction)
        } else {
            changes[changeID]!.transform = erasedTransform
            changes[changeID]!.action = erasedAction
        }
    }

    func apply(from oldGeometry: ScrollGeometry, to newGeometry: ScrollGeometry) {
        for change in changes.values {
            let oldTransformed = change.transform(oldGeometry)
            let newTransformed = change.transform(newGeometry)

            if oldTransformed != newTransformed {
                change.action(oldTransformed, newTransformed)
            }
        }
    }
}

struct FindContext {
    var isPresented: Binding<Bool>?
    var canFind = true
    var canReplace = true
}
