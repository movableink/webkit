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

#if canImport(Testing) && compiler(>=6.0)

import Testing
@_spi(Testing) import WebKit

@MainActor
struct RunLoopQueueTests {
    @Test
    func appendAndExecuteInOrder() async throws {
        let queue = RunLoopQueue()
        var changes: [Int] = []

        await withCheckedContinuation { (continuation: CheckedContinuation<Void, Never>) in
            queue.append {
                changes.append(1)
            }
            queue.append {
                changes.append(2)
            }
            queue.append {
                changes.append(3)
                continuation.resume()
            }
        }

        #expect(changes == [1, 2, 3])
    }

    @Test
    func processingTaskResetsAfterCompletion() async throws {
        let queue = RunLoopQueue()

        await withCheckedContinuation { (continuation: CheckedContinuation<Void, Never>) in
            queue.append {
                continuation.resume()
            }
        }

        // This ensures that the queue's task is set to nil before the next change is added.
        await Task.yield()

        await withCheckedContinuation { (continuation: CheckedContinuation<Void, Never>) in
            queue.append {
                continuation.resume()
            }
        }

        // The test passes if it gets to this point; if it times out, it has failed.
        #expect(true)
    }

    @Test
    func appendingWhileProcessingAddsToQueue() async throws {
        let queue = RunLoopQueue()
        var changes: [Int] = []

        await withCheckedContinuation { (continuation: CheckedContinuation<Void, Never>) in
            queue.append {
                changes.append(1)

                queue.append {
                    changes.append(2)
                }
                queue.append {
                    changes.append(3)
                    continuation.resume()
                }
            }
        }

        #expect(changes == [1, 2, 3])
    }
}

#endif // canImport(Testing) && compiler(>=6.0)
