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
#include <stdio.h>
#include "mmdata.hpp"

using namespace mmdata;

struct ValueObject
{
        SHMString v0;
        int64_t v1;
        double v2;
        SHMVector<int>::Type v3;
        ValueObject(const CharAllocator& alloc = CharAllocator() )
                : v0(alloc), v1(0), v2(1.0), v3(alloc)
        {
        }
};
typedef SHMHashMap<SHMString, ValueObject>::Type RootTable;

static int64_t image_maxsize = 100 * 1024 * 1024;
static std::string image_file = "./test.img";
static void build_data_image()
{
    MMFileData mfile(image_file, image_maxsize);
    RootTable* table = mfile.LoadRootWriteObject<RootTable>();
    CharAllocator& alloc = mfile.GetAllocator();
    for (size_t i = 0; i < 1; i++)
    {
        SHMString key;
        char tmp[100];
        sprintf(tmp, "key%d", i);
        key.assign(tmp);
        ValueObject v(alloc);
        sprintf(tmp, "value%d", i);
        v.v0.assign(tmp);
        v.v1 = i + 100;
        v.v2 = i * 1000;
        v.v3.push_back(i);
        v.v3.push_back(i + 1);
        v.v3.push_back(i + 2);
        table->insert(RootTable::value_type(key, v));
    }
    int64_t n = mfile.ShrinkWriteFile();
    printf("Build data image file with size:%lld\n", n);
}

static void load_data_image()
{
    MMFileData mfile(image_file);
    const RootTable* read_table = mfile.LoadRootReadObject<RootTable>();
    printf("Total %llu entry in data image.\n", read_table->size());
    RootTable::const_iterator found = read_table->find("key100");
    if (found != read_table->end())
    {
        const SHMString& key = found->first;
        const ValueObject& value = found->second;
        printf("FoundEntry: key:%s v0:%s v1:%lld  v3:[%d %d %d]\n", key.c_str(), value.v0.c_str(), value.v1, value.v3[0], value.v3[1],
                value.v3[2]);
    }
}

int main()
{
    build_data_image();

    load_data_image();
    return 0;
}
