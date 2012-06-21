/*
 * Copyright 2011 Vincent Povirk for CodeWeavers
 * Copyright 2012 Dmitry Timoshkov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#define COBJMACROS

#include "windef.h"
#include "objbase.h"
#include "wincodec.h"
#include "wincodecsdk.h"
#include "wine/test.h"

#define expect_blob(propvar, data, length) do { \
    ok((propvar).vt == VT_BLOB, "unexpected vt: %i\n", (propvar).vt); \
    if ((propvar).vt == VT_BLOB) { \
        ok(U(propvar).blob.cbSize == (length), "expected size %u, got %u\n", (ULONG)(length), U(propvar).blob.cbSize); \
        if (U(propvar).blob.cbSize == (length)) { \
            ok(!memcmp(U(propvar).blob.pBlobData, (data), (length)), "unexpected data\n"); \
        } \
    } \
} while (0)

#define IFD_BYTE 1
#define IFD_ASCII 2
#define IFD_SHORT 3
#define IFD_LONG 4
#define IFD_RATIONAL 5
#define IFD_SBYTE 6
#define IFD_UNDEFINED 7
#define IFD_SSHORT 8
#define IFD_SLONG 9
#define IFD_SRATIONAL 10
#define IFD_FLOAT 11
#define IFD_DOUBLE 12
#define IFD_IFD 13

#include "pshpack2.h"
struct IFD_entry
{
    SHORT id;
    SHORT type;
    ULONG count;
    LONG  value;
};

struct IFD_rational
{
    LONG numerator;
    LONG denominator;
};

static const struct ifd_data
{
    USHORT number_of_entries;
    struct IFD_entry entry[40];
    ULONG next_IFD;
    struct IFD_rational xres;
    DOUBLE double_val;
    struct IFD_rational srational_val;
    char string[14];
    SHORT short_val[4];
    LONG long_val[2];
    FLOAT float_val[2];
} IFD_data =
{
    19,
    {
        { 0xfe,  IFD_SHORT, 1, 1 }, /* NEWSUBFILETYPE */
        { 0x100, IFD_LONG, 1, 222 }, /* IMAGEWIDTH */
        { 0x101, IFD_LONG, 1, 333 }, /* IMAGELENGTH */
        { 0x102, IFD_SHORT, 1, 24 }, /* BITSPERSAMPLE */
        { 0x103, IFD_LONG, 1, 32773 }, /* COMPRESSION: packbits */
        { 0x11a, IFD_RATIONAL, 1, FIELD_OFFSET(struct ifd_data, xres) },
        { 0xf001, IFD_BYTE, 1, 0x11223344 },
        { 0xf002, IFD_BYTE, 4, 0x11223344 },
        { 0xf003, IFD_SBYTE, 1, 0x11223344 },
        { 0xf004, IFD_SSHORT, 1, 0x11223344 },
        { 0xf005, IFD_SSHORT, 2, 0x11223344 },
        { 0xf006, IFD_SLONG, 1, 0x11223344 },
        { 0xf007, IFD_FLOAT, 1, 0x11223344 },
        { 0xf008, IFD_DOUBLE, 1, FIELD_OFFSET(struct ifd_data, double_val) },
        { 0xf009, IFD_SRATIONAL, 1, FIELD_OFFSET(struct ifd_data, srational_val) },
        { 0xf00a, IFD_BYTE, 13, FIELD_OFFSET(struct ifd_data, string) },
        { 0xf00b, IFD_SSHORT, 4, FIELD_OFFSET(struct ifd_data, short_val) },
        { 0xf00c, IFD_SLONG, 2, FIELD_OFFSET(struct ifd_data, long_val) },
        { 0xf00d, IFD_FLOAT, 2, FIELD_OFFSET(struct ifd_data, float_val) },
    },
    0,
    { 900, 3 },
    1234567890.0987654321,
    { 0x1a2b3c4d, 0x5a6b7c8d },
    "Hello World!",
    { 0x0101, 0x0202, 0x0303, 0x0404 },
    { 0x11223344, 0x55667788 },
    { (FLOAT)1234.5678, (FLOAT)8765.4321 },
};
#include "poppack.h"

static const char metadata_unknown[] = "lalala";

