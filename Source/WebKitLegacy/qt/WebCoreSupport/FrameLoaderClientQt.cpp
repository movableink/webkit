/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008 Collabora Ltd. All rights reserved.
 * Coypright (C) 2008 Holger Hans Peter Freyther
 * Coypright (C) 2009, 2010 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "FrameLoaderClientQt.h"

#include "FrameNetworkingContextQt.h"
#include "QWebFrameAdapter.h"
#include "QWebFrameData.h"
#include "QWebPageAdapter.h"
#include "QtPluginWidgetAdapter.h"
#include "qwebhistory_p.h"
#include "qwebhistoryinterface.h"
#include "qwebpluginfactory.h"
#include "qwebsettings.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPalette>
#include <QStringList>
#include <WebCore/DOMWindow.h>
#include <WebCore/DOMWrapperWorld.h>
#include <WebCore/DocumentLoader.h>
#include <WebCore/EventHandler.h>
#include <WebCore/FrameInlines.h>
#include <WebCore/FrameLoader.h>
#include <WebCore/FrameLoadRequest.h>
#include <WebCore/FrameView.h>
#include <WebCore/HistoryController.h>
#include <WebCore/HTMLPlugInElement.h>
#include <WebCore/HTTPParsers.h>
#include <WebCore/HitTestResult.h>
#include <WebCore/MIMETypeRegistry.h>
#include <WebCore/LocalDOMWindow.h>
#include <WebCore/MouseEvent.h>
#include <WebCore/PluginData.h>
#include <WebCore/QNetworkReplyHandler.h>
#include <WebCore/QStyleHelpers.h>
#include <WebCore/ResourceLoadTrackerQt.h>
#include <WebCore/QWebPageClient.h>
#include <WebCore/ResourceHandle.h>
#include <WebCore/ResourceHandleInternal.h>
#include <WebCore/ScrollView.h>
#include <WebCore/Settings.h>
#include <WebCore/SubframeLoader.h>
#include <WebCore/SubresourceLoader.h>
#include <WebCore/UserGestureIndicator.h>
#include <WebCore/Widget.h>
#include <WebCore/PlatformStrategies.h>
#include <WebCore/LoaderStrategy.h>
#include <wtf/CompletionHandler.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/text/StringBuilder.h>

#if ENABLE(ICONDATABASE)
#include "IconDatabaseClientQt.h"
#endif

static QMap<uint64_t, QString> dumpAssignedUrls;

// Compare with the file "WebKit/Tools/DumpRenderTree/mac/FrameLoadDelegate.mm".
static QString drtDescriptionSuitableForTestResult(WebCore::Frame* webCoreFrame)
{
    QWebFrameAdapter* frame = QWebFrameAdapter::kit(webCoreFrame);
    QString name = String(webCoreFrame->tree().uniqueName());

    bool isMainFrame = frame == &frame->pageAdapter->mainFrameAdapter();
    if (isMainFrame) {
        if (!name.isEmpty())
            return QString::fromLatin1("main frame \"%1\"").arg(name);
        return QLatin1String("main frame");
    }
    if (!name.isEmpty())
        return QString::fromLatin1("frame \"%1\"").arg(name);
    return QLatin1String("frame (anonymous)");
}

static QString drtPrintFrameUserGestureStatus(WebCore::Frame* frame)
{
    if (WebCore::UserGestureIndicator::processingUserGesture())
        return QString::fromLatin1("Frame with user gesture \"%1\"").arg(QLatin1String("true"));
    return QString::fromLatin1("Frame with user gesture \"%1\"").arg(QLatin1String("false"));
}

static QString drtDescriptionSuitableForTestResult(const WTF::URL& kurl)
{
    if (kurl.isEmpty() || !kurl.protocolIsFile())
        return kurl.string();
    // Remove the leading path from file urls.
    return QString(kurl.string()).remove(WebCore::FrameLoaderClientQt::dumpResourceLoadCallbacksPath).mid(1);
}

static QString drtDescriptionSuitableForTestResult(const WebCore::ResourceError& error)
{
    QString failingURL = error.failingURL().string();
    return QString::fromLatin1("<NSError domain NSURLErrorDomain, code %1, failing URL \"%2\">").arg(error.errorCode()).arg(failingURL);
}

static QString drtDescriptionSuitableForTestResult(const WebCore::ResourceRequest& request)
{
    QString url = drtDescriptionSuitableForTestResult(request.url());
    QString httpMethod = request.httpMethod();
    QString mainDocumentUrl = drtDescriptionSuitableForTestResult(request.firstPartyForCookies());
    return QString::fromLatin1("<NSURLRequest URL %1, main document URL %2, http method %3>").arg(url).arg(mainDocumentUrl).arg(httpMethod);
}

static QString drtDescriptionSuitableForTestResult(const WebCore::ResourceResponse& response)
{
    QString url = drtDescriptionSuitableForTestResult(response.url());
    int httpStatusCode = response.httpStatusCode();
    return QString::fromLatin1("<NSURLResponse %1, http status code %2>").arg(url).arg(httpStatusCode);
}

static QString drtDescriptionSuitableForTestResult(const RefPtr<WebCore::Node> node, int exception)
{
    QString result;
    if (exception) {
        result.append(QLatin1String("ERROR"));
        return result;
    }
    if (!node) {
        result.append(QLatin1String("NULL"));
        return result;
    }
    result.append(node->nodeName());
    RefPtr<WebCore::Node> parent = node->parentNode();
    if (parent) {
        result.append(QLatin1String(" > "));
        result.append(drtDescriptionSuitableForTestResult(parent, 0));
    }
    return result;
}

