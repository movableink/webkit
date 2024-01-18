/*
 * Copyright (C) 2015 The Qt Company Ltd.
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
 *
 */

#include "QWebFrameAdapter.h"

#include "ChromeClientQt.h"
#include "FrameLoaderClientQt.h"
#include "QWebFrameData.h"
#include "QWebPageAdapter.h"
#include "qwebsecurityorigin.h"
#include "qwebsecurityorigin_p.h"
#include "qwebsettings.h"

#include <JavaScriptCore/APICast.h>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QPainter>
#include <WebCore/Chrome.h>
#include <WebCore/DocumentLoader.h>
#include <WebCore/Editing.h>
#include <WebCore/EventHandler.h>
#include <WebCore/FocusController.h>
#include <WebCore/FrameLoadRequest.h>
#include <WebCore/GraphicsContextQt.h>
#include <WebCore/HTMLCollection.h>
#include <WebCore/HTMLFormElement.h>
#include <WebCore/HTMLMetaElement.h>
#include <WebCore/HTTPParsers.h>
#include <WebCore/HitTestResult.h>
#include <WebCore/InspectorController.h>
#include <WebCore/IntRect.h>
#include <WebCore/IntSize.h>
#include <WebCore/JSLocalDOMWindow.h>
#include <WebCore/NavigationScheduler.h>
#include <WebCore/NetworkingContext.h>
#include <WebCore/ResourceRequest.h>
#include <WebCore/ScriptController.h>
#include <WebCore/ScriptSourceCode.h>
#include <WebCore/SubstituteData.h>
#include <WebCore/markup.h>
#include <WebCore/qt_runtime.h>
#include <wtf/URL.h>

#if USE(TILED_BACKING_STORE)
#include "TiledBackingStore.h"
#endif

#if ENABLE(QT_GESTURE_EVENTS)
#include  <WebCore/PlatformGestureEvent.h>
#include "WebEventConversion.h"
#endif

#if USE(TEXTURE_MAPPER)
#include "TextureMapperLayerClientQt.h"
#endif

using namespace WebCore;

static inline ResourceRequestCachePolicy cacheLoadControlToCachePolicy(uint cacheLoadControl)
{
    switch (cacheLoadControl) {
    case QNetworkRequest::AlwaysNetwork:
        return ResourceRequestCachePolicy::ReloadIgnoringCacheData;
    case QNetworkRequest::PreferCache:
        return ResourceRequestCachePolicy::ReturnCacheDataElseLoad;
    case QNetworkRequest::AlwaysCache:
        return ResourceRequestCachePolicy::ReturnCacheDataDontLoad;
    default:
        break;
    }
    return ResourceRequestCachePolicy::UseProtocolCachePolicy;
}

QWebFrameAdapter::QWebFrameAdapter()
    : pageAdapter(0)
    , horizontalScrollBarPolicy(Qt::ScrollBarAsNeeded)
    , verticalScrollBarPolicy(Qt::ScrollBarAsNeeded)
    , frame(0)
    , frameLoaderClient(0)
{
}

QWebFrameAdapter::~QWebFrameAdapter()
{
    if (frameLoaderClient)
        frameLoaderClient->m_webFrame = 0;
}

