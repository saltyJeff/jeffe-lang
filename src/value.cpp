#include "value.hpp"
#include "cstruct.hpp"
#include "jeffe_lang.h"
#include "objmeta.hpp"

static jeffe_value value_builder(jeffe_typetag tag, int8_t meta, uint64_t payload)
{
    uint64_t v = ((uint64_t)tag << TYPETAG_SHIFT) | (((uint64_t)meta & 0xFF) << 48) | (payload & PAYLOAD_MASK);
    jeffe_value ret;
    ret.v = v;
    return ret;
}

jeffe_value jeffe_value_nil()
{
    return value_builder(JEFFE_TYPETAG_NIL, 0, 0);
}
jeffe_value jeffe_value_char(char32_t c)
{
    return value_builder(JEFFE_TYPETAG_CHAR, 0, (uint32_t)c);
}
jeffe_value jeffe_value_bool(bool b)
{
    return value_builder(JEFFE_TYPETAG_BOOL, 0, (int)b);
}
jeffe_value jeffe_value_i32(int32_t i)
{
    return value_builder(JEFFE_TYPETAG_I32, 0, i);
}
jeffe_value jeffe_value_u32(uint32_t u)
{
    return value_builder(JEFFE_TYPETAG_U32, 0, u);
}
jeffe_value jeffe_value_f32(float f)
{
    union {
        float x;
        uint32_t y;
    } pun;
    pun.x = f;
    return value_builder(JEFFE_TYPETAG_F32, 0, pun.y);
}
jeffe_value jeffe_value_f64(double d)
{
    double *dptr = new double(d);
    return value_builder(JEFFE_TYPETAG_F64, 0, ptr_compress(dptr));
}
jeffe_value jeffe_value_i64(int64_t l)
{
    int64_t *lptr = new int64_t(l);
    return value_builder(JEFFE_TYPETAG_I64, 0, ptr_compress(lptr));
}
jeffe_value jeffe_value_u64(uint64_t ul)
{
    uint64_t *ulptr = new uint64_t(ul);
    return value_builder(JEFFE_TYPETAG_U64, 0, ptr_compress(ulptr));
}
jeffe_value jeffe_value_ptr(void *ptr)
{
    return value_builder(JEFFE_TYPETAG_PTR, 0, ptr_compress(ptr));
}

jeffe_value jeffe_value_cstruct(size_t sz)
{
    return value_builder(JEFFE_TYPETAG_CSTRUCT, 0, ptr_compress(cstruct_new(sz)));
}
jeffe_value jeffe_value_errnum(int8_t err, jeffe_strerror_fn fn)
{
    return value_builder(JEFFE_TYPETAG_ERRNUM, err, ptr_compress(reinterpret_cast<void *>(fn)));
}
jeffe_value jeffe_value_obj(jeffe_class_fn fn, size_t argn, const jeffe_value *argv)
{
    objmeta *meta = new objmeta(fn);
    jeffe_value ctor_ret = meta->invoke(JEFFE_OP_CTOR, argn, argv);
    // TODO: check ctor_ret is error
    return value_builder(JEFFE_TYPETAG_OBJ, 0, ptr_compress(meta->userdata_ptr()));
}
jeffe_typetag jeffe_value_type(jeffe_value v)
{
    return static_cast<jeffe_typetag>((v.v >> TYPETAG_SHIFT) & 0xFF);
}
