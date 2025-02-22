/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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
#include "FullscreenManager.h"

#if ENABLE(FULLSCREEN_API)

#include "Chrome.h"
#include "ChromeClient.h"
#include "Document.h"
#include "DocumentInlines.h"
#include "Element.h"
#include "ElementInlines.h"
#include "EventLoop.h"
#include "EventNames.h"
#include "HTMLDialogElement.h"
#include "HTMLIFrameElement.h"
#include "HTMLMediaElement.h"
#include "HTMLNames.h"
#include "JSDOMPromiseDeferred.h"
#include "LocalDOMWindow.h"
#include "LocalFrame.h"
#include "Logging.h"
#include "Page.h"
#include "PseudoClassChangeInvalidation.h"
#include "QualifiedName.h"
#include "Quirks.h"
#include "RenderBlock.h"
#include "SVGElementTypeHelpers.h"
#include "SVGSVGElement.h"
#include "Settings.h"
#include <wtf/LoggerHelper.h>
#include <wtf/TZoneMallocInlines.h>

#if ENABLE(MATHML)
#include "MathMLMathElement.h"
#endif

namespace WebCore {

WTF_MAKE_TZONE_ALLOCATED_IMPL(FullscreenManager);

using namespace HTMLNames;

FullscreenManager::FullscreenManager(Document& document)
    : m_document(document)
#if !RELEASE_LOG_DISABLED
    , m_logIdentifier(LoggerHelper::uniqueLogIdentifier())
#endif
{
}

FullscreenManager::~FullscreenManager() = default;

Element* FullscreenManager::fullscreenElement() const
{
    for (Ref element : makeReversedRange(document().topLayerElements())) {
        if (element->hasFullscreenFlag())
            return element.ptr();
    }

    return nullptr;
}

// https://fullscreen.spec.whatwg.org/#dom-element-requestfullscreen
void FullscreenManager::requestFullscreenForElement(Ref<Element>&& element, FullscreenCheckType checkType, CompletionHandler<void(ExceptionOr<void>)>&& completionHandler, HTMLMediaElementEnums::VideoFullscreenMode mode)
{
    auto identifier = LOGIDENTIFIER;

    enum class EmitErrorEvent : bool { No, Yes };
    auto handleError = [this, element, identifier, weakThis = WeakPtr { *this }](ASCIILiteral message, EmitErrorEvent emitErrorEvent, CompletionHandler<void(ExceptionOr<void>)>&& completionHandler) mutable {
        CheckedPtr checkedThis = weakThis.get();
        if (!checkedThis)
            return completionHandler(Exception { ExceptionCode::TypeError, message });
        ERROR_LOG(identifier, message);
        if (emitErrorEvent == EmitErrorEvent::Yes) {
            m_fullscreenErrorEventTargetQueue.append(WTFMove(element));
            protectedDocument()->scheduleRenderingUpdate(RenderingUpdateStep::Fullscreen);
        }
        completionHandler(Exception { ExceptionCode::TypeError, message });
    };

    // If pendingDoc is not fully active, then reject promise with a TypeError exception and return promise.
    if (!protectedDocument()->isFullyActive())
        return handleError("Cannot request fullscreen on a document that is not fully active."_s, EmitErrorEvent::No, WTFMove(completionHandler));

    // https://fullscreen.spec.whatwg.org/#fullscreen-element-ready-check
    auto fullscreenElementReadyCheck = [checkType] (auto element, auto document) -> ASCIILiteral {
        if (!element->isConnected())
            return "Cannot request fullscreen on a disconnected element."_s;

        if (element->isPopoverShowing())
            return "Cannot request fullscreen on an open popover."_s;

        if (checkType == EnforceIFrameAllowFullscreenRequirement && !PermissionsPolicy::isFeatureEnabled(PermissionsPolicy::Feature::Fullscreen, document))
            return "Fullscreen API is disabled by permissions policy."_s;

        return { };
    };

    auto isElementTypeAllowedForFullscreen = [] (const auto& element) {
        if (is<HTMLElement>(element) || is<SVGSVGElement>(element))
            return true;
#if ENABLE(MATHML)
        if (is<MathMLMathElement>(element))
            return true;
#endif
        return false;
    };

    // If any of the following conditions are true, terminate these steps and queue a task to fire
    // an event named fullscreenerror with its bubbles attribute set to true on the context object's
    // node document:
    if (!isElementTypeAllowedForFullscreen(element))
        return handleError("Cannot request fullscreen on a non-HTML element."_s, EmitErrorEvent::Yes, WTFMove(completionHandler));

    if (is<HTMLDialogElement>(element))
        return handleError("Cannot request fullscreen on a <dialog> element."_s, EmitErrorEvent::Yes, WTFMove(completionHandler));

    if (auto error = fullscreenElementReadyCheck(element, protectedDocument()))
        return handleError(error, EmitErrorEvent::Yes, WTFMove(completionHandler));

    if (!document().domWindow() || !document().domWindow()->consumeTransientActivation())
        return handleError("Cannot request fullscreen without transient activation."_s, EmitErrorEvent::Yes, WTFMove(completionHandler));

    if (UserGestureIndicator::processingUserGesture() && UserGestureIndicator::currentUserGesture()->gestureType() == UserGestureType::EscapeKey)
        return handleError("Cannot request fullscreen with Escape key as current gesture."_s, EmitErrorEvent::Yes, WTFMove(completionHandler));

    // There is a previously-established user preference, security risk, or platform limitation.
    if (!page() || !page()->isFullscreenManagerEnabled())
        return handleError("Fullscreen API is disabled."_s, EmitErrorEvent::Yes, WTFMove(completionHandler));

    bool hasKeyboardAccess = true;
    if (!page()->chrome().client().supportsFullScreenForElement(element, hasKeyboardAccess)) {
        // The new full screen API does not accept a "flags" parameter, so fall back to disallowing
        // keyboard input if the chrome client refuses to allow keyboard input.
        hasKeyboardAccess = false;

        if (!page()->chrome().client().supportsFullScreenForElement(element, hasKeyboardAccess))
            return handleError("Cannot request fullscreen with unsupported element."_s, EmitErrorEvent::Yes, WTFMove(completionHandler));
    }

    INFO_LOG(identifier);

    m_pendingFullscreenElement = RefPtr { element.ptr() };

    protectedDocument()->eventLoop().queueTask(TaskSource::MediaElement, [this, weakThis = WeakPtr { *this }, element = WTFMove(element), completionHandler = WTFMove(completionHandler), hasKeyboardAccess, fullscreenElementReadyCheck, handleError, identifier, mode] () mutable {
        CheckedPtr checkedThis = weakThis.get();
        if (!checkedThis)
            return completionHandler(Exception { ExceptionCode::TypeError });

        // Don't allow fullscreen if it has been cancelled or a different fullscreen element
        // has requested fullscreen.
        if (m_pendingFullscreenElement != element.ptr())
            return handleError("Fullscreen request aborted by a fullscreen request for another element."_s, EmitErrorEvent::Yes, WTFMove(completionHandler));

        // Don't allow fullscreen if we're inside an exitFullscreen operation.
        if (m_pendingExitFullscreen)
            return handleError("Fullscreen request aborted by a request to exit fullscreen."_s, EmitErrorEvent::Yes, WTFMove(completionHandler));

        // Don't allow fullscreen if document is hidden.
        auto document = protectedDocument();
        if (document->hidden() && mode != HTMLMediaElementEnums::VideoFullscreenModeInWindow)
            return handleError("Cannot request fullscreen in a hidden document."_s, EmitErrorEvent::Yes, WTFMove(completionHandler));

        // Fullscreen element ready check.
        if (auto error = fullscreenElementReadyCheck(element, protectedDocument()))
            return handleError(error, EmitErrorEvent::Yes, WTFMove(completionHandler));

        // Don't allow if element changed document.
        if (&element->document() != document.ptr())
            return handleError("Cannot request fullscreen because the associated document has changed."_s, EmitErrorEvent::Yes, WTFMove(completionHandler));

        // A descendant browsing context's document has a non-empty fullscreen element stack.
        bool descendantHasNonEmptyStack = false;
        for (RefPtr descendant = frame() ? frame()->tree().traverseNext() : nullptr; descendant; descendant = descendant->tree().traverseNext()) {
            auto* localFrame = dynamicDowncast<LocalFrame>(descendant.get());
            if (!localFrame)
                continue;
            if (localFrame->document()->fullscreenManager().fullscreenElement()) {
                descendantHasNonEmptyStack = true;
                break;
            }
        }
        if (descendantHasNonEmptyStack)
            return handleError("Cannot request fullscreen because a descendant document already has a fullscreen element."_s, EmitErrorEvent::Yes, WTFMove(completionHandler));

        // 5. Return, and run the remaining steps asynchronously.
        // 6. Optionally, perform some animation.
        m_areKeysEnabledInFullscreen = hasKeyboardAccess;
        document->eventLoop().queueTask(TaskSource::MediaElement, [this, weakThis = WTFMove(weakThis), element = WTFMove(element), completionHandler = WTFMove(completionHandler), handleError = WTFMove(handleError), identifier, mode] () mutable {
            CheckedPtr checkedThis = weakThis.get();
            if (!checkedThis)
                return completionHandler(Exception { ExceptionCode::TypeError });

            RefPtr page = this->page();
            if (!page || (this->document().hidden() && mode != HTMLMediaElementEnums::VideoFullscreenModeInWindow) || m_pendingFullscreenElement != element.ptr() || !element->isConnected())
                return handleError("Invalid state when requesting fullscreen."_s, EmitErrorEvent::Yes, WTFMove(completionHandler));

            INFO_LOG(identifier, "task - success");

            page->chrome().client().enterFullScreenForElement(element, mode, WTFMove(completionHandler), [weakThis = WTFMove(weakThis)] (bool success) {
                CheckedPtr checkedThis = weakThis.get();
                if (!checkedThis || !success)
                    return true;
                return checkedThis->didEnterFullscreen();
            });
        });

        // 7. Optionally, display a message indicating how the user can exit displaying the context object fullscreen.
    });
}

void FullscreenManager::cancelFullscreen()
{
    // The Mozilla "cancelFullscreen()" API behaves like the W3C "fully exit fullscreen" behavior, which
    // is defined as:
    // "To fully exit fullscreen act as if the exitFullscreen() method was invoked on the top-level browsing
    // context's document and subsequently empty that document's fullscreen element stack."
    RefPtr mainFrameDocument = this->mainFrameDocument();
    if (!mainFrameDocument)
        LOG_ONCE(SiteIsolation, "Unable to fully perform FullscreenManager::cancelFullscreen() without access to the main frame document ");

    if (!mainFrameDocument || !mainFrameDocument->fullscreenManager().fullscreenElement()) {
        // If there is a pending fullscreen element but no top document fullscreen element,
        // there is a pending task in enterFullscreen(). Cause it to cancel and fire an error
        // by clearing the pending fullscreen element.
        m_pendingFullscreenElement = nullptr;
        INFO_LOG(LOGIDENTIFIER, "Cancelling pending fullscreen request.");
        return;
    }

    INFO_LOG(LOGIDENTIFIER);

    m_pendingExitFullscreen = true;

    protectedDocument()->eventLoop().queueTask(TaskSource::MediaElement, [this, weakThis = WeakPtr { *this }, mainFrameDocument = WTFMove(mainFrameDocument), identifier = LOGIDENTIFIER] {
#if RELEASE_LOG_DISABLED
        UNUSED_PARAM(this);
#endif
        CheckedPtr checkedThis = weakThis.get();
        if (!checkedThis)
            return;

        if (!mainFrameDocument->page()) {
            INFO_LOG(identifier, "Top document has no page.");
            return;
        }

        // This triggers finishExitFullscreen with ExitMode::Resize, which fully exits the document.
        if (RefPtr fullscreenElement = mainFrameDocument->fullscreenManager().fullscreenElement()) {
            mainFrameDocument->page()->chrome().client().exitFullScreenForElement(fullscreenElement.get(), [weakThis = WeakPtr { *this }] {
                CheckedPtr checkedThis = weakThis.get();
                if (!checkedThis)
                    return;
                checkedThis->didExitFullscreen([] (auto) { });
            });
        } else
            INFO_LOG(identifier, "Top document has no fullscreen element");
    });
}

// https://fullscreen.spec.whatwg.org/#collect-documents-to-unfullscreen
static Vector<Ref<Document>> documentsToUnfullscreen(Frame& firstFrame)
{
    Vector<Ref<Document>> documents;
    if (RefPtr localFirstFrame = dynamicDowncast<LocalFrame>(firstFrame); localFirstFrame && localFirstFrame->document())
        documents.append(*localFirstFrame->document());
    for (RefPtr frame = firstFrame.tree().parent(); frame; frame = frame->tree().parent()) {
        RefPtr localFrame = dynamicDowncast<LocalFrame>(frame);
        if (!localFrame)
            continue;
        RefPtr document = localFrame->document();
        if (!document)
            continue;
        ASSERT(document->fullscreenManager().fullscreenElement());
        if (!document->fullscreenManager().isSimpleFullscreenDocument())
            break;
        if (RefPtr iframe = dynamicDowncast<HTMLIFrameElement>(document->ownerElement()); iframe && iframe->hasIFrameFullscreenFlag())
            break;
        documents.append(*document);
    }
    return documents;
}

static void clearFullscreenFlags(Element& element)
{
    element.setFullscreenFlag(false);
    if (auto* iframe = dynamicDowncast<HTMLIFrameElement>(element))
        iframe->setIFrameFullscreenFlag(false);
}

void FullscreenManager::exitFullscreen(CompletionHandler<void(ExceptionOr<void>)>&& completionHandler)
{
    INFO_LOG(LOGIDENTIFIER);

    Ref exitingDocument = document();
    auto mode = ExitMode::NoResize;
    Vector<Ref<Document>> exitDocuments;
    if (RefPtr exitingFrame = exitingDocument->frame())
        exitDocuments = documentsToUnfullscreen(*exitingFrame);

    RefPtr mainFrameDocument = this->mainFrameDocument();

    bool exitsTopDocument = exitDocuments.containsIf([&](auto& document) {
        return document.ptr() == mainFrameDocument.get();
    });
    if (!mainFrameDocument || (exitsTopDocument && mainFrameDocument->fullscreenManager().isSimpleFullscreenDocument())) {
        mode = ExitMode::Resize;
        if (mainFrameDocument)
            exitingDocument = *mainFrameDocument;
    }

    if (RefPtr element = exitingDocument->fullscreenManager().fullscreenElement(); element && !element->isConnected()) {
        queueFullscreenChangeEventForDocument(exitingDocument);
        clearFullscreenFlags(*element);
        element->removeFromTopLayer();
    }

    m_pendingExitFullscreen = true;

    // Return promise, and run the remaining steps in parallel.
    exitingDocument->eventLoop().queueTask(TaskSource::MediaElement, [this, completionHandler = WTFMove(completionHandler), weakThis = WeakPtr { *this }, mode, identifier = LOGIDENTIFIER] () mutable {
        CheckedPtr checkedThis = weakThis.get();
        if (!checkedThis)
            return completionHandler({ });

        RefPtr page = this->page();
        if (!page) {
            m_pendingExitFullscreen = false;
            ERROR_LOG(identifier, "task - Document not in page; bailing.");
            return completionHandler({ });
        }

        // If there is a pending fullscreen element but no fullscreen element
        // there is a pending task in requestFullscreenForElement(). Cause it to cancel and fire an error
        // by clearing the pending fullscreen element.
        RefPtr exitedFullscreenElement = fullscreenElement();
        if (!exitedFullscreenElement && m_pendingFullscreenElement) {
            INFO_LOG(identifier, "task - Cancelling pending fullscreen request.");
            m_pendingFullscreenElement = nullptr;
            m_pendingExitFullscreen = false;
            return completionHandler({ });
        }

        // Notify the chrome of the new full screen element.
        if (mode == ExitMode::Resize) {
            page->chrome().client().exitFullScreenForElement(exitedFullscreenElement.get(), [weakThis = WTFMove(weakThis), completionHandler = WTFMove(completionHandler)] mutable {
                CheckedPtr checkedThis = weakThis.get();
                if (!checkedThis)
                    return completionHandler({ });
                checkedThis->didExitFullscreen(WTFMove(completionHandler));
            });
        } else {
            if (RefPtr frame = document().frame())
                finishExitFullscreen(*frame, ExitMode::NoResize);

            // We just popped off one fullscreen element out of the top layer, query the new one.
            m_pendingFullscreenElement = fullscreenElement();
            if (m_pendingFullscreenElement) {
                page->chrome().client().enterFullScreenForElement(*m_pendingFullscreenElement, HTMLMediaElementEnums::VideoFullscreenModeStandard, WTFMove(completionHandler), [weakThis = WTFMove(weakThis)] (bool success) {
                    CheckedPtr checkedThis = weakThis.get();
                    if (!checkedThis || !success)
                        return true;
                    return checkedThis->didEnterFullscreen();
                });
            } else
                completionHandler({ });
        }
    });
}

void FullscreenManager::finishExitFullscreen(Frame& currentFrame, ExitMode mode)
{
    RefPtr currentLocalFrame = dynamicDowncast<LocalFrame>(currentFrame);
    if (currentLocalFrame && currentLocalFrame->document() && !currentLocalFrame->document()->fullscreenManager().fullscreenElement())
        return;

    // Let descendantDocs be an ordered set consisting of doc’s descendant browsing contexts' active documents whose fullscreen element is non-null, if any, in tree order.
    Vector<Ref<Document>> descendantDocuments;
    for (RefPtr descendant = currentFrame.tree().traverseNext(); descendant; descendant = descendant->tree().traverseNext()) {
        RefPtr localFrame = dynamicDowncast<LocalFrame>(descendant);
        if (!localFrame || !localFrame->document())
            continue;
        if (localFrame->document()->fullscreenManager().fullscreenElement())
            descendantDocuments.append(*localFrame->document());
    }

    auto unfullscreenDocument = [](const Ref<Document>& document) {
        Vector<Ref<Element>> toRemove;
        for (Ref element : document->topLayerElements()) {
            if (!element->hasFullscreenFlag())
                continue;
            clearFullscreenFlags(element);
            toRemove.append(element);
        }
        for (Ref element : toRemove)
            element->removeFromTopLayer();
    };

    auto exitDocuments = documentsToUnfullscreen(currentFrame);
    for (Ref exitDocument : exitDocuments) {
        queueFullscreenChangeEventForDocument(exitDocument);
        if (mode == ExitMode::Resize)
            unfullscreenDocument(exitDocument);
        else {
            RefPtr fullscreenElement = exitDocument->fullscreenManager().fullscreenElement();
            clearFullscreenFlags(*fullscreenElement);
            fullscreenElement->removeFromTopLayer();
        }
    }

    for (Ref descendantDocument : makeReversedRange(descendantDocuments)) {
        queueFullscreenChangeEventForDocument(descendantDocument);
        unfullscreenDocument(descendantDocument);
    }
}

bool FullscreenManager::isFullscreenEnabled() const
{
    // 4. The fullscreenEnabled attribute must return true if the context object and all ancestor
    // browsing context's documents have their fullscreen enabled flag set, or false otherwise.

    // Top-level browsing contexts are implied to have their allowFullscreen attribute set.
    return PermissionsPolicy::isFeatureEnabled(PermissionsPolicy::Feature::Fullscreen, protectedDocument());
}

ExceptionOr<void> FullscreenManager::willEnterFullscreen(Element& element, HTMLMediaElementEnums::VideoFullscreenMode mode)
{
#if !ENABLE(VIDEO)
    UNUSED_PARAM(mode);
#endif

    if (backForwardCacheState() != Document::NotInBackForwardCache) {
        ERROR_LOG(LOGIDENTIFIER, "Document in the BackForwardCache; bailing");
        return Exception { ExceptionCode::TypeError };
    }

    // Protect against being called after the document has been removed from the page.
    RefPtr protectedPage = page();
    if (!protectedPage) {
        ERROR_LOG(LOGIDENTIFIER, "Document no longer in page; bailing");
        return Exception { ExceptionCode::TypeError };
    }

    // The element is an open popover.
    if (element.isPopoverShowing()) {
        ERROR_LOG(LOGIDENTIFIER, "Element to fullscreen is an open popover; bailing.");
        return Exception { ExceptionCode::TypeError, "Cannot request fullscreen on an open popover."_s };
    }

    // If pending fullscreen element is unset or another element's was requested,
    // issue a cancel fullscreen request to the client
    if (m_pendingFullscreenElement != &element) {
        INFO_LOG(LOGIDENTIFIER, "Pending element mismatch; issuing exit fullscreen request");
        page()->chrome().client().exitFullScreenForElement(&element, [weakThis = WeakPtr { *this }] {
            CheckedPtr checkedThis = weakThis.get();
            if (!checkedThis)
                return;
            checkedThis->didExitFullscreen([] (auto) { });
        });
        return Exception { ExceptionCode::TypeError, "Element requested for fullscreen has changed."_s };
    }

    INFO_LOG(LOGIDENTIFIER);
    ASSERT(page()->isFullscreenManagerEnabled());

#if ENABLE(VIDEO)
    if (RefPtr mediaElement = dynamicDowncast<HTMLMediaElement>(element))
        mediaElement->willBecomeFullscreenElement(mode);
    else
#endif
        element.willBecomeFullscreenElement();

    ASSERT(&element == m_pendingFullscreenElement);
    m_pendingFullscreenElement = nullptr;

    m_fullscreenElement = &element;

    Vector<Ref<Element>> ancestors { { element } };
    for (RefPtr<Frame> frame = element.document().frame(); frame; frame = frame->tree().parent()) {
        if (RefPtr ownerElement = frame->ownerElement())
            ancestors.append(ownerElement.releaseNonNull());
    }

    for (auto ancestor : makeReversedRange(ancestors)) {
        auto hideUntil = ancestor->topmostPopoverAncestor(Element::TopLayerElementType::Other);
        ancestor->document().hideAllPopoversUntil(hideUntil, FocusPreviousElement::No, FireEvents::No);

        auto containingBlockBeforeStyleResolution = SingleThreadWeakPtr<RenderBlock> { };
        if (auto* renderer = ancestor->renderer())
            containingBlockBeforeStyleResolution = renderer->containingBlock();

        ancestor->setFullscreenFlag(true);
        ancestor->document().resolveStyle(Document::ResolveStyleType::Rebuild);

        // Remove before adding, so we always add at the end of the top layer.
        if (ancestor->isInTopLayer())
            ancestor->removeFromTopLayer();
        ancestor->addToTopLayer();

        queueFullscreenChangeEventForDocument(ancestor->document());

        RenderElement::markRendererDirtyAfterTopLayerChange(ancestor->checkedRenderer().get(), containingBlockBeforeStyleResolution.get());
    }

    if (RefPtr iframe = dynamicDowncast<HTMLIFrameElement>(element))
        iframe->setIFrameFullscreenFlag(true);

    return { };
}

bool FullscreenManager::didEnterFullscreen()
{
    RefPtr fullscreenElement = this->fullscreenElement();
    if (!fullscreenElement) {
        ERROR_LOG(LOGIDENTIFIER, "No fullscreenElement; bailing");
        return false;
    }

    if (backForwardCacheState() != Document::NotInBackForwardCache) {
        ERROR_LOG(LOGIDENTIFIER, "Document in the BackForwardCache; bailing");
        return false;
    }
    INFO_LOG(LOGIDENTIFIER);

    fullscreenElement->didBecomeFullscreenElement();
    return true;
}

bool FullscreenManager::willExitFullscreen()
{
    auto fullscreenElement = fullscreenOrPendingElement();
    if (!fullscreenElement) {
        ERROR_LOG(LOGIDENTIFIER, "No fullscreenOrPendingElement(); bailing");
        return false;
    }

    if (backForwardCacheState() != Document::NotInBackForwardCache) {
        ERROR_LOG(LOGIDENTIFIER, "Document in the BackForwardCache; bailing");
        return false;
    }
    INFO_LOG(LOGIDENTIFIER);

    fullscreenElement->willStopBeingFullscreenElement();
    return true;
}

void FullscreenManager::didExitFullscreen(CompletionHandler<void(ExceptionOr<void>)>&& completionHandler)
{
    if (backForwardCacheState() != Document::NotInBackForwardCache) {
        ERROR_LOG(LOGIDENTIFIER, "Document in the BackForwardCache; bailing");
        m_pendingExitFullscreen = false;
        return completionHandler(Exception { ExceptionCode::TypeError });
    }
    INFO_LOG(LOGIDENTIFIER);

    if (RefPtr frame = document().frame())
        finishExitFullscreen(frame->mainFrame(), ExitMode::Resize);

    if (auto fullscreenElement = fullscreenOrPendingElement())
        fullscreenElement->didStopBeingFullscreenElement();

    m_areKeysEnabledInFullscreen = false;

    m_fullscreenElement = nullptr;
    m_pendingFullscreenElement = nullptr;
    m_pendingExitFullscreen = false;

    completionHandler({ });
}

// https://fullscreen.spec.whatwg.org/#run-the-fullscreen-steps
void FullscreenManager::dispatchPendingEvents()
{
    // Since we dispatch events in this function, it's possible that the
    // document will be detached and GC'd. We protect it here to make sure we
    // can finish the function successfully.
    Ref<Document> protectedDocument(document());
    Deque<GCReachableRef<Node>> changeQueue;
    m_fullscreenChangeEventTargetQueue.swap(changeQueue);
    Deque<GCReachableRef<Node>> errorQueue;
    m_fullscreenErrorEventTargetQueue.swap(errorQueue);

    dispatchFullscreenChangeOrErrorEvent(changeQueue, EventType::Change, /* shouldNotifyMediaElement */ true);
    dispatchFullscreenChangeOrErrorEvent(errorQueue, EventType::Error, /* shouldNotifyMediaElement */ false);
}

void FullscreenManager::dispatchEventForNode(Node& node, EventType eventType)
{
    switch (eventType) {
    case EventType::Change: {
        node.dispatchEvent(Event::create(eventNames().fullscreenchangeEvent, Event::CanBubble::Yes, Event::IsCancelable::No, Event::IsComposed::Yes));
        bool shouldEmitUnprefixed = !(node.hasEventListeners(eventNames().webkitfullscreenchangeEvent) && node.hasEventListeners(eventNames().fullscreenchangeEvent)) && !(node.document().hasEventListeners(eventNames().webkitfullscreenchangeEvent) && node.document().hasEventListeners(eventNames().fullscreenchangeEvent));
        if (shouldEmitUnprefixed)
            node.dispatchEvent(Event::create(eventNames().webkitfullscreenchangeEvent, Event::CanBubble::Yes, Event::IsCancelable::No, Event::IsComposed::Yes));
        break;
    }
    case EventType::Error:
        node.dispatchEvent(Event::create(eventNames().fullscreenerrorEvent, Event::CanBubble::Yes, Event::IsCancelable::No, Event::IsComposed::Yes));
        node.dispatchEvent(Event::create(eventNames().webkitfullscreenerrorEvent, Event::CanBubble::Yes, Event::IsCancelable::No, Event::IsComposed::Yes));
        break;
    }
}

void FullscreenManager::dispatchFullscreenChangeOrErrorEvent(Deque<GCReachableRef<Node>>& queue, EventType eventType, bool shouldNotifyMediaElement)
{
    // Step 3 of https://fullscreen.spec.whatwg.org/#run-the-fullscreen-steps
    while (!queue.isEmpty()) {
        auto node = queue.takeFirst();

        // Gaining or losing fullscreen state may change viewport arguments
        node->protectedDocument()->updateViewportArguments();

#if ENABLE(VIDEO)
        if (shouldNotifyMediaElement) {
            if (RefPtr mediaElement = dynamicDowncast<HTMLMediaElement>(node.get()))
                mediaElement->enteredOrExitedFullscreen();
        }
#else
        UNUSED_PARAM(shouldNotifyMediaElement);
#endif
        // If the element was removed from our tree, also message the documentElement. Since we may
        // have a document hierarchy, check that node isn't in another document.
        if (!node->isConnected() || &node->document() != &document())
            queue.append(document());
        else
            dispatchEventForNode(node.get(), eventType);
    }
}

void FullscreenManager::exitRemovedFullscreenElement(Element& element)
{
    ASSERT(element.hasFullscreenFlag());

    if (fullscreenElement() == &element) {
        INFO_LOG(LOGIDENTIFIER, "Fullscreen element removed; exiting fullscreen");
        exitFullscreen([] (auto) { });
    } else
        clearFullscreenFlags(element);
}

bool FullscreenManager::isAnimatingFullscreen() const
{
    return m_isAnimatingFullscreen;
}

void FullscreenManager::setAnimatingFullscreen(bool flag)
{
    if (m_isAnimatingFullscreen == flag)
        return;

    INFO_LOG(LOGIDENTIFIER, flag);

    std::optional<Style::PseudoClassChangeInvalidation> styleInvalidation;
    if (RefPtr fullscreenElement = this->fullscreenElement())
        emplace(styleInvalidation, *fullscreenElement, { { CSSSelector::PseudoClass::InternalAnimatingFullscreenTransition, flag } });
    m_isAnimatingFullscreen = flag;
}

void FullscreenManager::clear()
{
    m_fullscreenElement = nullptr;
    m_pendingFullscreenElement = nullptr;
}

void FullscreenManager::emptyEventQueue()
{
    m_fullscreenChangeEventTargetQueue.clear();
    m_fullscreenErrorEventTargetQueue.clear();
}

void FullscreenManager::queueFullscreenChangeEventForDocument(Document& document)
{
    RefPtr target = document.fullscreenManager().fullscreenElement();
    if (!target) {
        ASSERT_NOT_REACHED();
        return;
    }
    document.fullscreenManager().addElementToChangeEventQueue(*target);
    document.scheduleRenderingUpdate(RenderingUpdateStep::Fullscreen);
}

bool FullscreenManager::isSimpleFullscreenDocument() const
{
    bool foundFullscreenFlag = false;
    for (Ref element : document().topLayerElements()) {
        if (element->hasFullscreenFlag()) {
            if (foundFullscreenFlag)
                return false;
            foundFullscreenFlag = true;
        }
    }
    return foundFullscreenFlag;
}

#if !RELEASE_LOG_DISABLED
WTFLogChannel& FullscreenManager::logChannel() const
{
    return LogFullscreen;

}
#endif

}

#endif