void QWebFrameAdapter::load(const QNetworkRequest& req, QNetworkAccessManager::Operation operation, const QByteArray& body)
{
    if (frame->tree().parent())
        pageAdapter->insideOpenCall = true;

    QUrl url = ensureAbsoluteUrl(req.url());

    WebCore::ResourceRequest request(url);

    switch (operation) {
    case QNetworkAccessManager::HeadOperation:
        request.setHTTPMethod("HEAD"_s);
        break;
    case QNetworkAccessManager::GetOperation:
        request.setHTTPMethod("GET"_s);
        break;
    case QNetworkAccessManager::PutOperation:
        request.setHTTPMethod("PUT"_s);
        break;
    case QNetworkAccessManager::PostOperation:
        request.setHTTPMethod("POST"_s);
        break;
    case QNetworkAccessManager::DeleteOperation:
        request.setHTTPMethod("DELETE"_s);
        break;
    case QNetworkAccessManager::CustomOperation:
        request.setHTTPMethod(String::fromLatin1(req.attribute(QNetworkRequest::CustomVerbAttribute).toByteArray().constData()));
        break;
    case QNetworkAccessManager::UnknownOperation:
        // eh?
        break;
    }

    QVariant cacheLoad = req.attribute(QNetworkRequest::CacheLoadControlAttribute);
    if (cacheLoad.isValid()) {
        bool ok;
        uint cacheLoadValue = cacheLoad.toUInt(&ok);
        if (ok)
            request.setCachePolicy(cacheLoadControlToCachePolicy(cacheLoadValue));
    }

    QList<QByteArray> httpHeaders = req.rawHeaderList();
    for (int i = 0; i < httpHeaders.size(); ++i) {
        const QByteArray &headerName = httpHeaders.at(i);
        request.addHTTPHeaderField(QByteArrayView(headerName), QByteArrayView(req.rawHeader(headerName)));
    }

    if (!body.isEmpty())
        request.setHTTPBody(WebCore::FormData::create(body.constData(), body.size()));

    frame->loader().load(WebCore::FrameLoadRequest(*frame, request));

    if (frame->tree().parent())
        pageAdapter->insideOpenCall = false;
}

bool QWebFrameAdapter::hasView() const
{
    return frame && frame->view();
}

#if ENABLE(QT_GESTURE_EVENTS)
void QWebFrameAdapter::handleGestureEvent(QGestureEventFacade* gestureEvent)
{
    ASSERT(frame && frame->view());
    switch (gestureEvent->type) {
    case Qt::TapGesture:
        frame->eventHandler().handleGestureEvent(convertGesture(gestureEvent));
        break;
    case Qt::TapAndHoldGesture:
        frame->eventHandler().sendContextMenuEventForGesture(convertGesture(gestureEvent));
        break;
    default:
        ASSERT_NOT_REACHED();
    }
}
#endif

QVariant QWebFrameAdapter::evaluateJavaScript(const QString &scriptSource)
{
    ScriptController& scriptController = frame->script();
    QVariant rc;
    int distance = 0;
    JSC::JSValue value = scriptController.executeScriptIgnoringException(scriptSource, JSC::SourceTaintedOrigin::Untainted);
    JSC::JSGlobalObject* lexicalGlobalObject = scriptController.globalObject(mainThreadNormalWorld())->globalObject();
    JSValueRef* ignoredException = 0;
    JSC::JSLockHolder lock(lexicalGlobalObject);
    JSValueRef valueRef = toRef(lexicalGlobalObject, value);
    rc = JSC::Bindings::convertValueToQVariant(toRef(lexicalGlobalObject), valueRef, QMetaType::Void, &distance, ignoredException);
    return rc;
}

void QWebFrameAdapter::addToJavaScriptWindowObject(const QString& name, QObject* object, ValueOwnership ownership)
{
    if (!pageAdapter->settings->testAttribute(QWebSettings::JavascriptEnabled))
        return;
    JSC::Bindings::QtInstance::ValueOwnership valueOwnership = static_cast<JSC::Bindings::QtInstance::ValueOwnership>(ownership);
    JSLocalDOMWindow* window = toJSLocalDOMWindow(frame, mainThreadNormalWorld());
    JSC::Bindings::RootObject* root;
    if (valueOwnership == JSC::Bindings::QtInstance::QtOwnership)
        root = frame->script().cacheableBindingRootObject();
    else
        root = frame->script().bindingRootObject();

    if (!window) {
        qDebug() << "Warning: couldn't get window object";
        return;
    }
    if (!root) {
        qDebug() << "Warning: couldn't get root object";
        return;
    }

    JSC::JSGlobalObject* lexicalGlobalObject = window->globalObject();
    JSC::JSLockHolder lock(lexicalGlobalObject);

    JSC::JSObject* runtimeObject = JSC::Bindings::QtInstance::getQtInstance(object, root, valueOwnership)->createRuntimeObject(lexicalGlobalObject);

    JSC::PutPropertySlot slot(window);
    window->methodTable()->put(window, lexicalGlobalObject, JSC::Identifier::fromString(lexicalGlobalObject->vm(), reinterpret_cast_ptr<const UChar*>(name.constData()), name.length()), runtimeObject, slot);
}

QString QWebFrameAdapter::toHtml() const
{
    if (!frame->document())
        return QString();
    return serializeFragment(*frame->document(), SerializedNodes::SubtreeIncludingNode);
}

