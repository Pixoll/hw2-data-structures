#pragma once

#include "dh_hash_map.h"
#include "lp_hash_map.h"
#include "performance.h"
#include "qp_hash_map.h"
#include "sc_hash_map.h"
#include "user.h"

#include <cmath>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

/** Range by which to take timing measures */
const int TIMING_MEASURE_RANGE = 100;

/** Struct containing measurements for all hash maps */
typedef struct measurement {
    uint64 sc  = 0;
    uint64 lp  = 0;
    uint64 qp  = 0;
    uint64 dh  = 0;
    uint64 stl = 0;
} measurement;

template <typename K> void stl_map_info(stringstream &out, unordered_map<K, const User *, function<int(K)>> &map) {
    out << "[stl] map info:\n"
        << "max size: " << (uint64)map.bucket_count() << "\n"
        << "size: " << (uint64)map.size() << "\n"
        << "load factor: " << map.load_factor() << "\n"
        << "size in memory: "
        << (uint64)(sizeof(map)
                    + map.size()
                          * (sizeof(list<pair<const K, const User *>>) + sizeof(pair<const K, const User *>) + sizeof(User)))
        << " B\n"
        << endl;
}

/**
 * Run N amount of tests on all hash maps
 * Measurement results are saved in a file prefixed by `file_name_prefix`
 */
