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

#include "mmdata.hpp"
#include "malloc-2.8.3.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>

namespace mmdata
{
    static inline int64_t allign_page(int64_t size)
    {
        int page = sysconf(_SC_PAGESIZE);
        uint32_t left = size % page;
        if (left > 0)
        {
            return size - left;
        }
        return size;
    }

    MMData::MMData()
            : meta_(NULL)
    {

    }
    int MMData::CreateMeta(void* buf, int64_t memsize, bool create_allocator,bool reinit)
    {
        printf("####[%d]CreateMeta %d %d %d\n",getpid(),  memsize, create_allocator,reinit);
        if (create_allocator && memsize > 0 && memsize < ALLOCATOR_META_SIZE * 2)
        {
            err = "Too small mem size to create allocator.";
            return -1;
        }
        meta_ = (Meta*) buf;
        if (memsize > 0)
        {
            Meta* meta = (Meta*) buf;
            if(create_allocator && reinit)
            {
            	memset(meta, 0, sizeof(Meta));
                meta->size = memsize - ALLOCATOR_META_SIZE;
                meta->size = allign_page(meta->size);
            }
            void* mspace_buf = (char*) meta + ALLOCATOR_META_SIZE;
            void* mspace = create_mspace_with_base(mspace_buf, meta->size, 0, true);
            if(reinit)
            {
                meta->mspace_offset = (char*) mspace - (char*) buf;
            }

        }
        if (create_allocator)
        {
            MemorySpaceInfo mspace_info;
            mspace_info.space = buf;
            allocator_ = Allocator<char>(mspace_info);
        }
        if (memsize > 0 && reinit)
        {
            ::new ((void*) (meta_->naming_table)) NamingTable(allocator_);
        }
        return 0;
    }
    TypeRefItemPtr MMData::NewTypeRefItem(const std::string& type, SHMItemConstructor* cons, SHMItemDestructor* destroy,
            uint32_t ref)
    {
        TypeRefItemPtr ptr = New<TypeRefItem>();
        ptr->destroy = destroy;
        ptr->val = cons(*this);
        ptr->ref = ref;
        ptr->type.assign(type.c_str(), type.size());
        return ptr;
    }
    void MMData::Delete(TypeRefItemPtr p)
    {
        if (NULL == p.get() || NULL == p->val.get())
        {
            return;
        }
        p->destroy(*this, p->val.get());
        Delete(p.get());
    }

    int64_t MMFileData::ShrinkToFit()
    {
        if (readonly_)
        {
            err = "MMFileData is in readonly mode.";
            return -1;
        }
        if (databuf_.buf == NULL)
        {
            err = "MMFileData is not opened.";
            return -1;
        }
        CharAllocator& alloc = GetAllocator();
        size_t used_space = alloc.used_space();
        databuf_.Close();
        truncate(file_.c_str(), used_space);
        return used_space;
    }

    ShmData::ShmData(bool auto_dettach)
            : shmid(-1), shm_buf(NULL), dettach(auto_dettach)
    {

    }

    int ShmData::OpenShm(const std::string& f, const ShmOpenOptions& options)
    {
        key_t shm_key = ftok(f.c_str(), 0);
        if (shm_key == -1)
        {
            int err = errno;
            error_reason.append("'ftok' error:").append(strerror(err));
            return -1;
        }
        printf("####[%d]Open %d %d %d\n", getpid(), options.readonly, options.recreate, options.reinit);
        shmid = shmget(shm_key, 0, 0);
        if (!options.readonly)
        {
            if (shmid != -1 && options.recreate)
            {
                shmctl(shmid, IPC_RMID, NULL);
                shmid = -1;
            }
            if(shmid == -1)
            {
                shmid = shmget(shm_key, options.size, IPC_CREAT | IPC_EXCL | 0666);
            }
        }
        if (shmid == -1)
        {
            int err = errno;
            error_reason.append("'shmget' error:").append(strerror(err));
            return -1;
        }
        shm_buf = shmat(shmid, NULL, 0);
        if (shm_buf == (void*) -1)
        {
            int err = errno;
            error_reason.append("'shmat' error:").append(strerror(err));
            return -1;
        }
        if (options.readonly)
        {
            return OpenRead(shm_buf);
        }
        return OpenWrite(shm_buf,  options.size, true, options.reinit);
    }
    ShmData::~ShmData()
    {
        if (dettach)
        {
            shmdt(shm_buf);
        }
    }

    HeapMMData::HeapMMData()

    {
        memset(meta_buf, 0, sizeof(Meta) );
    }
    int HeapMMData::OpenWrite()
    {
        return MMData::OpenWrite(meta_buf, sizeof(Meta)*2, false);
    }
    int HeapMMData::OpenRead()
    {
        return MMData::OpenRead(meta_buf);
    }
    HeapMMData::~HeapMMData()
    {
    }
}

