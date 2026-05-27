#pragma once
#include <jeffe_lang.h>
#include <string>
#include <sstream>
#include "doctest.h"

namespace doctest {
    template<> struct StringMaker<struct jeffe_value> {
        static String convert(const struct jeffe_value& value) {
            std::ostringstream os;
            os << "jeffe_value{" << jeffe_strtypetag(jeffe_value_typetag(value));
            
            struct jeffe_value_holder h = jeffe_value_held_data(value);
            switch (h.typetag) {
                case JEFFE_TYPETAG_NIL: break;
                case JEFFE_TYPETAG_CHAR: os << ", '" << (char)h.c << "'"; break;
                case JEFFE_TYPETAG_BOOL: os << ", " << (h.b ? "true" : "false"); break;
                case JEFFE_TYPETAG_I32: os << ", " << h.i32; break;
                case JEFFE_TYPETAG_U32: os << ", " << h.u32; break;
                case JEFFE_TYPETAG_F32: os << ", " << h.f32; break;
                case JEFFE_TYPETAG_F64: os << ", " << h.f64; break;
                case JEFFE_TYPETAG_I64: os << ", " << h.i64; break;
                case JEFFE_TYPETAG_U64: os << ", " << h.u64; break;
                case JEFFE_TYPETAG_PTR: os << ", " << h.ptr; break;
                case JEFFE_TYPETAG_CSTRUCT: os << ", " << h.cstruct; break;
                case JEFFE_TYPETAG_ERRNUM: os << ", " << (int)h.errnum.errnum; break;
                case JEFFE_TYPETAG_OBJ: os << ", " << h.obj.userdata; break;
            }
            os << "}";
            std::string result = os.str();
            return doctest::String(result.c_str());
        }
    };
}