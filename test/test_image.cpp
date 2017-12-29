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
#include "mmdata_util.hpp"
#include "white_list.proto.hpp"

using namespace mmdata;

static void build_json_data_file()
{
    typedef std::vector<RECMD_SHM_DATA_V3::WhiteListData> DataList;
    CharAllocator alloc;
    {
        DataList data_list;
        char tmp[1024];
        for (int i = 0; i < 12345; i++)
        {
            RECMD_SHM_DATA_V3::WhiteListData v(alloc);
            sprintf(tmp, "imei%d", i);
            v.imei.assign(tmp, strlen(tmp));
            for (int j = 0; j < 3; j++)
            {
                RECMD_SHM_DATA_V3::WhiteListItem item(alloc);
                item.ruleid = i + i * j;
                item.testid = j + i * j;
                v.items.push_back(item);
            }
            data_list.push_back(v);
        }
        WriteListToJson(data_list, "test.json", false);
    }
}

static void build_data_image()
{
    DataImageBuilder builder;
    DataImageBuildOptions options;
    options.dst_file = "test.img";
    options.src_file = "test.json";
    builder.Build<RECMD_SHM_DATA_V3::WhiteListData>(options);
}
static void load_data_image()
{
    MMFileData mfile("test.img");
    typedef typename RECMD_SHM_DATA_V3::WhiteListData::table_type RootTable;
    printf("###Table size:%d\n", sizeof(RootTable));
    const RootTable* read_table = mfile.LoadRootReadObject<RootTable>();
    printf("Total %llu entry in data image with hash:%llu\n", read_table->size(), read_table->GetHash());
    RootTable::const_iterator found = read_table->find("imei1000");
    if(found != read_table->end())
    {
        const SHMString& key = found->first;
        printf("Found entry for key: %s\n", key.c_str());
    }

}

int main()
{
    build_json_data_file();
    build_data_image();
    load_data_image();
    return 0;
}
