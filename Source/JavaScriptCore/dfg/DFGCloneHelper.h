/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if ENABLE(DFG_JIT)

#include "DFGBlockInsertionSet.h"
#include <wtf/HashMap.h>

namespace JSC { namespace DFG {

class CloneHelper {
public:
    CloneHelper(Graph& graph)
        : m_graph(graph)
        , m_blockInsertionSet(graph)
    {
        ASSERT(graph.m_form == ThreadedCPS || graph.m_form == LoadStore);
    }

    static bool isNodeCloneable(Graph&, HashSet<Node*>&, Node*);

    template<typename CustomizeSuccessors>
    BasicBlock* cloneBlock(BasicBlock* const, const CustomizeSuccessors&);
    BasicBlock* blockClone(BasicBlock*);

    void clear();
    void finalize();

private:
    Node* cloneNode(BasicBlock*, Node*);
    Node* cloneNodeImpl(BasicBlock*, Node*);

    Graph& m_graph;
    BlockInsertionSet m_blockInsertionSet;
    UncheckedKeyHashMap<Node*, Node*> m_nodeClones;
    UncheckedKeyHashMap<BasicBlock*, BasicBlock*> m_blockClones;
};

template<typename CustomizeSuccessors>
BasicBlock* CloneHelper::cloneBlock(BasicBlock* const block, const CustomizeSuccessors& customizeSuccessors)
{
    auto iter = m_blockClones.find(block);
    if (iter != m_blockClones.end())
        return iter->value;

    auto* clone = m_blockInsertionSet.insert(m_graph.numBlocks(), block->executionCount);
    clone->cfaHasVisited = false;
    clone->cfaDidFinish = false;
    m_blockClones.add(block, clone);

    // 1. Clone phis
    clone->phis.resize(block->phis.size());
    for (size_t i = 0; i < block->phis.size(); ++i) {
        Node* bodyPhi = block->phis[i];
        Node* phiClone = m_graph.addNode(bodyPhi->prediction(), bodyPhi->op(), bodyPhi->origin, OpInfo(bodyPhi->variableAccessData()));
        m_nodeClones.add(bodyPhi, phiClone);
        clone->phis[i] = phiClone;
    }

    // 2. Clone nodes.
    for (Node* node : *block)
        cloneNode(clone, node);

    // 3. Clone variables and tail and head.
    auto replaceOperands = [&](auto& nodes) ALWAYS_INLINE_LAMBDA {
        for (uint32_t i = 0; i < nodes.size(); ++i) {
            if (auto& node = nodes.at(i)) {
                // This is used to update variablesAtHead (CPS BasicBlock Phis) and variablesAtTail
                // (CPS BasicBlock Phis + locals). Thus, we must have a clone for each of them.
                node = m_nodeClones.get(node);
                ASSERT(node);
            }
        }
    };
    replaceOperands(clone->variablesAtTail = block->variablesAtTail);
    replaceOperands(clone->variablesAtHead = block->variablesAtHead);

    // 4. Clone successors. (predecessors will be fixed in the resetReachability of finalize)
    if (!customizeSuccessors(block, clone)) {
        ASSERT(clone->numSuccessors() == block->numSuccessors());
        for (uint32_t i = 0; i < clone->numSuccessors(); ++i) {
            auto& successor = clone->successor(i);
            ASSERT(successor == block->successor(i));
            successor = cloneBlock(successor, customizeSuccessors);
        }
    }

#if ASSERT_ENABLED
    clone->cloneSource = block;
#endif
    return clone;
}

#define FOR_EACH_NODE_CLONE_STATUS(CLONE_STATUS) \
    CLONE_STATUS(ArithAdd, Common) \
    CLONE_STATUS(ArithBitAnd, Common) \
    CLONE_STATUS(ArithBitLShift, Common) \
    CLONE_STATUS(ArithBitNot, Common) \
    CLONE_STATUS(ArithBitOr, Common) \
    CLONE_STATUS(ArithBitRShift, Common) \
    CLONE_STATUS(ArithBitXor, Common) \
    CLONE_STATUS(ArithDiv, Common) \
    CLONE_STATUS(ArithMod, Common) \
    CLONE_STATUS(ArithMul, Common) \
    CLONE_STATUS(ArithSub, Common) \
    CLONE_STATUS(ArrayifyToStructure, Common) \
    CLONE_STATUS(AssertNotEmpty, Common) \
    CLONE_STATUS(BitURShift, Common) \
    CLONE_STATUS(Branch, Special) \
    CLONE_STATUS(Check, Common) \
    CLONE_STATUS(CheckArray, Common) \
    CLONE_STATUS(CheckStructure, Common) \
    CLONE_STATUS(CheckVarargs, Common) \
    CLONE_STATUS(CompareEq, Common) \
    CLONE_STATUS(CompareGreater, Common) \
    CLONE_STATUS(CompareGreaterEq, Common) \
    CLONE_STATUS(CompareLess, Common) \
    CLONE_STATUS(CompareLessEq, Common) \
    CLONE_STATUS(CompareStrictEq, Common) \
    CLONE_STATUS(DoubleRep, Common) \
    CLONE_STATUS(ExitOK, Common) \
    CLONE_STATUS(FilterCallLinkStatus, Common) \
    CLONE_STATUS(Flush, Common) \
    CLONE_STATUS(GetButterfly, Common) \
    CLONE_STATUS(GetByVal, Common) \
    CLONE_STATUS(GetLocal, Common) \
    CLONE_STATUS(InvalidationPoint, Common) \
    CLONE_STATUS(JSConstant, Common) \
    CLONE_STATUS(Jump, Common) \
    CLONE_STATUS(LoopHint, Common) \
    CLONE_STATUS(MovHint, Common) \
    CLONE_STATUS(NewArrayWithConstantSize, Common) \
    CLONE_STATUS(NewArrayWithSize, Common) \
    CLONE_STATUS(PhantomLocal, Common) \
    CLONE_STATUS(Phi, PreCloned) \
    CLONE_STATUS(PurifyNaN, Common) \
    CLONE_STATUS(PutByVal, Common) \
    CLONE_STATUS(PutByValAlias, Common) \
    CLONE_STATUS(SetArgumentDefinitely, Common) \
    CLONE_STATUS(SetLocal, Common) \
    CLONE_STATUS(ValueRep, Common) \
    CLONE_STATUS(ValueToInt32, Common) \
    CLONE_STATUS(ZombieHint, Common)

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
