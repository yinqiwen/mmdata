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

#ifndef SRC_MMDATA_HPP_
#define SRC_MMDATA_HPP_
#include "common_types.hpp"
#include "mmap.hpp"
#include "allocator.hpp"
#include <new>
#include <iosfwd>

namespace mmdata
{
    class MMData;
    typedef void SHMItemDestructor(MMData&, void*);
    typedef void* SHMItemConstructor(MMData&);

    template<typename T>
    struct SHMDataDestructor
    {
            static void Free(MMData& mdata, void* p);
    };

    struct TypeRefItem
    {
            VoidPtr val;
            SHMString type;
            uint32_t ref;
            SHMItemDestructor* destroy;
            TypeRefItem(CharAllocator& alloc, uint32_t c = 0)
                    : type(alloc), ref(c), destroy(NULL)
            {
                if (0 == ref)
                {
                    ref = 1;
                }
            }
            template<typename T>
            T* Get()
            {
                return (T*) (val.get());
            }
            uint32_t DecRef()
            {
                if (0 == ref)
                {
                    return 0;
                }
                ref--;
                return ref;
            }
    };
    typedef boost::interprocess::offset_ptr<TypeRefItem> TypeRefItemPtr;

    class MMData
    {
        protected:
            Meta* meta_;
            CharAllocator allocator_;
            std::string err;

            template<typename T>
            T* LoadRootObject()
            {
                //Meta* meta = (Meta*) buf;
                return (T*) (meta_->root_object);
            }
            NamingTable* GetNamingTable()
            {
                //Meta* meta = (Meta*) allocator_.get_space().space.get();
                return (NamingTable*) (meta_->naming_table);
            }
            int CreateMeta(void* buf, int64_t memsize, bool create_allocator);
        public:
            MMData();
            int OpenWrite(void* buf, int64_t size, bool create_allocator= true)
            {
                if (NULL == meta_ && 0 != CreateMeta(buf, size, create_allocator))
                {
                    return -1;
                }
                return 0;
            }
            int OpenRead(const void* buf)
            {
                if (NULL == meta_)
                {
                    meta_ = (Meta*) buf;
                }
                return 0;
            }
            const std::string& GetLastErr() const
            {
                return err;
            }
            CharAllocator& GetAllocator()
            {
                return allocator_;
            }

            TypeRefItemPtr NewTypeRefItem(const std::string& type, SHMItemConstructor* cons, SHMItemDestructor* destroy,
                    uint32_t ref = 1);
            template<typename T>
            TypeRefItemPtr NewTypeRefItem(const std::string& type, uint32_t ref = 1)
            {
                TypeRefItemPtr ptr = New<TypeRefItem>();
                ptr->destroy = SHMDataDestructor<T>::Free;
                ptr->val = New<T>();
                ptr->ref = ref;
                ptr->type.assign(type.c_str(), type.size());
                return ptr;
            }
            template<typename T, typename Arg>
            TypeRefItemPtr NewTypeRefItemWithArg(const std::string& type, const Arg& arg, uint32_t ref = 1)
            {
                TypeRefItemPtr ptr = New<TypeRefItem>();
                ptr->destroy = SHMDataDestructor<T>::Free;
                ptr->val = New<T, Arg>(arg);
                ptr->ref = ref;
                ptr->type.assign(type.c_str(), type.size());
                return ptr;
            }
            void Delete(TypeRefItemPtr p);

            template<typename T>
            T* New()
            {
                Allocator<T> alloc(allocator_);
                T* v = alloc.allocate(1);
                ::new ((void*) (v)) T(allocator_);
                return v;
            }
            template<typename T, typename Arg>
            T* New(const Arg& arg)
            {
                Allocator<T> alloc(allocator_);
                T* v = alloc.allocate(1);
                ::new ((void*) (v)) T(arg);
                return v;
            }
            template<typename T>
            void Delete(T* p)
            {
                Allocator<T> alloc(allocator_);
                alloc.destroy(p);
                alloc.deallocate(p);
            }

            template<typename T>
            T* NewNamingObject(const std::string& str)
            {
                if (NULL == meta_)
                {
                    return NULL;
                }
                SHMString key(str.data(), str.size(), allocator_);
                VoidPtr nil;
                std::pair<NamingTable::iterator, bool> ret = GetNamingTable()->insert(
                        NamingTable::value_type(key, nil));
                if (!ret.second)
                {
                    //duplicate
                    return NULL;
                }
                T* v = New<T>();
                ret.first->second = v;
                return v;
            }
            template<typename T>
            T* GetNamingObject(const std::string& str, bool create_ifnotexist = false)
            {
                if (NULL == meta_)
                {
                    printf("###NULL META\n");
                    return NULL;
                }
                SHMString key(str.data(), str.size(), allocator_);
                NamingTable::const_iterator found = GetNamingTable()->find(key);
                if (found != GetNamingTable()->end())
                {
                    return (T*) (found->second.get());
                }
                if (!create_ifnotexist)
                {
                    return NULL;
                }
                return NewNamingObject<T>(str);

            }
            template<typename T>
            bool DeleteNamingObject(const std::string& str, T* p)
            {
                SHMString key(str.data(), str.size(), allocator_);
                const NamingTable* namings = GetNamingTable();
                NamingTable::iterator found = namings->find(key);
                if (found == GetNamingTable()->end())
                {
                    return false;
                }
                GetNamingTable()->erase(found);
                Delete<T>(p);
                return true;
            }