static const char metadata_tEXt[] = {
    0,0,0,14, /* chunk length */
    't','E','X','t', /* chunk type */
    'w','i','n','e','t','e','s','t',0, /* keyword */
    'v','a','l','u','e', /* text */
    0x3f,0x64,0x19,0xf3 /* chunk CRC */
};

static const char pngimage[285] = {
0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x08,0x02,0x00,0x00,0x00,0x90,0x77,0x53,
0xde,0x00,0x00,0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,0x0b,0x13,0x00,0x00,0x0b,
0x13,0x01,0x00,0x9a,0x9c,0x18,0x00,0x00,0x00,0x07,0x74,0x49,0x4d,0x45,0x07,0xd5,
0x06,0x03,0x0f,0x07,0x2d,0x12,0x10,0xf0,0xfd,0x00,0x00,0x00,0x0c,0x49,0x44,0x41,
0x54,0x08,0xd7,0x63,0xf8,0xff,0xff,0x3f,0x00,0x05,0xfe,0x02,0xfe,0xdc,0xcc,0x59,
0xe7,0x00,0x00,0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82
};

static const char *debugstr_guid(REFIID riid)
{
    static char buf[50];

    if(!riid)
        return "(null)";

    sprintf(buf, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            riid->Data1, riid->Data2, riid->Data3, riid->Data4[0],
            riid->Data4[1], riid->Data4[2], riid->Data4[3], riid->Data4[4],
            riid->Data4[5], riid->Data4[6], riid->Data4[7]);

    return buf;
}

static IStream *create_stream(const char *data, int data_size)
{
    HRESULT hr;
    IStream *stream;
    HGLOBAL hdata;
    void *locked_data;

    hdata = GlobalAlloc(GMEM_MOVEABLE, data_size);
    ok(hdata != 0, "GlobalAlloc failed\n");
    if (!hdata) return NULL;

    locked_data = GlobalLock(hdata);
    memcpy(locked_data, data, data_size);
    GlobalUnlock(hdata);

    hr = CreateStreamOnHGlobal(hdata, TRUE, &stream);
    ok(hr == S_OK, "CreateStreamOnHGlobal failed, hr=%x\n", hr);

    return stream;
}

static void load_stream(IUnknown *reader, const char *data, int data_size)
{
    HRESULT hr;
    IWICPersistStream *persist;
    IStream *stream;
    LARGE_INTEGER pos;
    ULARGE_INTEGER cur_pos;

    stream = create_stream(data, data_size);
    if (!stream)
        return;

    hr = IUnknown_QueryInterface(reader, &IID_IWICPersistStream, (void**)&persist);
    ok(hr == S_OK, "QueryInterface failed, hr=%x\n", hr);

    if (SUCCEEDED(hr))
    {
        hr = IWICPersistStream_LoadEx(persist, stream, NULL, WICPersistOptionsDefault);
        ok(hr == S_OK, "LoadEx failed, hr=%x\n", hr);

        IWICPersistStream_Release(persist);
    }

    pos.QuadPart = 0;
    hr = IStream_Seek(stream, pos, SEEK_CUR, &cur_pos);
    ok(hr == S_OK, "IStream_Seek error %#x\n", hr);
    /* IFD metadata reader doesn't rewind the stream to the start */
    ok(cur_pos.QuadPart == 0 || cur_pos.QuadPart <= data_size,
       "current stream pos is at %x/%x, data size %x\n", cur_pos.u.LowPart, cur_pos.u.HighPart, data_size);

    IStream_Release(stream);
}

