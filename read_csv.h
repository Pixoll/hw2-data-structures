#pragma once

#include "user.h"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#define timegm _mkgmtime
#endif

using namespace std;

typedef unordered_map<uint64, User *> user_map;

/** Parse timestamp string into a time_t (int64) */
time_t string_to_time(const string &field) {
    istringstream input(field);
    tm result;
    input >> get_time(&result, "%a %b %d %H:%M:%S +0000 %Y");
    return timegm(&result);
}

/** Read the entire CSV file */
vector<const User *> read_csv(const char *file_name) {
    cout << "reading .csv" << endl;

    ifstream csv(file_name);
    // Store all users in a hash map for ~O(1) lookup
    user_map users;
    string row;

    // remove first line
    getline(csv, row);

    while (!csv.eof()) {
        getline(csv, row);

        if (!csv.good())
            break;

        // Separate columns into an array of strings
        string fields[7];
        int i = 0;

        for (const char c : row) {
            switch (c) {
                case ',': i++; break;
                default:  fields[i] += c; break;
            }
        }

        // Parse each value when adequate
        const uint64 id        = stold(fields[1]);
        const uint32 tweets    = stoul(fields[3]);
        const uint32 friends   = stoul(fields[4]);
        const uint32 followers = stoul(fields[5]);

        User *existent = users[id];
        if (existent != nullptr) {
            // Update stats and add university if one already exists
            existent->update_stats(tweets, friends, followers);
            existent->add_university(fields[0].c_str());
            continue;
        }

        // Create and insert
        User *user = new User(id, fields[2].c_str(), tweets, friends, followers, string_to_time(fields[6]));
        user->add_university(fields[0].c_str());

        users[user->id] = user;
    }

    csv.close();

    // Hash map -> vector
    vector<const User *> users_vector;
    for (user_map::iterator::value_type pair : users)
        users_vector.push_back(pair.second);

    return users_vector;
}