QString QWebFrameAdapter::toPlainText() const
{
    if (frame->view() && frame->view()->layoutContext().isLayoutPending())
        frame->view()->layoutContext().layout();

    Element* documentElement = frame->document()->documentElement();
    if (documentElement)
        return documentElement->innerText();
    return QString();
}

void QWebFrameAdapter::setContent(const QByteArray &data, const QString &mimeType, const QUrl &baseUrl)
{
    URL kurl(baseUrl);
    WebCore::ResourceRequest request(kurl);
    WTF::RefPtr<WebCore::SharedBuffer> buffer = WebCore::SharedBuffer::create(data.constData(), data.length());
    QString actualMimeType;
    WTF::StringView encoding;
    if (mimeType.isEmpty())
        actualMimeType = QLatin1String("text/html");
    else {
        actualMimeType = extractMIMETypeFromMediaType(mimeType);
        encoding = extractCharsetFromMediaType(String(mimeType));
    }
    WebCore::ResourceResponse response(URL(), actualMimeType, buffer->size(), encoding.toStringWithoutCopying());
    // FIXME: visibility?
    WebCore::SubstituteData substituteData(WTFMove(buffer), URL(), response, SubstituteData::SessionHistoryVisibility::Hidden);
    frame->loader().load(WebCore::FrameLoadRequest(*frame, request, substituteData));
}

void QWebFrameAdapter::setHtml(const QString &html, const QUrl &baseUrl)
{
    URL kurl(baseUrl);
    WebCore::ResourceRequest request(kurl);
    const QByteArray utf8 = html.toUtf8();
    WTF::RefPtr<WebCore::SharedBuffer> data = WebCore::SharedBuffer::create(utf8.constData(), utf8.length());
    WebCore::ResourceResponse response(URL(), "text/html"_s, data->size(), "utf-8"_s);
    // FIXME: visibility?
    WebCore::SubstituteData substituteData(WTFMove(data), URL(), response, SubstituteData::SessionHistoryVisibility::Hidden);
    frame->loader().load(WebCore::FrameLoadRequest(*frame, request, substituteData));
}

QMultiMap<QString, QString> QWebFrameAdapter::metaData() const
{
    if (!frame->document())
        return {};

    QMultiMap<QString, QString> map;
    Document* doc = frame->document();
    auto list = doc->getElementsByTagName("meta"_s);
    unsigned len = list->length();
    for (unsigned i = 0; i < len; i++) {
        HTMLMetaElement* meta = static_cast<HTMLMetaElement*>(list->item(i));
        map.insert(meta->name().string(), meta->content().string());
    }
    return map;
}

QPoint QWebFrameAdapter::scrollPosition() const
{
    if (!frame || !frame->view())
        return QPoint();
    return QPoint(frame->view()->scrollOffset());
}

QRect QWebFrameAdapter::frameRect() const
{
    if (!frame || !frame->view())
        return QRect();
    return QRect(frame->view()->frameRect());
}

QSize QWebFrameAdapter::contentsSize() const
{
    FrameView* view = frame->view();
    if (!view)
        return QSize();
    return QSize(view->contentsWidth(), view->contentsHeight());
}

void QWebFrameAdapter::setZoomFactor(qreal factor)
{
    if (pageAdapter->settings->testAttribute(QWebSettings::ZoomTextOnly))
        frame->setTextZoomFactor(factor);
    else
        frame->setPageZoomFactor(factor);
}

void QWebFrameAdapter::setTextSizeMultiplier(qreal factor)
{
    pageAdapter->settings->setAttribute(QWebSettings::ZoomTextOnly, true);
    frame->setPageAndTextZoomFactors(1, factor);
}

qreal QWebFrameAdapter::zoomFactor() const
{
    return pageAdapter->settings->testAttribute(QWebSettings::ZoomTextOnly) ? frame->textZoomFactor() : frame->pageZoomFactor();
}

void QWebFrameAdapter::init(QWebPageAdapter* pageAdapter)
{
    QWebFrameData frameData(pageAdapter->page.get());
    init(pageAdapter, &frameData);
}

