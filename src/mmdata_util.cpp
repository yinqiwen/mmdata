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

#include "mmdata_util.hpp"
#include <map>

namespace mmdata
{
    struct HelperFuncs
    {
            DataImageBuildFunc* build;
            TestMemoryFunc* test;
            uint64_t struct_hash;
            HelperFuncs()
                    : build(NULL), test(NULL),struct_hash(0)
            {
            }
    };
    typedef std::map<std::string, HelperFuncs> HelperFuncTable;
    static HelperFuncTable* g_helper_func_table = NULL;
    HelperFuncRegister::HelperFuncRegister(const char* name, DataImageBuildFunc* func, TestMemoryFunc* test, uint64_t struct_hash)
    {
        if (NULL == g_helper_func_table)
        {
            g_helper_func_table = new HelperFuncTable;
        }
        DataImageBuildFunc* current = GetBuildFuncByName(name);
        if (current != NULL)
        {
            //duplicate name
            abort();
        }
        HelperFuncs helper;
        helper.build = func;
        helper.test = test;
        helper.struct_hash = struct_hash;
        (*g_helper_func_table)[name] = helper;
    }

    DataImageBuildFunc* GetBuildFuncByName(const std::string& name)
    {
        if (NULL == g_helper_func_table)
        {
            return NULL;
        }
        HelperFuncTable::const_iterator found = g_helper_func_table->find(name);
        if (found != g_helper_func_table->end())
        {
            return found->second.build;
        }
        return NULL;
    }

    TestMemoryFunc* GetTestFuncByName(const std::string& name)
    {
        if (NULL == g_helper_func_table)
        {
            return NULL;
        }
        HelperFuncTable::const_iterator found = g_helper_func_table->find(name);
        if (found != g_helper_func_table->end())
        {
            return found->second.test;
        }
        return NULL;
    }
    uint64_t GetRootStructHash(const std::string& name)
    {
        if (NULL == g_helper_func_table)
        {
            return 0;
        }
        HelperFuncTable::const_iterator found = g_helper_func_table->find(name);
        if (found != g_helper_func_table->end())
        {
            return found->second.struct_hash;
        }
        return 0;
    }
}

