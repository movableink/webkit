/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008 Collabora Ltd. All rights reserved.
 *
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef FrameLoaderClientQt_h
#define FrameLoaderClientQt_h

#include <WebCore/FormState.h>
#include <WebCore/LocalFrameLoaderClient.h>
#include <WebCore/ResourceResponse.h>
#include <WebCore/ResourceError.h>

#include <QObject>
#include <QUrl>
#include <wtf/URL.h>

QT_BEGIN_NAMESPACE
class QNetworkReply;
QT_END_NAMESPACE

class QWebFrame;
class QWebFrameAdapter;

namespace WebCore {

class AuthenticationChallenge;
class DocumentLoader;
class Element;
class FormState;
class NavigationAction;
class FrameNetworkingContext;
class ResourceLoader;

struct LoadErrorResetToken;

class FrameLoaderClientQt final : public QObject, public LocalFrameLoaderClient {
    Q_OBJECT

    friend class ::QWebFrameAdapter;
    bool callErrorPageExtension(const ResourceError&);

Q_SIGNALS:
    void titleChanged(const QString& title);
    void unsupportedContent(QNetworkReply*);

public:
    FrameLoaderClientQt();
    ~FrameLoaderClientQt();

    void setFrame(QWebFrameAdapter*, LocalFrame*);

    bool hasWebView() const override; // mainly for assertions

    void makeRepresentation(DocumentLoader*) override { }

    void forceLayoutForNonHTML() override;

    void setCopiesOnScroll() override;

    void detachedFromParent2() override;
    void detachedFromParent3() override;

    void assignIdentifierToInitialRequest(WebCore::ResourceLoaderIdentifier, WebCore::DocumentLoader*, const WebCore::ResourceRequest&) override;

    void dispatchWillSendRequest(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier, WebCore::ResourceRequest&, const WebCore::ResourceResponse&) override;
    bool shouldUseCredentialStorage(DocumentLoader*, WebCore::ResourceLoaderIdentifier identifier) override;
    void dispatchDidReceiveAuthenticationChallenge(DocumentLoader*, WebCore::ResourceLoaderIdentifier identifier, const AuthenticationChallenge&) override;
    void dispatchDidReceiveResponse(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier, const WebCore::ResourceResponse&) override;
    void dispatchDidReceiveContentLength(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier, int) override;
    void dispatchDidFinishLoading(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier) override;
    void dispatchDidFailLoading(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier, const WebCore::ResourceError&) override;
    bool dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int) override;
    void dispatchLoadEventToOwnerElementInAnotherProcess() override { };

    void dispatchDidDispatchOnloadEvents() override;
    void dispatchDidReceiveServerRedirectForProvisionalLoad() override;
    void dispatchDidCancelClientRedirect() override;
    void dispatchWillPerformClientRedirect(const URL&, double interval, WallTime fireDate, LockBackForwardList) override;
    void dispatchDidNavigateWithinPage() override;
    void dispatchDidChangeLocationWithinPage() override;
    void dispatchDidPushStateWithinPage() override;
    void dispatchDidReplaceStateWithinPage() override;
    void dispatchDidPopStateWithinPage() override;
    void dispatchWillClose() override;
    void dispatchDidReceiveIcon() override;
    void dispatchDidStartProvisionalLoad() override;
    void dispatchDidReceiveTitle(const StringWithDirection&) override;
    void dispatchDidCommitLoad(std::optional<HasInsecureContent>, std::optional<WebCore::UsedLegacyTLS>, std::optional<WebCore::WasPrivateRelayed>) override;
    void dispatchDidFailProvisionalLoad(const ResourceError&, WillContinueLoading, WillInternallyHandleFailure) override;
    void dispatchDidFailLoad(const WebCore::ResourceError&) override;
    void dispatchDidFinishDocumentLoad() override;
    void dispatchDidFinishLoad() override;
    void dispatchDidReachLayoutMilestone(OptionSet<LayoutMilestone>) override;

    WebCore::LocalFrame* dispatchCreatePage(const WebCore::NavigationAction&, NewFrameOpenerPolicy) override;
    void dispatchShow() override;

    void dispatchDecidePolicyForResponse(const WebCore::ResourceResponse&, const WebCore::ResourceRequest&, PolicyCheckIdentifier, const String& downloadAttribute, FramePolicyFunction&&) override;
    void dispatchDecidePolicyForNewWindowAction(const WebCore::NavigationAction&, const WebCore::ResourceRequest&, FormState*, const String&, PolicyCheckIdentifier, FramePolicyFunction&&) override;
    void dispatchDecidePolicyForNavigationAction(const WebCore::NavigationAction&, const WebCore::ResourceRequest&, const ResourceResponse& redirectResponse, FormState*, PolicyDecisionMode, PolicyCheckIdentifier, FramePolicyFunction&&) override;
    void cancelPolicyCheck() override;

    void dispatchUnableToImplementPolicy(const WebCore::ResourceError&) override;

    void dispatchWillSendSubmitEvent(Ref<FormState>&&) override { }
    void dispatchWillSubmitForm(FormState&, CompletionHandler<void()>&&) override;

    void revertToProvisionalState(DocumentLoader*) override { }
    void setMainDocumentError(DocumentLoader*, const ResourceError&) override;

    void setMainFrameDocumentReady(bool) override;

    void startDownload(const WebCore::ResourceRequest&, const String& suggestedName = String()) override;

