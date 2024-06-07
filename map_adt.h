#pragma once

#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

/**
 * Abstract data type describing the base map structure
 */
template <typename K, typename V> class map_adt {
  public:
    virtual V get(K key)             = 0;
    virtual V put(K key, V value)    = 0;
    virtual V remove(K key)          = 0;
    virtual uint32 size()            = 0;
    virtual bool empty()             = 0;
    virtual void clear()             = 0;
    virtual void rehash(uint32 size) = 0;
    virtual vector<K> keys()         = 0;
    virtual vector<V> values()       = 0;
};

bool is_prime(uint32 n) {
    if (n < 3)
        return n == 2;
    if (n % 2 == 0)
        return false;

    const uint32 limit = sqrt(n);
    for (uint32 i = 3; i <= limit; i += 2)
        if (n % i == 0)
            return false;

    return true;
}

inline uint32 find_next_prime(uint32 n) {
    while (!is_prime(n))
        n++;
    return n;
}