namespace WebCore {

bool FrameLoaderClientQt::dumpFrameLoaderCallbacks = false;
bool FrameLoaderClientQt::dumpUserGestureInFrameLoaderCallbacks = false;
bool FrameLoaderClientQt::dumpWillCacheResponseCallbacks = false;
bool FrameLoaderClientQt::dumpResourceLoadCallbacks = false;
bool FrameLoaderClientQt::sendRequestReturnsNullOnRedirect = false;
bool FrameLoaderClientQt::sendRequestReturnsNull = false;
bool FrameLoaderClientQt::dumpResourceResponseMIMETypes = false;
bool FrameLoaderClientQt::deferMainResourceDataLoad = true;
bool FrameLoaderClientQt::dumpHistoryCallbacks = false;

QStringList FrameLoaderClientQt::sendRequestClearHeaders;
QString FrameLoaderClientQt::dumpResourceLoadCallbacksPath;
bool FrameLoaderClientQt::policyDelegateEnabled = false;
bool FrameLoaderClientQt::policyDelegatePermissive = false;
QMap<QString, QString> FrameLoaderClientQt::URLsToRedirect = QMap<QString, QString>();

// Taken from the file "WebKit/Tools/DumpRenderTree/chromium/WebViewHost.cpp".
static const char* navigationTypeToString(NavigationType type)
{
    switch (type) {
    case NavigationType::LinkClicked:
        return "link clicked";
    case NavigationType::FormSubmitted:
        return "form submitted";
    case NavigationType::BackForward:
        return "back/forward";
    case NavigationType::Reload:
        return "reload";
    case NavigationType::FormResubmitted:
        return "form resubmitted";
    case NavigationType::Other:
        return "other";
    }
    return "illegal value";
}

FrameLoaderClientQt::FrameLoaderClientQt(WebCore::FrameLoader& frameLoader)
    : WebCore::LocalFrameLoaderClient(frameLoader)
    , m_frame(0)
    , m_webFrame(0)
    , m_isOriginatingLoad(false)
    , m_isDisplayingErrorPage(false)
    , m_shouldSuppressLoadStarted(false)
{
}


FrameLoaderClientQt::~FrameLoaderClientQt()
{
    // Delete QWebFrame (handle()), which owns QWebFramePrivate, which
    // _is_ a QWebFrameAdapter.
    if (m_webFrame)
        delete m_webFrame->handle();
    m_frame = 0;
    m_webFrame = 0;
}

void FrameLoaderClientQt::setFrame(QWebFrameAdapter* webFrame, LocalFrame* frame)
{
    m_webFrame = webFrame;
    m_frame = frame;

    if (!m_webFrame || !m_webFrame->pageAdapter) {
        qWarning("FrameLoaderClientQt::setFrame frame without Page!");
        return;
    }

    if (frame && frame->isMainFrame()) {
        auto& tracker = WebCore::ResourceLoadTrackerQt::instance();

        connect(&tracker, SIGNAL(resourceLoadStarted(QUrl,QString,QtResourceRequestInfo,bool)), 
                this, SLOT(onResourceLoadStarted(QUrl,QString,QtResourceRequestInfo,bool)));
        connect(&tracker, SIGNAL(resourceLoadFinished(QUrl,QString,qint64,QtResourceTimingInfo,bool,bool)),
                this, SLOT(onResourceLoadFinished(QUrl,QString,qint64,QtResourceTimingInfo,bool,bool)));
    }

    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)),
        m_webFrame->pageAdapter->handle(), SIGNAL(unsupportedContent(QNetworkReply*)));

    connect(this, SIGNAL(titleChanged(QString)),
        m_webFrame->handle(), SIGNAL(titleChanged(QString)));

    connect(this, SIGNAL(resourceLoadStarted(QUrl,QString,QtResourceRequestInfo,bool)),
        m_webFrame->pageAdapter->handle(), SIGNAL(resourceLoadStarted(QUrl,QString,QtResourceRequestInfo,bool)));
    connect(this, SIGNAL(resourceLoadFinished(QUrl,QString,qint64,QtResourceTimingInfo,bool,bool)),
        m_webFrame->pageAdapter->handle(), SIGNAL(resourceLoadFinished(QUrl,QString,qint64,QtResourceTimingInfo,bool,bool)));
}

bool FrameLoaderClientQt::hasWebView() const
{
    // notImplemented();
    return true;
}

void FrameLoaderClientQt::savePlatformDataToCachedFrame(CachedFrame*)
{
    notImplemented();
}

void FrameLoaderClientQt::transitionToCommittedFromCachedFrame(CachedFrame*)
{
}

void FrameLoaderClientQt::transitionToCommittedForNewPage(InitializingIframe)
{
    ASSERT(m_frame);
    ASSERT(m_webFrame);

    QObject* qWebPage = m_webFrame->pageAdapter->handle();
    QBrush brush = qWebPage->property("palette").value<QPalette>().brush(QPalette::Base);
    QColor backgroundColor = brush.style() == Qt::SolidPattern ? brush.color() : QColor();

    const QSize preferredLayoutSize = qWebPage->property("preferredContentsSize").toSize();

    ScrollbarMode hScrollbar = (ScrollbarMode) m_webFrame->scrollBarPolicy(Qt::Horizontal);
    ScrollbarMode vScrollbar = (ScrollbarMode) m_webFrame->scrollBarPolicy(Qt::Vertical);
    bool hLock = hScrollbar != ScrollbarMode::Auto;
    bool vLock = vScrollbar != ScrollbarMode::Auto;

    m_frame->createView(qWebPage->property("viewportSize").toSize(),
        Color(backgroundColor),
        preferredLayoutSize.isValid() ? IntSize(preferredLayoutSize) : IntSize(),
        preferredLayoutSize.isValid(),
        hScrollbar, hLock,
        vScrollbar, vLock);

    bool isMainFrame = m_frame == &m_frame->page()->mainFrame();
    if (isMainFrame &&m_webFrame->pageAdapter->client) {
        bool resizesToContents = m_webFrame->pageAdapter->client->viewResizesToContentsEnabled();

        m_frame->view()->setPaintsEntireContents(resizesToContents);
        m_frame->view()->setDelegatedScrollingMode(resizesToContents ? DelegatedScrollingMode::DelegatedToNativeScrollView : DelegatedScrollingMode::NotDelegated);
    }
}

void FrameLoaderClientQt::didRestoreFromBackForwardCache()
{
}

void FrameLoaderClientQt::forceLayoutForNonHTML()
{
}


void FrameLoaderClientQt::setCopiesOnScroll()
{
    // Apparently this is mac specific.
}


void FrameLoaderClientQt::detachedFromParent2()
{
}


void FrameLoaderClientQt::detachedFromParent3()
{
}

