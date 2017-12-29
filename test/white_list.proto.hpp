#ifndef WHITE_LIST_PROTO_HPP_
#define WHITE_LIST_PROTO_HPP_
#include "kcfg.hpp"
#include "mmdata_kcfg.hpp"
#include "mmdata.hpp"

namespace RECMD_SHM_DATA_V3
{
    struct WhiteListItem
    {
        int64_t testid;
        int64_t ruleid;

        KCFG_DEFINE_FIELDS(testid,ruleid)

        WhiteListItem(const mmdata::CharAllocator& alloc):testid(0),ruleid(0)
        {}
    };
    struct WhiteListDataTable;
    struct WhiteListData
    {
        typedef mmdata::SHMString key_type;
        typedef mmdata::SHMVector<WhiteListItem>::Type value_type;
        typedef WhiteListDataTable table_type;
        key_type imei;
        value_type items;

        KCFG_DEFINE_FIELDS(imei,items)

        WhiteListData(const mmdata::CharAllocator& alloc):imei(alloc),items(alloc)
        {}

        const key_type& GetKey() const { return imei; }
        const value_type& GetValue() const { return items; }
    };
    typedef mmdata::SHMHashMap<mmdata::SHMString, mmdata::SHMVector<WhiteListItem>::Type>::Type WhiteListDataTableParent;

    struct WhiteListDataTable:public WhiteListDataTableParent
    {
        WhiteListDataTable(const mmdata::CharAllocator& alloc):WhiteListDataTableParent(alloc)
        {
        }

        static uint64_t GetHash() const { return 18446744073622913952;}
    };
}
#endif /* WHITE_LIST_PROTO_HPP_ */
