#include <jeffe_lang.h>

const char *jeffe_builtin_strerror(int8_t err)
{
    switch (err)
    {
    case JEFFE_ERRNO_NOTIMPL: return "Not implemented";
    default: return "skibidi toilet rizz ohio gyatt sigma";
    }
}