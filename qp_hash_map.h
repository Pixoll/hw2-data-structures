#pragma once

#include "map_adt.h"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <vector>

using namespace std;

/**
 * Quadratic Probing Hash Map
 */
template <typename K, typename V> class qp_hash_map : virtual public map_adt<K, V> {
  private:
    /**
     * key-value pair node
     */
    class hash_node {
      public:
        K key;
        V value;

        hash_node(K key, V value) : key(key), value(value) {}
    };

    /** Target load factor */
    constexpr static const double LOAD_FACTOR_THRESHOLD = 0.75;

    /** Max size of the table */
    uint32 max_size;
    /** Size threshold at which the table should be rehashed */
    uint32 size_threshold;
    /** Table where all the nodes reside */
    vector<hash_node *> table;
    /** Current size of the table */
    uint32 current_size = 0;
    /** Hash function to calculate the initial index to insert the value at */
    function<int(K)> hash_fn;

  public:
    /** Constructor that takes the hash function as a parameter */
    qp_hash_map(uint32 initial_size, function<int(K)> hash_fn)
        : max_size(initial_size), size_threshold(initial_size * LOAD_FACTOR_THRESHOLD), table(initial_size, nullptr),
          hash_fn(hash_fn) {
        if (hash_fn == nullptr) {
            cerr << "hash_fn cannot be null." << endl;
            exit(1);
        }
    }

    /** Deconstructor, frees allocated memory */
    ~qp_hash_map() {
        this->clear();
    }

    /** Get the value paired with the key */
    V get(K key) {
        const int index = this->hash_fn(key) % this->max_size;
        uint32 counter  = 0;

        hash_node *node = this->table[index];

        // Stop once we run through the entire table or find a match
        while (counter <= this->max_size && (node != nullptr ? node->key != key : true)) {
            counter++;
            const int new_index = (index + (counter * counter)) % this->max_size;
            node                = this->table[new_index];
        }

        return node != nullptr ? node->value : nullptr;
    }

    /** Insert a key-value pair */
    V put(K key, V value) {
        if (this->current_size >= this->size_threshold) {
            cout << "[qp] passed load factor threshold, rehashing" << endl;
            this->rehash(this->current_size * 2);
        }

        const int hash_index = this->hash_fn(key) % this->max_size;
        int insert_index     = hash_index;
        int counter          = 0;

        hash_node *node = this->table[hash_index];

        // Stop once we find an empty node or a match
        while (node != nullptr && node->key != key) {
            counter++;
            insert_index = (hash_index + (counter * counter)) % this->max_size;
            node         = this->table[insert_index];
        }

        // Found empty node
        if (node == nullptr) {
            this->table[insert_index] = new hash_node(key, value);
            this->current_size++;
            return nullptr;
        }

        // Match -> override value
        V previous_value = node->value;
        node->value      = value;

        return previous_value;
    }

    /** Remove a key-value pair by it's key */
    V remove(K key) {
        const int hash_index = this->hash_fn(key) % this->max_size;
        int value_index      = hash_index;
        uint32 counter       = 0;

        hash_node *node = this->table[hash_index];

        // Stop once we run through the entire table or find a match
        while (counter <= this->max_size && (node != nullptr ? node->key != key : true)) {
            counter++;
            value_index = (hash_index + (counter * counter)) % this->max_size;
            node        = this->table[value_index];
        }

        // No match
        if (node == nullptr)
            return nullptr;

        // Match -> delete node
        V value = node->value;

        this->table[value_index] = nullptr;
        delete node;
        this->current_size--;

        return value;
    }

    /** Get the current size of the map */
    uint32 size() {
        return this->current_size;
    }

    /** Whether the map is empty */
    bool empty() {
        return this->current_size == 0;
    }

    /** Clear the map - frees allocated memory */
    void clear() {
        for (uint32 i = 0; i < this->max_size; i++) {
            hash_node *node = this->table[i];
            if (node == nullptr)
                continue;

            this->table[i] = nullptr;
            delete node;
        }

        this->current_size = 0;
    }

    /**
     * Rehash table for new target size
     * If the size isn't a prime, the next prime is selected
     * Very costly operation, can be avoided by choosing an appropriate initial size
     * Can also end up being recursive if there's integer overflow
     */
    void rehash(uint32 size) {
        // Move, don't copy
        vector<hash_node *> nodes = move(this->table);

        for (uint32 i = 0; i < this->max_size; i++) {
            this->table[i] = nullptr;
        }

        // Calculate new size and apply
        const uint32 new_size = find_next_prime(size);
        this->table.resize(new_size);
        this->current_size   = 0;
        this->max_size       = new_size;
        this->size_threshold = new_size * LOAD_FACTOR_THRESHOLD;

        // Reinsert nodes
        for (hash_node *node : nodes)
            if (node != nullptr)
                this->put(node->key, node->value);
    }

    /**
     * Vector with all the stored keys
     * Does not guarantee the same order as they were inserted
     */
    vector<K> keys() {
        vector<K> result;

        for (hash_node *node : this->table)
            if (node != nullptr)
                result.push_back(node->key);

        return result;
    }

    /**
     * Vector with all the stored values
     * Does not guarantee the same order as they were inserted
     */
    vector<V> values() {
        vector<V> result;

        for (hash_node *node : this->table)
            if (node != nullptr)
                result.push_back(node->value);

        return result;
    }

    /** Print information about the hash map */
    void info(stringstream &out) {
        out << "[qp] map info:\n"
            << "max size: " << this->max_size << "\n"
            << "size: " << this->current_size << "\n"
            << "load factor: " << (double)this->current_size / this->max_size << "\n"
            << "size in memory: "
            << sizeof(*this)
                   + this->current_size
                         * (sizeof(hash_node *) + sizeof(hash_node) + (is_pointer<V>::value ? sizeof(*(V){nullptr}) : 0))
            << " B\n"
            << endl;
    }
};
