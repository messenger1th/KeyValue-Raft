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
#include <fstream>


using std::cout, std::endl;
/*TODO: delete above*/

using ms = std::chrono::milliseconds;
using s = std::chrono::seconds;

/* provided by application layer */
void execute_command(const string& command);

constexpr auto delay = 1000;
constexpr size_t null = 0;
constexpr ms client_request_frequency(100);
constexpr size_t max_send_log_size = 5;

class LogEntry {
public:
    size_t term{0};
    size_t index{0};
    std::string command{"Hello"};

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



    void as_candidate();    /* be a candidate */
    void as_leader();    /* be a leader */

    /* help function*/
    void read_config();
    void starts_up();


private: /* Data mentioned in paper. */

    /* persistent part */
    size_t current_term{0};
    size_t vote_for{null};

    std::shared_mutex logs_mutex;
    std::vector<LogEntry> logs{LogEntry()}; // start from 1.

    /* volatile part */
    std::mutex commit_index_mutex; // this only for leader's commit_index updated by multi thread.
    size_t commit_index{0};
    size_t last_applied{0};


private: /* extra information*/

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

    size_t id;

    /* connection information of other server */
    std::unordered_map<size_t, NetAddress> other_servers;
    std::unordered_map<size_t, unique_ptr<buttonrpc>> other_server_connections;

    /* leader unique*/
    unordered_map<size_t, size_t> next_index;
    unordered_map<size_t, atomic<size_t>> match_index;

private:

    /* when receive more up-to-date term, update current_term & be a follower*/
    void update_current_term(size_t term) {
        //TODO: write log before commit;
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
        std::shared_lock<std::shared_mutex> get_last_log_info_lock(this->logs_mutex);
        return {this->logs.back().term, this->logs.back().index};
    }

    size_t get_send_log_size(size_t server_id, bool is_conflict = false) {
        if (is_conflict) {
            return 0;
        }
        auto current_last_log_index = get_last_log_info().second;
        return min(max_send_log_size, current_last_log_index - next_index[server_id] + 1);
    }


private: /* debug part */

    //TODO: make sure only one thread is applying to state machine,
    void update_commit_index(size_t value) {
        if (this->get_log_term(value) != this->current_term) {
            return ;
        }
        std::unique_lock<std::mutex> lock(this->commit_index_mutex);
        if (value > this->commit_index) {
//            printf("commit_index update success [%lu]->[%lu]\n", this->commit_index, value);
            commit_index = value;
//            thread t(&Server::apply_entries, this); t.detach();
        }
    }

    void append_log_simulate() {
        std::unique_lock<std::shared_mutex> lock(logs_mutex);
        this->logs.emplace_back(this->current_term, logs.size(), "Hello");
    }

    bool find_match_index_median_check(size_t mid) {
        int count = 1; // 1: the leader one.
        for (const auto& [k, v]: this->match_index) {
            if (v >= mid) {
                ++count;
            }
        }
        return count >= majority_count;
    }

    size_t find_match_index_median() {
        int l = min_element(match_index.begin(), match_index.end(), [] (const auto& p1, const auto& p2) ->bool {
            return p1.second < p2.second;
        })->second;
        int r = max_element(match_index.begin(), match_index.end(), [] (const auto& p1, const auto& p2) ->bool {
            return p1.second < p2.second;
        })->second;
        if (l == r) {
            return l;
        }
        int res = l;
        while (l <= r) {
            int mid = (r - l) / 2 + l;
            if (find_match_index_median_check(mid)) {
                res = mid;
                l = mid + 1;
            } else {
                r = mid - 1;
            }
        }
        return res;
    }

    std::string get_log_string(size_t start, size_t end) {
        string res;
        std::shared_lock<std::shared_mutex> get_log_string_lock(this->logs_mutex);
        size_t start_index = this->logs[0].index;
        for (auto& i = start; i < end; ++i) {
            const auto& log = logs[i - start_index];
            res += " " + to_string(log.term) + " " + to_string(log.index) + " " + log.command;
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

    vector<string> get_logs_command(size_t start_index, size_t end_index) {
        vector<string> commands;
        {
            std::shared_lock<std::shared_mutex> log_lock(this->logs_mutex);
            size_t log_start_index = logs[0].index;
            for (size_t i = start_index; i < end_index; ++i) {
                commands.emplace_back(logs[i - log_start_index].command);
            }
        }
        return commands;
    }

    size_t get_log_term(size_t index) {
        std::shared_lock<std::shared_mutex> get_log_term_lock(this->logs_mutex);
        size_t start_index = this->logs[0].index;
        return logs[index - start_index].term;
    }

    bool match_prev_log_term(size_t index, size_t term) {
        return get_total_log_size() > index && get_log_term(index) == term;
    }

    size_t get_total_log_size() {
        return logs[0].index + logs.size();
    }

    bool log_conflict(size_t index, size_t term)  {
        return get_log_term(index) != term;
    }

    void remove_conflict_logs(size_t index)  {
        std::unique_lock<std::shared_mutex> remove_conflict_logs_lock(this->logs_mutex);
        this->logs.resize(index - logs[0].index + 1);
    }

    void append_logs(const vector<LogEntry>& entries);

    void apply_entries() {
        vector<string> commands = get_logs_command(this->last_applied + 1, this->commit_index + 1);
        for (const auto& command: commands) {
            execute_command(command);
        }
        this->last_applied += commands.size();
    }

    void write_log() {
        const string file_name = "log" + to_string(this->id);
        ofstream log_writer(file_name, ios::app);
        {
            std::unique_lock<std::shared_mutex> write_log_lock(this->logs_mutex);
            for (size_t i = 1; i < logs.size(); ++i) {
                log_writer << logs[i];
            }
        }
        log_writer.close();
    }

    void load_log() {
        const string file_name = "log" + to_string(this->id);
        ifstream  log_loader(file_name);
        {
            std::unique_lock<std::shared_mutex> load_log_lock(this->logs_mutex);
            LogEntry Temp;
            while (log_loader >> Temp) {

            }
        }
    }
};


#endif //MIT6_824_C_SERVER_HPP
