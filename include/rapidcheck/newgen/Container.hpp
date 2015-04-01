#pragma once

#include "rapidcheck/newgen/Arbitrary.h"
#include "rapidcheck/newgen/Tuple.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/newgen/detail/ShrinkValueIterator.h"

namespace rc {
namespace newgen {
namespace detail {

RC_SFINAE_TRAIT(IsAssociativeContainer, typename T::key_type)
RC_SFINAE_TRAIT(IsMapContainer, typename T::mapped_type)

template<typename T>
using Shrinkables = std::vector<Shrinkable<T>>;

template<typename K, typename V>
using ShrinkablePairs = std::vector<Shrinkable<std::pair<K, V>>>;

template<typename Container, typename T>
Container toContainer(const Shrinkables<T> &shrinkables)
{
    return Container(makeShrinkValueIterator(begin(shrinkables)),
                     makeShrinkValueIterator(end(shrinkables)));
}

template<typename T, typename Predicate>
Shrinkables<T> generateShrinkables(const Random &random,
                                   int size,
                                   int count,
                                   const Gen<T> &gen,
                                   Predicate predicate)
{
    Random r(random);
    Shrinkables<T> shrinkables;
    shrinkables.reserve(count);

    int currentSize = size;
    while (shrinkables.size() < count) {
        auto shrinkable = gen(r.split(), currentSize);
        if (predicate(shrinkable)) {
            shrinkables.push_back(std::move(shrinkable));
        } else {
            // TODO give up eventually
            currentSize++;
        }
    }

    return shrinkables;
}

template<typename Container>
struct GenerateCollection
{
    template<typename T>
    static Shrinkables<T> generateElements(const Random &random,
                                           int size,
                                           int count,
                                           const Gen<T> &gen)
    {
        return generateShrinkables(
            random,
            size,
            count,
            gen,
            fn::constant(true));
    }

    template<typename T>
    static Seq<Shrinkables<T>> shrinkElements(const Shrinkables<T> &shrinkables)
    {
        return shrink::newEachElement(
            shrinkables, [](const Shrinkable<T> &s) {
                return s.shrinks();
            });
    }
};

template<typename Container,
         bool = IsAssociativeContainer<Container>::value,
         bool = IsMapContainer<Container>::value>
struct GenerateContainer : public GenerateCollection<Container> {};

template<typename Set>
struct GenerateContainer<Set, true, false>
{
    template<typename T>
    static Shrinkables<T> generateElements(const Random &random,
                                           int size,
                                           int count,
                                           const Gen<T> &gen)
    {
        Set set;
        return generateShrinkables(
            random, size, count, gen,
            [&](const Shrinkable<T> &s) {
                // We want only values that can be inserted
                return set.insert(s.value()).second;
            });
    }

    template<typename T>
    static Seq<Shrinkables<T>> shrinkElements(const Shrinkables<T> &shrinkables)
    {
        // We use a shared_ptr here both because T might not be copyable and
        // because we don't really need to copy it since we don't modify it.
        std::shared_ptr<const Set> set = std::make_shared<Set>(
            toContainer<Set>(shrinkables));
        return shrink::newEachElement(
            shrinkables, [=](const Shrinkable<T> &s) {
                return seq::filter(
                    s.shrinks(),
                    [=](const Shrinkable<T> &x) {
                        // Here we filter out shrinks that collide with
                        // another value in the set because that would
                        // produce an identical set.
                        return set->find(x.value()) == set->end();
                    });
            });
    }
};

template<typename Map>
struct GenerateContainer<Map, true, true>
{
    template<typename K, typename V>
    static ShrinkablePairs<K, V> generateElements(
        const Random &random,
        int size,
        int count,
        const Gen<K> &keyGen,
        const Gen<V> &valueGen)
    {
        Random r(random);
        Map map;
        auto dummyValue = valueGen(Random(), 0);
        auto keyShrinkables = generateShrinkables(
            r.split(), size, count, keyGen,
            [&](const Shrinkable<K> &s) {
                // We want only keys that can be inserted
                return map.insert(
                    std::make_pair(s.value(), dummyValue.value())).second;
            });

        auto valueShrinkables = generateShrinkables(
            r, size, count, valueGen, fn::constant(true));

        ShrinkablePairs<K, V> shrinkablePairs;
        shrinkablePairs.reserve(count);
        for (std::size_t i = 0; i < count; i++) {
            shrinkablePairs.push_back(
                shrinkable::pair(
                    std::move(keyShrinkables[i]),
                    std::move(valueShrinkables[i])));
        }

        return shrinkablePairs;
    }

