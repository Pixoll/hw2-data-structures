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

    // Prepare the test
    sc_hash_map<K, const User *> sc_map(SC_N, sc_hash_fn);
    lp_hash_map<K, const User *> lp_map(L_N, l_hash_fn);
    qp_hash_map<K, const User *> qp_map(L_N, l_hash_fn);
    dh_hash_map<K, const User *> dh_map(L_N, l_hash_fn, dh_hash_fn);
    unordered_map<K, const User *, function<int(K)>> stl_map(SC_N, sc_hash_fn);

    stringstream timings, results;
    timings << "users,op,map,time\n";

    measurement t_put, t_get, t_remove;
    double stl_load_factor = 0;
    performance p, total;
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
                t_put.sc += p.end();

                p.start();
                lp_map.put(key, user);
                t_put.lp += p.end();

                p.start();
                qp_map.put(key, user);
                t_put.qp += p.end();

                p.start();
                dh_map.put(key, user);
                t_put.dh += p.end();

                p.start();
                stl_map[key] = user;
                t_put.stl += p.end();
            }

            timings << end_range << ",put,sc," << t_put.sc << "\n"
                    << end_range << ",put,lp," << t_put.lp << "\n"
                    << end_range << ",put,qp," << t_put.qp << "\n"
                    << end_range << ",put,dh," << t_put.dh << "\n"
                    << end_range << ",put,stl," << t_put.stl << "\n";

            start_range = end_range;
            t_put       = {0, 0, 0, 0, 0};
        }

        if (n_test == 0) {
            // Record maps information to print at the end
            stl_load_factor = stl_map.load_factor();

            sc_map.info(results);
            lp_map.info(results);
            qp_map.info(results);
            dh_map.info(results);
        }

        start_range = 0;
        // map.get(k) tests
        for (int _ = 0; _ < ranges_amount; _++) {
            const int end_range = min(start_range + TIMING_MEASURE_RANGE, users_size);

            for (int i = start_range; i < end_range; i++) {
                const User *user = users[i];
                const K key      = get_key_fn(user);

                p.start();
                sc_map.get(key);
                t_get.sc += p.end();

                p.start();
                lp_map.get(key);
                t_get.lp += p.end();

                p.start();
                qp_map.get(key);
                t_get.qp += p.end();

                p.start();
                dh_map.get(key);
                t_get.dh += p.end();

                p.start();
                stl_map[key];
                t_get.stl += p.end();

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

            timings << end_range << ",get,sc," << t_get.sc << "\n"
                    << end_range << ",get,lp," << t_get.lp << "\n"
                    << end_range << ",get,qp," << t_get.qp << "\n"
                    << end_range << ",get,dh," << t_get.dh << "\n"
                    << end_range << ",get,stl," << t_get.stl << "\n";

            start_range = end_range;
            t_get       = {0, 0, 0, 0, 0};
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
                t_remove.sc += p.end();

                p.start();
                lp_map.remove(key);
                t_remove.lp += p.end();

                p.start();
                qp_map.remove(key);
                t_remove.qp += p.end();

                p.start();
                dh_map.remove(key);
                t_remove.dh += p.end();

                p.start();
                stl_map.erase(key);
                t_remove.stl += p.end();

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

            timings << end_range << ",remove,sc," << t_remove.sc << "\n"
                    << end_range << ",remove,lp," << t_remove.lp << "\n"
                    << end_range << ",remove,qp," << t_remove.qp << "\n"
                    << end_range << ",remove,dh," << t_remove.dh << "\n"
                    << end_range << ",remove,stl," << t_remove.stl << "\n";

            start_range = end_range;
            t_remove    = {0, 0, 0, 0, 0};
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
    }

    cout << "total time: " << total.end<performance::milliseconds>() / 1e3 << " s\n"
         << "saving timing data...\n";

    // Save measurements data
    ofstream timings_file("data/" + file_name_prefix + ".csv");
    timings_file << timings.rdbuf();
    timings_file.close();
    timings.clear();

    cout << "saved\n\n"
         << results.rdbuf() //
         << "stl load factor: " << stl_load_factor << endl;
}