template <typename K, int SC_N, int L_N>
void run_tests(
    string file_name_prefix,
    const int tests,
    const vector<const User *> &users,
    function<K(const User *)> get_key_fn,
    function<int(const K &)> sc_hash_fn,
    function<int(const K &)> l_hash_fn,
    function<int(const K &)> dh_hash_fn
) {
    // Print time at which the test was started
    time_t now = time(nullptr);
    char time_string[20];
    strftime(time_string, 20, "%F %T", localtime(&now));

    cout << "\n==========================================================\n\n"
         << time_string << "\n"
         << "running " << tests << "x " << file_name_prefix << " tests...\n"
         << endl;

    const int users_size    = users.size();
    const int ranges_amount = ceilf((float)users_size / TIMING_MEASURE_RANGE);

    performance p, total;

    // Prepare the test
    p.start();
    sc_hash_map<K, const User *> sc_map(SC_N, sc_hash_fn);
    int t_c = p.end();
    cout << "[sc] creation: " << t_c / 1e3 << " μs\n";

    p.start();
    lp_hash_map<K, const User *> lp_map(L_N, l_hash_fn);
    t_c = p.end();
    cout << "[lp] creation: " << t_c / 1e3 << " μs\n";

    p.start();
    qp_hash_map<K, const User *> qp_map(L_N, l_hash_fn);
    t_c = p.end();
    cout << "[qb] creation: " << t_c / 1e3 << " μs\n";

    p.start();
    dh_hash_map<K, const User *> dh_map(L_N, l_hash_fn, dh_hash_fn);
    t_c = p.end();
    cout << "[dh] creation: " << t_c / 1e3 << " μs\n";

    p.start();
    unordered_map<K, const User *, function<int(K)>> stl_map(SC_N, sc_hash_fn);
    t_c = p.end();
    cout << "[stl] creation: " << t_c / 1e3 << " μs\n\n";

    stringstream timings, results;
    timings << "users,op,map,time\n";

    measurement times;
    total.start();

    // Run N tests
    for (int n_test = 0; n_test < tests; n_test++) {
        int start_range = 0;
        // map.put(k, v) tests
        for (int _ = 0; _ < ranges_amount; _++) {
            const int end_range = min(start_range + TIMING_MEASURE_RANGE, users_size);

            for (int i = start_range; i < end_range; i++) {
                const User *user = users[i];
                const K key      = get_key_fn(user);

                p.start();
                sc_map.put(key, user);
                times.sc += p.end();

                p.start();
                lp_map.put(key, user);
                times.lp += p.end();

                p.start();
                qp_map.put(key, user);
                times.qp += p.end();

                p.start();
                dh_map.put(key, user);
                times.dh += p.end();

                p.start();
                stl_map[key] = user;
                times.stl += p.end();
            }

            timings << end_range << ",put,sc," << times.sc << "\n"
                    << end_range << ",put,lp," << times.lp << "\n"
                    << end_range << ",put,qp," << times.qp << "\n"
                    << end_range << ",put,dh," << times.dh << "\n"
                    << end_range << ",put,stl," << times.stl << "\n";

            start_range = end_range;
            times       = {0, 0, 0, 0, 0};
        }

        if (n_test == 0) {
            // Record maps information to print at the end
            sc_map.info(results);
            lp_map.info(results);
            qp_map.info(results);
            dh_map.info(results);
            stl_map_info(results, stl_map);
        }

        start_range = 0;
        // map.get(k) (hit) tests
        for (int _ = 0; _ < ranges_amount; _++) {
            const int end_range = min(start_range + TIMING_MEASURE_RANGE, users_size);

            for (int i = start_range; i < end_range; i++) {
                const User *user = users[i];
                const K key      = get_key_fn(user);

                p.start();
                sc_map.get(key);
                times.sc += p.end();

                p.start();
                lp_map.get(key);
                times.lp += p.end();

                p.start();
                qp_map.get(key);
                times.qp += p.end();

                p.start();
                dh_map.get(key);
                times.dh += p.end();

                p.start();
                stl_map[key];
                times.stl += p.end();

                /*
                if (sc_f == nullptr || lp_f == nullptr || qp_f == nullptr || dh_f == nullptr) {
                    cerr << "[";

                    if (sc_f == nullptr)
                        cerr << " sc";
                    if (lp_f == nullptr)
                        cerr << " lp";
                    if (qp_f == nullptr)
                        cerr << " qp";
                    if (dh_f == nullptr)
                        cerr << " dh";

                    cerr << " ] missing value for " << key << endl;
                }
                */
            }

            timings << end_range << ",get_(hit),sc," << times.sc << "\n"
                    << end_range << ",get_(hit),lp," << times.lp << "\n"
                    << end_range << ",get_(hit),qp," << times.qp << "\n"
                    << end_range << ",get_(hit),dh," << times.dh << "\n"
                    << end_range << ",get_(hit),stl," << times.stl << "\n";

            start_range = end_range;
            times       = {0, 0, 0, 0, 0};
        }

        start_range = 0;
        // map.remove(k) tests
        for (int _ = 0; _ < ranges_amount; _++) {
            const int end_range = min(start_range + TIMING_MEASURE_RANGE, users_size);

            for (int i = start_range; i < end_range; i++) {
                const User *user = users[i];
                const K key      = get_key_fn(user);

                p.start();
                sc_map.remove(key);
                times.sc += p.end();

                p.start();
                lp_map.remove(key);
                times.lp += p.end();

                p.start();
                qp_map.remove(key);
                times.qp += p.end();

                p.start();
                dh_map.remove(key);
                times.dh += p.end();

                p.start();
                stl_map.erase(key);
                times.stl += p.end();

                /*
                if (sc_f == nullptr || lp_f == nullptr || qp_f == nullptr || dh_f == nullptr) {
                    cerr << "[";

                    if (sc_f == nullptr)
                        cerr << " sc";
                    if (lp_f == nullptr)
                        cerr << " lp";
                    if (qp_f == nullptr)
                        cerr << " qp";
                    if (dh_f == nullptr)
                        cerr << " dh";

                    cerr << " ] didn't remove " << key << endl;
                }
                */
            }

            timings << end_range << ",remove,sc," << times.sc << "\n"
                    << end_range << ",remove,lp," << times.lp << "\n"
                    << end_range << ",remove,qp," << times.qp << "\n"
                    << end_range << ",remove,dh," << times.dh << "\n"
                    << end_range << ",remove,stl," << times.stl << "\n";

            start_range = end_range;
            times       = {0, 0, 0, 0, 0};
        }

        /*
        if (n_test == 0 && (!sc_map.empty() || !lp_map.empty() || !qp_map.empty() || !dh_map.empty() || !stl_map.empty())) {
            cerr << "some maps aren't empty after map.remove(k):\n"
                 << "sc: " << sc_map.size() << "\n"
                 << "lp: " << lp_map.size() << "\n"
                 << "qp: " << qp_map.size() << "\n"
                 << "dh: " << dh_map.size() << "\n"
                 << "stl: " << stl_map.size() << endl;

            exit(1);
        }
        */

        start_range = 0;
        // map.get(k) (miss) tests
        for (int _ = 0; _ < ranges_amount; _++) {
            const int end_range = min(start_range + TIMING_MEASURE_RANGE, users_size);

            for (int i = start_range; i < end_range; i++) {
                const User *user = users[i];
                const K key      = get_key_fn(user);

                p.start();
                sc_map.get(key);
                times.sc += p.end();

                p.start();
                lp_map.get(key);
                times.lp += p.end();

                p.start();
                qp_map.get(key);
                times.qp += p.end();

                p.start();
                dh_map.get(key);
                times.dh += p.end();

                p.start();
                stl_map[key];
                times.stl += p.end();
            }

            timings << end_range << ",get_(miss),sc," << times.sc << "\n"
                    << end_range << ",get_(miss),lp," << times.lp << "\n"
                    << end_range << ",get_(miss),qp," << times.qp << "\n"
                    << end_range << ",get_(miss),dh," << times.dh << "\n"
                    << end_range << ",get_(miss),stl," << times.stl << "\n";

            start_range = end_range;
            times       = {0, 0, 0, 0, 0};
        }
    }

    cout << "total time: " << total.end<performance::milliseconds>() / 1e3 << " s\n"
         << "saving timing data...\n";

    // Save measurements data
    ofstream timings_file("data/" + file_name_prefix + ".csv");
    timings_file << timings.rdbuf();
    timings_file.close();
    timings.clear();

    cout << "saved\n\n" << results.rdbuf() << endl;
}
