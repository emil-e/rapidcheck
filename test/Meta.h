#pragma once

namespace rc {
namespace meta {

template<typename MetaFunction,
         typename ...Types>
struct ForEachType;

template<typename MetaFunction>
struct ForEachType<MetaFunction>
{ static void exec() {} };

template<typename MetaFunction,
         typename Type,
         typename ...Types>
struct ForEachType<MetaFunction, Type, Types...>
{
    static void exec()
    {
        MetaFunction::template exec<Type>();
        ForEachType<MetaFunction, Types...>::exec();
    }
};

template<typename MetaFunction,
         typename ...Types>
void forEachType()
{
    ForEachType<MetaFunction, Types...>::exec();
}

} // namespace meta
} // namespace rc