void QWebFrameAdapter::init(QWebPageAdapter* pageAdapter, QWebFrameData* frameData)
{
    this->pageAdapter = pageAdapter;
    frame = frameData->frame.get();
    frameLoaderClient = frameData->frameLoaderClient;
    frameLoaderClient->setFrame(this, frame);

    frame->init();
}

QWebFrameAdapter* QWebFrameAdapter::kit(const Frame* frame)
{
    auto* localFrame = dynamicDowncast<LocalFrame>(frame);
    return static_cast<const FrameLoaderClientQt&>(localFrame->loader().client()).webFrame();
}

QUrl QWebFrameAdapter::ensureAbsoluteUrl(const QUrl& url)
{
    // FIXME: it would be nice if we could avoid doing this.
    // Convert to URL and back to preserve the old behavior (e.g. fixup of single slash in 'http:/')
    QUrl validatedUrl = URL(url);

    if (!validatedUrl.isValid() || !validatedUrl.isRelative())
        return validatedUrl;

    // This contains the URL with absolute path but without
    // the query and the fragment part.
    QUrl baseUrl = QUrl::fromLocalFile(QFileInfo(validatedUrl.toLocalFile()).absoluteFilePath());

    // The path is removed so the query and the fragment parts are there.
    QString pathRemoved = validatedUrl.toString(QUrl::RemovePath);
    QUrl toResolve(pathRemoved);

    return baseUrl.resolved(toResolve);
}

QWebHitTestResultPrivate* QWebFrameAdapter::hitTestContent(const QPoint& pos) const
{
    if (!frame->view() || !frame->contentRenderer())
        return 0;

    constexpr OptionSet<HitTestRequest::Type> hitType { HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active, HitTestRequest::Type::IgnoreClipping, HitTestRequest::Type::DisallowUserAgentShadowContent };
    HitTestResult result = frame->eventHandler().hitTestResultAtPoint(frame->view()->windowToContents(pos), hitType);

    if (result.scrollbar())
        return 0;
    return new QWebHitTestResultPrivate(result);
}

QWebElement QWebFrameAdapter::documentElement() const
{
    Document* doc = frame->document();
    if (!doc)
        return QWebElement();
    return QWebElement(doc->documentElement());
}

QWebElement QWebFrameAdapter::ownerElement() const
{
    return QWebElement(frame->ownerElement());
}

QString QWebFrameAdapter::title() const
{
    if (frame->document())
        return frame->loader().documentLoader()->title().string;
    return QString();
}

void QWebFrameAdapter::clearCoreFrame()
{
    DocumentLoader* documentLoader = frame->loader().activeDocumentLoader();
    Q_ASSERT(documentLoader);
    documentLoader->writer().begin();
    documentLoader->writer().end();
}


static inline bool isCoreFrameClear(WebCore::LocalFrame* frame)
{
    return frame->document()->url().isEmpty();
}

QUrl QWebFrameAdapter::baseUrl() const
{
    if (isCoreFrameClear(frame))
        return url.resolved(QUrl());
    return frame->document()->baseURL();
}

void QWebFrameAdapter::renderCompositedLayers(WebCore::GraphicsContext& context, const WebCore::IntRect& clip)
{
#if USE(TEXTURE_MAPPER)
    WebCore::Page* page = frame->page();
    if (!page)
        return;
    if (TextureMapperLayerClientQt* client = static_cast<ChromeClientQt&>(page->chrome().client()).m_textureMapperLayerClient.get())
        client->renderCompositedLayers(context, clip);
#endif
}

// FIXME: this might not be necessary, but for the sake of not breaking things, we'll use that for now.
QUrl QWebFrameAdapter::coreFrameUrl() const
{
    return frame->document()->url();
}

QUrl QWebFrameAdapter::lastRequestedUrl() const
{
    return frameLoaderClient->lastRequestedUrl();
}

QWebSecurityOrigin QWebFrameAdapter::securityOrigin() const
{
    QWebSecurityOriginPrivate* priv = new QWebSecurityOriginPrivate(frame->document()->securityOrigin().data());
    return QWebSecurityOrigin(priv);
}

QString QWebFrameAdapter::uniqueName() const
{
    return frame->tree().uniqueName().string();
}

