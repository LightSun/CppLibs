#include "type.h"

namespace Zafkiel::Reflection
{
std::string Numeric::GetNameOfKind(Kind kind)
{
    switch (kind)
    {
    case Kind::Int8: return "Int8";
    case Kind::Int16: return "Int16";
    case Kind::Int32: return "Int32";
    case Kind::Int64: return "Int64";
    case Kind::Float: return "Float";
    case Kind::Double: return "Double";
    default: return "Unknown";
    }
}

Class &Class::AddProperty(const std::shared_ptr<Property> &prop)
{
    properties.emplace_back(prop);
    return *this;
}

}
