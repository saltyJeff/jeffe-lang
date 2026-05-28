#include <jeffe_lang.h>

struct jeffe_tuple_data
{
public:
    size_t len;
    jeffe_value *vals;
    jeffe_tuple_data(size_t argn, const jeffe_value *argv)
    {
        this->len = argn;
        vals = new jeffe_value[argn];
        for (size_t i = 0; i < argn; i++)
        {
            this->vals[i] = jeffe_copy(argv[i]);
        }
    }
    ~jeffe_tuple_data()
    {
        for (size_t i = 0; i < len; i++)
        {
            jeffe_destroy(vals[i]);
        }
        delete[] vals;
    }
};

struct jeffe_value jeffe_tuple_class_fn(void **userdata, int op, size_t argn, const jeffe_value *argv)
{
    switch (op)
    {
    case JEFFE_OP_CTOR:
    {
        *userdata = new jeffe_tuple_data(argn, argv);
        return jeffe_value_nil();
    }
    case JEFFE_OP_DTOR:
    {
        delete static_cast<jeffe_tuple_data *>(*userdata);
        return jeffe_value_nil();
    }
    case JEFFE_OP_GET:
    {
        if (argn != 1)
        {
            return jeffe_value_errnum(JEFFE_ERRNO_ARGTYPE, jeffe_builtin_strerror);
        }
        intptr_t idx;
        if (!jeffe_as_index(argv[0], &idx))
        {
            return jeffe_value_errnum(JEFFE_ERRNO_ARGTYPE, jeffe_builtin_strerror);
        }
        jeffe_tuple_data *data = static_cast<jeffe_tuple_data *>(*userdata);
        if (idx < 0 || idx >= static_cast<intptr_t>(data->len))
        {
            return jeffe_value_errnum(JEFFE_ERRNO_OUTOFBOUNDS, jeffe_builtin_strerror);
        }
        return jeffe_copy(data->vals[idx]);
    }
    case JEFFE_OP_CLASSNAME:
    {
        return jeffe_value_ptr(const_cast<char *>("tuple"));
    }
    case JEFFE_OP_LEN:
    {
        jeffe_tuple_data *data = static_cast<jeffe_tuple_data *>(*userdata);
        return jeffe_value_u64(data->len);
    }
    case JEFFE_OP_ITER:
    {
        if (argn != 1)
        {
            return jeffe_value_errnum(JEFFE_ERRNO_ARGTYPE, jeffe_builtin_strerror);
        }
        jeffe_tuple_data *data = static_cast<jeffe_tuple_data *>(*userdata);
        if (data->len == 0)
        {
            return jeffe_value_nil();
        }
        intptr_t idx = 0;
        jeffe_value_holder holder = jeffe_value_held_data(argv[0]);
        if (holder.typetag != JEFFE_TYPETAG_NIL)
        {
            if (!jeffe_as_index(argv[0], &idx))
            {
                return jeffe_value_errnum(JEFFE_ERRNO_ARGTYPE, jeffe_builtin_strerror);
            }
            if (idx < 0 || idx >= static_cast<intptr_t>(data->len))
            {
                return jeffe_value_nil();
            }
        }
        jeffe_value elements[2];
        elements[0] = jeffe_copy(data->vals[idx]);
        if (idx == static_cast<intptr_t>(data->len) - 1)
        {
            elements[1] = jeffe_value_nil();
        }
        else
        {
            elements[1] = jeffe_value_i32(static_cast<int32_t>(idx + 1));
        }
        jeffe_value ret_tuple = jeffe_tuple(2, elements);
        jeffe_destroy(elements[0]);
        jeffe_destroy(elements[1]);
        return ret_tuple;
    }
    case JEFFE_OP_CMP:
    {
        if (argn != 1)
        {
            return jeffe_value_errnum(JEFFE_ERRNO_ARGTYPE, jeffe_builtin_strerror);
        }
        jeffe_value_holder rhs_holder = jeffe_value_held_data(argv[0]);
        if (rhs_holder.typetag != JEFFE_TYPETAG_OBJ || rhs_holder.obj.fn != jeffe_tuple_class_fn)
        {
            return jeffe_value_errnum(JEFFE_ERRNO_NOTIMPL, jeffe_builtin_strerror);
        }
        jeffe_tuple_data *dataA = static_cast<jeffe_tuple_data *>(*userdata);
        jeffe_tuple_data *dataB = static_cast<jeffe_tuple_data *>(*rhs_holder.obj.userdata);

        size_t min_len = (dataA->len < dataB->len) ? dataA->len : dataB->len;
        for (size_t i = 0; i < min_len; i++)
        {
            jeffe_value cmp_res = jeffe_cmp(dataA->vals[i], dataB->vals[i]);
            jeffe_value_holder cmp_holder = jeffe_value_held_data(cmp_res);
            if (cmp_holder.typetag == JEFFE_TYPETAG_ERRNUM)
            {
                return cmp_res;
            }
            if (cmp_holder.i32 != 0)
            {
                return cmp_res;
            }
        }
        if (dataA->len < dataB->len) return jeffe_value_i32(-1);
        if (dataA->len > dataB->len) return jeffe_value_i32(1);
        return jeffe_value_i32(0);
    }
    }
    return jeffe_value_errnum(JEFFE_ERRNO_NOTIMPL, jeffe_builtin_strerror);
}
struct jeffe_value jeffe_tuple(size_t argn, const struct jeffe_value *argv)
{
    return jeffe_value_obj(jeffe_tuple_class_fn, argn, argv);
}