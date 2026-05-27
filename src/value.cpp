#include "value.hpp"
#include "cstruct.hpp"
#include "jeffe_lang.h"
#include "objmeta.hpp"

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
    union
    {
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
jeffe_typetag jeffe_value_typetag(jeffe_value v)
{
    return static_cast<jeffe_typetag>((v.v >> TYPETAG_SHIFT) & 0xFF);
}
jeffe_value_holder jeffe_value_held_data(jeffe_value v)
{
    jeffe_value_holder holder;
    holder.typetag = jeffe_value_typetag(v);
    switch (holder.typetag)
    {
    case JEFFE_TYPETAG_NIL: break;
    case JEFFE_TYPETAG_CHAR: holder.c = static_cast<char32_t>(v.v & PAYLOAD_MASK); break;
    case JEFFE_TYPETAG_BOOL: holder.b = static_cast<bool>(v.v & PAYLOAD_MASK); break;
    case JEFFE_TYPETAG_I32: holder.i32 = static_cast<int32_t>(v.v & PAYLOAD_MASK); break;
    case JEFFE_TYPETAG_U32: holder.u32 = static_cast<uint32_t>(v.v & PAYLOAD_MASK); break;
    case JEFFE_TYPETAG_F32:
    {
        union
        {
            uint32_t y;
            float x;
        } pun;
        pun.y = static_cast<uint32_t>(v.v & PAYLOAD_MASK);
        holder.f32 = pun.x;
        break;
    }
    case JEFFE_TYPETAG_F64: holder.f64 = *reinterpret_cast<double *>(ptr_uncompress(v.v & PAYLOAD_MASK)); break;
    case JEFFE_TYPETAG_I64: holder.i64 = *reinterpret_cast<int64_t *>(ptr_uncompress(v.v & PAYLOAD_MASK)); break;
    case JEFFE_TYPETAG_U64: holder.u64 = *reinterpret_cast<uint64_t *>(ptr_uncompress(v.v & PAYLOAD_MASK)); break;
    case JEFFE_TYPETAG_PTR: holder.ptr = ptr_uncompress(v.v & PAYLOAD_MASK); break;
    case JEFFE_TYPETAG_CSTRUCT: holder.cstruct = ptr_uncompress(v.v & PAYLOAD_MASK); break;
    case JEFFE_TYPETAG_ERRNUM:
        holder.errnum.errnum = static_cast<int8_t>((v.v >> 48) & 0xFF);
        holder.errnum.fn = reinterpret_cast<jeffe_strerror_fn>(ptr_uncompress(v.v & PAYLOAD_MASK));
        break;
    case JEFFE_TYPETAG_OBJ:
    {
        objmeta *meta = objmeta::from_value(v);
        holder.obj.userdata = meta->userdata_ptr();
        holder.obj.fn = meta->class_fn();
        break;
    }
    }
    return holder;
}
const char *jeffe_strtypetag(enum jeffe_typetag tag)
{
    switch (tag)
    {
    case JEFFE_TYPETAG_NIL: return "NIL";
    case JEFFE_TYPETAG_CHAR: return "CHAR";
    case JEFFE_TYPETAG_BOOL: return "BOOL";
    case JEFFE_TYPETAG_I32: return "I32";
    case JEFFE_TYPETAG_U32: return "U32";
    case JEFFE_TYPETAG_F32: return "F32";
    case JEFFE_TYPETAG_F64: return "F64";
    case JEFFE_TYPETAG_I64: return "I64";
    case JEFFE_TYPETAG_U64: return "U64";
    case JEFFE_TYPETAG_PTR: return "PTR";
    case JEFFE_TYPETAG_CSTRUCT: return "CSTRUCT";
    case JEFFE_TYPETAG_ERRNUM: return "ERRNUM";
    case JEFFE_TYPETAG_OBJ: return "OBJ";
    default: return "UNKNOWN";
    }
}