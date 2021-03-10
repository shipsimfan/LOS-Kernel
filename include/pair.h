#pragma once

template <class K, class V> struct Pair {
    Pair(K key, V value) : key(key), value(value) {}

    K key;
    V value;
};