void FrameLoaderClientQt::dispatchDidDispatchOnloadEvents()
{
    // Don't need this one.
    if (dumpFrameLoaderCallbacks)
        printf("%s - didHandleOnloadEventsForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));
}


void FrameLoaderClientQt::dispatchDidReceiveServerRedirectForProvisionalLoad()
{
    if (dumpFrameLoaderCallbacks)
        printf("%s - didReceiveServerRedirectForProvisionalLoadForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

    notImplemented();
}


void FrameLoaderClientQt::dispatchDidCancelClientRedirect()
{
    if (dumpFrameLoaderCallbacks)
        printf("%s - didCancelClientRedirectForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

    notImplemented();
}


void FrameLoaderClientQt::dispatchWillPerformClientRedirect(const URL& url, double, WTF::WallTime, LockBackForwardList)
{
    if (dumpFrameLoaderCallbacks)
        printf("%s - willPerformClientRedirectToURL: %s \n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)), qPrintable(drtDescriptionSuitableForTestResult(url)));

    if (dumpUserGestureInFrameLoaderCallbacks)
        printf("%s - in willPerformClientRedirect\n", qPrintable(drtPrintFrameUserGestureStatus(m_frame)));

    notImplemented();
}

void FrameLoaderClientQt::dispatchDidNavigateWithinPage()
{
    if (!m_webFrame)
        return;

    FrameLoader& loader = m_frame->loader();
    bool loaderCompleted = !(loader.activeDocumentLoader() && loader.activeDocumentLoader()->isLoadingInAPISense());

    if (!loaderCompleted)
        return;

    dispatchDidCommitLoad(std::nullopt, std::nullopt, std::nullopt); // QTFIXME
    dispatchDidFinishLoad();
}

void FrameLoaderClientQt::dispatchDidChangeLocationWithinPage()
{
    if (dumpFrameLoaderCallbacks)
        printf("%s - didChangeLocationWithinPageForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

    if (!m_webFrame)
        return;

    m_webFrame->emitUrlChanged();
    m_webFrame->pageAdapter->updateNavigationActions();
}


void FrameLoaderClientQt::dispatchDidPushStateWithinPage()
{
    if (dumpFrameLoaderCallbacks)
        printf("%s - dispatchDidPushStateWithinPage\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

    dispatchDidNavigateWithinPage();
}

void FrameLoaderClientQt::dispatchDidReplaceStateWithinPage()
{
    if (dumpFrameLoaderCallbacks)
        printf("%s - dispatchDidReplaceStateWithinPage\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

    dispatchDidNavigateWithinPage();
}

void FrameLoaderClientQt::dispatchDidPopStateWithinPage()
{
    // No need to call dispatchDidNavigateWithinPage here, it's already been done in loadInSameDocument().
}

void FrameLoaderClientQt::dispatchWillClose()
{
}


void FrameLoaderClientQt::dispatchDidStartProvisionalLoad()
{
    if (dumpFrameLoaderCallbacks)
        printf("%s - didStartProvisionalLoadForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

    if (dumpUserGestureInFrameLoaderCallbacks)
        printf("%s - in didStartProvisionalLoadForFrame\n", qPrintable(drtPrintFrameUserGestureStatus(m_frame)));

    m_lastRequestedUrl = m_frame->loader().activeDocumentLoader()->url();

    if (!m_webFrame)
        return;
    emitLoadStarted();

    m_webFrame->didStartProvisionalLoad();
}


void FrameLoaderClientQt::dispatchDidReceiveTitle(const StringWithDirection& title)
{
    // FIXME: Use direction of title.
    if (dumpFrameLoaderCallbacks)
        printf("%s - didReceiveTitle: %s\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)), qPrintable(QString(title.string)));

    if (!m_webFrame)
        return;

    Q_EMIT titleChanged(title.string);
}


void FrameLoaderClientQt::dispatchDidCommitLoad(std::optional<HasInsecureContent>, std::optional<WebCore::UsedLegacyTLS>, std::optional<WebCore::WasPrivateRelayed>)
{
    if (dumpFrameLoaderCallbacks)
        printf("%s - didCommitLoadForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

    if (m_frame->tree().parent() || !m_webFrame)
        return;

    m_webFrame->emitUrlChanged();
    m_webFrame->pageAdapter->updateNavigationActions();

    // We should assume first the frame has no title. If it has, then the above dispatchDidReceiveTitle()
    // will be called very soon with the correct title.
    // This properly resets the title when we navigate to a URI without a title.
    Q_EMIT titleChanged(QString());

    bool isMainFrame = (m_frame == &m_frame->page()->mainFrame());
    if (!isMainFrame)
        return;

    Q_EMIT m_webFrame->pageAdapter->emitViewportChangeRequested();
}


void FrameLoaderClientQt::dispatchDidFinishDocumentLoad()
{
    if (dumpFrameLoaderCallbacks)
        printf("%s - didFinishDocumentLoadForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

    if (QWebPageAdapter::drtRun) {
        int unloadEventCount = m_frame->document()->domWindow()->pendingUnloadEventListeners();
        if (unloadEventCount)
            printf("%s - has %u onunload handler(s)\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)), unloadEventCount);
    }

    if (m_frame->tree().parent() || !m_webFrame)
        return;

    m_webFrame->pageAdapter->updateNavigationActions();
}


void FrameLoaderClientQt::dispatchDidFinishLoad()
{
    if (dumpFrameLoaderCallbacks)
        printf("%s - didFinishLoadForFrame\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

    if (!m_webFrame)
        return;

    m_webFrame->pageAdapter->updateNavigationActions();
    emitLoadFinished(true);
}

void FrameLoaderClientQt::dispatchDidReachLayoutMilestone(OptionSet<LayoutMilestone> milestones)
{
    if (!m_webFrame)
        return;

    if (milestones & LayoutMilestone::DidFirstVisuallyNonEmptyLayout)
        m_webFrame->emitInitialLayoutCompleted();
}

void FrameLoaderClientQt::dispatchShow()
{
    notImplemented();
}


void FrameLoaderClientQt::cancelPolicyCheck()
{
    // qDebug() << "FrameLoaderClientQt::cancelPolicyCheck";
}


void FrameLoaderClientQt::dispatchWillSubmitForm(FormState&, CompletionHandler<void()>&& completionHandler)
{
    notImplemented();
    // FIXME: This is surely too simple.
    completionHandler();
}

void FrameLoaderClientQt::setMainFrameDocumentReady(bool)
{
    // This is only interesting once we provide an external API for the DOM.
}


void FrameLoaderClientQt::willChangeTitle(DocumentLoader*)
{
    // No need for, dispatchDidReceiveTitle is the right callback.
}


void FrameLoaderClientQt::didChangeTitle(DocumentLoader*)
{
    // No need for, dispatchDidReceiveTitle is the right callback.
}


void FrameLoaderClientQt::finishedLoading(DocumentLoader*)
{
    // Plugins would be notified here, if they still existed
}

bool FrameLoaderClientQt::canShowMIMETypeAsHTML(const String& MIMEType) const
{
    notImplemented();
    return false;
}
    
bool FrameLoaderClientQt::canShowMIMEType(const String& MIMEType) const
{
    String type = MIMEType.convertToASCIILowercase(); // FIXME: Do we really need it?
    if (MIMETypeRegistry::canShowMIMEType(type))
        return true;

    return false;
}

bool FrameLoaderClientQt::representationExistsForURLScheme(StringView) const
{
    return false;
}


String FrameLoaderClientQt::generatedMIMETypeForURLScheme(StringView) const
{
    notImplemented();
    return String();
}


void FrameLoaderClientQt::frameLoadCompleted()
{
    // Note that this can be called multiple times.
    if (!m_webFrame)
        return;
    m_webFrame->pageAdapter->updateNavigationActions();
}


void FrameLoaderClientQt::restoreViewState()
{
    if (!m_webFrame)
        return;
    m_webFrame->pageAdapter->emitRestoreFrameStateRequested(m_webFrame);
}


void FrameLoaderClientQt::provisionalLoadStarted()
{
    // Don't need to do anything here.
}


void FrameLoaderClientQt::didFinishLoad()
{
    // notImplemented();
}


void FrameLoaderClientQt::prepareForDataSourceReplacement()
{
}

void FrameLoaderClientQt::setTitle(const StringWithDirection& title, const URL& url)
{
    // Used by Apple WebKit to update the title of an existing history item.
    // QtWebKit doesn't accomodate this on history items. If it ever does,
    // it should be privateBrowsing-aware. For now, we are just passing
    // globalhistory layout tests.
    // FIXME: Use direction of title.
    if (dumpHistoryCallbacks) {
        printf("WebView updated the title for history URL \"%s\" to \"%s\".\n",
            qPrintable(drtDescriptionSuitableForTestResult(url)),
            qPrintable(QString(title.string)));
    }
}


String FrameLoaderClientQt::userAgent(const URL& url) const
{
    if (m_webFrame)
        return m_webFrame->pageAdapter->userAgentForUrl(url).remove(QLatin1Char('\n')).remove(QLatin1Char('\r'));
    return String();
}

void FrameLoaderClientQt::dispatchDidReceiveIcon()
{
    if (m_webFrame)
        m_webFrame->emitIconChanged();
}

bool FrameLoaderClientQt::canHandleRequest(const WebCore::ResourceRequest&) const
{
    return true;
}

void FrameLoaderClientQt::dispatchDidClearWindowObjectInWorld(DOMWrapperWorld &world)
{
    if (&world != &mainThreadNormalWorldSingleton())
        return;

    if (m_webFrame) {
        m_webFrame->didClearWindowObject();
        m_webFrame->pageAdapter->clearCustomActions();
    }
}

void FrameLoaderClientQt::registerForIconNotification()
{
#if ENABLE(ICONDATABASE)
    if (shouldRegister)
        connect(IconDatabaseClientQt::instance(), SIGNAL(iconLoadedForPageURL(QString)), this, SLOT(onIconLoadedForPageURL(QString)), Qt::UniqueConnection);
    else
        disconnect(IconDatabaseClientQt::instance(), SIGNAL(iconLoadedForPageURL(QString)), this, SLOT(onIconLoadedForPageURL(QString)));
#endif
}

void FrameLoaderClientQt::onIconLoadedForPageURL(const QString& url)
{
#if ENABLE(ICONDATABASE)
    if (m_webFrame && m_webFrame->url == url)
        m_webFrame->emitIconChanged();
#endif
}


void FrameLoaderClientQt::updateGlobalHistory()
{
    QWebHistoryInterface* history = QWebHistoryInterface::defaultInterface();
    WebCore::DocumentLoader* loader = m_frame->loader().documentLoader();
    if (history)
        history->addHistoryEntry(loader->urlForHistory().string());

    if (dumpHistoryCallbacks) {
        printf("WebView navigated to url \"%s\" with title \"%s\" with HTTP equivalent method \"%s\".  The navigation was %s and was %s%s.\n",
            qPrintable(drtDescriptionSuitableForTestResult(loader->urlForHistory())),
            qPrintable(QString(loader->title().string)),
            qPrintable(QString(loader->request().httpMethod())),
            ((loader->substituteData().isValid() || (loader->response().httpStatusCode() >= 400)) ? "a failure" : "successful"),
            ((!loader->clientRedirectSourceForHistory().isEmpty()) ? "a client redirect from " : "not a client redirect"),
            (!loader->clientRedirectSourceForHistory().isEmpty()) ? qPrintable(drtDescriptionSuitableForTestResult(loader->clientRedirectSourceForHistory())) : "");
    }
}

void FrameLoaderClientQt::updateGlobalHistoryRedirectLinks()
{
    // Apple WebKit is the only port that makes use of this callback. It calls
    // WebCore::HistoryItem::addRedirectURL() with the contents of
    // loader->[server|client]RedirectDestinationForHistory().
    // WebCore can associate a bunch of redirect URLs with a particular
    // item in the history, presumably this allows Safari to skip the redirections
    // when navigating to that history item. That might be a feature Qt wants to
    // offer through QWebHistoryInterface in the future. For now, we're just
    // passing tests in LayoutTests/http/tests/globalhistory.
    WebCore::DocumentLoader* loader = m_frame->loader().documentLoader();

    if (!loader->clientRedirectSourceForHistory().isNull()) {
        if (dumpHistoryCallbacks) {
            printf("WebView performed a client redirect from \"%s\" to \"%s\".\n",
                qPrintable(QString(loader->clientRedirectSourceForHistory())),
                qPrintable(QString(loader->clientRedirectDestinationForHistory())));
        }
    }

    if (!loader->serverRedirectSourceForHistory().isNull()) {
        if (dumpHistoryCallbacks) {
            printf("WebView performed a server redirect from \"%s\" to \"%s\".\n",
                qPrintable(QString(loader->serverRedirectSourceForHistory())),
                qPrintable(QString(loader->serverRedirectDestinationForHistory())));
        }
    }
}

ShouldGoToHistoryItem FrameLoaderClientQt::shouldGoToHistoryItem(HistoryItem&, IsSameDocumentNavigation, ProcessSwapDisposition) const
{
    return ShouldGoToHistoryItem::Yes;
}

bool FrameLoaderClientQt::supportsAsyncShouldGoToHistoryItem() const
{
    return false;
}

void FrameLoaderClientQt::shouldGoToHistoryItemAsync(HistoryItem& item, CompletionHandler<void(ShouldGoToHistoryItem)>&& completionHandler) const
{
    completionHandler(shouldGoToHistoryItem(item, IsSameDocumentNavigation::No, ProcessSwapDisposition()));
}

void FrameLoaderClientQt::didDisplayInsecureContent()
{
    if (dumpFrameLoaderCallbacks)
        printf("didDisplayInsecureContent\n");

    notImplemented();
}

void FrameLoaderClientQt::didRunInsecureContent(SecurityOrigin&)
{
    if (dumpFrameLoaderCallbacks)
        printf("didRunInsecureContent\n");

    notImplemented();
}

void FrameLoaderClientQt::saveViewStateToItem(WebCore::HistoryItem& item)
{
    QWebHistoryItem historyItem(new QWebHistoryItemPrivate(item));
    m_webFrame->pageAdapter->emitSaveFrameStateRequested(m_webFrame, &historyItem);
}

bool FrameLoaderClientQt::canCachePage() const
{
    return true;
}

void FrameLoaderClientQt::setMainDocumentError(WebCore::DocumentLoader* loader, const WebCore::ResourceError& error)
{
    // Plugins removed
}

// FIXME: This function should be moved into WebCore.
void FrameLoaderClientQt::committedLoad(WebCore::DocumentLoader* loader, const SharedBuffer& buffer)
{
    loader->commitData(buffer);

    // If we are sending data to MediaDocument, we should stop here and cancel the request.
    if (m_frame->document()->isMediaDocument())
        loader->cancelMainResourceLoad(platformStrategies()->loaderStrategy()->pluginWillHandleLoadError(loader->response()));
}



bool FrameLoaderClientQt::shouldFallBack(const WebCore::ResourceError& error) const
{
    using Error = NeverDestroyed<const ResourceError>;
    static Error cancelledError(platformStrategies()->loaderStrategy()->cancelledError(ResourceRequest()));
    static Error pluginWillHandleLoadError(platformStrategies()->loaderStrategy()->pluginWillHandleLoadError(ResourceResponse()));
    static Error errorInterruptedForPolicyChange(platformStrategies()->loaderStrategy()->interruptedForPolicyChangeError(ResourceRequest()));

    if (error.errorCode() == cancelledError.get().errorCode() && error.domain() == cancelledError.get().domain())
        return false;

    if (error.errorCode() == errorInterruptedForPolicyChange.get().errorCode() && error.domain() == errorInterruptedForPolicyChange.get().domain())
        return false;

    if (error.errorCode() == pluginWillHandleLoadError.get().errorCode() && error.domain() == pluginWillHandleLoadError.get().domain())
        return false;

    return true;
}

Ref<WebCore::DocumentLoader> FrameLoaderClientQt::createDocumentLoader(WebCore::ResourceRequest&& request, WebCore::SubstituteData&& substituteData)
{
    Ref<DocumentLoader> loader = DocumentLoader::create(WTFMove(request), WTFMove(substituteData));
    if (!deferMainResourceDataLoad || substituteData.isValid())
        loader->setDeferMainResourceDataLoad(false);
    return loader;
}

void FrameLoaderClientQt::convertMainResourceLoadToDownload(DocumentLoader* documentLoader, const ResourceRequest& request, const ResourceResponse&)
{
    if (!m_webFrame)
        return;

    QNetworkReplyHandler* handler = documentLoader->mainResourceLoader()->handle()->getInternal()->m_job;
    if (!handler) {
        qWarning("Attempted to download unsupported URL %s", request.url().string().span8().data());
        return;
    }

    QNetworkReply* reply = handler->release();
    if (reply) {
        if (m_webFrame->pageAdapter->forwardUnsupportedContent) {
            Q_EMIT unsupportedContent(reply);
        } else {
            reply->abort();
            reply->deleteLater();
        }
    }
}

void FrameLoaderClientQt::assignIdentifierToInitialRequest(WebCore::ResourceLoaderIdentifier identifier, WebCore::IsMainResourceLoad, WebCore::DocumentLoader*, const WebCore::ResourceRequest& request)
{
    if (dumpResourceLoadCallbacks)
        dumpAssignedUrls[identifier.toUInt64()] = drtDescriptionSuitableForTestResult(request.url());
}

static void blockRequest(WebCore::ResourceRequest& request)
{
    request.setURL(QUrl());
}

static bool isLocalhost(const QString& host)
{
    return host == QLatin1String("127.0.0.1") || host == QLatin1String("localhost");
}

static bool hostIsUsedBySomeTestsToGenerateError(const QString& host)
{
    return host == QLatin1String("255.255.255.255");
}

void FrameLoaderClientQt::dispatchWillSendRequest(WebCore::DocumentLoader*, ResourceLoaderIdentifier identifier, WebCore::ResourceRequest& newRequest, const WebCore::ResourceResponse& redirectResponse)
{
    if (dumpResourceLoadCallbacks)
        printf("%s - willSendRequest %s redirectResponse %s\n",
            qPrintable(dumpAssignedUrls[identifier.toUInt64()]),
            qPrintable(drtDescriptionSuitableForTestResult(newRequest)),
            (redirectResponse.isNull()) ? "(null)" : qPrintable(drtDescriptionSuitableForTestResult(redirectResponse)));

    if (sendRequestReturnsNull) {
        blockRequest(newRequest);
        return;
    }

    if (sendRequestReturnsNullOnRedirect && !redirectResponse.isNull()) {
        printf("Returning null for this redirect\n");
        blockRequest(newRequest);
        return;
    }

    QUrl url = newRequest.url();
    QString host = url.host();
    QString urlScheme = url.scheme().toLower();

    if (QWebPageAdapter::drtRun
        && !host.isEmpty()
        && (urlScheme == QLatin1String("http") || urlScheme == QLatin1String("https"))) {

        QUrl testURL = m_webFrame->pageAdapter->mainFrameAdapter().frameLoaderClient->lastRequestedUrl();
        QString testHost = testURL.host();
        QString testURLScheme = testURL.scheme().toLower();

        if (!isLocalhost(host)
            && !hostIsUsedBySomeTestsToGenerateError(host)
            && ((testURLScheme != QLatin1String("http") && testURLScheme != QLatin1String("https")) || isLocalhost(testHost))) {
            printf("Blocked access to external URL %s\n", qPrintable(drtDescriptionSuitableForTestResult(newRequest.url())));
            blockRequest(newRequest);
            return;
        }
    }

    for (int i = 0; i < sendRequestClearHeaders.size(); ++i)
        newRequest.setHTTPHeaderField(String::fromLatin1(sendRequestClearHeaders.at(i).toLocal8Bit().constData()), String());

    if (QWebPageAdapter::drtRun) {
        QMap<QString, QString>::const_iterator it = URLsToRedirect.constFind(url.toString());
        if (it != URLsToRedirect.constEnd())
            newRequest.setURL(QUrl(it.value()));
    }
    // Seems like the Mac code doesn't do anything here by default neither.
    // qDebug() << "FrameLoaderClientQt::dispatchWillSendRequest" << request.isNull() << url;
}

bool FrameLoaderClientQt::shouldUseCredentialStorage(DocumentLoader*, WebCore::ResourceLoaderIdentifier)
{
    notImplemented();
    return false;
}

void FrameLoaderClientQt::dispatchDidReceiveAuthenticationChallenge(DocumentLoader*, WebCore::ResourceLoaderIdentifier, const AuthenticationChallenge&)
{
    notImplemented();
}

void FrameLoaderClientQt::dispatchDidReceiveResponse(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier identifier, const WebCore::ResourceResponse& response)
{

    m_response = response;
    if (dumpWillCacheResponseCallbacks)
        printf("%s - willCacheResponse: called\n",
            qPrintable(dumpAssignedUrls[identifier.toUInt64()]));

    if (dumpResourceLoadCallbacks)
        printf("%s - didReceiveResponse %s\n",
            qPrintable(dumpAssignedUrls[identifier.toUInt64()]),
            qPrintable(drtDescriptionSuitableForTestResult(response)));

    if (dumpResourceResponseMIMETypes) {
        printf("%s has MIME type %s\n",
            qPrintable(QString(response.url().lastPathComponent().toString())),
            qPrintable(QString(String(response.mimeType()))));
    }
}

void FrameLoaderClientQt::dispatchDidReceiveContentLength(WebCore::DocumentLoader*, WebCore::ResourceLoaderIdentifier, int)
{
}

void FrameLoaderClientQt::dispatchDidFinishLoading(WebCore::DocumentLoader*, WebCore::IsMainResourceLoad, WebCore::ResourceLoaderIdentifier identifier)
{
    if (dumpResourceLoadCallbacks)
        printf("%s - didFinishLoading\n",
            (dumpAssignedUrls.contains(identifier.toUInt64()) ? qPrintable(dumpAssignedUrls[identifier.toUInt64()]) : "<unknown>"));
}

void FrameLoaderClientQt::dispatchDidFailLoading(WebCore::DocumentLoader* loader, WebCore::IsMainResourceLoad, WebCore::ResourceLoaderIdentifier identifier, const WebCore::ResourceError& error)
{
    if (dumpResourceLoadCallbacks)
        printf("%s - didFailLoadingWithError: %s\n",
            (dumpAssignedUrls.contains(identifier.toUInt64()) ? qPrintable(dumpAssignedUrls[identifier.toUInt64()]) : "<unknown>"),
            qPrintable(drtDescriptionSuitableForTestResult(error)));
}

bool FrameLoaderClientQt::dispatchDidLoadResourceFromMemoryCache(WebCore::DocumentLoader*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&, int)
{
    notImplemented();
    return false;
}

bool FrameLoaderClientQt::callErrorPageExtension(const WebCore::ResourceError& error)
{
    QWebPageAdapter* page = m_webFrame->pageAdapter;
    if (!page->supportsErrorPageExtension())
        return false;
    QWebPageAdapter::ErrorPageOption option;
    option.url = QUrl(error.failingURL());
    option.frame = m_webFrame;
    option.domain = error.domain();
    option.error = error.errorCode();
    option.errorString = error.localizedDescription();

    QWebPageAdapter::ErrorPageReturn output;

    if (!page->errorPageExtension(&option, &output))
        return false;

    m_isDisplayingErrorPage = true;
    m_shouldSuppressLoadStarted = true;

    URL baseUrl(output.baseUrl);
    URL failingUrl(option.url);

    WebCore::ResourceRequest request(WTFMove(baseUrl));
    WTF::RefPtr<WebCore::SharedBuffer> buffer = WebCore::SharedBuffer::create(std::span { output.content.constData(), static_cast<size_t>(output.content.size()) });
    WebCore::ResourceResponse response(URL(failingUrl), WTF::String(output.contentType), buffer->size(), WTF::String(output.encoding));
    // FIXME: visibility?
    WebCore::SubstituteData substituteData(WTFMove(buffer), WTFMove(failingUrl), WTFMove(response), SubstituteData::SessionHistoryVisibility::Hidden);
    m_frame->loader().load(WebCore::FrameLoadRequest(*m_frame, WTFMove(request), WTFMove(substituteData)));

    m_shouldSuppressLoadStarted = false;

    return true;
}

void FrameLoaderClientQt::dispatchDidFailProvisionalLoad(const WebCore::ResourceError& error, WillContinueLoading, WillInternallyHandleFailure)
{
    if (dumpFrameLoaderCallbacks)
        printf("%s - didFailProvisionalLoadWithError\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

    if (!error.isNull() && !error.isCancellation()) {
        callErrorPageExtension(error);
    }

    if (m_webFrame)
        emitLoadFinished(false);
}

void FrameLoaderClientQt::dispatchDidFailLoad(const WebCore::ResourceError& error)
{
    if (dumpFrameLoaderCallbacks)
        printf("%s - didFailLoadWithError\n", qPrintable(drtDescriptionSuitableForTestResult(m_frame)));

    if (!error.isNull() && !error.isCancellation()) {
        callErrorPageExtension(error);
    }

    if (m_webFrame)
        emitLoadFinished(false);
}

WebCore::LocalFrame* FrameLoaderClientQt::dispatchCreatePage(const WebCore::NavigationAction&, NewFrameOpenerPolicy)
{
    if (!m_webFrame)
        return 0;
    QWebPageAdapter* newPage = m_webFrame->pageAdapter->createWindow(/* modalDialog = */ false);
    if (!newPage)
        return 0;
    return newPage->mainFrameAdapter().frame;
}

void FrameLoaderClientQt::dispatchDecidePolicyForResponse(const WebCore::ResourceResponse& response, const WebCore::ResourceRequest&, const String& downloadAttribute, FramePolicyFunction&& function)
{
    // We need to call directly here.
    switch (response.httpStatusCode()) {
    case 205:
        // FIXME: a 205 response requires that the requester reset the document view.
        // Fallthrough
    case 204:
        function(PolicyAction::Ignore);
        return;
    }

    if (WebCore::contentDispositionType(response.httpHeaderField(HTTPHeaderName::ContentDisposition)) == WebCore::ContentDispositionAttachment)
        function(PolicyAction::Download);
    else if (canShowMIMEType(response.mimeType()))
        function(PolicyAction::Use);
    else
        function(PolicyAction::Download);
}

void FrameLoaderClientQt::dispatchDecidePolicyForNewWindowAction(const WebCore::NavigationAction& action, const WebCore::ResourceRequest& request, FormState*, const WTF::String&, std::optional<HitTestResult>&&, FramePolicyFunction&& function)
{
    Q_ASSERT(m_webFrame);
    QNetworkRequest r(request.toNetworkRequest(m_frame->loader().networkingContext()));

    if (!m_webFrame->pageAdapter->acceptNavigationRequest(0, r, (int)action.type())) {
        if (action.type() == NavigationType::FormSubmitted || action.type() == NavigationType::FormResubmitted)
            m_frame->loader().resetMultipleFormSubmissionProtection();

        if (action.type() == NavigationType::LinkClicked && r.url().hasFragment())
            m_frame->loader().activeDocumentLoader()->setLastCheckedRequest(ResourceRequest());

        function(PolicyAction::Ignore);
        return;
    }
    function(PolicyAction::Use);
}

void FrameLoaderClientQt::dispatchDecidePolicyForNavigationAction(const WebCore::NavigationAction& action, const WebCore::ResourceRequest& request, const ResourceResponse& redirectResponse, WebCore::FormState*, const String&, std::optional<NavigationIdentifier>, std::optional<HitTestResult>&&, bool, IsPerformingHTTPFallback, WebCore::SandboxFlags, WebCore::PolicyDecisionMode, FramePolicyFunction&& function)
{
    Q_ASSERT(m_webFrame);
    QNetworkRequest r(request.toNetworkRequest(m_frame->loader().networkingContext()));
    PolicyAction result;

    // Currently, this is only enabled by DRT.
    if (policyDelegateEnabled) {
        RefPtr<Node> node;
        auto keyStateEventData = action.keyStateEventData();
        if (auto mouseEventData = action.mouseEventData()) {
            constexpr OptionSet<HitTestRequest::Type> hitType { HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active, HitTestRequest::Type::DisallowUserAgentShadowContent, HitTestRequest::Type::AllowChildFrameContent };
            node = m_webFrame->frame->eventHandler().hitTestResultAtPoint(mouseEventData->absoluteLocation, hitType).innerNonSharedNode();
        }

        printf("Policy delegate: attempt to load %s with navigation type '%s'%s\n",
            qPrintable(drtDescriptionSuitableForTestResult(request.url())), navigationTypeToString(action.type()),
            (node) ? qPrintable(QString::fromLatin1(" originating from ") + drtDescriptionSuitableForTestResult(node, 0)) : "");

        if (policyDelegatePermissive)
            result = PolicyAction::Use;
        else
            result = PolicyAction::Ignore;

        m_webFrame->pageAdapter->acceptNavigationRequest(m_webFrame, r, (int)action.type());
        function(result);
        return;
    }

    if (!m_webFrame->pageAdapter->acceptNavigationRequest(m_webFrame, r, (int)action.type())) {
        if (action.type() == NavigationType::FormSubmitted || action.type() == NavigationType::FormResubmitted)
            m_frame->loader().resetMultipleFormSubmissionProtection();

        if (action.type() == NavigationType::LinkClicked && r.url().hasFragment())
            m_frame->loader().activeDocumentLoader()->setLastCheckedRequest(ResourceRequest());

        function(PolicyAction::Ignore);
        return;
    }
    function(PolicyAction::Use);
}

void FrameLoaderClientQt::dispatchUnableToImplementPolicy(const WebCore::ResourceError&)
{
    notImplemented();
}

void FrameLoaderClientQt::startDownload(const WebCore::ResourceRequest& request, const String& /* suggestedName */, FromDownloadAttribute)
{
    if (!m_webFrame)
        return;

    QNetworkRequest r = request.toNetworkRequest(m_frame->loader().networkingContext());
    if (r.url().isValid())
        m_webFrame->pageAdapter->emitDownloadRequested(r);
}

RefPtr<LocalFrame> FrameLoaderClientQt::createFrame(const AtomString& name, HTMLFrameOwnerElement& ownerElement)
{
    if (!m_webFrame)
        return nullptr;

    QWebFrameData frameData(m_frame->page(), &ownerElement, String(name));

    QWebFrameAdapter* childWebFrame = m_webFrame->createChildFrame(&frameData);
    // The creation of the frame may have run arbitrary JavaScript that removed it from the page already.
    if (!childWebFrame->frame->page()) {
        QPointer<QObject> qWebFrame = childWebFrame->handle();
        frameData.frame.leakRef(); // QTFIXME: ???
        ASSERT_UNUSED(qWebFrame, !qWebFrame);
        return nullptr;
    }

    m_webFrame->pageAdapter->emitFrameCreated(childWebFrame);

    return frameData.frame;
}

ObjectContentType FrameLoaderClientQt::objectContentType(const URL& url, const String& mimeTypeIn)
{
    // qDebug()<<" ++++++++++++++++ url is "<<url.string()<<", mime = "<<mimeTypeIn;
    QFileInfo fi(url.path().toString());
    String extension = fi.suffix();
    if (MIMETypeRegistry::isApplicationPluginMIMEType(mimeTypeIn))
        return ObjectContentType::PlugIn;

    if (url.isEmpty() && !mimeTypeIn.length())
        return ObjectContentType::None;

    String mimeType = mimeTypeIn;
    if (!mimeType.length())
        mimeType = MIMETypeRegistry::mimeTypeForExtension(extension);

    if (!mimeType.length())
        return ObjectContentType::Frame;

    if (MIMETypeRegistry::isSupportedImageMIMEType(mimeType))
        return ObjectContentType::Image;
    
    if (MIMETypeRegistry::isSupportedNonImageMIMEType(mimeType))
        return ObjectContentType::Frame;

    if (url.protocol() == "about"_s)
        return ObjectContentType::Frame;

    return ObjectContentType::None;
}

static const CSSPropertyID qstyleSheetProperties[] = {
    CSSPropertyColor,
    CSSPropertyFontFamily,
    CSSPropertyFontSize,
    CSSPropertyFontStyle,
    CSSPropertyFontWeight
};

const unsigned numqStyleSheetProperties = sizeof(qstyleSheetProperties) / sizeof(qstyleSheetProperties[0]);

class QtPluginWidget: public Widget {
public:
    QtPluginWidget(QtPluginWidgetAdapter* w)
        : Widget(w->handle())
        , m_adapter(w)
    {
        setBindingObject(w->handle());
    }

    ~QtPluginWidget()
    {
        delete m_adapter;
    }

    inline QtPluginWidgetAdapter* widgetAdapter() const
    {
        return m_adapter;
    }

    void invalidateRect(const IntRect& r) override
    { 
        if (platformWidget())
            widgetAdapter()->update(r);
    }
    void frameRectsChanged() override
    {
        QtPluginWidgetAdapter* widget = widgetAdapter();
        if (!widget)
            return;
        QRect windowRect = convertToContainingWindow(IntRect(0, 0, frameRect().width(), frameRect().height()));

        ScrollView* parentScrollView = parent();
        QRect clipRect;
        if (parentScrollView) {
            ASSERT_WITH_SECURITY_IMPLICATION(parentScrollView->isLocalFrameView());
            clipRect = downcast<LocalFrameView>(parentScrollView)->windowClipRect();
            clipRect.translate(-windowRect.x(), -windowRect.y());
        }
        widget->setGeometryAndClip(windowRect, clipRect, isVisible());
    }

    void show() override
    {
        Widget::show();
        handleVisibility();
    }
    void hide() override
    {
        Widget::hide();
        if (platformWidget())
            widgetAdapter()->setVisible(false);
    }

private:
    QtPluginWidgetAdapter* m_adapter;

    void handleVisibility()
    {
        if (!isVisible())
            return;
        widgetAdapter()->setVisible(true);
    }
};


RefPtr<Widget> FrameLoaderClientQt::createPlugin(HTMLPlugInElement& element, const URL& url, const Vector<AtomString>& paramNames, const Vector<AtomString>& paramValues, const String& mimeType, bool loadManually)
{
    // qDebug()<<"------ Creating plugin in FrameLoaderClientQt::createPlugin for "<<url.string() << mimeType;
    // qDebug()<<"------\t url = "<<url.string();

    if (!m_webFrame)
        return nullptr;

    QStringList params;
    QStringList values;
    QString classid(element.getAttribute("classid"_s).string());

    for (unsigned i = 0; i < paramNames.size(); ++i) {
        params.append(String(paramNames[i]));
        if (paramNames[i] == "classid"_s)
            classid = String(paramValues[i]);
    }
    for (unsigned i = 0; i < paramValues.size(); ++i)
        values.append(String(paramValues[i]));

    QString urlStr(url.string());
    QUrl qurl = urlStr;

    QObject* pluginAdapter = 0;

    if (MIMETypeRegistry::isApplicationPluginMIMEType(mimeType)) {
        pluginAdapter = m_webFrame->pageAdapter->createPlugin(classid, qurl, params, values);
#ifndef QT_NO_STYLE_STYLESHEET
        QtPluginWidgetAdapter* widget = qobject_cast<QtPluginWidgetAdapter*>(pluginAdapter);
        if (widget && equalLettersIgnoringASCIICase(mimeType, "application/x-qt-styled-widget"_s)) {

            StringBuilder styleSheet;
            styleSheet.append(element.getAttribute("style"_s));
            if (!styleSheet.isEmpty())
                styleSheet.append(';');

            for (unsigned i = 0; i < numqStyleSheetProperties; ++i) {
                CSSPropertyID property = qstyleSheetProperties[i];

                styleSheet.append(nameString(property));
                styleSheet.append(':');
                styleSheet.append(QStyleHelpers::getComputedStyleProperty(element, property));
                styleSheet.append(';');
            }

            widget->setStyleSheet(styleSheet.toString());
        }
#endif // QT_NO_STYLE_STYLESHEET
    }

    if (!pluginAdapter) {
        QWebPluginFactory* factory = m_webFrame->pageAdapter->pluginFactory;
        if (factory)
            pluginAdapter = m_webFrame->pageAdapter->adapterForWidget(factory->create(mimeType, qurl, params, values));
    }
    if (pluginAdapter) {
        QtPluginWidgetAdapter* widget = qobject_cast<QtPluginWidgetAdapter*>(pluginAdapter);
        if (widget) {
            QObject* parentWidget = 0;
            if (m_webFrame->pageAdapter->client)
                parentWidget = m_webFrame->pageAdapter->client->pluginParent();
            if (parentWidget) // Don't reparent to nothing (i.e. keep whatever parent QWebPage::createPlugin() chose.
                widget->setWidgetParent(parentWidget);
            widget->setVisible(false);
            RefPtr<QtPluginWidget> w = adoptRef(new QtPluginWidget(widget));
            // Make sure it's invisible until properly placed into the layout.
            w->setFrameRect(IntRect(0, 0, 0, 0));
            return w;
        }

        // FIXME: Make things work for widgetless plugins as well.
        delete pluginAdapter;
    }

    return nullptr;
}

void FrameLoaderClientQt::redirectDataToPlugin(Widget& pluginWidget)
{
}

AtomString FrameLoaderClientQt::overrideMediaType() const
{
    if (m_webFrame && m_webFrame->pageAdapter && m_webFrame->pageAdapter->settings)
        return m_webFrame->pageAdapter->settings->cssMediaType();

    return AtomString();
}

Ref<FrameNetworkingContext> FrameLoaderClientQt::createNetworkingContext()
{
    QVariant value = m_webFrame->pageAdapter->handle()->property("_q_MIMESniffingDisabled");
    bool MIMESniffingDisabled = value.isValid() && value.toBool();

    return FrameNetworkingContextQt::create(m_frame, m_webFrame->handle(), !MIMESniffingDisabled);
}

QWebFrameAdapter* FrameLoaderClientQt::webFrame() const
{
    return m_webFrame;
}

void FrameLoaderClientQt::emitLoadStarted()
{
    if (m_shouldSuppressLoadStarted)
        return;

    m_isDisplayingErrorPage = false;

    m_webFrame->emitLoadStarted(m_isOriginatingLoad);
}

void FrameLoaderClientQt::emitLoadFinished(bool ok)
{
    if (ok && m_isDisplayingErrorPage)
        return;

    // Signal handlers can lead to a new load, that will use the member again.
    const bool wasOriginatingLoad = m_isOriginatingLoad;
    m_isOriginatingLoad = false;

    m_webFrame->emitLoadFinished(wasOriginatingLoad, ok);
}

void FrameLoaderClientQt::willReplaceMultipartContent()
{
    notImplemented();
}

void FrameLoaderClientQt::didReplaceMultipartContent()
{
    notImplemented();
}

void FrameLoaderClientQt::updateCachedDocumentLoader(WebCore::DocumentLoader &)
{
    notImplemented();
}

void FrameLoaderClientQt::prefetchDNS(const WTF::String &)
{
    notImplemented();
}

void FrameLoaderClientQt::sendH2Ping(const URL& url, CompletionHandler<void(Expected<Seconds, WebCore::ResourceError>&&)>&& completionHandler)
{
    ASSERT_NOT_REACHED();
    completionHandler(makeUnexpected(WebCore::internalError(url)));
}

RefPtr<HistoryItem> FrameLoaderClientQt::createHistoryItemTree(bool clipAtTarget, BackForwardItemIdentifier itemID) const
{
    // Create history item tree like the Mac implementation
    Ref coreMainFrame = m_frame->rootFrame();
    return coreMainFrame->loader().history().createItemTree(*m_frame, clipAtTarget, itemID);
}

void FrameLoaderClientQt::onResourceLoadStarted(const QUrl& url, const QString& type, const QtResourceRequestInfo& requestInfo, bool fromCache)
{
    Q_EMIT resourceLoadStarted(url, type, requestInfo, fromCache);
}

void FrameLoaderClientQt::onResourceLoadFinished(const QUrl& url, const QString& type, qint64 size, const QtResourceTimingInfo& timing, bool fromCache, bool success)
{
    Q_EMIT resourceLoadFinished(url, type, size, timing, fromCache, success);
}

}

#include "moc_FrameLoaderClientQt.cpp"
