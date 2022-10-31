//
// Created by epoch on 10/24/22.
//

#ifndef MIT6_824_C_SERVER_HPP
#define MIT6_824_C_SERVER_HPP
#include <vector>
#include <cstddef>
#include <stdlib.h>
#include <string>
#include <shared_mutex>

#include "Timer.hpp"
#include "buttonrpc.hpp"

/* debug */
#include <iostream>
#include <queue>


using std::cout, std::endl;
/*TODO: delete above*/

using ms = std::chrono::milliseconds;
using s = std::chrono::seconds;


constexpr auto delay = 1000;
constexpr size_t null = 0;
constexpr ms client_request_frequency(100);

class LogEntry {
public:
    size_t term{0};
    size_t index{0};
    std::string command{""};

    LogEntry() = default;
    LogEntry(size_t term, size_t index, const string &command);

};

ostream& operator<<(ostream& out, const LogEntry& entry);

struct VoteResult {
    size_t term{0};
    bool vote_granted{false};
};

struct AppendResult {
    size_t term{0};
    bool success{false};
};

class NetAddress{
public:
    NetAddress() = default;

    NetAddress(const string& ip, const size_t& port): ip(ip), port(port) {}
    std::string ip;
    size_t port;
};


class Server {
public:
    enum class State {Follower, Candidate, Leader};

    /*format: ip:port, like "127.0.0.1:5555" */
    Server(size_t id, const std::string &pair);
    Server(size_t id, const std::string &IP, const size_t &port);

    /* RPC */
    std::string Hello(size_t id);

    VoteResult request_vote(size_t term, size_t candidate_id, size_t last_log_index, size_t last_log_term);
    AppendResult append_entries(size_t term, size_t leader_id, size_t prev_log_index, size_t prev_log_term, const string &entries, size_t leader_commit);


    /* be a candidate */
    void as_candidate();

    /* be a leader */
    void as_leader();

    /* help function*/
    void read_config();
    void starts_up();


private: /* Data mentioned in paper. */

    /* persistent part */
    size_t current_term{0};
    size_t vote_for{null};

    std::shared_mutex logs_m;
    std::vector<LogEntry> logs{LogEntry()}; // start from 1.

    /* volatile part */
    size_t commit_index{0};
    size_t last_applied{0};


private: /* extra information*/

    /* Log Part */
    std::mutex last_log_m;
    size_t last_log_term{0};
    size_t last_log_index{0};

    /* dynamically change when member change */
    size_t majority_count{2};


    /* state information */
    std::atomic<State> state{State::Follower};


    /* election timer relevant property */
    ms election_timer_base{delay};
    ms election_timer_fluctuate{delay};
    Timer<ms> election_timer{delay};

    /* election timer relevant function */
    void start_election_timer();

    /* id -> ip:port */
    size_t id;

    /* connection information of other server */
    std::unordered_map<size_t, NetAddress> other_servers;
    std::unordered_map<size_t, unique_ptr<buttonrpc>> other_server_connections;

    /* leader unique*/
    unordered_map<size_t, size_t> next_index;
    unordered_map<size_t, size_t> match_index; //TODO: what is this for ?


private:

    /* when receive more up-to-date term, update current_term & be a follower*/
    void update_current_term(size_t term) {
        this->vote_for = null;
        this->current_term = term;
        be_follower();
    }

    void be_follower() {
        if (this->state != State::Follower) {
            this->state = State::Follower;
            start_election_timer();
        } else {
            this->election_timer.restart();
        }
    }

    void send_log_heartbeat(size_t server_id);


    inline size_t cluster_size() {
        return this->other_server_connections.size() + 1;
    }

    pair<size_t, size_t> get_last_log_info() {
        std::shared_lock<std::shared_mutex> lock{this->logs_m};
        return {last_log_term, last_log_index};
    }


private: /* debug part */
    /* */


    /* simulated client RPQ request */

    void simulate_client() {

    }

    void client_server(vector<size_t>& next_index, vector<size_t>& match_index) {

    }


    void append_log_simulate() {
        unique_lock<std::shared_mutex> lock(logs_m);
        this->logs.emplace_back(this->current_term, logs.size(), "Hello");
        this->last_log_term = logs.back().term;
        this->last_log_index = logs.back().index;
    }



    std::string get_log_string(size_t start, size_t end) {
        string res;
        for (auto& i = start; i < end; ++i) {
            res = res + " " + to_string(logs[i].term) + " " + to_string(logs[i].index) + " " + logs[i].command;
        }
        return res;
    }

    vector<LogEntry> parse_string_logs(const string& s) {
        vector<LogEntry> res;
        istringstream read(s);
        LogEntry entry;
        while (read >> entry.term >> entry.index >> entry.command) {
            res.emplace_back(entry);
        }
        return res;
    }

    size_t get_log_term(size_t index) {
        //TODO: update it after log compaction(use base + offset)
//        cout << "erroe" << endl;
//        cout << index << endl;
//        cout << logs.size() << index << endl;
//        printf("logs.size() = [%lu], index = %lu", logs.size(), index);
        return logs[index].term;
    }

    bool match_prev_log_term(size_t index, size_t term) {
        return get_total_log_size() > index && get_log_term(index) == term;
    }

    size_t get_total_log_size() {
        //TODO: update it after log compaction(use base + size());
        return logs.size();
    }

    bool log_conflict(size_t index, size_t term)  {
        //TODO: update if after change way of log store.
        printf("get_log_term->%lu term->%lu\n", get_log_term(index), term);
//        cout << get_log_term(index) << term << endl;
        return get_log_term(index) != term;
    }

    void remove_conflict_logs(size_t index);

    void append_logs(const vector<LogEntry>& entries);
};


#endif //MIT6_824_C_SERVER_HPP
