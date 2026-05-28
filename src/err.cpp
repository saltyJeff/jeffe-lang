#include <jeffe_lang.h>

const char *jeffe_builtin_strerror(int8_t err)
{
    switch (err)
    {
    case JEFFE_ERRNO_UNDEFINED: return "Undefined error";
    case JEFFE_ERRNO_NOTIMPL: return "Not implemented";
    case JEFFE_ERRNO_ARGTYPE: return "Invalid argument type";
    case JEFFE_ERRNO_OUTOFBOUNDS: return "Index out of bounds";
    case JEFFE_ERRNO_NOTORDERED: return "Not ordered";
    default: return "skibidi toilet rizz ohio gyatt sigma";
    }
}