/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Instruction.h"
#include <wtf/Vector.h>

namespace JSC {

class InstructionStreamWriter;
struct Instruction;

using InstructionVector = Vector<uint8_t, 0, UnsafeVectorOverflow>;

class InstructionStream {
    WTF_MAKE_FAST_ALLOCATED;

    friend class InstructionStreamWriter;
    friend class CachedInstructionStream;
public:
    virtual ~InstructionStream() { }
    size_t sizeInBytes() const;

    using Offset = unsigned;

private:
    template<class InstructionStream>
    class BaseRef {
        WTF_MAKE_FAST_ALLOCATED;

        friend InstructionStream;

    public:
        BaseRef(const BaseRef<InstructionStream>& other)
            : m_stream(other.m_stream)
            ,  m_index(other.m_index)
        { }

        void operator=(const BaseRef<InstructionStream>& other)
        {
            m_stream = other.m_stream;
            m_index = other.m_index;
        }

        inline const Instruction* operator->() const { return unwrap(); }
        inline const Instruction* ptr() const { return unwrap(); }

        bool operator!=(const BaseRef<InstructionStream>& other) const
        {
            return &m_stream != &other.m_stream || m_index != other.m_index;
        }

        BaseRef next() const
        {
            return BaseRef { m_stream, m_index + ptr()->size() };
        }

        inline Offset offset() const
        {
            return m_index;
        }

        bool isValid() const
        {
            return m_index < m_stream.size();
        }

        BaseRef(InstructionStream& stream, size_t index)
            : m_stream(stream)
            , m_index(index)
        { }

    private:
        inline const Instruction* unwrap() const { return reinterpret_cast<const Instruction*>(&m_stream.instructions()[m_index]); }
        InstructionStream& m_stream;
        Offset m_index;
    };

public:
    using Ref = BaseRef<const InstructionStream>;

private:
    class iterator : public Ref {
        friend class InstructionStream;

    public:
        using Ref::Ref;

        Ref& operator*()
        {
            return *this;
        }

        iterator operator++()
        {
            m_index += ptr()->size();
            return *this;
        }
    };

public:
    inline iterator begin() const
    {
        return iterator { *this, 0 };
    }

    inline iterator end() const
    {
        return iterator { *this, size() };
    }

    inline const Ref at(Offset offset) const
    {
        ASSERT(offset < size());
        return Ref { *this, offset };
    }

    virtual size_t size() const = 0;
    virtual const uint8_t* instructions() const = 0;

    bool contains(Instruction *) const;

protected:
    InstructionStream();
};

class InstructionStreamReader : public InstructionStream {
    friend class CachedInstructionStream;

public:
    InstructionStreamReader(InstructionVector&&);

    const uint8_t* instructions() const override
    {
        return m_instructions;
    }

    size_t size() const override
    {
        return m_size;
    }

    ~InstructionStreamReader()
    {
        if (m_owned)
            fastFree(const_cast<uint8_t*>(m_instructions));
    }

private:
    InstructionStreamReader()
    {
    }

    bool m_owned { false };
    size_t m_size;
    const uint8_t* m_instructions;
};

class InstructionStreamWriter : public InstructionStream {
    friend class BytecodeRewriter;
public:
    InstructionStreamWriter()
    { }

    const uint8_t* instructions() const override
    {
        return m_stream.data();
    }

    size_t size() const override
    {
        return m_stream.size();
    }

    uint8_t* mutableInstructions()
    {
        return m_stream.data();
    }


    class MutableRef : public BaseRef<InstructionStreamWriter> {
        friend class InstructionStreamWriter;

    protected:
        using BaseRef<InstructionStreamWriter>::BaseRef;

    public:
        Ref freeze() const { return Ref { m_stream, m_index }; }
        inline Instruction* operator->() { return unwrap(); }
        inline Instruction* ptr() { return unwrap(); }
        inline operator Ref()
        {
            return Ref { m_stream, m_index };
        }

    private:
        inline Instruction* unwrap() { return reinterpret_cast<Instruction*>(&m_stream.mutableInstructions()[m_index]); }
    };

    inline MutableRef ref(Offset offset)
    {
        ASSERT(offset < m_stream.size());
        return MutableRef { *this, offset };
    }

    void seek(unsigned position)
    {
        ASSERT(position <= m_stream.size());
        m_position = position;
    }

    unsigned position()
    {
        return m_position;
    }

    void write(uint8_t byte)
    {
        ASSERT(!m_finalized);
        if (m_position < m_stream.size())
            m_stream[m_position++] = byte;
        else {
            m_stream.append(byte);
            m_position++;
        }
    }

    void write(uint16_t h)
    {
        ASSERT(!m_finalized);
        uint8_t bytes[2];
        std::memcpy(bytes, &h, sizeof(h));

        // Though not always obvious, we don't have to invert the order of the
        // bytes written here for CPU(BIG_ENDIAN). This is because the incoming
        // i value is already ordered in big endian on CPU(BIG_EDNDIAN) platforms.
        write(bytes[0]);
        write(bytes[1]);
    }

    void write(uint32_t i)
    {
        ASSERT(!m_finalized);
        uint8_t bytes[4];
        std::memcpy(bytes, &i, sizeof(i));

        // Though not always obvious, we don't have to invert the order of the
        // bytes written here for CPU(BIG_ENDIAN). This is because the incoming
        // i value is already ordered in big endian on CPU(BIG_EDNDIAN) platforms.
        write(bytes[0]);
        write(bytes[1]);
        write(bytes[2]);
        write(bytes[3]);
    }

    void rewind(MutableRef& ref)
    {
        ASSERT(ref.offset() < m_stream.size());
        m_stream.shrink(ref.offset());
        m_position = ref.offset();
    }

    std::unique_ptr<InstructionStream> finalize()
    {
        m_finalized = true;
        m_stream.shrinkToFit();
        return std::unique_ptr<InstructionStreamReader> { new InstructionStreamReader(WTFMove(m_stream)) };
    }

    MutableRef ref()
    {
        return MutableRef { *this, m_position };
    }

    void swap(InstructionStreamWriter& other)
    {
        std::swap(m_finalized, other.m_finalized);
        std::swap(m_position, other.m_position);
        m_stream.swap(other.m_stream);
    }

private:
    class iterator : public MutableRef {
        friend class InstructionStreamWriter;

    protected:
        using MutableRef::MutableRef;

    public:
        MutableRef& operator*()
        {
            return *this;
        }

        iterator operator++()
        {
            m_index += ptr()->size();
            return *this;
        }
    };

public:
    iterator begin()
    {
        return iterator { *this, 0 };
    }

    iterator end()
    {
        return iterator { *this, m_stream.size() };
    }

private:
    unsigned m_position { 0 };
    bool m_finalized { false };
    InstructionVector m_stream;
};


} // namespace JSC