    void willChangeTitle(DocumentLoader*) override;
    void didChangeTitle(DocumentLoader*) override;

    void committedLoad(WebCore::DocumentLoader*, const SharedBuffer&) override;
    void finishedLoading(DocumentLoader*) override;

    void updateGlobalHistory() override;
    void updateGlobalHistoryRedirectLinks() override;
    bool shouldGoToHistoryItem(HistoryItem&) const override;
    void didDisplayInsecureContent() override;
    void didRunInsecureContent(SecurityOrigin&, const URL&) override;

    ResourceError cancelledError(const ResourceRequest&) const override;
    ResourceError blockedError(const ResourceRequest&) const override;
    ResourceError cannotShowURLError(const ResourceRequest&) const override;
    ResourceError interruptedForPolicyChangeError(const ResourceRequest&) const override;

    ResourceError cannotShowMIMETypeError(const ResourceResponse&) const override;
    ResourceError fileDoesNotExistError(const ResourceResponse&) const override;
    ResourceError httpsUpgradeRedirectLoopError(const ResourceRequest&) const override;
    ResourceError httpNavigationWithHTTPSOnlyError(const ResourceRequest&) const override;
    ResourceError pluginWillHandleLoadError(const ResourceResponse&) const override;

    bool shouldFallBack(const ResourceError&) const override;

    bool canHandleRequest(const WebCore::ResourceRequest&) const override;
    bool canShowMIMEType(const String& MIMEType) const override;
    bool canShowMIMETypeAsHTML(const String& MIMEType) const override;
    bool representationExistsForURLScheme(StringView URLScheme) const override;
    String generatedMIMETypeForURLScheme(StringView URLScheme) const override;

    void frameLoadCompleted() override;
    void saveViewStateToItem(WebCore::HistoryItem&) override;
    void restoreViewState() override;
    void provisionalLoadStarted() override;
    void didFinishLoad() override;
    void prepareForDataSourceReplacement() override;

    Ref<WebCore::DocumentLoader> createDocumentLoader(const WebCore::ResourceRequest&, const WebCore::SubstituteData&) override;
    void setTitle(const StringWithDirection&, const URL&) override;

    String userAgent(const WTF::URL&) const override;

    void savePlatformDataToCachedFrame(WebCore::CachedFrame*) override;
    void transitionToCommittedFromCachedFrame(WebCore::CachedFrame*) override;
    void transitionToCommittedForNewPage() override;

    void didRestoreFromBackForwardCache() override;

    bool canCachePage() const override;
    void convertMainResourceLoadToDownload(DocumentLoader*, const ResourceRequest&, const WebCore::ResourceResponse&) override;

    RefPtr<LocalFrame> createFrame(const AtomString& name, HTMLFrameOwnerElement&) override;
    RefPtr<Widget> createPlugin(const IntSize&, HTMLPlugInElement&, const URL&, const Vector<AtomString>&, const Vector<AtomString>&, const String&, bool) override;
    void redirectDataToPlugin(Widget& pluginWidget) override;

    ObjectContentType objectContentType(const URL&, const String& mimeTypeIn) override;
    AtomString overrideMediaType() const override;

    void dispatchDidClearWindowObjectInWorld(DOMWrapperWorld&) override;

    void registerForIconNotification() override;

    void willReplaceMultipartContent() override;
    void didReplaceMultipartContent() override;
    ResourceError blockedByContentBlockerError(const ResourceRequest &) const override;
    void updateCachedDocumentLoader(DocumentLoader &) override;
    void prefetchDNS(const WTF::String &) override;

    Ref<FrameNetworkingContext> createNetworkingContext() override;

    const URL& lastRequestedUrl() const { return m_lastRequestedUrl; }

    QWebFrameAdapter* webFrame() const;

    void originatingLoadStarted() { m_isOriginatingLoad = true; }

    void sendH2Ping(const URL&, CompletionHandler<void(Expected<Seconds, ResourceError>&&)>&&) override;

    void broadcastFrameRemovalToOtherProcesses() override { };
    void broadcastMainFrameURLChangeToOtherProcesses(const URL&) override { };

    static bool dumpFrameLoaderCallbacks;
    static bool dumpUserGestureInFrameLoaderCallbacks;
    static bool dumpResourceLoadCallbacks;
    static bool dumpResourceResponseMIMETypes;
    static bool dumpWillCacheResponseCallbacks;
    static QString dumpResourceLoadCallbacksPath;
    static bool sendRequestReturnsNullOnRedirect;
    static bool sendRequestReturnsNull;
    static QStringList sendRequestClearHeaders;
    static bool policyDelegateEnabled;
    static bool policyDelegatePermissive;
    static bool deferMainResourceDataLoad;
    static bool dumpHistoryCallbacks;
    static QMap<QString, QString> URLsToRedirect;

private Q_SLOTS:
    void onIconLoadedForPageURL(const QString&);

private:
    void emitLoadStarted();
    void emitLoadFinished(bool ok);

    LocalFrame *m_frame;
    QWebFrameAdapter *m_webFrame;
    ResourceResponse m_response;

    WTF::URL m_lastRequestedUrl;
    bool m_isOriginatingLoad;

    // QTFIXME: consider introducing some sort of flags for storing state
    bool m_isDisplayingErrorPage;
    bool m_shouldSuppressLoadStarted;
};

}

#endif
