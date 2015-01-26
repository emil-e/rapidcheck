#pragma once

namespace rc {

template<typename T>
class Arbitrary : public gen::Generator<T>
{
public:
    static_assert(std::is_integral<T>::value,
                  "No specialization of Arbitrary for type");

    T generate() const override
    {
        using namespace detail;

        int size = std::min(gen::currentSize(), gen::kNominalSize);
        RandomEngine::Atom r;
        // TODO this switching shouldn't be done here. pickAtom?
        ImplicitParam<param::CurrentNode> currentNode;
        if (*currentNode != nullptr) {
            r = currentNode->atom();
        } else {
            ImplicitParam<param::RandomEngine> randomEngine;
            r = randomEngine->nextAtom();
        }

        // We vary the size by using different number of bits. This way, we can
        // be sure that the max value can also be generated.
        int nBits = (size * std::numeric_limits<T>::digits) / gen::kNominalSize;
        if (nBits == 0)
            return 0;
        constexpr RandomEngine::Atom randIntMax =
            std::numeric_limits<RandomEngine::Atom>::max();
        RandomEngine::Atom mask = ~((randIntMax - 1) << (nBits - 1));

        T x = static_cast<T>(r & mask);
        if (std::numeric_limits<T>::is_signed)
        {
            // Use the topmost bit as the signed bit. Even in the case of a
            // signed 64-bit integer, it won't be used since it actually IS the
            // sign bit.
            constexpr int basicBits =
                std::numeric_limits<RandomEngine::Atom>::digits;
            x *= ((r >> (basicBits - 1)) == 0) ? 1 : -1;
        }

        return x;
    }

    shrink::IteratorUP<T> shrink(T value) const override
    {
        std::vector<T> constants;
        if (value < 0)
            constants.push_back(-value);

        return shrink::sequentially(
            shrink::constant(constants),
            shrink::towards(value, static_cast<T>(0)));
    }
};

// Base for float and double arbitrary instances
template<typename T>
class ArbitraryReal : public gen::Generator<T>
{
public:
    T generate() const override
    {
        int64_t i = *gen::arbitrary<int64_t>();
        T x = static_cast<T>(i) / std::numeric_limits<int64_t>::max();
        return std::pow<T>(1.1, gen::currentSize()) * x;
    }

    shrink::IteratorUP<T> shrink(T value) const override
    {
        std::vector<T> constants;

        if (value < 0)
            constants.push_back(-value);

        T truncated = std::trunc(value);
        if (std::abs(truncated) < std::abs(value))
            constants.push_back(truncated);

        return shrink::constant(constants);
    }
};

template<>
class Arbitrary<float> : public ArbitraryReal<float> {};

template<>
class Arbitrary<double> : public ArbitraryReal<double> {};

template<>
class Arbitrary<bool> : public gen::Generator<bool>
{
public:
    bool generate() const override
    {
        return (*gen::resize(gen::kNominalSize,
                                 gen::arbitrary<uint8_t>()) & 0x1) == 0;
    }

    shrink::IteratorUP<bool> shrink(bool value)
    {
        if (value)
            return shrink::constant<bool>({false});
        else
            return shrink::nothing<bool>();
    }
};

template<typename ...Types>
class Arbitrary<std::tuple<Types...>>
    : public gen::TupleOf<Arbitrary<Types>...>
{
public:
    Arbitrary() : gen::TupleOf<Arbitrary<Types>...>(
            gen::arbitrary<Types>()...) {}
};

template<typename T1, typename T2>
class Arbitrary<std::pair<T1, T2>>
    : public gen::PairOf<Arbitrary<detail::DecayT<T1>>,
                         Arbitrary<detail::DecayT<T2>>>
{
public:
    Arbitrary()
        : gen::PairOf<Arbitrary<detail::DecayT<T1>>,
                      Arbitrary<detail::DecayT<T2>>>(
            gen::arbitrary<detail::DecayT<T1>>(),
            gen::arbitrary<detail::DecayT<T2>>()) {}
};

// Base template class for collection types
template<typename Coll, typename ValueType>
class ArbitraryCollection : public gen::Collection<Coll, Arbitrary<ValueType>>
{
public:
    typedef gen::Collection<Coll, Arbitrary<ValueType>> CollectionGen;
    ArbitraryCollection() : CollectionGen(gen::arbitrary<ValueType>()) {}
};

// std::vector
template<typename T, typename Allocator>
class Arbitrary<std::vector<T, Allocator>>
    : public ArbitraryCollection<std::vector<T, Allocator>, T> {};

// std::deque
template<typename T, typename Allocator>
class Arbitrary<std::deque<T, Allocator>>
    : public ArbitraryCollection<std::deque<T, Allocator>, T> {};

// std::forward_list
template<typename T, typename Allocator>
class Arbitrary<std::forward_list<T, Allocator>>
    : public ArbitraryCollection<std::forward_list<T, Allocator>, T> {};

// std::list
template<typename T, typename Allocator>
class Arbitrary<std::list<T, Allocator>>
    : public ArbitraryCollection<std::list<T, Allocator>, T> {};

// std::set
template<typename Key, typename Compare, typename Allocator>
class Arbitrary<std::set<Key, Compare, Allocator>>
    : public ArbitraryCollection<std::set<Key, Compare, Allocator>, Key> {};

// std::map
template<typename Key, typename T, typename Compare, typename Allocator>
class Arbitrary<std::map<Key, T, Compare, Allocator>>
    : public ArbitraryCollection<std::map<Key, T, Compare, Allocator>,
                                 std::pair<Key, T>> {};

// std::multiset
template<typename Key, typename Compare, typename Allocator>
class Arbitrary<std::multiset<Key, Compare, Allocator>>
    : public ArbitraryCollection<std::multiset<Key, Compare, Allocator>, Key> {};

// std::multimap
template<typename Key, typename T, typename Compare, typename Allocator>
class Arbitrary<std::multimap<Key, T, Compare, Allocator>>
    : public ArbitraryCollection<std::multimap<Key, T, Compare, Allocator>,
                                 std::pair<Key, T>> {};

// std::unordered_set
template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
class Arbitrary<std::unordered_set<Key, Hash, KeyEqual, Allocator>>
    : public ArbitraryCollection<std::unordered_set<Key, Hash, KeyEqual, Allocator>,
                                 Key> {};

// std::unordered_map
template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
class Arbitrary<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>>
    : public ArbitraryCollection<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>,
                                 std::pair<Key, T>> {};

// std::unordered_multiset
template<typename Key, typename Hash, typename KeyEqual, typename Allocator>
class Arbitrary<std::unordered_multiset<Key, Hash, KeyEqual, Allocator>>
    : public ArbitraryCollection<std::unordered_multiset<Key, Hash, KeyEqual, Allocator>,
                                 Key> {};

// std::unordered_multimap
template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
class Arbitrary<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>>
    : public ArbitraryCollection<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>,
                                 std::pair<Key, T>> {};

// std::basic_string
template<typename T, typename Traits, typename Allocator>
class Arbitrary<std::basic_string<T, Traits, Allocator>>
    : public gen::Collection<std::basic_string<T, Traits, Allocator>, gen::Character<T>>
{
public:
    typedef std::basic_string<T, Traits, Allocator> StringType;
    Arbitrary() : gen::Collection<StringType, gen::Character<T>>(
        gen::character<T>()) {}
};

// std::array
template<typename T, std::size_t N>
class Arbitrary<std::array<T, N>>
    : public ArbitraryCollection<std::array<T, N>, T> {};

} // namespace rc