    template<typename K, typename V>
    static Seq<ShrinkablePairs<K, V>> shrinkElements(
        const ShrinkablePairs<K, V> &shrinkablePairs)
    {
        // We use a shared_ptr here both because K and V might not be copyable
        // and because we don't really need to copy it since we don't modify it.
        std::shared_ptr<const Map> map = std::make_shared<Map>(
            toContainer<Map>(shrinkablePairs));
        return shrink::newEachElement(
            shrinkablePairs, [=](const Shrinkable<std::pair<K, V>> &elem) {
                return seq::filter(
                    elem.shrinks(),
                    [=](const Shrinkable<std::pair<K, V>> &elemShrink) {
                        // Here we filter out values with keys that collide
                        // with other keys of the map. However, if the key
                        // is the same, that means that something else
                        // in this shrink since we expect shrinks to not
                        // equal the original.
                        // NOTE: This places the restriction that the key must
                        // have an equality operator that works but that's
                        // usually true for types used as keys anyway.
                        const auto shrinkValue = elemShrink.value();
                        return
                            (map->find(shrinkValue.first) == map->end()) ||
                            (shrinkValue.first == elem.value().first);
                    });
            });
    }
};

template<typename MultiMap>
struct GenerateMultiMap : public GenerateCollection<MultiMap>
{
    template<typename K, typename V>
    static ShrinkablePairs<K, V> generateElements(
        const Random &random,
        int size,
        int count,
        const Gen<K> &keyGen,
        const Gen<V> &valueGen)
    {
        // We treat this as a normal collection since we don't need to worry
        // about duplicate keys et.c.
        return GenerateCollection<MultiMap>::generateElements(
            random, size, count, newgen::pair(keyGen, valueGen));
    }
};

template<typename ...Args>
struct GenerateContainer<std::multiset<Args...>, true, false>
    : public GenerateCollection<std::multiset<Args...>> {};

template<typename ...Args>
struct GenerateContainer<std::unordered_multiset<Args...>, true, false>
    : public GenerateCollection<std::unordered_multiset<Args...>> {};

template<typename ...Args>
struct GenerateContainer<std::multimap<Args...>, true, true>
    : public GenerateMultiMap<std::multimap<Args...>> {};

template<typename ...Args>
struct GenerateContainer<std::unordered_multimap<Args...>, true, true>
    : public GenerateMultiMap<std::unordered_multimap<Args...>> {};


template<typename Container>
struct ContainerArbitrary1
{
    static Gen<Container> arbitrary()
    {
        return newgen::container<Container>(
            newgen::arbitrary<typename Container::value_type>());
    }
};

template<typename Container>
struct ContainerArbitrary2
{
    static Gen<Container> arbitrary()
    {
        return newgen::container<Container>(
            newgen::arbitrary<typename Container::key_type>(),
            newgen::arbitrary<typename Container::mapped_type>());
    }
};

#define SPECIALIZE_SEQUENCE_ARBITRARY1(Container)                       \
    template<typename ...Args>                                          \
    class DefaultArbitrary<Container<Args...>>                          \
        : public newgen::detail::ContainerArbitrary1<Container<Args...>> {};


#define SPECIALIZE_SEQUENCE_ARBITRARY2(Container)                       \
    template<typename ...Args>                                          \
    class DefaultArbitrary<Container<Args...>>                          \
        : public newgen::detail::ContainerArbitrary2<Container<Args...>> {};

SPECIALIZE_SEQUENCE_ARBITRARY1(std::vector)
SPECIALIZE_SEQUENCE_ARBITRARY1(std::deque)
SPECIALIZE_SEQUENCE_ARBITRARY1(std::forward_list)
SPECIALIZE_SEQUENCE_ARBITRARY1(std::list)
SPECIALIZE_SEQUENCE_ARBITRARY1(std::set)
SPECIALIZE_SEQUENCE_ARBITRARY1(std::multiset)
SPECIALIZE_SEQUENCE_ARBITRARY1(std::unordered_set)
SPECIALIZE_SEQUENCE_ARBITRARY1(std::unordered_multiset)

SPECIALIZE_SEQUENCE_ARBITRARY2(std::map)
SPECIALIZE_SEQUENCE_ARBITRARY2(std::multimap)
SPECIALIZE_SEQUENCE_ARBITRARY2(std::unordered_map)
SPECIALIZE_SEQUENCE_ARBITRARY2(std::unordered_multimap)

#undef SPECIALIZE_SEQUENCE_ARBITRARY1
#undef SPECIALIZE_SEQUENCE_ARBITRARY2

} // namespace detail

template<typename Container, typename ...Ts>
Gen<Container> container(Gen<Ts> ...gens)
{
    using namespace detail;
    using Generate = GenerateContainer<Container>;

    return [=](const Random &random, int size) {
        Random r(random);
        int count = r.split().next() % (size + 1);
        auto shrinkables = Generate::generateElements(
            r, size, count, std::move(gens)...);

        using Elements = decltype(shrinkables);
        return shrinkable::map(
            shrinkable::shrinkRecur(
                std::move(shrinkables), [](const Elements &elements) {
                    return seq::concat(
                        shrink::newRemoveChunks(elements),
                        Generate::shrinkElements(elements));
                }),
            &toContainer<Container, typename Elements::value_type::ValueType>);
    };
}

} // namespace newgen
} // namespace rc
