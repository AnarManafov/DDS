// Copyright 2015 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__Uint32Cmd__
#define __DDS__Uint32Cmd__

// DDS
#include "BasicCmd.h"

namespace dds
{
    struct SUint32Cmd : public SBasicCmd<SUint32Cmd>
    {
        SUint32Cmd()
            : m_nUint32(0)
        {
        }
        SUint32Cmd(uint32_t _number)
            : m_nUint32(_number)
        {
        }
        void normalizeToLocal() const;
        void normalizeToRemote() const;
        size_t size() const
        {
            return sizeof(m_nUint32);
        }
        void _convertFromData(const MiscCommon::BYTEVector_t& _data);
        void _convertToData(MiscCommon::BYTEVector_t* _data) const;
        bool operator==(const SUint32Cmd& val) const
        {
            return (m_nUint32 == val.m_nUint32);
        }

        mutable uint32_t m_nUint32;
    };
    inline std::ostream& operator<<(std::ostream& _stream, const SUint32Cmd& val)
    {
        return _stream << "a number attached to the command: " << val.m_nUint32;
    }
    inline bool operator!=(const SUint32Cmd& lhs, const SUint32Cmd& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif /* defined(__DDS__Uint32Cmd__) */