static void test_metadata_unknown(void)
{
    HRESULT hr;
    IWICMetadataReader *reader;
    IWICEnumMetadataItem *enumerator;
    IWICMetadataBlockReader *blockreader;
    PROPVARIANT schema, id, value;
    ULONG items_returned;

    hr = CoCreateInstance(&CLSID_WICUnknownMetadataReader, NULL, CLSCTX_INPROC_SERVER,
        &IID_IWICMetadataReader, (void**)&reader);
    ok(hr == S_OK, "CoCreateInstance failed, hr=%x\n", hr);
    if (FAILED(hr)) return;

    load_stream((IUnknown*)reader, metadata_unknown, sizeof(metadata_unknown));

    hr = IWICMetadataReader_GetEnumerator(reader, &enumerator);
    ok(hr == S_OK, "GetEnumerator failed, hr=%x\n", hr);

    if (SUCCEEDED(hr))
    {
        PropVariantInit(&schema);
        PropVariantInit(&id);
        PropVariantInit(&value);

        hr = IWICEnumMetadataItem_Next(enumerator, 1, &schema, &id, &value, &items_returned);
        ok(hr == S_OK, "Next failed, hr=%x\n", hr);
        ok(items_returned == 1, "unexpected item count %i\n", items_returned);

        if (hr == S_OK && items_returned == 1)
        {
            ok(schema.vt == VT_EMPTY, "unexpected vt: %i\n", schema.vt);
            ok(id.vt == VT_EMPTY, "unexpected vt: %i\n", id.vt);
            expect_blob(value, metadata_unknown, sizeof(metadata_unknown));

            PropVariantClear(&schema);
            PropVariantClear(&id);
            PropVariantClear(&value);
        }

        hr = IWICEnumMetadataItem_Next(enumerator, 1, &schema, &id, &value, &items_returned);
        ok(hr == S_FALSE, "Next failed, hr=%x\n", hr);
        ok(items_returned == 0, "unexpected item count %i\n", items_returned);

        IWICEnumMetadataItem_Release(enumerator);
    }

    hr = IWICMetadataReader_QueryInterface(reader, &IID_IWICMetadataBlockReader, (void**)&blockreader);
    ok(hr == E_NOINTERFACE, "QueryInterface failed, hr=%x\n", hr);

    if (SUCCEEDED(hr))
        IWICMetadataBlockReader_Release(blockreader);

    IWICMetadataReader_Release(reader);
}

