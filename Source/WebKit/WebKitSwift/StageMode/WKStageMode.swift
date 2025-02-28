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
    private let kDragToRotationMultiplier: Float = 3.0
    private let kPitchSettleAnimationDuration: Double = 0.2
    
    private var stageModeOperation: WKStageModeOperation = .none
    
    /// The parent container on which pitch changes will be applied
    let interactionContainer: Entity
    
    /// The nested child container on which yaw changes will be applied
    /// We need to separate rotation-related transforms into two entities so that we can later apply post-gesture animations along the yaw and pitch separately
    let turntableInteractionContainer: Entity
    
    let modelEntity: WKSRKEntity
    
    weak var delegate: WKStageModeInteractionAware?
    
    private var driverInitialized: Bool = false
    private var initialManipulationPose: Transform = .identity
    private var previousManipulationPose: Transform = .identity
    private var initialTargetPose: Transform = .identity
    private var initialTurntablePose: Transform = .identity
    
    // Animation Controllers
    private var pitchSettleAnimationController: AnimationPlaybackController? = nil
    
    private var pitchAnimationIsPlaying: Bool {
        pitchSettleAnimationController?.isPlaying ?? false
    }
    
    // MARK: ObjC Exposed API
    @objc(interactionContainerRef)
    var interactionContainerRef: REEntityRef {
        self.interactionContainer.__coreEntity.__as(REEntityRef.self)
    }
    
    @objc(stageModeInteractionInProgress)
    var stageModeInteractionInProgress: Bool {
        self.driverInitialized && self.stageModeOperation != .none
    }
    
    @objc(initWithModel:container:delegate:)
    init(with model: WKSRKEntity, container: REEntityRef, delegate: WKStageModeInteractionAware?) {
        self.modelEntity = model
        self.interactionContainer = Entity()
        self.turntableInteractionContainer = Entity()
        self.interactionContainer.name = "WebKit:InteractionContainerEntity"
        self.turntableInteractionContainer.name = "WebKit:TurntableContainerEntity"
        self.delegate = delegate
        
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
        driverInitialized = true

        let initialCenter = modelEntity.interactionPivotPoint
        let initialTransform = modelEntity.transform
        let transformMatrix = Transform(scale: initialTransform.scale, rotation: initialTransform.rotation, translation: initialTransform.translation)
        self.interactionContainer.setPosition(initialCenter, relativeTo: nil)
        self.modelEntity.interactionContainerDidRecenter(transformMatrix.matrix)

        let initialPoseTransform = Transform(matrix: transform)
        initialManipulationPose = initialPoseTransform
        previousManipulationPose = initialPoseTransform
        initialTargetPose = interactionContainer.transform
        initialTurntablePose = turntableInteractionContainer.transform
        self.delegate?.stageModeInteractionDidUpdateModel()
    }
    
    @objc(interactionDidUpdate:)
    func interactionDidUpdate(_ transform: simd_float4x4) {
        let poseTransform = Transform(matrix: transform)
        switch stageModeOperation {
        case .orbit:
            do {
                let xyDelta = (poseTransform.translation._inMeters - initialManipulationPose.translation._inMeters).xy * kDragToRotationMultiplier

                // Apply pitch along global x axis
                let containerPitchRotation = Rotation3D(angle: .init(radians: xyDelta.y), axis: .x)
                self.interactionContainer.orientation = initialTargetPose.rotation * containerPitchRotation.quaternion.quatf

                // Apply yaw along local y axis
                let turntableYawRotation = Rotation3D(angle: .init(radians: xyDelta.x), axis: .y)
                self.turntableInteractionContainer.orientation = initialTurntablePose.rotation * turntableYawRotation.quaternion.quatf

                break
            }
        default:
            break
        }

        previousManipulationPose = poseTransform
        self.delegate?.stageModeInteractionDidUpdateModel()
    }
    
    @objc(interactionDidEnd)
    func interactionDidEnd() {
        driverInitialized = false
        initialManipulationPose = .identity
        previousManipulationPose = .identity
        
        // Settle the pitch of the interaction container
        pitchSettleAnimationController = self.interactionContainer.move(to: initialTargetPose, relativeTo: self.interactionContainer.parent, duration: kPitchSettleAnimationDuration, timingFunction: .easeOut)
        subscribeToPitchChanges()
    }
    
    @objc(operationDidUpdate:)
    func operationDidUpdate(_ operation: WKStageModeOperation) {
        self.stageModeOperation = operation
    }
    
    private func subscribeToPitchChanges() {
        if pitchAnimationIsPlaying {
            withObservationTracking {
#if USE_APPLE_INTERNAL_SDK
                // FIXME: https://bugs.webkit.org/show_bug.cgi?id=288873
#endif
            } onChange: {
                Task { @MainActor in
                    self.delegate?.stageModeInteractionDidUpdateModel()
                    
                    // Because the onChange only gets called once, we need to re-subscribe to the function while we are animating
                    self.subscribeToPitchChanges()
                }
            }
        }
    }
}

extension simd_float3 {
    // Based on visionOS's Points Per Meter (PPM) heuristics
    var _inMeters: simd_float3 {
        self / 1360.0
    }

    var xy: simd_float2 {
        return .init(x, y)
    }

    var double3: simd_double3 {
        return .init(Double(x), Double(y), Double(z))
    }
}

extension simd_quatd {
    var quatf: simd_quatf {
        return .init(ix: Float(self.imag.x), iy: Float(self.imag.y), iz: Float(self.imag.z), r: Float(self.real))
    }
}

#endif