// This code is loosely copied from ChromeClientGtk.cpp.
static bool shouldCoalesceRects(const QRegion& clip)
{
    const int rectThreshold = 10;
    const float wastedSpaceThreshold = 0.75f;

    bool useUnionedRect = (clip.rectCount() <= 1) || (clip.rectCount() > rectThreshold);
    if (!useUnionedRect) {
        // Attempt to guess whether or not we should use the unioned rect or the individual rects.
        // We do this by computing the percentage of "wasted space" in the union. If that wasted space
        // is too large, then we will do individual rect painting instead.
        float unionPixels = (clip.boundingRect().width() * clip.boundingRect().height());
        float singlePixels = 0;
        for (const QRect& rect : clip)
            singlePixels += rect.width() * rect.height();

        float wastedSpace = 1 - (singlePixels / unionPixels);
        if (wastedSpace <= wastedSpaceThreshold)
            return true;
    }

    return useUnionedRect;
}

void QWebFrameAdapter::renderRelativeCoords(QPainter* painter, int layers, const QRegion& clip)
{
    GraphicsContextQt context(painter);
    if (context.paintingDisabled() && !context.invalidatingControlTints())
        return;

    if (!frame->view() || !frame->contentRenderer())
        return;

    if (clip.rectCount() == 0)
        return;

    WebCore::LocalFrameView* view = frame->view();
    view->updateLayoutAndStyleIfNeededRecursive();

    if (layers & ContentsLayer) {
        QRect clipBoundingRect = clip.boundingRect();
        for (const QRect &clipRect : shouldCoalesceRects(clip) ? clipBoundingRect : clip) {
            QRect rect = clipRect.intersected(view->frameRect());

            context.save();
            painter->setClipRect(clipRect, Qt::IntersectClip);

            int x = view->x();
            int y = view->y();

            int scrollX = view->scrollX();
            int scrollY = view->scrollY();

            context.translate(x, y);
            rect.translate(-x, -y);
            context.translate(-scrollX, -scrollY);
            rect.translate(scrollX, scrollY);
            context.clip(view->visibleContentRect());

            view->paintContents(context, rect);

            context.restore();
        }
        renderCompositedLayers(context, IntRect(clipBoundingRect));
    }
    renderFrameExtras(context, layers, clip);

    if (frame->page()->inspectorController().highlightedNode()) {
        context.save();
        frame->page()->inspectorController().drawHighlight(context);
        context.restore();
    }
}

void QWebFrameAdapter::renderFrameExtras(GraphicsContext& context, int layers, const QRegion& clip)
{
    if (!(layers & (PanIconLayer | ScrollBarLayer)))
        return;
    QPainter* painter = context.platformContext()->painter();
    WebCore::LocalFrameView* view = frame->view();
    for (const QRect& clipRect : clip) {
        QRect intersectedRect = clipRect.intersected(view->frameRect());

        painter->save();
        painter->setClipRect(clipRect, Qt::IntersectClip);

        int x = view->x();
        int y = view->y();

        if (layers & ScrollBarLayer
            && !view->scrollbarsSuppressed()
            && (view->horizontalScrollbar() || view->verticalScrollbar())) {

            QRect rect = intersectedRect;
            context.translate(x, y);
            rect.translate(-x, -y);
            view->paintScrollbars(context, rect);
            context.translate(-x, -y);
        }

#if ENABLE(PAN_SCROLLING)
        if (layers & PanIconLayer)
            view->paintPanScrollIcon(context);
#endif

        painter->restore();
    }
}

#if USE(TILED_BACKING_STORE)
void QWebFrameAdapter::setTiledBackingStoreFrozen(bool frozen)
{
    WebCore::TiledBackingStore* backingStore = frame->tiledBackingStore();
    if (!backingStore)
        return;
    backingStore->setContentsFrozen(frozen);
}

bool QWebFrameAdapter::tiledBackingStoreFrozen() const
{
    WebCore::TiledBackingStore* backingStore = frame->tiledBackingStore();
    if (!backingStore)
        return false;
    return backingStore->contentsFrozen();
}

void QWebFrameAdapter::setTiledBackingStoreContentsScale(float scale)
{
    WebCore::TiledBackingStore* backingStore = frame->tiledBackingStore();
    if (!backingStore)
        return;
    backingStore->setContentsScale(scale);
}

