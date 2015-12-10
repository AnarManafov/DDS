// Copyright 2014 GSI, Inc. All rights reserved.
//
//
//
#ifndef __DDS__BasicCmd__
#define __DDS__BasicCmd__

// MiscCommon
#include "def.h"
#include "INet.h"
// STD
#include <sstream>

namespace inet = MiscCommon::INet;

namespace dds
{
    namespace protocol_api
    {
        struct SAttachmentDataProvider
        {
            SAttachmentDataProvider(MiscCommon::BYTEVector_t* _data)
                : m_data(_data)
                , m_pos(0)
            {
            }

            SAttachmentDataProvider(const MiscCommon::BYTEVector_t& _data)
                : m_data(const_cast<MiscCommon::BYTEVector_t*>(&_data))
                , m_pos(0)
            {
            }

            template <typename T>
            SAttachmentDataProvider& get(T& _value)
            {
                inet::readData(&_value, m_data, &m_pos);
                return *this;
            }

            template <typename T>
            const SAttachmentDataProvider& put(const T& _value) const
            {
                inet::pushData(_value, m_data);
                return *this;
            }

          private:
            MiscCommon::BYTEVector_t* m_data;
            size_t m_pos;
        };

        template <class _Owner>
        struct SBasicCmd
        {
            void convertFromData(const MiscCommon::BYTEVector_t& _data)
            {
                _Owner* p = reinterpret_cast<_Owner*>(this);
                if (_data.size() < p->size())
                {
                    std::stringstream ss;
                    ss << "Protocol message data is too short, expected " << p->size() << " received " << _data.size();
                    throw std::runtime_error(ss.str());
                }
                p->_convertFromData(_data);
            }
            void convertToData(MiscCommon::BYTEVector_t* _data) const
            {
                const _Owner* p = reinterpret_cast<const _Owner*>(this);
                p->_convertToData(_data);
            }
        };
    }
}

#endif /* defined(__DDS__BasicCmd__) */