static void test_metadata_tEXt(void)
{
    HRESULT hr;
    IWICMetadataReader *reader;
    IWICEnumMetadataItem *enumerator;
    PROPVARIANT schema, id, value;
    ULONG items_returned, count;
    GUID format;

    PropVariantInit(&schema);
    PropVariantInit(&id);
    PropVariantInit(&value);

    hr = CoCreateInstance(&CLSID_WICPngTextMetadataReader, NULL, CLSCTX_INPROC_SERVER,
        &IID_IWICMetadataReader, (void**)&reader);
    todo_wine ok(hr == S_OK, "CoCreateInstance failed, hr=%x\n", hr);
    if (FAILED(hr)) return;

    hr = IWICMetadataReader_GetCount(reader, NULL);
    ok(hr == E_INVALIDARG, "GetCount failed, hr=%x\n", hr);

    hr = IWICMetadataReader_GetCount(reader, &count);
    ok(hr == S_OK, "GetCount failed, hr=%x\n", hr);
    ok(count == 0, "unexpected count %i\n", count);

    load_stream((IUnknown*)reader, metadata_tEXt, sizeof(metadata_tEXt));

    hr = IWICMetadataReader_GetCount(reader, &count);
    ok(hr == S_OK, "GetCount failed, hr=%x\n", hr);
    ok(count == 1, "unexpected count %i\n", count);

    hr = IWICMetadataReader_GetEnumerator(reader, NULL);
    ok(hr == E_INVALIDARG, "GetEnumerator failed, hr=%x\n", hr);

    hr = IWICMetadataReader_GetEnumerator(reader, &enumerator);
    ok(hr == S_OK, "GetEnumerator failed, hr=%x\n", hr);

    if (SUCCEEDED(hr))
    {
        hr = IWICEnumMetadataItem_Next(enumerator, 1, &schema, &id, &value, &items_returned);
        ok(hr == S_OK, "Next failed, hr=%x\n", hr);
        ok(items_returned == 1, "unexpected item count %i\n", items_returned);

        if (hr == S_OK && items_returned == 1)
        {
            ok(schema.vt == VT_EMPTY, "unexpected vt: %i\n", schema.vt);
            ok(id.vt == VT_LPSTR, "unexpected vt: %i\n", id.vt);
            ok(!strcmp(U(id).pszVal, "winetest"), "unexpected id: %s\n", U(id).pszVal);
            ok(value.vt == VT_LPSTR, "unexpected vt: %i\n", value.vt);
            ok(!strcmp(U(value).pszVal, "value"), "unexpected value: %s\n", U(value).pszVal);

            PropVariantClear(&schema);
            PropVariantClear(&id);
            PropVariantClear(&value);
        }

        hr = IWICEnumMetadataItem_Next(enumerator, 1, &schema, &id, &value, &items_returned);
        ok(hr == S_FALSE, "Next failed, hr=%x\n", hr);
        ok(items_returned == 0, "unexpected item count %i\n", items_returned);

        IWICEnumMetadataItem_Release(enumerator);
    }

    hr = IWICMetadataReader_GetMetadataFormat(reader, &format);
    ok(hr == S_OK, "GetMetadataFormat failed, hr=%x\n", hr);
    ok(IsEqualGUID(&format, &GUID_MetadataFormatChunktEXt), "unexpected format %s\n", debugstr_guid(&format));

    hr = IWICMetadataReader_GetMetadataFormat(reader, NULL);
    ok(hr == E_INVALIDARG, "GetMetadataFormat failed, hr=%x\n", hr);

    id.vt = VT_LPSTR;
    U(id).pszVal = CoTaskMemAlloc(strlen("winetest") + 1);
    strcpy(U(id).pszVal, "winetest");

    hr = IWICMetadataReader_GetValue(reader, NULL, &id, NULL);
    ok(hr == S_OK, "GetValue failed, hr=%x\n", hr);

    hr = IWICMetadataReader_GetValue(reader, &schema, NULL, &value);
    ok(hr == E_INVALIDARG, "GetValue failed, hr=%x\n", hr);

    hr = IWICMetadataReader_GetValue(reader, &schema, &id, &value);
    ok(hr == S_OK, "GetValue failed, hr=%x\n", hr);
    ok(value.vt == VT_LPSTR, "unexpected vt: %i\n", id.vt);
    ok(!strcmp(U(value).pszVal, "value"), "unexpected value: %s\n", U(value).pszVal);
    PropVariantClear(&value);

    strcpy(U(id).pszVal, "test");

    hr = IWICMetadataReader_GetValue(reader, &schema, &id, &value);
    ok(hr == WINCODEC_ERR_PROPERTYNOTFOUND, "GetValue failed, hr=%x\n", hr);

    PropVariantClear(&id);

    hr = IWICMetadataReader_GetValueByIndex(reader, 0, NULL, NULL, NULL);
    ok(hr == S_OK, "GetValueByIndex failed, hr=%x\n", hr);

    hr = IWICMetadataReader_GetValueByIndex(reader, 0, &schema, NULL, NULL);
    ok(hr == S_OK, "GetValueByIndex failed, hr=%x\n", hr);
    ok(schema.vt == VT_EMPTY, "unexpected vt: %i\n", schema.vt);

    hr = IWICMetadataReader_GetValueByIndex(reader, 0, NULL, &id, NULL);
    ok(hr == S_OK, "GetValueByIndex failed, hr=%x\n", hr);
    ok(id.vt == VT_LPSTR, "unexpected vt: %i\n", id.vt);
    ok(!strcmp(U(id).pszVal, "winetest"), "unexpected id: %s\n", U(id).pszVal);
    PropVariantClear(&id);

    hr = IWICMetadataReader_GetValueByIndex(reader, 0, NULL, NULL, &value);
    ok(hr == S_OK, "GetValueByIndex failed, hr=%x\n", hr);
    ok(value.vt == VT_LPSTR, "unexpected vt: %i\n", value.vt);
    ok(!strcmp(U(value).pszVal, "value"), "unexpected value: %s\n", U(value).pszVal);
    PropVariantClear(&value);

    hr = IWICMetadataReader_GetValueByIndex(reader, 1, NULL, NULL, NULL);
    ok(hr == E_INVALIDARG, "GetValueByIndex failed, hr=%x\n", hr);

    IWICMetadataReader_Release(reader);
}

