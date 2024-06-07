#pragma once

#include <cstring>
#include <ctime>
#include <sstream>
#include <string>

typedef unsigned int uint32;
typedef unsigned long long uint64;

using namespace std;

/** Transform timestamp value into a human-readable string */
string timestamp_to_string(time_t timestamp) {
    tm *time = gmtime(&timestamp);
    char dest[32];
    strftime(dest, 32, "%FT%TZ", time);
    return string(dest);
}

/**
 * Class representing a user from the dataset
 */
class User {
  private:
    static const int MAX_USERNAME_LEN = 16;
    static const int MAX_UNIVERSITIES = 11;

  public:
    /** The id */
    uint64 id;
    /** The username */
    char username[MAX_USERNAME_LEN];
    /** Tweets count */
    uint32 tweets;
    /** Friends count */
    uint32 friends;
    /** Followers count */
    uint32 followers;
    /**
     * Time the user was created at
     * Stored as time_t (int64) to reduce the used space
     */
    time_t created_at;
    /** Usernames of the universities the user follows */
    char universities[MAX_UNIVERSITIES][MAX_USERNAME_LEN];

    /** Constructor that takes most of the key information */
    User(uint64 id, const char *username, uint32 tweets, uint32 friends, uint32 followers, time_t created_at)
        : id(id), tweets(tweets), friends(friends), followers(followers), created_at(created_at) {
        strncpy(this->username, username, MAX_USERNAME_LEN);
        this->username[MAX_USERNAME_LEN - 1] = 0;

        for (int i = 0; i < MAX_UNIVERSITIES; i++)
            this->universities[i][0] = 0;
    }

    /**
     * Assuming the provided data wasn't all collected at the exact same time,
     * stats must be updated as the entires at the end of the file would be the newest
     */
    void update_stats(uint32 tweets, uint32 friends, uint32 followers) {
        this->tweets    = tweets;
        this->friends   = friends;
        this->followers = followers;
    }

    /**
     * Add a university to the ones the user follows
     * Does nothing if the university was already included (possible data duplication)
     */
    void add_university(const char *university) {
        for (int i = 0; i < MAX_UNIVERSITIES; i++) {
            if (strncmp(this->universities[i], university, MAX_USERNAME_LEN) == 0)
                return;

            if (this->universities[i][0] == 0) {
                strncpy(this->universities[i], university, MAX_USERNAME_LEN);
                this->universities[i][MAX_USERNAME_LEN - 1] = 0;
                return;
            }
        }
    }

    /** Builds a string with all the information of this user */
    string to_string() const {
        ostringstream oss;

        oss << this->id << "\t";
        oss << this->username << "\t";
        oss << this->tweets << "\t";
        oss << this->friends << "\t";
        oss << this->followers << "\t";
        oss << timestamp_to_string(this->created_at) << "\t";
        oss << this->universities[0];

        for (int i = 1; i < MAX_UNIVERSITIES; i++) {
            if (this->universities[i][0] == 0)
                break;

            oss << ", " << this->universities[i];
        }

        return oss.str();
    }
};
