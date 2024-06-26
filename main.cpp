#include "performance.h"
#include "read_csv.h"
#include "tests.h"
#include "user.h"

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
    return mod - (id % mod);
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

    return mod - (hashed % mod);
}

template <int size> int username_default_hash(const string &username) {
    return size - (hash<string>{}(username) % size);
}

template <int size> int username_djb2_hash(const string &username) {
    uint32 hash_val = 0;

    for (const char c : username)
        hash_val = ((hash_val << 5) + hash_val) + c;

    return size - (hash_val % size);
}

template <int size> int username_sdbm_hash(const string &username) {
    uint32 hash_val = 0;

    for (const char c : username)
        hash_val = c + (hash_val << 6) + (hash_val << 16) - hash_val;

    return size - (hash_val % size);
}

template <int size> int username_seeded_hash(const string &username) {
    int h = 0;

    for (const char c : username)
        h = (127 * h + c) % size;

    return size - h;
}

template <int size> int username_shifting_hash(const string &username) {
    uint32 hash = 0;

    for (const char c : username) {
        hash += c;
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return size - (hash % size);
}

int main(const int argc, const char *argv[]) {
    // Number of tests to run (default: 100)
    const int tests = argc > 1 ? max(stoi(argv[1]), 1) : 100;

    const vector<const User *> users = read_csv("universities_followers.csv");

    if (filesystem::exists("data")) {
        filesystem::remove_all("data");
    }

    filesystem::create_directory("data");

    // Run all the tests

    performance t;
    t.start();

    run_tests<uint64, SC_N, L_N>(
        "id_mod", //
        tests,
        users,
        [](const User *user) { return user->id; },
        mod_hash<SC_N>,
        mod_hash<L_N>,
        mod_hash<DH_N>
    );

    run_tests<uint64, SC_N, L_N>(
        "id_folding", //
        tests,
        users,
        [](const User *user) { return user->id; },
        folding_hash<SC_N>,
        folding_hash<L_N>,
        mod_hash<DH_N>
    );

    run_tests<string, SC_N, L_N>(
        "username_djb2", //
        tests,
        users,
        [](const User *user) { return user->username; },
        username_djb2_hash<SC_N>,
        username_djb2_hash<L_N>,
        username_default_hash<DH_N>
    );

    run_tests<string, SC_N, L_N>(
        "username_sdbm", //
        tests,
        users,
        [](const User *user) { return user->username; },
        username_sdbm_hash<SC_N>,
        username_sdbm_hash<L_N>,
        username_default_hash<DH_N>
    );

    run_tests<string, SC_N, L_N>(
        "username_shifting", //
        tests,
        users,
        [](const User *user) { return user->username; },
        username_shifting_hash<SC_N>,
        username_shifting_hash<L_N>,
        username_default_hash<DH_N>
    );

    run_tests<string, SC_N, L_N>(
        "username_seeded", //
        tests,
        users,
        [](const User *user) { return user->username; },
        username_seeded_hash<SC_N>,
        username_seeded_hash<L_N>,
        username_default_hash<DH_N>
    );

    cout << "\n==========================================================\n\n"
         << "total time: " << t.end<performance::milliseconds>() / 1e3 << " s" << endl;

    return 0;
}
