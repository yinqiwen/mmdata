/*
 *Copyright (c) 2017-2017, yinqiwen <yinqiwen@gmail.com>
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of Redis nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 *BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MMDATA_KCFG_HPP_
#define MMDATA_KCFG_HPP_
#include "common_types.hpp"
#include "rapidjson/document.h"
#include <boost/lexical_cast.hpp>

namespace kcfg
{
    template<typename T>
    struct SHMConstructor
    {
            T value;
            SHMConstructor(const mmdata::CharAllocator& alloc)
                    : value(alloc)
            {
            }
    };
    template<>
    struct SHMConstructor<float>
    {
            float value;
            SHMConstructor(const mmdata::CharAllocator& alloc)
                    : value(0)
            {
            }
    };
    template<>
    struct SHMConstructor<double>
    {
            double value;
            SHMConstructor(const mmdata::CharAllocator& alloc)
                    : value(0)
            {
            }
    };

    template<>
    struct SHMConstructor<int8_t>
    {
            int8_t value;
            SHMConstructor(const mmdata::CharAllocator& alloc)
                    : value(0)
            {
            }
    };
    template<>
    struct SHMConstructor<uint8_t>
    {
            uint8_t value;
            SHMConstructor(const mmdata::CharAllocator& alloc)
                    : value(0)
            {
            }
    };
    template<>
    struct SHMConstructor<int16_t>
    {
            int16_t value;
            SHMConstructor(const mmdata::CharAllocator& alloc)
                    : value(0)
            {
            }
    };
    template<>
    struct SHMConstructor<uint16_t>
    {
            uint16_t value;
            SHMConstructor(const mmdata::CharAllocator& alloc)
                    : value(0)
            {
            }
    };
    template<>
    struct SHMConstructor<uint32_t>
    {
            uint32_t value;
            SHMConstructor(const mmdata::CharAllocator& alloc)
                    : value(0)
            {
            }
    };
    template<>
    struct SHMConstructor<int32_t>
    {
            int32_t value;
            SHMConstructor(const mmdata::CharAllocator& alloc)
                    : value(0)
            {
            }
    };
    template<>
    struct SHMConstructor<int64_t>
    {
            int64_t value;
            SHMConstructor(const mmdata::CharAllocator& alloc)
                    : value(0)
            {
            }
    };
    template<>
    struct SHMConstructor<uint64_t>
    {
            uint64_t value;
            SHMConstructor(const mmdata::CharAllocator& alloc)
                    : value(0)
            {
            }
    };

    /*
     * the
     */
    const rapidjson::Value* getJsonFiledValue(const rapidjson::Value& json, const char* name);
    void addJsonMember(rapidjson::Value& json, rapidjson::Value::AllocatorType& allocator, const char* name,
            rapidjson::Value& json_value);

    template<typename T>
    inline bool Parse(const rapidjson::Value& json, const char* name, T& v);

    template<typename K, typename V>
    inline bool Parse(const rapidjson::Value& json, const char* name,
            boost::unordered_map<K, V, boost::hash<K>, std::equal_to<K>, mmdata::Allocator<std::pair<const K, V> > >& v)
    {
        typedef boost::unordered_map<K, V, boost::hash<K>, std::equal_to<K>, mmdata::Allocator<std::pair<const K, V> > > LocalMapType;
        const rapidjson::Value* val = getJsonFiledValue(json, name);
        if (NULL == val)
        {
            return false;
        }
        if (!val->IsObject())
        {
            return false;
        }
        v.reserve(val->MemberCount());
        rapidjson::Value::ConstMemberIterator it = val->MemberBegin();
        while (it != val->MemberEnd())
        {
            SHMConstructor<V> value(v.get_allocator());
            SHMConstructor<K> key(v.get_allocator());
            try
            {
                key.value = boost::lexical_cast<K>(it->name.GetString(), it->name.GetStringLength());
            }
            catch (boost::bad_lexical_cast& e)
            {
                return false;
            }

            if (Parse(it->value, "", value.value))
            {
                v.insert(typename LocalMapType::value_type(key.value, value.value));
            }
            else
            {
                v.clear();
                return false;
            }
            it++;
        }
        return true;
    }

    template<>
    inline bool Parse(const rapidjson::Value& json, const char* name, mmdata::SHMString& v)
    {
        const rapidjson::Value* val = getJsonFiledValue(json, name);
        if (NULL == val)
        {
            return false;
        }
        if (!val->IsString())
        {
            return false;
        }
        v.assign(val->GetString(), val->GetStringLength());
        return true;
    }

    template<typename V>
    inline bool Parse(const rapidjson::Value& json, const char* name,
            boost::unordered_map<mmdata::SHMString, V, boost::hash<mmdata::SHMString>, std::equal_to<mmdata::SHMString>,
                    mmdata::Allocator<std::pair<const mmdata::SHMString, V> > >& v)
    {
        typedef typename mmdata::SHMHashMap<mmdata::SHMString, V>::Type LocalMapType;
        const rapidjson::Value* val = getJsonFiledValue(json, name);
        if (NULL == val)
        {
            return false;
        }
        if (!val->IsObject())
        {
            return false;
        }
        v.reserve(val->MemberCount());
        rapidjson::Value::ConstMemberIterator it = val->MemberBegin();
        while (it != val->MemberEnd())
        {
            SHMConstructor<V> value(v.get_allocator());
            mmdata::SHMString key(v.get_allocator());
            key.assign(it->name.GetString(), it->name.GetStringLength());
            if (Parse(it->value, "", value.value))
            {
                v.insert(typename LocalMapType::value_type(key, value.value));
            }
            else
            {
                v.clear();
                return false;
            }
            it++;
        }
        return true;
    }

    template<typename K, typename V>
    inline bool Parse(const rapidjson::Value& json, const char* name,
            boost::container::map<K, V, std::less<K>, mmdata::Allocator<std::pair<const K, V> > >& v)
    {
        typedef typename boost::container::map<K, V, std::less<K>, mmdata::Allocator<std::pair<const K, V> > > LocalMapType;
        const rapidjson::Value* val = getJsonFiledValue(json, name);
        if (NULL == val)
        {
            return false;
        }
        if (!val->IsObject())
        {
            return false;
        }
        rapidjson::Value::ConstMemberIterator it = val->MemberBegin();
        while (it != val->MemberEnd())
        {
            SHMConstructor<K> key(v.get_allocator());
            try
            {
                key.value = boost::lexical_cast<K>(it->name.GetString(), it->name.GetStringLength());
            }
            catch (boost::bad_lexical_cast& e)
            {
                return false;
            }
            SHMConstructor<V> value(v.get_allocator());
            if (Parse(it->value, "", value.value))
            {
                v.insert(typename LocalMapType::value_type(key.value, value.value));
            }
            else
            {
                v.clear();
                return false;
            }
            it++;
        }
        return true;
    }

    template<typename T>
    inline bool Parse(const rapidjson::Value& json, const char* name,
            typename boost::container::vector<T, mmdata::Allocator<T> >& v)
    {
        const rapidjson::Value* val = getJsonFiledValue(json, name);
        if (NULL == val)
        {
            return false;
        }
        if (!val->IsArray())
        {
            return false;
        }
        v.reserve(val->Size());
        for (rapidjson::Value::ConstValueIterator ait = val->Begin(); ait != val->End(); ++ait)
        {
            SHMConstructor<T> value(v.get_allocator());
            if (Parse(*ait, "", value.value))
            {
                v.push_back(value.value);
            }
            else
            {
                v.clear();
                return false;
            }
        }
        v.shrink_to_fit();
        return true;
    }

    template<typename T>
    void Serialize(rapidjson::Value& json, rapidjson::Value::AllocatorType& allocator, const char* name, const T& v);
    template<>
    inline void Serialize(rapidjson::Value& json, rapidjson::Value::AllocatorType& allocator, const char* name,
            const mmdata::SHMString& v)
    {
        rapidjson::Value json_value(v.c_str(), v.size(), allocator);
        addJsonMember(json, allocator, name, json_value);
    }

    template<typename V>
    inline void Serialize(rapidjson::Value& json, rapidjson::Value::AllocatorType& allocator, const char* name,
            const boost::unordered_map<mmdata::SHMString, V, boost::hash<mmdata::SHMString>,
                    std::equal_to<mmdata::SHMString>, mmdata::Allocator<std::pair<const mmdata::SHMString, V> > >& v)
    {
        typedef boost::unordered_map<mmdata::SHMString, V, boost::hash<mmdata::SHMString>,
                std::equal_to<mmdata::SHMString>, mmdata::Allocator<std::pair<const mmdata::SHMString, V> > > LocalMapType;
        rapidjson::Value json_value(rapidjson::kObjectType);
        typename LocalMapType::const_iterator it = v.begin();
        while (it != v.end())
        {
            const V& vv = it->second;
            Serialize(json_value, allocator, it->first.c_str(), vv);
            it++;
        }
        addJsonMember(json, allocator, name, json_value);
    }

    template<typename K, typename V>
    inline void Serialize(rapidjson::Value& json, rapidjson::Value::AllocatorType& allocator, const char* name,
            const boost::container::map<K, V, std::less<K>, mmdata::Allocator<std::pair<const K, V> > >& v)
    {
        typedef boost::container::map<K, V, std::less<K>, mmdata::Allocator<std::pair<const K, V> > > LocalMapType;
        rapidjson::Value json_value(rapidjson::kObjectType);
        typename LocalMapType::const_iterator it = v.begin();
        while (it != v.end())
        {
            const K& kk = it->first;
            std::string json_key;
            try
            {
                json_key = boost::lexical_cast<std::string, K>(kk);
            }
            catch (boost::bad_lexical_cast& e)
            {
                return;
            }
            const V& vv = it->second;
            Serialize(json_value, allocator, json_key.c_str(), vv);
            it++;
        }
        addJsonMember(json, allocator, name, json_value);
    }

    template<typename K, typename V>
    inline void Serialize(rapidjson::Value& json, rapidjson::Value::AllocatorType& allocator, const char* name,
            const boost::unordered_map<K, V, boost::hash<K>, std::equal_to<K>, mmdata::Allocator<std::pair<const K, V> > >& v)
    {
        rapidjson::Value json_value(rapidjson::kObjectType);
        typename boost::unordered_map<K, V, boost::hash<K>, std::equal_to<K>, mmdata::Allocator<std::pair<const K, V> > >::const_iterator it =
                v.begin();
        while (it != v.end())
        {
            const K& kk = it->first;
            std::string json_key;
            try
            {
                json_key = boost::lexical_cast<std::string, K>(kk);
            }
            catch (boost::bad_lexical_cast& e)
            {
                return;
            }
            const V& vv = it->second;
            Serialize(json_value, allocator, json_key.c_str(), vv);
            it++;
        }
        addJsonMember(json, allocator, name, json_value);
    }
    template<typename K>
    inline void Serialize(rapidjson::Value& json, rapidjson::Value::AllocatorType& allocator, const char* name,
            const typename boost::container::vector<K, mmdata::Allocator<K> >& v)
    {
        rapidjson::Value json_value(rapidjson::kArrayType);
        for (size_t i = 0; i < v.size(); i++)
        {
            const K& vv = v[i];
            Serialize(json_value, allocator, "", vv);
        }
        addJsonMember(json, allocator, name, json_value);
    }
}

#endif /* KCFG_HPP_ */
