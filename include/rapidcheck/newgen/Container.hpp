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
    static Seq<Shrinkables<T>> shrinksOf(const Shrinkables<T> &shrinkables)
    {
        return seq::concat(
            shrink::newRemoveChunks(shrinkables),
            shrink::newEachElement(
                shrinkables, [](const Shrinkable<T> &s) {
                    return s.shrinks();
                }));
    }

    template<typename T>
    static Shrinkable<Container> generate(const Random &random,
                                          int size,
                                          int count,
                                          const Gen<T> &gen)
    {
        return shrinkable::map(
            shrinkable::shrinkRecur(
                generateShrinkables(
                    random,
                    size,
                    count,
                    gen,
                    fn::constant(true)),
                &shrinksOf<T>),
            &toContainer<Container, T>);
    };
};

template<typename Container,
         bool = IsAssociativeContainer<Container>::value,
         bool = IsMapContainer<Container>::value>
struct GenerateContainer : public GenerateCollection<Container> {};

template<typename Set>
struct GenerateContainer<Set, true, false>
{
    template<typename T>
    static Seq<Shrinkables<T>> shrinksOf(const Shrinkables<T> &shrinkables)
    {
        // We use a shared_ptr here both because T might not be copyable and
        // because we don't really need to copy it since we don't modify it.
        std::shared_ptr<const Set> set = std::make_shared<Set>(
            toContainer<Set>(shrinkables));
        return seq::concat(
            shrink::newRemoveChunks(shrinkables),
            shrink::newEachElement(
                shrinkables, [=](const Shrinkable<T> &s) {
                    return seq::filter(
                        s.shrinks(),
                        [=](const Shrinkable<T> &x) {
                            // Here we filter out shrinks that collide with
                            // another value in the set because that would
                            // produce an identical set.
                            return set->find(x.value()) == set->end();
                        });
                }));
    }

    template<typename T>
    static Shrinkable<Set> generate(const Random &random,
                                    int size,
                                    int count,
                                    const Gen<T> &gen)
    {
        Set set;
        auto shrinkables = generateShrinkables(
            random, size, count, gen,
            [&](const Shrinkable<T> &s) {
                // We want only values that can be inserted
                return set.insert(s.value()).second;
            });

        return shrinkable::map(
            shrinkable::shrinkRecur(
                std::move(shrinkables),
                &shrinksOf<T>),
            &toContainer<Set, T>);
    };
};

template<typename Map>
struct GenerateContainer<Map, true, true>
{
    template<typename K, typename V>
    static ShrinkablePairs<K, V> generatePairs(
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
    static Seq<ShrinkablePairs<K, V>> shrinksOf(
        const ShrinkablePairs<K, V> &shrinkablePairs)
    {
        // We use a shared_ptr here both because K and V might not be copyable
        // and because we don't really need to copy it since we don't modify it.
        std::shared_ptr<const Map> map = std::make_shared<Map>(
            toContainer<Map>(shrinkablePairs));
        return seq::concat(
            shrink::newRemoveChunks(shrinkablePairs),
            shrink::newEachElement(
                shrinkablePairs, [=](const Shrinkable<std::pair<K, V>> &elem) {
                    return seq::filter(
                        elem.shrinks(),
                        [=](const Shrinkable<std::pair<K, V>> &elemShrink) {
                            // Here we filter out values with keys that collide
                            // with other keys of the map. However, if the key
                            // is the same, that means that something else
                            // in this shrink since we expect shrinks to not
                            // equal the original.
                            const auto shrinkValue = elemShrink.value();
                            return
                                (map->find(shrinkValue.first) == map->end()) ||
                                (shrinkValue.first == elem.value().first);
                        });
                }));
    }

    template<typename K, typename V>
    static Shrinkable<Map> generate(
        const Random &random,
        int size,
        int count,
        const Gen<K> &keyGen,
        const Gen<V> &valueGen)
    {
        return shrinkable::map(
            shrinkable::shrinkRecur(
                generatePairs(random, size, count, keyGen, valueGen),
                &shrinksOf<K, V>),
            &toContainer<Map, std::pair<K, V>>);
    };
};

template<typename MultiMap>
struct GenerateMultiMap
{
    template<typename K, typename V>
    static Shrinkable<MultiMap> generate(
        const Random &random,
        int size,
        int count,
        const Gen<K> &keyGen,
        const Gen<V> &valueGen)
    {
        // We treat this as a normal collection since we don't need to worry
        // about duplicate keys et.c.
        return GenerateCollection<MultiMap>::generate(
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
    return [=](const Random &random, int size) {
        Random r(random);
        int count = r.split().next() % (size + 1);
        return detail::GenerateContainer<Container>::generate(
            random, size, count, std::move(gens)...);
    };
}

} // namespace newgen
} // namespace rc
