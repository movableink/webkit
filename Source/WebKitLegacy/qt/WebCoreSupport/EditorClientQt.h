/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Apple Computer, Inc.
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

#ifndef EditorClientQt_h
#define EditorClientQt_h

#include "TextCheckerClientQt.h"
#include <WebCore/EditorClient.h>
#include <wtf/Forward.h>
#include <wtf/RefCounted.h>

class QWebPage;
class QWebPageAdapter;

namespace WebCore {

class EditorClientQt : public EditorClient {
    WTF_MAKE_FAST_ALLOCATED;
public:
    EditorClientQt(QWebPageAdapter*);
    
    bool shouldDeleteRange(const std::optional<SimpleRange>&) override;
    bool smartInsertDeleteEnabled() override;
#if USE(AUTOMATIC_TEXT_REPLACEMENT)
    void toggleSmartInsertDelete() override;
#endif
    bool isSelectTrailingWhitespaceEnabled() const override;
    bool isContinuousSpellCheckingEnabled() override;
    void toggleContinuousSpellChecking() override;
    bool isGrammarCheckingEnabled() override;
    void toggleGrammarChecking() override;
    int spellCheckerDocumentTag() override;

    bool shouldBeginEditing(const SimpleRange&) override;
    bool shouldEndEditing(const SimpleRange&) override;
    bool shouldInsertNode(Node&, const std::optional<SimpleRange>&, EditorInsertAction) override;
    bool shouldInsertText(const String&, const std::optional<SimpleRange>&, EditorInsertAction) override;
    bool shouldChangeSelectedRange(const std::optional<SimpleRange>& fromRange, const std::optional<SimpleRange>& toRange, Affinity, bool stillSelecting) override;

    bool shouldApplyStyle(const StyleProperties&, const std::optional<SimpleRange>&) override;

    bool shouldMoveRangeAfterDelete(const SimpleRange&, const SimpleRange&) override;

    void didBeginEditing() override;
    void respondToChangedContents() override;
    void respondToChangedSelection(LocalFrame*) override;
    void didEndEditing() override;
    void willWriteSelectionToPasteboard(const std::optional<SimpleRange>&) override;
    void didWriteSelectionToPasteboard() override;
    void getClientPasteboardData(const std::optional<SimpleRange>&, Vector<String>& pasteboardTypes, Vector<RefPtr<WebCore::SharedBuffer> >& pasteboardData) override;

    void registerUndoStep(UndoStep&) override;
    void registerRedoStep(UndoStep&) override;
    void clearUndoRedoOperations() override;

    bool canCopyCut(LocalFrame*, bool defaultValue) const override;
    bool canPaste(LocalFrame*, bool defaultValue) const override;
    bool canUndo() const override;
    bool canRedo() const override;
    
    void undo() override;
    void redo() override;

    void handleKeyboardEvent(KeyboardEvent&) override;
    void handleInputMethodKeydown(KeyboardEvent&) override;

    void textFieldDidBeginEditing(Element&) override;
    void textFieldDidEndEditing(Element&) override;
    void textDidChangeInTextField(Element&) override;
    bool doTextFieldCommandFromEvent(Element&, KeyboardEvent*) override;
    void textWillBeDeletedInTextField(Element&) override;
    void textDidChangeInTextArea(Element&) override;

    void updateSpellingUIWithGrammarString(const String&, const GrammarDetail&) override;
    void updateSpellingUIWithMisspelledWord(const String&) override;
    void showSpellingUI(bool show) override;
    bool spellingUIIsShowing() override;
    void willSetInputMethodState() override;
    void setInputMethodState(Element*) override;
    TextCheckerClient* textChecker() override { return &m_textCheckerClient; }

    bool supportsGlobalSelection() override;

    void didApplyStyle() override;
    void discardedComposition(LocalFrame *) override;
    void overflowScrollPositionChanged() override;
    void subFrameScrollPositionChanged() final { }

    void didEndUserTriggeredSelectionChanges() final;
    void updateEditorStateAfterLayoutIfEditabilityChanged() final;
    DOMPasteAccessResponse requestDOMPasteAccess(DOMPasteAccessCategory, const WTF::String& originIdentifier) final;
    void canceledComposition() final;
    void didUpdateComposition() final;
    bool performTwoStepDrop(DocumentFragment&, const SimpleRange& destination, bool isMove) final;

    bool isEditing() const;

    static bool dumpEditingCallbacks;
    static bool acceptsEditing;

private:
    TextCheckerClientQt m_textCheckerClient;
    QWebPageAdapter* m_page;
    bool m_editing;
    bool m_inUndoRedo; // our undo stack works differently - don't re-enter!
};

}

#endif

// vim: ts=4 sw=4 et
