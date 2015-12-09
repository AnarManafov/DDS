// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__TestCmd__
#define __DDS__TestCmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    struct STestCmd : public protocol_api::SBasicCmd<STestCmd>
    {
        STestCmd()
            : m_uint16(0)
            , m_uint32(0)
            , m_uint64(0)
            , m_string1()
            , m_string2()
            , m_string3()
            , m_string4()
            , m_vuint16()
            , m_vuint32()
            , m_vuint64()
            , m_vstring1()
            , m_vstring2()
        {
        }
        size_t size() const
        {
            size_t size = sizeof(m_uint16) + sizeof(m_uint32) + sizeof(m_uint64) +
                          (m_string1.size() + sizeof(uint32_t)) + (m_string2.size() + sizeof(uint32_t)) +
                          (m_string3.size() + sizeof(uint32_t)) + (m_string4.size() + sizeof(uint32_t)) +
                          (m_vuint16.size() * sizeof(uint16_t) + sizeof(uint32_t)) +
                          (m_vuint32.size() * sizeof(uint32_t) + sizeof(uint32_t)) +
                          (m_vuint64.size() * sizeof(uint64_t) + sizeof(uint32_t));
            for (const auto& v : m_vstring1)
            {
                size += v.size();
            }
            size += sizeof(uint32_t);

            for (const auto& v : m_vstring2)
            {
                size += v.size();
            }
            size += sizeof(uint32_t);

            return size;
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const STestCmd& val) const
        {
            return (m_uint16 == val.m_uint16 && m_uint32 == val.m_uint32 && m_uint64 == val.m_uint64 &&
                    m_string1 == val.m_string1 && m_string2 == val.m_string2 && m_string3 == val.m_string3 &&
                    m_string4 == val.m_string4 && m_vuint16 == val.m_vuint16 && m_vuint32 == val.m_vuint32 &&
                    m_vuint64 == val.m_vuint64 && m_vstring1 == val.m_vstring1 && m_vstring2 == val.m_vstring2);
        }

        uint16_t m_uint16;
        uint32_t m_uint32;
        uint64_t m_uint64;
        std::string m_string1;
        std::string m_string2;
        std::string m_string3;
        std::string m_string4;
        std::vector<uint16_t> m_vuint16;
        std::vector<uint32_t> m_vuint32;
        std::vector<uint64_t> m_vuint64;
        std::vector<std::string> m_vstring1;
        std::vector<std::string> m_vstring2;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const STestCmd& val)
    {
        return _stream << "TestCmd";
    }
    inline bool operator!=(const STestCmd& lhs, const STestCmd& rhs)
    {
        return !(lhs == rhs);
    }
}

#endif /* defined(__DDS__TestCmd__) */