bool QWebFrameAdapter::renderFromTiledBackingStore(QPainter* painter, const QRegion& clip)
{
    // No tiled backing store? Tell the caller to fall back to regular rendering.
    if (!frame->tiledBackingStore())
        return false;

    // FIXME: We should set the backing store viewport earlier than in paint
    frame->tiledBackingStore()->coverWithTilesIfNeeded();

    if (!frame->view() || !frame->contentRenderer())
        return true;

    QVector<QRect> vector = clip.rects();
    if (vector.isEmpty())
        return true;

    GraphicsContext context(painter);

    WebCore::LocalFrameView* view = frame->view();

    int scrollX = view->scrollX();
    int scrollY = view->scrollY();
    QRect frameRect = view->frameRect();

    for (int i = 0; i < vector.size(); ++i) {
        const QRect& clipRect = vector.at(i);

        context.save();
        QRect rect = clipRect.intersected(frameRect);
        context.translate(-scrollX, -scrollY);
        rect.translate(scrollX, scrollY);
        context.clip(rect);

        frame->tiledBackingStore()->paint(&context, rect);

        context.restore();
    }

    renderCompositedLayers(&context, IntRect(clip.boundingRect()));
    renderFrameExtras(&context, QWebFrameAdapter::ScrollBarLayer | QWebFrameAdapter::PanIconLayer, clip);
    return true;
}
#endif

void QWebFrameAdapter::_q_orientationChanged()
{
#if ENABLE(ORIENTATION_EVENTS) && HAVE(QTSENSORS)
    int orientation;

    switch (m_orientation.reading()->orientation()) {
    case QOrientationReading::TopUp:
        orientation = 0;
        break;
    case QOrientationReading::TopDown:
        orientation = 180;
        break;
    case QOrientationReading::LeftUp:
        orientation = -90;
        break;
    case QOrientationReading::RightUp:
        orientation = 90;
        break;
    case QOrientationReading::FaceUp:
    case QOrientationReading::FaceDown:
        // WebCore unable to handle it
    default:
        return;
    }
    frame->sendOrientationChangeEvent(orientation);
#endif
}

QList<QObject*> QWebFrameAdapter::childFrames() const
{
    QList<QObject*> originatingObjects;
    if (frame) {
        for (Frame* child = frame->tree().firstChild(); child; child = child->tree().nextSibling()) {
            auto* childLocalFrame = dynamicDowncast<WebCore::LocalFrame>(child);
            if (childLocalFrame) {
                FrameLoader& loader = childLocalFrame->loader();
                originatingObjects.append(loader.networkingContext()->originatingObject());
            }
        }
    }
    return originatingObjects;
}

bool QWebFrameAdapter::hasFocus() const
{
    Frame* ff = frame->page()->focusController().focusedFrame();
    return ff && QWebFrameAdapter::kit(ff) == this;
}

void QWebFrameAdapter::setFocus()
{
    frame->page()->focusController().setFocusedFrame(frame);
}

void QWebFrameAdapter::setScrollBarPolicy(Qt::Orientation orientation, Qt::ScrollBarPolicy policy)
{
    Q_ASSERT((int)ScrollbarMode::Auto == (int)Qt::ScrollBarAsNeeded);
    Q_ASSERT((int)ScrollbarMode::AlwaysOff == (int)Qt::ScrollBarAlwaysOff);
    Q_ASSERT((int)ScrollbarMode::AlwaysOn == (int)Qt::ScrollBarAlwaysOn);

    if (orientation == Qt::Horizontal) {
        horizontalScrollBarPolicy = policy;
        if (frame->view()) {
            frame->view()->setHorizontalScrollbarMode((ScrollbarMode)policy, policy != Qt::ScrollBarAsNeeded /* lock */);
            frame->view()->updateCanHaveScrollbars();
        }
    } else {
        verticalScrollBarPolicy = policy;
        if (frame->view()) {
            frame->view()->setVerticalScrollbarMode((ScrollbarMode)policy, policy != Qt::ScrollBarAsNeeded /* lock */);
            frame->view()->updateCanHaveScrollbars();
        }
    }
}

void QWebFrameAdapter::scrollToAnchor(const QString &anchor)
{
    LocalFrameView* view = frame->view();
    if (view)
        view->scrollToFragment(URL(QUrl(anchor)));
}

