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

#if os(visionOS)

import Combine
import RealityKit
import Spatial
import Foundation
 @_spi(RealityKit) import RealityKit
import WebKitSwift
import os
import simd

/// A driver that maps all gesture updates to the specific transform we want for the specified StageMode behavior
@MainActor
@objc(WKStageModeInteractionDriver)
final public class WKStageModeInteractionDriver: NSObject {
    private var stageModeOperation: WKStageModeOperation = .none
    
    /// The parent container on which pitch changes will be applied
    let interactionContainer: Entity
    
    /// The nested child container on which yaw changes will be applied
    /// We need to separate rotation-related transforms into two entities so that we can later apply post-gesture animations along the yaw and pitch separately
    let turntableInteractionContainer: Entity
    
    let modelEntity: WKSRKEntity
    
    // MARK: ObjC Exposed API
    @objc(interactionContainerRef)
    var interactionContainerRef: REEntityRef {
        self.interactionContainer.__coreEntity.__as(REEntityRef.self)
    }
    
    @objc(initWithModel:container:)
    init(with model: WKSRKEntity, container: REEntityRef) {
        self.modelEntity = model
        self.interactionContainer = Entity()
        self.turntableInteractionContainer = Entity()
        self.interactionContainer.name = "WebKit:InteractionContainerEntity"
        self.turntableInteractionContainer.name = "WebKit:TurntableContainerEntity"
        
        let containerEntity = Entity.__fromCore(__EntityRef.__fromCore(container))
        self.interactionContainer.setParent(containerEntity, preservingWorldTransform: true)
        self.turntableInteractionContainer.setPosition(self.interactionContainer.position(relativeTo: nil), relativeTo: nil)
        self.turntableInteractionContainer.setParent(self.interactionContainer, preservingWorldTransform: true)
    }
    
    @objc(setContainerTransformInPortal)
    func setContainerTransformInPortal() {
        // Configure entity hierarchy after we have correctly positioned the model
        self.interactionContainer.setPosition(modelEntity.interactionPivotPoint, relativeTo: nil)
        modelEntity.setParent(self.turntableInteractionContainer.__coreEntity.__as(REEntityRef.self), preservingWorldTransform: true)
    }
    
    @objc(removeInteractionContainerFromSceneOrParent)
    func removeInteractionContainerFromSceneOrParent() {
        self.interactionContainer.removeFromParent()
        self.turntableInteractionContainer.removeFromParent()
    }
    
    @objc(interactionDidBegin:)
    func interactionDidBegin(_ transform: simd_float4x4) {
        // FIXME: https://bugs.webkit.org/show_bug.cgi?id=287601
    }
    
    @objc(interactionDidUpdate:)
    func interactionDidUpdate(_ transform: simd_float4x4) {
        // FIXME: https://bugs.webkit.org/show_bug.cgi?id=287601
    }
    
    @objc(interactionDidEnd)
    func interactionDidEnd() {
        // FIXME: https://bugs.webkit.org/show_bug.cgi?id=287601
    }
    
    @objc(operationDidUpdate:)
    func operationDidUpdate(_ operation: WKStageModeOperation) {
        self.stageModeOperation = operation
    }
}

#endif
