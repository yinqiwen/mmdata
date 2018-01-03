/*
 *Copyright (c) 2017-2018, yinqiwen <yinqiwen@gmail.com>
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

#ifndef SRC_COMMON_TYPES_HPP_
#define SRC_COMMON_TYPES_HPP_
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/list.hpp>
#include <boost/interprocess/containers/set.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include "allocator.hpp"

namespace mmdata
{
    typedef Allocator<char> CharAllocator;
    typedef boost::interprocess::offset_ptr<void> VoidPtr;
    typedef boost::interprocess::basic_string<char, std::char_traits<char>, Allocator<char> > SHMString;

    template<typename T>
    struct SHMVector
    {
            typedef boost::interprocess::vector<T, Allocator<T> > Type;
    };

    template<typename T>
    struct SHMDeque
    {
            typedef boost::interprocess::deque<T, Allocator<T> > Type;
    };

    template<typename T>
    struct SHMList
    {
            typedef boost::interprocess::list<T, Allocator<T> > Type;
    };

    template<typename T, typename Less= std::less<T> >
    struct SHMSet
    {
            typedef boost::interprocess::set<T, Less, Allocator<T> > Type;
    };

    template<typename KeyType, typename MappedType, typename Hash = boost::hash<KeyType>, typename Equal = std::equal_to<KeyType> >
    struct SHMHashMap
    {
            typedef std::pair<const KeyType, MappedType> ValueType;
            typedef Allocator<ValueType> ShmemAllocator;
            typedef boost::unordered_map<KeyType, MappedType, Hash, Equal,
                    ShmemAllocator> Type;
    };

    template<typename KeyType, typename MappedType, typename Less = std::less<KeyType> >
    struct SHMMap
    {
            typedef std::pair<const KeyType, MappedType> ValueType;
            typedef Allocator<ValueType> ShmemAllocator;
            typedef boost::interprocess::map<KeyType, MappedType, Less, ShmemAllocator> Type;
    };

    template<typename KeyType, typename Hash = boost::hash<KeyType>, typename Equal = std::equal_to<KeyType> >
    struct SHMHashSet
    {
            typedef boost::unordered_set<KeyType, Hash, Equal, Allocator<KeyType> > Type;
    };

    typedef SHMHashMap<SHMString, VoidPtr>::Type NamingTable;
}

#endif /* SRC_COMMON_TYPES_HPP_ */