static void test_metadata_IFD(void)
{
    static const struct test_data
    {
        ULONG type, id;
        int count; /* if VT_VECTOR */
        LONGLONG value[13];
    } td[19] =
    {
        { VT_UI2, 0xfe, 0, { 1 } },
        { VT_UI4, 0x100, 0, { 222 } },
        { VT_UI4, 0x101, 0, { 333 } },
        { VT_UI2, 0x102, 0, { 24 } },
        { VT_UI4, 0x103, 0, { 32773 } },
        { VT_UI8, 0x11a, 0, { ((LONGLONG)3 << 32) | 900 } },
        { VT_UI1, 0xf001, 0, { 0x44 } },
        { VT_UI1|VT_VECTOR, 0xf002, 4, { 0x44, 0x33, 0x22, 0x11 } },
        { VT_I1, 0xf003, 0, { 0x44 } },
        { VT_I2, 0xf004, 0, { 0x3344 } },
        { VT_I2|VT_VECTOR, 0xf005, 2, { 0x3344, 0x1122 } },
        { VT_I4, 0xf006, 0, { 0x11223344 } },
        { VT_R4, 0xf007, 0, { 0x11223344 } },
        { VT_R8, 0xf008, 0, { ((LONGLONG)0x41d26580 << 32) | 0xb486522c } },
        { VT_I8, 0xf009, 0, { ((LONGLONG)0x5a6b7c8d << 32) | 0x1a2b3c4d } },
        { VT_UI1|VT_VECTOR, 0xf00a, 13, { 'H','e','l','l','o',' ','W','o','r','l','d','!',0 } },
        { VT_I2|VT_VECTOR, 0xf00b, 4, { 0x0101, 0x0202, 0x0303, 0x0404 } },
        { VT_I4|VT_VECTOR, 0xf00c, 2, { 0x11223344, 0x55667788 } },
        { VT_R4|VT_VECTOR, 0xf00d, 2, { 0x449a522b, 0x4608f5ba } },
    };
    HRESULT hr;
    IWICMetadataReader *reader;
    IWICMetadataBlockReader *blockreader;
    IWICEnumMetadataItem *enumerator;
    PROPVARIANT schema, id, value;
    ULONG items_returned, count, i;
    GUID format;

    PropVariantInit(&schema);
    PropVariantInit(&id);
    PropVariantInit(&value);

    hr = CoCreateInstance(&CLSID_WICIfdMetadataReader, NULL, CLSCTX_INPROC_SERVER,
        &IID_IWICMetadataReader, (void**)&reader);
    ok(hr == S_OK, "CoCreateInstance error %#x\n", hr);

    hr = IWICMetadataReader_GetCount(reader, NULL);
    ok(hr == E_INVALIDARG, "GetCount error %#x\n", hr);

    hr = IWICMetadataReader_GetCount(reader, &count);
    ok(hr == S_OK, "GetCount error %#x\n", hr);
    ok(count == 0, "unexpected count %u\n", count);

    load_stream((IUnknown*)reader, (const char *)&IFD_data, sizeof(IFD_data));

    hr = IWICMetadataReader_GetCount(reader, &count);
    ok(hr == S_OK, "GetCount error %#x\n", hr);
    ok(count == sizeof(td)/sizeof(td[0]), "unexpected count %u\n", count);

    hr = IWICMetadataReader_GetEnumerator(reader, NULL);
    ok(hr == E_INVALIDARG, "GetEnumerator error %#x\n", hr);

    hr = IWICMetadataReader_GetEnumerator(reader, &enumerator);
    ok(hr == S_OK, "GetEnumerator error %#x\n", hr);

    for (i = 0; i < count; i++)
    {
        hr = IWICEnumMetadataItem_Next(enumerator, 1, &schema, &id, &value, &items_returned);
        ok(hr == S_OK, "Next error %#x\n", hr);
        ok(items_returned == 1, "unexpected item count %u\n", items_returned);

        ok(schema.vt == VT_EMPTY, "%u: unexpected vt: %u\n", i, schema.vt);
        ok(id.vt == VT_UI2, "%u: unexpected vt: %u\n", i, id.vt);
        ok(U(id).uiVal == td[i].id, "%u: expected id %#x, got %#x\n", i, td[i].id, U(id).uiVal);
        ok(value.vt == td[i].type, "%u: expected vt %#x, got %#x\n", i, td[i].type, value.vt);
        if (value.vt & VT_VECTOR)
        {
            ULONG j;
            switch (value.vt & ~VT_VECTOR)
            {
            case VT_I1:
            case VT_UI1:
                ok(td[i].count == U(value).caub.cElems, "%u: expected cElems %d, got %d\n", i, td[i].count, U(value).caub.cElems);
                for (j = 0; j < U(value).caub.cElems; j++)
                    ok(td[i].value[j] == U(value).caub.pElems[j], "%u: expected value[%d] %#x/%#x, got %#x\n", i, j, (ULONG)td[i].value[j], (ULONG)(td[i].value[j] >> 32), U(value).caub.pElems[j]);
                break;
            case VT_I2:
            case VT_UI2:
                ok(td[i].count == U(value).caui.cElems, "%u: expected cElems %d, got %d\n", i, td[i].count, U(value).caui.cElems);
                for (j = 0; j < U(value).caui.cElems; j++)
                    ok(td[i].value[j] == U(value).caui.pElems[j], "%u: expected value[%d] %#x/%#x, got %#x\n", i, j, (ULONG)td[i].value[j], (ULONG)(td[i].value[j] >> 32), U(value).caui.pElems[j]);
                break;
            case VT_I4:
            case VT_UI4:
            case VT_R4:
                ok(td[i].count == U(value).caul.cElems, "%u: expected cElems %d, got %d\n", i, td[i].count, U(value).caui.cElems);
                for (j = 0; j < U(value).caul.cElems; j++)
                    ok(td[i].value[j] == U(value).caul.pElems[j], "%u: expected value[%d] %#x/%#x, got %#x\n", i, j, (ULONG)td[i].value[j], (ULONG)(td[i].value[j] >> 32), U(value).caul.pElems[j]);
                break;
            default:
                ok(0, "%u: array of type %d is not handled\n", i, value.vt & ~VT_VECTOR);
                break;
            }
        }
        else
            ok(U(value).uhVal.QuadPart == td[i].value[0], "%u: unexpected value: %d/%d\n", i, U(value).uhVal.u.LowPart, U(value).uhVal.u.HighPart);

        PropVariantClear(&schema);
        PropVariantClear(&id);
        PropVariantClear(&value);
    }

    hr = IWICEnumMetadataItem_Next(enumerator, 1, &schema, &id, &value, &items_returned);
    ok(hr == S_FALSE, "Next should fail\n");
    ok(items_returned == 0, "unexpected item count %u\n", items_returned);

    IWICEnumMetadataItem_Release(enumerator);

    hr = IWICMetadataReader_GetMetadataFormat(reader, &format);
    ok(hr == S_OK, "GetMetadataFormat error %#x\n", hr);
    ok(IsEqualGUID(&format, &GUID_MetadataFormatIfd), "unexpected format %s\n", debugstr_guid(&format));

    hr = IWICMetadataReader_GetMetadataFormat(reader, NULL);
    ok(hr == E_INVALIDARG, "GetMetadataFormat should fail\n");

    hr = IWICMetadataReader_GetValueByIndex(reader, 0, NULL, NULL, NULL);
    ok(hr == S_OK, "GetValueByIndex error %#x\n", hr);

    hr = IWICMetadataReader_GetValueByIndex(reader, count - 1, NULL, NULL, NULL);
    ok(hr == S_OK, "GetValueByIndex error %#x\n", hr);

    hr = IWICMetadataReader_GetValueByIndex(reader, 0, &schema, NULL, NULL);
    ok(hr == S_OK, "GetValueByIndex error %#x\n", hr);
    ok(schema.vt == VT_EMPTY, "unexpected vt: %u\n", schema.vt);

    hr = IWICMetadataReader_GetValueByIndex(reader, count - 1, &schema, NULL, NULL);
    ok(hr == S_OK, "GetValueByIndex error %#x\n", hr);
    ok(schema.vt == VT_EMPTY, "unexpected vt: %u\n", schema.vt);

    hr = IWICMetadataReader_GetValueByIndex(reader, 0, NULL, &id, NULL);
    ok(hr == S_OK, "GetValueByIndex error %#x\n", hr);
    ok(id.vt == VT_UI2, "unexpected vt: %u\n", id.vt);
    ok(U(id).uiVal == 0xfe, "unexpected id: %#x\n", U(id).uiVal);
    PropVariantClear(&id);

    hr = IWICMetadataReader_GetValueByIndex(reader, 0, NULL, NULL, &value);
    ok(hr == S_OK, "GetValueByIndex error %#x\n", hr);
    ok(value.vt == VT_UI2, "unexpected vt: %u\n", value.vt);
    ok(U(value).ulVal == 1, "unexpected id: %u\n", U(value).ulVal);
    PropVariantClear(&value);

    hr = IWICMetadataReader_GetValueByIndex(reader, count, &schema, NULL, NULL);
    ok(hr == E_INVALIDARG, "GetValueByIndex should fail\n");

    hr = IWICMetadataReader_QueryInterface(reader, &IID_IWICMetadataBlockReader, (void**)&blockreader);
    ok(hr == E_NOINTERFACE, "QueryInterface failed, hr=%x\n", hr);

    if (SUCCEEDED(hr))
        IWICMetadataBlockReader_Release(blockreader);

    IWICMetadataReader_Release(reader);
}

