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

namespace mmdata
{
    class MMData
    {
        protected:
            CharAllocator allocator_;
            std::string err;
            int CreateAllocator(void* buf, int64_t memsize);
            template<typename T>
            T* LoadRootObject(const void* buf)
            {
                Meta* meta = (Meta*) buf;
                return (T*) (meta->root_object);
            }
        public:
            MMData();
            const std::string& GetLastErr() const
            {
                return err;
            }
            CharAllocator& GetAllocator()
            {
                return allocator_;
            }
            template<typename T>
            T* LoadRootWriteObject(void* buf, int64_t size)
            {
                if (0 != CreateAllocator(buf, size))
                {
                    return NULL;
                }
                T* root = LoadRootObject<T>(buf);
                ::new ((void*) (root)) T(allocator_);
                return root;
            }

            template<typename T>
            const T* LoadRootReadObject(const void* mem)
            {
                return (const T*) LoadRootObject<T>(mem);
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
                    return LoadRootObject<T>(databuf_.buf);
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
                return MMData::LoadRootWriteObject<T>(databuf_.buf, databuf_.size);
            }
            template<typename T>
            const T* LoadRootReadObject()
            {
                if (databuf_.buf != NULL)
                {
                    return MMData::LoadRootReadObject<T>(databuf_.buf);
                }
                if (databuf_.OpenRead(file_) < 0)
                {
                    err = databuf_.GetLastErr();
                    return NULL;
                }
                readonly_ = true;
                return MMData::LoadRootReadObject<T>(databuf_.buf);
            }
            int64_t ShrinkWriteFile();
    };
}

#endif /* SRC_MMDATA_HPP_ */
