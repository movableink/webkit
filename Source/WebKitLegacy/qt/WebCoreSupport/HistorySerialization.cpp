/*
 * Copyright (C) 2005, 2006, 2008, 2011, 2014 Apple Inc. All rights reserved.
 * Copyright (C) 2016 Konstantin Tokarev <annulen@yandex.ru>
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

#include "HistorySerialization.h"
#include "LegacyHistoryItemClient.h"

#include <WebCore/FormData.h>
#include <WebCore/KeyedDecoderQt.h>
#include <WebCore/KeyedEncoderQt.h>

namespace WebCore {

enum class FormDataType {
    Data,
    EncodedFile,
    EncodedBlob,
};

static void encodeElement(KeyedEncoder& encoder, const FormDataElement& element)
{
    switchOn(element.data,
        [&] (const Vector<uint8_t>& bytes) {
            encoder.encodeEnum("type"_s, FormDataType::Data);
            encoder.encodeBytes("data"_s, bytes.data(), bytes.size());
        }, [&] (const FormDataElement::EncodedFileData& fileData) {
            encoder.encodeEnum("type"_s, FormDataType::EncodedFile);
            encoder.encodeString("filename"_s, fileData.filename);
            encoder.encodeInt64("fileStart"_s, fileData.fileStart);
            encoder.encodeInt64("fileLength"_s, fileData.fileLength);
            if (fileData.expectedFileModificationTime)
                encoder.encodeDouble("expectedFileModificationTime"_s, fileData.expectedFileModificationTime.value().secondsSinceEpoch().seconds());
        }, [&] (const FormDataElement::EncodedBlobData& blobData) {
            encoder.encodeEnum("type"_s, FormDataType::EncodedBlob);
            encoder.encodeString("url"_s, blobData.url.string());
        });
}

static bool decodeElement(KeyedDecoder& decoder, FormDataElement& element)
{
    FormDataType type;
    if (!decoder.decodeEnum("type"_s, type, [](FormDataType type) {
        switch (type) {
        case FormDataType::Data:
        case FormDataType::EncodedFile:
        case FormDataType::EncodedBlob:
            return true;
        }

        return false;
    }))
        return false;

    switch (type) {
    case FormDataType::Data: {
        Vector<uint8_t> bytes;
        if (!decoder.decodeBytes("data"_s, bytes))
            return false;
        element.data = WTFMove(bytes);
        break;
    }
    case FormDataType::EncodedFile: {
        FormDataElement::EncodedFileData fileData;
        if (!decoder.decodeString("filename"_s, fileData.filename))
            return false;

        int64_t fileStart;
        if (!decoder.decodeInt64("fileStart"_s, fileStart))
            return false;
        if (fileStart < 0)
            return false;

        int64_t fileLength;
        if (!decoder.decodeInt64("fileLength"_s, fileLength))
            return false;
        if (fileLength != BlobDataItem::toEndOfFile && fileLength < fileStart)
            return false;

        fileData.fileStart = fileStart;
        fileData.fileLength = fileLength;

        double expectedFileModificationTime; // Optional field
        if (decoder.decodeDouble("expectedFileModificationTime"_s, expectedFileModificationTime))
            fileData.expectedFileModificationTime = WallTime::fromRawSeconds(expectedFileModificationTime);

        element.data = WTFMove(fileData);
        break;
    }

    case FormDataType::EncodedBlob: {
        String blobURLString;
        if (!decoder.decodeString("url"_s, blobURLString))
            return false;

        element.data = FormDataElement::EncodedBlobData { URL(URL(), blobURLString) };
        break;
    }
    }

    return true;
}

void encodeFormData(const FormData& formData, KeyedEncoder& encoder)
{
    encoder.encodeBool("alwaysStream"_s, formData.alwaysStream());
    encoder.encodeBytes("boundary"_s, reinterpret_cast<const uint8_t*>(formData.boundary().data()), formData.boundary().size());

    encoder.encodeObjects("elements"_s, formData.elements().begin(), formData.elements().end(), [](KeyedEncoder& encoder, const FormDataElement& element) {
        encodeElement(encoder, element);
    });

    encoder.encodeInt64("identifier"_s, formData.identifier());
}

RefPtr<FormData> decodeFormData(KeyedDecoder& decoder)
{
    RefPtr<FormData> data = FormData::create();

    bool alwaysStream;
    if (!decoder.decodeBool("alwaysStream"_s, alwaysStream))
        return nullptr;
    data->setAlwaysStream(alwaysStream);

    if (!decoder.decodeBytes("boundary"_s, const_cast<Vector<char>&>(data->boundary())))
        return nullptr;

    if (!decoder.decodeObjects("elements"_s, const_cast<Vector<FormDataElement>&>(data->elements()), [](KeyedDecoder& decoder, FormDataElement& element) {
        return decodeElement(decoder, element);
    }))
        return nullptr;

    int64_t identifier;
    if (!decoder.decodeInt64("identifier"_s, identifier))
        return nullptr;

    data->setIdentifier(identifier);
    return data;
}

static void encodeBackForwardTreeNode(KeyedEncoder& encoder, const HistoryItem& item)
{
    encoder.encodeObjects("children"_s, item.children().begin(), item.children().end(),
        encodeBackForwardTreeNode);

    encoder.encodeString("originalURLString"_s, item.originalURLString());
    encoder.encodeString("urlString"_s, item.urlString());

    encoder.encodeInt64("documentSequenceNumber"_s, item.documentSequenceNumber());

    encoder.encodeObjects("documentState"_s, item.documentState().begin(), item.documentState().end(), [](KeyedEncoder& encoder, const String& string) {
        encoder.encodeString("string"_s, string);
    });

    encoder.encodeString("formContentType"_s, item.formContentType());

    encoder.encodeConditionalObject("formData"_s, const_cast<HistoryItem&>(item).formData(), [](KeyedEncoder& encoder, const FormData& formData) {
        encodeFormData(formData, encoder);
    });

    encoder.encodeInt64("itemSequenceNumber"_s, item.itemSequenceNumber());

    encoder.encodeString("referrer"_s, item.referrer());

    encoder.encodeObject("scrollPosition"_s, item.scrollPosition(), [](KeyedEncoder& encoder, const IntPoint& scrollPosition) {
        encoder.encodeInt32("x"_s, scrollPosition.x());
        encoder.encodeInt32("y"_s, scrollPosition.y());
    });

    encoder.encodeFloat("pageScaleFactor"_s, item.pageScaleFactor());

    encoder.encodeConditionalObject("stateObject"_s, item.stateObject(), [](KeyedEncoder& encoder, const SerializedScriptValue& stateObject) {
        encoder.encodeBytes("data"_s, stateObject.wireBytes().data(), stateObject.wireBytes().size());
    });

    encoder.encodeString("target"_s, item.target());
}

void encodeBackForwardTree(KeyedEncoderQt& encoder, const HistoryItem& item)
{
    encoder.encodeString("title"_s, item.title());
    encodeBackForwardTreeNode(encoder, item);
    if (item.userData().isValid())
        encoder.encodeVariant("userData"_s, item.userData());
}

template<typename F>
bool decodeString(KeyedDecoder& decoder, const String& key, F&& consumer)
{
    String tmp;
    if (!decoder.decodeString(key, tmp))
        return false;
    consumer(WTFMove(tmp));
    return true;
}

static bool decodeBackForwardTreeNode(KeyedDecoder& decoder, HistoryItem& item)
{
    if (!decodeString(decoder, "urlString"_s, [&item](String&& str) {
        item.setURLString(str);
    }))
        return false;

    decodeString(decoder, "originalURLString"_s, [&item](String&& str) {
        item.setOriginalURLString(str);
    });

    Vector<int> ignore;
    decoder.decodeObjects("children"_s, ignore, [&item](KeyedDecoder& decoder, int&) {
        Ref<HistoryItem> element = HistoryItem::create(LegacyHistoryItemClient::singleton());
        if (decodeBackForwardTreeNode(decoder, element)) {
            item.addChildItem(WTFMove(element));
            return true;
        }
        return false;
    });

    int64_t documentSequenceNumber;
    if (decoder.decodeInt64("documentSequenceNumber"_s, documentSequenceNumber))
        item.setDocumentSequenceNumber(documentSequenceNumber);

    Vector<AtomString> documentState;
    decoder.decodeObjects("documentState"_s, documentState, [](KeyedDecoder& decoder, AtomString& string) -> bool {
        String s { string };
        return decoder.decodeString("string"_s, s);
    });
    item.setDocumentState(documentState);

    decodeString(decoder, "formContentType"_s, [&item](String&& str) {
        item.setFormContentType(str);
    });

    RefPtr<FormData> formData;
    if (decoder.decodeObject("formData"_s, formData, [](KeyedDecoder& decoder, RefPtr<FormData>& formData) {
        formData = decodeFormData(decoder);
        return formData != nullptr;
    }))
        item.setFormData(WTFMove(formData));

    int64_t itemSequenceNumber;
    if (decoder.decodeInt64("itemSequenceNumber"_s, itemSequenceNumber))
        item.setItemSequenceNumber(itemSequenceNumber);

    decodeString(decoder, "referrer"_s, [&item](String&& str) {
        item.setReferrer(str);
    });

    int ignore2;
    decoder.decodeObject("scrollPosition"_s, ignore2, [&item](KeyedDecoder& decoder, int&) -> bool {
        int x, y;
        if (!decoder.decodeInt32("x"_s, x))
            return false;
        if (!decoder.decodeInt32("y"_s, y))
            return false;
        item.setScrollPosition(IntPoint(x, y));
        return true;
    });

    float pageScaleFactor;
    if (decoder.decodeFloat("pageScaleFactor"_s, pageScaleFactor))
        item.setPageScaleFactor(pageScaleFactor);

    RefPtr<SerializedScriptValue> stateObject;
    decoder.decodeConditionalObject("stateObject"_s, stateObject, [](KeyedDecoder& decoder, RefPtr<SerializedScriptValue>& stateObject) -> bool {
        Vector<uint8_t> bytes;
        if (decoder.decodeBytes("data"_s, bytes)) {
            stateObject = SerializedScriptValue::createFromWireBytes(WTFMove(bytes));
            return true;
        }
        return false;
    });

    decodeString(decoder, "target"_s, [&item](String&& str) {
        item.setTarget(AtomString(str));
    });

    return true;
}

bool decodeBackForwardTree(KeyedDecoderQt& decoder, HistoryItem& item)
{
    if (!decodeString(decoder, "title"_s, [&item](String&& str) {
        item.setTitle(str);
    }))
        return false;

    QVariant userData;
    if (decoder.decodeVariant("userData"_s, userData))
        item.setUserData(userData);

    return decodeBackForwardTreeNode(decoder, item);
}

}