static void test_metadata_Exif(void)
{
    HRESULT hr;
    IWICMetadataReader *reader;
    IWICMetadataBlockReader *blockreader;
    UINT count=0;

    hr = CoCreateInstance(&CLSID_WICExifMetadataReader, NULL, CLSCTX_INPROC_SERVER,
        &IID_IWICMetadataReader, (void**)&reader);
    todo_wine ok(hr == S_OK, "CoCreateInstance error %#x\n", hr);
    if (FAILED(hr)) return;

    hr = IWICMetadataReader_GetCount(reader, NULL);
    ok(hr == E_INVALIDARG, "GetCount error %#x\n", hr);

    hr = IWICMetadataReader_GetCount(reader, &count);
    ok(hr == S_OK, "GetCount error %#x\n", hr);
    ok(count == 0, "unexpected count %u\n", count);

    hr = IWICMetadataReader_QueryInterface(reader, &IID_IWICMetadataBlockReader, (void**)&blockreader);
    ok(hr == E_NOINTERFACE, "QueryInterface failed, hr=%x\n", hr);

    if (SUCCEEDED(hr))
        IWICMetadataBlockReader_Release(blockreader);

    IWICMetadataReader_Release(reader);
}

static void test_create_reader(void)
{
    HRESULT hr;
    IWICComponentFactory *factory;
    IStream *stream;
    IWICMetadataReader *reader;
    UINT count=0;
    GUID format;

    hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
        &IID_IWICComponentFactory, (void**)&factory);
    ok(hr == S_OK, "CoCreateInstance failed, hr=%x\n", hr);

    stream = create_stream(metadata_tEXt, sizeof(metadata_tEXt));

    hr = IWICComponentFactory_CreateMetadataReaderFromContainer(factory,
        &GUID_ContainerFormatPng, NULL, WICPersistOptionsDefault,
        stream, &reader);