void QWebFrameAdapter::scrollBy(int dx, int dy)
{
    if (!frame->view())
        return;
    frame->view()->scrollBy(IntSize(dx, dy));
}

void QWebFrameAdapter::setScrollBarValue(Qt::Orientation orientation, int value)
{
    Scrollbar* sb;
    sb = (orientation == Qt::Horizontal) ? horizontalScrollBar() : verticalScrollBar();
    if (sb) {
        if (value < 0)
            value = 0;
        else if (value > scrollBarMaximum(orientation))
            value = scrollBarMaximum(orientation);
        sb->scrollableArea().scrollToOffsetWithoutAnimation(orientation == Qt::Horizontal ? ScrollbarOrientation::Horizontal : ScrollbarOrientation::Vertical, value);
    }
}

int QWebFrameAdapter::scrollBarValue(Qt::Orientation orientation) const
{
    Scrollbar* sb;
    sb = (orientation == Qt::Horizontal) ? horizontalScrollBar() : verticalScrollBar();
    if (sb)
        return sb->value();
    return 0;
}

int QWebFrameAdapter::scrollBarMaximum(Qt::Orientation orientation) const
{
    Scrollbar* sb;
    sb = (orientation == Qt::Horizontal) ? horizontalScrollBar() : verticalScrollBar();
    if (sb)
        return sb->maximum();
    return 0;
}

QRect QWebFrameAdapter::scrollBarGeometry(Qt::Orientation orientation) const
{
    Scrollbar* sb;
    sb = (orientation == Qt::Horizontal) ? horizontalScrollBar() : verticalScrollBar();
    if (sb)
        return sb->frameRect();
    return QRect();
}

WebCore::Scrollbar* QWebFrameAdapter::horizontalScrollBar() const
{
    if (!frame->view())
        return 0;
    return frame->view()->horizontalScrollbar();
}

WebCore::Scrollbar* QWebFrameAdapter::verticalScrollBar() const
{
    if (!frame->view())
        return 0;
    return frame->view()->verticalScrollbar();
}


void QWebFrameAdapter::updateBackgroundRecursively(const QColor& backgroundColor)
{
    ASSERT(frame->view());
    frame->view()->updateBackgroundRecursively(Color(backgroundColor));
}

void QWebFrameAdapter::cancelLoad()
{
    frame->navigationScheduler().cancel();
}

// ========== QWebHitTestResultPrivate implementation ===========

QWebHitTestResultPrivate::QWebHitTestResultPrivate(const WebCore::HitTestResult &hitTest)
    : isContentEditable(false)
    , isContentSelected(false)
    , isScrollBar(false)
    , webCoreFrame(0)
{
    if (!hitTest.innerNode())
        return;
    pos = hitTest.roundedPointInInnerNodeFrame();
    WebCore::TextDirection dir;
    title = hitTest.title(dir);
    linkText = hitTest.textContent();
    linkUrl = hitTest.absoluteLinkURL();
    linkTitle = hitTest.titleDisplayString();
    alternateText = hitTest.altDisplayString();
    imageUrl = hitTest.absoluteImageURL();
    mediaUrl = hitTest.absoluteMediaURL();
    innerNode = hitTest.innerNode();
    innerNode->ref();
    innerNonSharedNode = hitTest.innerNonSharedNode();
    innerNonSharedNode->ref();
    boundingRect = (innerNonSharedNode && innerNonSharedNode->renderer())? innerNonSharedNode->renderer()->absoluteBoundingBoxRect() : IntRect();
    WebCore::Image *img = hitTest.image();
    if (img)
        pixmap = QPixmap::fromImage(img->nativeImageForCurrentFrame()->platformImage());

    WebCore::LocalFrame *wframe = hitTest.targetFrame();
    if (wframe) {
        linkTargetFrame = QWebFrameAdapter::kit(wframe)->handle();
        webCoreFrame = wframe;
        webCoreFrame->ref();
    }
    linkElement = QWebElement(hitTest.URLElement());

    isContentEditable = hitTest.isContentEditable();
    isContentSelected = hitTest.isSelected();
    isScrollBar = hitTest.scrollbar();

    WebCore::LocalFrame *innerNodeFrame = hitTest.innerNodeFrame();
    if (innerNodeFrame)
        frame = QWebFrameAdapter::kit(innerNodeFrame)->handle();

    enclosingBlock = QWebElement(WebCore::enclosingBlock(innerNode).get());
}