            template<typename T>
            T* LoadRootWriteObject()
            {
                if (NULL == meta_)
                {
                    return NULL;
                }
                T* root = LoadRootObject<T>();
                ::new ((void*) (root)) T(allocator_);
                return root;
            }

            template<typename T>
            const T* LoadRootReadObject()
            {
                if (NULL == meta_)
                {
                    return NULL;
                }
                return (const T*) LoadRootObject<T>();
            }
            template<typename T>
            T* GetRootWriteObject()
            {
                return (T*) (meta_->root_object);
            }

            template<typename T>
            const T* GetRootReadObject(const void* mem)
            {
                return (const T*) (meta_->root_object);
            }

    };

    class MMFileData: public MMData
    {
        private:
            std::string file_;
            int64_t file_size_;
            MMapBuf databuf_;
            bool readonly_;
        public:
            MMFileData(const std::string& f, int64_t size = -1)
                    : file_(f), file_size_(size), readonly_(false)
            {
            }
            template<typename T>
            T* LoadRootWriteObject()
            {
                if (databuf_.buf != NULL)
                {
                    return LoadRootObject<T>();
                }
                if (file_size_ <= 0)
                {
                    err = "MMap File size too small or negative.";
                    return NULL;
                }
                if (databuf_.OpenWrite(file_, file_size_, true) < 0)
                {
                    err = databuf_.GetLastErr();
                    return NULL;
                }
                readonly_ = false;
                int ret = OpenWrite(databuf_.buf, databuf_.size);
                if (0 != ret)
                {
                    return NULL;
                }
                return MMData::LoadRootWriteObject<T>();
            }
            template<typename T>
            const T* LoadRootReadObject()
            {
                if (databuf_.buf != NULL)
                {
                    return MMData::LoadRootReadObject<T>();
                }
                if (databuf_.OpenRead(file_) < 0)
                {
                    err = databuf_.GetLastErr();
                    return NULL;
                }
                readonly_ = true;
                int ret = OpenRead(databuf_.buf);
                if (0 != ret)
                {
                    return NULL;
                }
                return MMData::LoadRootReadObject<T>();
            }
            int64_t ShrinkToFit();
    };

    struct ShmOpenOptions
    {
            int64_t size;
            bool recreate;
            bool readonly;
            ShmOpenOptions():size(0),recreate(false),readonly(false)
            {
            }
    };

    class ShmData: public MMData
    {
        private:
            int shmid;
            void* shm_buf;
            std::string error_reason;
            bool dettach;
        public:
            ShmData(bool auto_dettach = true);
            const std::string& LastError() const
            {
                return error_reason;
            }
            int GetId() const
            {
                return shmid;
            }
            int OpenShm(const std::string& f, const ShmOpenOptions& options);
            ~ShmData();
    };
    class HeapMMData: public MMData
    {
        private:
            char meta_buf[sizeof(Meta)];
        public:
            HeapMMData();
            int OpenWrite();
            int OpenRead();

            ~HeapMMData();
    };

    template<typename T>
    void SHMDataDestructor<T>::Free(MMData& mdata, void* p)
    {
        mdata.Delete((T*) p);
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& out, const typename boost::container::vector<T, mmdata::Allocator<T> >& v)
    {
        out << '[';
        if (!v.empty())
        {
            std::copy(v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
        }
        out << "]";
        return out;
    }

    template<typename K, typename V>
    std::ostream& operator<<(std::ostream& out,
            const boost::unordered_map<K, V, boost::hash<K>, std::equal_to<K>, mmdata::Allocator<std::pair<const K, V> > >& v)
    {
        out << '{';
        typename boost::unordered_map<K, V, boost::hash<K>, std::equal_to<K>, mmdata::Allocator<std::pair<const K, V> > >::const_iterator it =
                v.begin();
        while (it != v.end())
        {
            out << it->first << "->" << it->second << ",";
            it++;
        }
        out << "}";
        return out;
    }
    template<typename K, typename V>
    std::ostream& operator<<(std::ostream& out,
            const boost::container::map<K, V, std::less<K>, mmdata::Allocator<std::pair<const K, V> > >& v)
    {
        out << '{';
        typename boost::container::map<K, V, std::less<K>, mmdata::Allocator<std::pair<const K, V> > >::const_iterator it =
                v.begin();
        while (it != v.end())
        {
            out << it->first << "->" << it->second << ",";
            it++;
        }
        out << "}";
        return out;
    }
}

#endif /* SRC_MMDATA_HPP_ */