todo_wine
    ok(hr == S_OK, "CreateMetadataReaderFromContainer failed, hr=%x\n", hr);
    if (FAILED(hr)) return;

    if (SUCCEEDED(hr))
    {
        hr = IWICMetadataReader_GetCount(reader, &count);
        ok(hr == S_OK, "GetCount failed, hr=%x\n", hr);
        ok(count == 1, "unexpected count %i\n", count);

        hr = IWICMetadataReader_GetMetadataFormat(reader, &format);
        ok(hr == S_OK, "GetMetadataFormat failed, hr=%x\n", hr);
        ok(IsEqualGUID(&format, &GUID_MetadataFormatChunktEXt), "unexpected format %s\n", debugstr_guid(&format));

        IWICMetadataReader_Release(reader);
    }

    hr = IWICComponentFactory_CreateMetadataReaderFromContainer(factory,
        &GUID_ContainerFormatWmp, NULL, WICPersistOptionsDefault,
        stream, &reader);
    ok(hr == S_OK, "CreateMetadataReaderFromContainer failed, hr=%x\n", hr);

    if (SUCCEEDED(hr))
    {
        hr = IWICMetadataReader_GetCount(reader, &count);
        ok(hr == S_OK, "GetCount failed, hr=%x\n", hr);
        ok(count == 1, "unexpected count %i\n", count);

        hr = IWICMetadataReader_GetMetadataFormat(reader, &format);
        ok(hr == S_OK, "GetMetadataFormat failed, hr=%x\n", hr);
        ok(IsEqualGUID(&format, &GUID_MetadataFormatUnknown), "unexpected format %s\n", debugstr_guid(&format));

        IWICMetadataReader_Release(reader);
    }

    IStream_Release(stream);

    IWICComponentFactory_Release(factory);
}