QWebHitTestResultPrivate::QWebHitTestResultPrivate(const QWebHitTestResultPrivate& other)
{
    innerNode = 0;
    innerNonSharedNode = 0;
    webCoreFrame = 0;
    *this = other;
}

QWebHitTestResultPrivate& QWebHitTestResultPrivate::operator=(const QWebHitTestResultPrivate& other)
{
    pos = other.pos;
    boundingRect = other.boundingRect;
    enclosingBlock = other.enclosingBlock;
    title = other.title;
    linkText = other.linkText;
    linkUrl = other.linkUrl;
    linkTitle = other.linkTitle;
    linkTargetFrame = other.linkTargetFrame;
    linkElement = other.linkElement;
    alternateText = other.alternateText;
    imageUrl = other.imageUrl;
    mediaUrl = other.mediaUrl;
    pixmap = other.pixmap;
    isContentEditable = other.isContentEditable;
    isContentSelected = other.isContentSelected;
    isScrollBar = other.isScrollBar;
    frame = other.frame;

    if (innerNode)
        innerNode->deref();
    innerNode = other.innerNode;
    if (innerNode)
        innerNode->ref();

    if (innerNonSharedNode)
        innerNonSharedNode->deref();
    innerNonSharedNode = other.innerNonSharedNode;
    if (innerNonSharedNode)
        innerNonSharedNode->ref();

    if (webCoreFrame)
        webCoreFrame->deref();
    webCoreFrame = other.webCoreFrame;
    if (webCoreFrame)
        webCoreFrame->ref();

    return *this;
}

QWebHitTestResultPrivate::~QWebHitTestResultPrivate()
{
    if (innerNode)
        innerNode->deref();
    if (innerNonSharedNode)
        innerNonSharedNode->deref();
    if (webCoreFrame)
        webCoreFrame->deref();
}

QWebElement QWebHitTestResultPrivate::elementForInnerNode() const
{
    // Uses the similar logic as HitTestResult::innerElement().
    for (Node* node = innerNonSharedNode; node; node = node->parentNode()) {
        if (node->isElementNode())
            return QWebElement(downcast<Element>(node));
    }

    return QWebElement();
}

// ======================================================

QSize QWebFrameAdapter::customLayoutSize() const
{
    ASSERT(&pageAdapter->mainFrameAdapter() == this);
    FrameView* view = frame->view();
    ASSERT(view);
    if (view->useFixedLayout())
        return view->fixedLayoutSize();
    return QSize();
}

void QWebFrameAdapter::setCustomLayoutSize(const QSize& size)
{
    ASSERT(&pageAdapter->mainFrameAdapter() == this);
    LocalFrameView* view = frame->view();
    ASSERT(view);

    if (size.isValid()) {
        view->setUseFixedLayout(true);
        view->setFixedLayoutSize(size);
    } else if (view->useFixedLayout())
        view->setUseFixedLayout(false);

    view->layoutContext().layout();
}

void QWebFrameAdapter::setFixedVisibleContentRect(const QRect& rect)
{
    ASSERT(&pageAdapter->mainFrameAdapter() == this);
    LocalFrameView* view = frame->view();
    ASSERT(view);
    view->setFixedVisibleContentRect(rect);
}

void QWebFrameAdapter::setViewportSize(const QSize& size)
{
    ASSERT(&pageAdapter->mainFrameAdapter() == this);
    LocalFrameView* view = frame->view();
    ASSERT(view);
    view->resize(size);
    if (view->needsLayout())
        view->layoutContext().layout();
    view->adjustViewSize();
}


void QWebFrameAdapter::setPaintsEntireContents(bool resizesToContents)
{
    ASSERT(&pageAdapter->mainFrameAdapter() == this);
    ASSERT(frame->view());
    frame->view()->setPaintsEntireContents(resizesToContents);
}

void QWebFrameAdapter::setDelegatesScrolling(bool resizesToContents)
{
    ASSERT(&pageAdapter->mainFrameAdapter() == this);
    ASSERT(frame->view());
    frame->view()->setDelegatedScrollingMode(resizesToContents ? DelegatedScrollingMode::DelegatedToNativeScrollView : DelegatedScrollingMode::NotDelegated);
}
