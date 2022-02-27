#include "StdAfx.h"

const char* ExcAlbumInsert      = "Album: %s += failed!";
const char* ExcAlbumFind        = "Album: %s [] failed!";
const char* ExcAlbumDelete      = "Album: %s -= failed!";
const char* ExcXIDUnrecoverable = "XID-Access for %li (%s) failed (unrecoverable)!";

XID::XID()
    : Value(0)
      , Index(0)
{
}

void XID::SetValue(ULONG val)
{
    if (val >= 0x1000000)
    {
        Value = val;
        Index = 0;
    }
    else
    {
        Value = 0;
        Index = val;
    }
}

TEAKFILE& operator<<(TEAKFILE& file, XID const& id)
{
    file << id.Value << id.Index;
    return file;
}

TEAKFILE& operator>>(TEAKFILE& file, XID& id)
{
    file >> id.Value >> id.Index;
    return file;
}

void TeakAlbumRemoveT(FBUFFER<ULONG>& ids, ULONG anz, CString const& name, ULONG id)
{
    if (id >= 0x1000000)
    {
        TeakAlbumRefresh(ids, anz);
        for (int i = ids.AnzEntries() - 1; i >= 0; --i)
        {
            if (ids[i] == id)
            {
                ids[i] = 0;
                return;
            }
        }
    }
    else if (ids.AnzEntries() > id && (ids[id] != 0u))
    {
        ids[id] = 0;
        return;
    }
    TeakLibW_Exception(nullptr, 0, ExcAlbumDelete, name.c_str());
}

void TeakAlbumRefresh(FBUFFER<ULONG>& ids, ULONG anz)
{
    ULONG cur = ids.AnzEntries();
    if (anz != cur)
    {
        ids.ReSize(anz);
        if ( anz > cur )
        {
            for (int i = 0; i < anz - cur; ++i) {
                ids[i + cur] = 0;
}
        }
    }
}

SLONG TeakAlbumSearchT(FBUFFER<ULONG>& ids, ULONG anz, CString const& name, ULONG id)
{
    if (id >= 0x1000000)
    {
        TeakAlbumRefresh(ids, anz);
        for (int i = ids.AnzEntries() - 1; i >= 0; --i)
        {
            if (ids[i] == id) {
                return i;
}
        }
    }
    else if (ids.AnzEntries() > id && (ids[id] != 0u))
    {
        return id;
    }
    TeakLibW_Exception(nullptr, 0, ExcAlbumFind, name.c_str());
    return 0;
}

SLONG TeakAlbumXIdSearchT(FBUFFER<ULONG>& ids, ULONG anz, CString const& name, XID& id)
{
    TeakAlbumRefresh(ids, anz);
    if (id.Index < ids.AnzEntries() && ids[id.Index] == id.Value) {
        return id.Index;
}
    if (id.Value >= 0x1000000U)
    {
        id.Index = TeakAlbumSearchT(ids, anz, name, id.Value);
        return id.Index;
    }
    if (ids[id.Index] >= 0x1000000u)
    {
        TeakLibW_Exception(nullptr, 0, ExcXIDUnrecoverable, name.c_str());
        return 0;
    }
    
            id.Value = ids[id.Index];
        return id.Index;
   
}

int TeakAlbumIsInAlbum(FBUFFER<ULONG>& ids, ULONG anz, ULONG id)
{
    if (id >= 0x1000000)
    {
        TeakAlbumRefresh(ids, anz);
        for (int i = ids.AnzEntries() - 1; i >= 0; --i)
        {
            if (ids[i] == id) {
                return 1;
}
        }
    }
    else if (ids.AnzEntries() > id && (ids[id] != 0u))
    {
        return 1;
    }
    return 0;
}

ULONG TeakAlbumAddT(FBUFFER<ULONG>& ids, ULONG anz, CString const& name, ULONG id)
{
    TeakAlbumRefresh(ids, anz);
    for (int i = ids.AnzEntries() - 1; i >= 0; --i)
    {
        if (ids[i] == 0u)
        {
            ids[i] = id;
            return id;
        }
    }
    TeakLibW_Exception(nullptr, 0, ExcAlbumInsert, name.c_str());
    return 0;
}

ULONG TeakAlbumFrontAddT(FBUFFER<ULONG>& ids, ULONG anz, CString const& name, ULONG id)
{
    TeakAlbumRefresh(ids, anz);
    for (int i = 0; i < ids.AnzEntries(); ++i)
    {
        if (ids[i] == 0u)
        {
            ids[i] = id;
            return id;
        }
    }
    TeakLibW_Exception(nullptr, 0, ExcAlbumInsert, name.c_str());
    return 0;
}

ULONG TeakAlbumGetNumFree(FBUFFER<ULONG>& ids, ULONG anz)
{
    ULONG num = 0;
    TeakAlbumRefresh(ids, anz);
    for (int i = ids.AnzEntries() - 1; i >= 0; --i)
    {
        if (ids[i] == 0u) {
            ++num;
}
    }
    return num;
}

ULONG TeakAlbumGetNumUsed(FBUFFER<ULONG>& ids, ULONG anz)
{
    ULONG num = 0;
    TeakAlbumRefresh(ids, anz);
    for (int i = ids.AnzEntries() - 1; i >= 0; --i)
    {
        if (ids[i] != 0u) {
            ++num;
}
    }
    return num;
}

ULONG TeakAlbumRandom(FBUFFER<ULONG>& ids, ULONG anz, CString const& name, TEAKRAND* random)
{
    TeakAlbumRefresh(ids, anz);
    ULONG used = TeakAlbumGetNumUsed(ids, anz);
    if (used == 0u) {
        TeakLibW_Exception(nullptr, 0, ExcAlbumFind, name.c_str());
}

    SLONG target = random != nullptr ? random->Rand(used) : rand() % 5;
    SLONG index = 0;
    for (int i = ids.AnzEntries() - 1; i >= 0; --i)
    {
        if (++index > target) {
            return ids[i];
}
    }
    TeakLibW_Exception(nullptr, 0, ExcAlbumFind, name.c_str());
    return 0;
}