static void test_metadata_png(void)
{
    IStream *stream;
    IWICBitmapDecoder *decoder;
    IWICBitmapFrameDecode *frame;
    IWICMetadataBlockReader *blockreader;
    IWICMetadataReader *reader;
    GUID containerformat;
    HRESULT hr;
    UINT count;

    hr = CoCreateInstance(&CLSID_WICPngDecoder, NULL, CLSCTX_INPROC_SERVER,
        &IID_IWICBitmapDecoder, (void**)&decoder);
    ok(hr == S_OK, "CoCreateInstance failed, hr=%x\n", hr);

    if (FAILED(hr)) return;

    stream = create_stream(pngimage, sizeof(pngimage));

    hr = IWICBitmapDecoder_Initialize(decoder, stream, WICDecodeMetadataCacheOnLoad);
    ok(hr == S_OK, "Initialize failed, hr=%x\n", hr);

    hr = IWICBitmapDecoder_QueryInterface(decoder, &IID_IWICMetadataBlockReader, (void**)&blockreader);
    ok(hr == E_NOINTERFACE, "QueryInterface failed, hr=%x\n", hr);

    hr = IWICBitmapDecoder_GetFrame(decoder, 0, &frame);
    ok(hr == S_OK, "GetFrame failed, hr=%x\n", hr);

    hr = IWICBitmapFrameDecode_QueryInterface(frame, &IID_IWICMetadataBlockReader, (void**)&blockreader);
    ok(hr == S_OK, "QueryInterface failed, hr=%x\n", hr);

    if (SUCCEEDED(hr))
    {
        hr = IWICMetadataBlockReader_GetContainerFormat(blockreader, NULL);
        ok(hr == E_INVALIDARG, "GetContainerFormat failed, hr=%x\n", hr);

        hr = IWICMetadataBlockReader_GetContainerFormat(blockreader, &containerformat);
        ok(hr == S_OK, "GetContainerFormat failed, hr=%x\n", hr);
        ok(IsEqualGUID(&containerformat, &GUID_ContainerFormatPng), "unexpected container format\n");

        hr = IWICMetadataBlockReader_GetCount(blockreader, NULL);
        todo_wine ok(hr == E_INVALIDARG, "GetCount failed, hr=%x\n", hr);

        hr = IWICMetadataBlockReader_GetCount(blockreader, &count);
        todo_wine ok(hr == S_OK, "GetCount failed, hr=%x\n", hr);
        todo_wine ok(count == 1, "unexpected count %d\n", count);

        if (0)
        {
            /* Crashes on Windows XP */
            hr = IWICMetadataBlockReader_GetReaderByIndex(blockreader, 0, NULL);
            ok(hr == E_INVALIDARG, "GetReaderByIndex failed, hr=%x\n", hr);
        }

        hr = IWICMetadataBlockReader_GetReaderByIndex(blockreader, 0, &reader);
        todo_wine ok(hr == S_OK, "GetReaderByIndex failed, hr=%x\n", hr);

        if (SUCCEEDED(hr))
        {
            hr = IWICMetadataReader_GetMetadataFormat(reader, &containerformat);
            ok(IsEqualGUID(&containerformat, &GUID_MetadataFormatChunktIME) ||
               broken(IsEqualGUID(&containerformat, &GUID_MetadataFormatUnknown)) /* Windows XP */,
               "unexpected container format\n");

            IWICMetadataReader_Release(reader);
        }

        hr = IWICMetadataBlockReader_GetReaderByIndex(blockreader, 1, &reader);
        todo_wine ok(hr == WINCODEC_ERR_VALUEOUTOFRANGE, "GetReaderByIndex failed, hr=%x\n", hr);

        IWICMetadataBlockReader_Release(blockreader);
    }

    IWICBitmapFrameDecode_Release(frame);

    IWICBitmapDecoder_Release(decoder);

    IWICStream_Release(stream);
}

START_TEST(metadata)
{
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    test_metadata_unknown();
    test_metadata_tEXt();
    test_metadata_IFD();
    test_metadata_Exif();
    test_create_reader();
    test_metadata_png();

    CoUninitialize();
}
