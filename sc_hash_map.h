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
 * Separate Chaining Hash Map
 */
template <typename K, typename V> class sc_hash_map : virtual public map_adt<K, V> {
  private:
    /**
     * key-value pair node
     * Includes a pointer to the next node to act as a linked list
     */
    class hash_node {
      public:
        K key;
        V value;
        hash_node *next;

        hash_node(K key, V value) : key(key), value(value) {
            this->next = nullptr;
        }
    };

    /** Target load factor */
    constexpr static const double LOAD_FACTOR_THRESHOLD = 1.0;

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

    /** Recursively frees the memory of a linked list from the tail node to the root */
    static void destroy_list(hash_node *node) {
        if (node->next == nullptr)
            return;

        destroy_list(node->next);
        delete node->next;
    }

  public:
    /** Constructor that takes the hash function as a parameter */
    sc_hash_map(uint32 initial_size, function<int(K)> hash_fn)
        : max_size(initial_size), size_threshold(initial_size * LOAD_FACTOR_THRESHOLD), table(initial_size, nullptr),
          hash_fn(hash_fn) {
        if (hash_fn == nullptr) {
            cerr << "hash_fn cannot be null." << endl;
            exit(1);
        }
    }

    /** Deconstructor, frees allocated memory */
    ~sc_hash_map() {
        this->clear();
    }

    /** Get the value paired with the key */
    V get(K key) {
        const int index = this->hash_fn(key) % this->max_size;

        hash_node *node = this->table[index];

        // Stop once we run through the entire table or find a match
        while (node != nullptr && node->key != key) {
            node = node->next;
        }

        return node != nullptr ? node->value : nullptr;
    }

    /** Insert a key-value pair */
    V put(K key, V value) {
        if (this->current_size >= this->size_threshold) {
            cout << "[sc] passed load factor threshold, rehashing" << endl;
            this->rehash(this->current_size * 2);
        }

        const int index = this->hash_fn(key) % this->max_size;

        hash_node *destination = this->table[index];

        // Create new list if bucket is empty
        if (destination == nullptr) {
            this->current_size++;
            this->table[index] = new hash_node(key, value);
            return nullptr;
        }

        hash_node *previous_node = nullptr;

        // Stop once we get to the end of the list or find a match
        while (destination != nullptr && destination->key != key) {
            previous_node = destination;
            destination   = destination->next;
        }

        // End of the list
        if (destination == nullptr) {
            this->current_size++;
            previous_node->next = new hash_node(key, value);
            return nullptr;
        }

        // Match -> override value
        V previous_value   = destination->value;
        destination->value = value;

        return previous_value;
    }

    /** Remove a key-value pair by it's key */
    V remove(K key) {
        const int index = this->hash_fn(key) % this->max_size;

        hash_node *node     = this->table[index];
        hash_node *previous = nullptr;

        // Stop once we run through the entire list or find a match
        while (node != nullptr && node->key != key) {
            previous = node;
            node     = node->next;
        }

        // No match
        if (node == nullptr)
            return nullptr;

        // Match -> delete node from list
        V value = node->value;

        if (previous == nullptr) {
            this->table[index] = node->next;
        } else {
            this->table[index] = previous;
            previous->next     = node->next;
        }

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

            destroy_list(node);
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
        for (hash_node *node : nodes) {
            while (node != nullptr) {
                this->put(node->key, node->value);
                node = node->next;
            }
        }
    }

    /**
     * Vector with all the stored keys
     * Does not guarantee the same order as they were inserted
     */
    vector<K> keys() {
        vector<K> result;

        for (hash_node *node : this->table) {
            while (node != nullptr) {
                result.push_back(node->key);
                node = node->next;
            }
        }

        return result;
    }

    /**
     * Vector with all the stored values
     * Does not guarantee the same order as they were inserted
     */
    vector<V> values() {
        vector<V> result;

        for (hash_node *node : this->table) {
            while (node != nullptr) {
                result.push_back(node->value);
                node = node->next;
            }
        }

        return result;
    }

    /** Print information about the hash map */
    void info(stringstream &out) {
        int max_depth = 0, filled = 0;

        for (hash_node *node : this->table) {
            int depth = 0;
            while (node != nullptr) {
                node = node->next;
                depth++;
            }

            max_depth = max(max_depth, depth);
            if (depth > 0)
                filled++;
        }

        out << "[sc] map info:\n"
            << "max size: " << this->max_size << "\n"
            << "max depth: " << max_depth << " in same bucket" << "\n"
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
