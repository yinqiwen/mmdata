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

#ifndef SRC_MMDATA_UTIL_HPP_
#define SRC_MMDATA_UTIL_HPP_

#include "kcfg.hpp"
#include "mmdata_kcfg.hpp"
#include "mmdata.hpp"
#include <fstream>

namespace mmdata
{
    struct DataImageBuildOptions
    {
            int64_t max_image_size;
            std::string src_file;
            std::string dst_file;
            void* dst_buf;
            size_t dst_buf_size;
            DataImageBuildOptions()
                    : max_image_size(10 * 1024 * 1024 * 1024LL), dst_buf(NULL), dst_buf_size(0)
            {
            }
    };

    struct DataImageBuilder
    {
            std::string err;
            template<typename T>
            int64_t Build(const DataImageBuildOptions& options)
            {
                std::ifstream file(options.src_file.c_str());
                if (!file.is_open())
                {
                    err = "Open source file failed.";
                    return -1;
                }
                if (NULL == options.dst_buf && options.dst_file.empty())
                {
                    err = "No dst file or buf specified.";
                    return -1;
                }
                typedef typename T::table_type RootTable;
                RootTable* table = NULL;
                CharAllocator* alloc = NULL;
                MMData mdata;
                MMFileData mfile(options.dst_file, options.max_image_size);
                if (!options.dst_file.empty())
                {
                    table = mfile.LoadRootWriteObject<RootTable>();
                    alloc = &(mfile.GetAllocator());
                }
                else
                {
                    table = mdata.LoadRootWriteObject<RootTable>(options.dst_buf, options.dst_buf_size);
                    alloc = &(mdata.GetAllocator());
                }
                std::string line;
                while (std::getline(file, line))
                {
                    if (line.empty())
                    {
                        continue;
                    }
                    T entry(*alloc);
                    if (kcfg::ParseFromJsonString(line, entry))
                    {
                        table->insert(typename RootTable::value_type(entry.GetKey(), entry.GetValue()));
                    }
                }
                if (!options.dst_file.empty())
                {
                    return mfile.ShrinkToFit();
                }
                return alloc->used_space();
            }
    };

    template<typename Container>
    int WriteListToJson(const Container& v, const std::string& file, bool append = true)
    {
        FILE* f = fopen(file.c_str(), append ? "a+" : "w+");
        if (NULL == f)
        {
            return -1;
        }
        typename Container::const_iterator it = v.begin();
        while (it != v.end())
        {
            std::string json;
            kcfg::WriteToJsonString(*it, json);
            fprintf(f, "%s\n", json.c_str());
            it++;
        }
        fclose(f);
        return 0;
    }

    typedef int64_t DataImageBuildFunc(mmdata::DataImageBuildOptions& options, uint64_t& hash, std::string& err);
    typedef int TestMemoryFunc(const void* mem, const std::string& json_key);
    struct HelperFuncRegister
    {
            HelperFuncRegister(const char* name, DataImageBuildFunc* func, TestMemoryFunc* test, uint64_t struct_hash);
    };

    DataImageBuildFunc* GetBuildFuncByName(const std::string& name);
    TestMemoryFunc* GetTestFuncByName(const std::string& name);
    uint64_t GetRootStructHash(const std::string& nam);

}

#endif /* SRC_MMDATA_UTIL_HPP_ */
