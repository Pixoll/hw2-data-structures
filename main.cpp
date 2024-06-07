#include "read_csv"
#include "tests"
#include "user"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#define lltoa _i64toa
#endif

using namespace std;

// Sizes for all the hash maps
// #define as they must be available for the preprocessor

#define SC_N 20011 // 14983
#define L_N 27367
#define DH_N 27361

// -- All hash functions to be tested -- //

template <int mod> int mod_hash(uint64 id) {
    return id % mod;
}

template <int mod> int folding_hash(uint64 id) {
    if (id < 1000000000)
        return mod_hash<mod>(id);

    char string_id[20];
    lltoa(id, string_id, 10);
    const int length        = strnlen(string_id, 20);
    const int chunks_amount = length > 15 ? 3 : 2;
    const int chunk_size    = ceil((float)length / chunks_amount);

    int hashed = 0;
    for (int i = 0; i < chunks_amount; i++) {
        const int start = chunk_size * i;
        const int end   = start + chunk_size;
        char chunk[chunk_size + 1];

        int k = 0;
        for (int j = start; j < end; j++)
            chunk[k++] = string_id[j];

        chunk[k] = 0;
        hashed += atoi(chunk);
    }

    return hashed % mod;
}

template <int size> int username_hash(const string &username) {
    uint32 hash_val = 0;

    for (const char c : username)
        hash_val = 31 * hash_val + c;

    return hash_val % size;
}

template <int size> int username_double_hash(const string &username) {
    int h = 0;

    for (const char c : username)
        h = (127 * h + c) % size;

    return h;
}

template <int size> int username_hash_2(const string &username) {
    uint32 hash = 0;

    for (const char c : username) {
        hash += c;
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash % size;
}

int main(const int argc, const char *argv[]) {
    // Number of tests to run (default: 100)
    const int tests = argc > 1 ? max(stoi(argv[1]), 1) : 100;

    const vector<const User *> users = read_csv("universities_followers.csv");

    filesystem::create_directory("data");

    // Run all the tests

    run_tests<uint64, SC_N, L_N>(
        "id_mod",
        tests,
        users,
        [](const User *user) { return user->id; },
        mod_hash<SC_N>,
        mod_hash<L_N>,
        [](uint64 id) { return DH_N - (id % DH_N); }
    );

    run_tests<uint64, SC_N, L_N>(
        "id_folding",
        tests,
        users,
        [](const User *user) { return user->id; },
        folding_hash<SC_N>,
        folding_hash<L_N>,
        [](uint64 id) { return DH_N - (id % DH_N); }
    );

    run_tests<string, SC_N, L_N>(
        "username_1", //
        tests,
        users,
        [](const User *user) { return user->username; },
        username_hash<SC_N>,
        username_hash<L_N>,
        username_double_hash<L_N>
    );

    run_tests<string, SC_N, L_N>(
        "username_2", //
        tests,
        users,
        [](const User *user) { return user->username; },
        username_hash_2<SC_N>,
        username_hash_2<L_N>,
        username_double_hash<L_N>
    );

    return 0;
}
