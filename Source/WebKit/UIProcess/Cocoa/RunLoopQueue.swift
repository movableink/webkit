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

#if compiler(>=6.0)

import Foundation

/// Provides the ability to schedule the execution of arbitrary blocks such that each block is guaranteed
/// to be on a unique run loop tick.
///
/// Consider the following example:
///
///     @Observable
///     @MainActor
///     final class Foo {
///         var x = 0
///     }
///
///     let foo = Foo()
///     foo.x += 1
///     foo.x += 1
///
/// The increments to `foo.x` will trivially be done in the same run-loop tick, and so the changes may be
/// coalesced when the property is observed.
///
/// To ensure each change is reflected by Observable, you can use `RunLoopQueue` when performing the changes:
///
///     let foo = Foo()
///     let queue = RunLoopQueue()
///
///     queue.append { foo.x += 1 }
///     queue.append { foo.x += 1 }
///
/// This results in three unique run-loop ticks; the original, and the two that happen with the changes.
///
/// - Note: Coalescing changes to observable properties is generally the preferred behavior; only use this if you
/// have a specific reason to avoid this behavior.
@MainActor
@_spi(Testing)
public final class RunLoopQueue {
    // FIXME: Consider implementing this in an actor-agnostic manner.

    /// The type of change the queue contains.
    public typealias Change = () -> Void

    private var pendingChanges: [Change] = []
    private var processingTask: Task<Void, Never>?

    /// Creates an empty queue.
    public init() {
    }

    /// Adds a new change to the queue, which will be executed on the next available run-loop tick,
    /// once it is at the start of the queue.
    ///
    /// - Parameter change: The change that should be evaluated, which must by synchronous and isolated to the main actor.
    public func append(change: @escaping Change) {
        pendingChanges.append(change)

        // Start the processing task if one is not already active, which ensures that
        // multiple processing loops aren't accidentally started.
        guard processingTask == nil else {
            return
        }

        processingTask = Task { [weak self] in
            await self?.process()
        }
    }

    private func process() async {
        // Keep processing the changes until there are none left.
        while !pendingChanges.isEmpty {
            // Pop the first change from the queue.
            let next = pendingChanges.removeFirst()

            // Execute the change, which will be happening on a run-loop tick at least one after the one that
            // `enqueue(change:)` was called on due to the `Task` created.
            next()

            // This is used to explicitly yield control, which suspends the current task's
            // execution and allows the MainActor's executor to process other pending work before resuming this loop iteration.
            // Effectively, this causes the loop to wait a single run-loop cycle before continuing.
            await Task.yield()

            // The loop will now resume here after yielding.
        }

        // Once the loop finishes, then pendingChanges is empty, and so the processing task
        // is set to `nil` so that a new one can later be started if more items are enqueued later.
        processingTask = nil
    }
}

#endif // compiler(>=6.0)
