//
// Created by epoch on 10/24/22.
//

#ifndef MIT6_824_C_RAFT_HPP
#define MIT6_824_C_RAFT_HPP
#include <vector>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <shared_mutex>

//todo: find a posix fork function rather than Linux unique.
#include <unistd.h>

#include "Timer.hpp"
#include "buttonrpc.hpp"

/* debug */
#include <iostream>
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
istream& operator>>(istream& in, LogEntry& entry);

struct VoteResult {
    size_t term{0};
    bool vote_granted{false};
};

struct AppendResult {
    size_t term{0};
    bool success{false};
    size_t expect_index{0};
    AppendResult() = default;
    AppendResult(size_t term, bool success, size_t expect_index): term(term), success(success), expect_index(expect_index) {}
};

class NetAddress{
public:
    NetAddress() = default;

    NetAddress(const string& ip, size_t port): ip(ip), port(port) {}
    std::string ip;
    size_t port;
};


class Raft {
public:
    enum class State {Follower, Candidate, Leader};

    /* format: ip:port, like "127.0.0.1:5555" */
    Raft(size_t id, const std::string &pair);
    Raft(size_t id, const std::string &IP, const size_t &port);

    /* RPC */
    std::string Hello(size_t id);
    VoteResult request_vote(size_t term, size_t candidate_id, size_t last_log_index, size_t last_log_term);
    AppendResult append_entries(size_t term, size_t leader_id, size_t prev_log_index, size_t prev_log_term, const string &entries, size_t leader_commit);

    void become_candidate();    /* be a candidate */
    void become_leader();    /* be a leader */

    /* configuring function before start serve */
    void configure();
    void load_persistent_value();
    void load_connection_configuration();
    void set_default_value();
    void start_service();


    /* application layer */
    //todo: modify apply function to virtual function after test.
    virtual void apply(const string& command) {  }
    virtual void install_snapshot(const string& filename) { /* do nothing. */ }
    virtual void load_snapshot(const string& filename) { /* do nothing. */ }

private:
    void load_snapshot() {
        ifstream reader(get_last_include_info_filename());
        size_t temp_term, temp_index;

        //todo: modify the way of judging whether the total snapshot has been written correctly or not.
        if (reader >> temp_term >> temp_index) {
            this->last_applied = temp_index;
            this->load_snapshot(get_snapshot_filename());
            printf("- snapshot done.\n");
        } else {
            //TODO: output information.
            printf("- snapshot warning: no snapshot found. \n");
        }
    }

private:
    size_t id;
    size_t port;
    string ip;

private:
    pair<size_t, size_t> get_last_include_info(const string& filename) {
        pair<size_t, size_t> res;
        ifstream reader(filename);
        reader >> res.first >> res.second;
        reader.close();
        return res;
    }

    string get_snapshot_filename() {
        return "snapshot/snapshot" + to_string(this->id) + ".txt";
    }

    string  get_last_include_info_filename() {
        return "last_include_info/last_include_info" + to_string(this->id) + ".txt" ;
    }

    void write_last_include_index(size_t last_include_term, size_t last_include_index) {
        //TODO: make this two steps:
        // 1. write to temp file
        // 2. rename temp file to real file.
        const string filename = get_last_include_info_filename();
        ofstream writer(filename);
        writer << last_include_term << " " << last_include_index;
        writer.close();
    }

private: /* Data mentioned in paper. */

    /* persistent part */
    size_t current_term{0};
    size_t vote_for{null};

    std::shared_mutex logs_mutex;
    std::vector<LogEntry> logs; // start from 1, if not log backup.

    /* volatile part */
    std::mutex commit_index_mutex; // this only for leader's commit_index updated by multi thread.
    size_t commit_index{0};
    std::mutex last_applied_mutex;
    size_t last_applied{0};


private:
    std::condition_variable apply_cv;

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

    /* connection information of other server */
    std::unordered_map<size_t, NetAddress> other_servers;
    std::unordered_map<size_t, unique_ptr<buttonrpc>> other_server_connections;

    /* leader unique */
    unordered_map<size_t, size_t> next_index;
    unordered_map<size_t, atomic<size_t>> match_index;
    std::atomic<size_t> match_index_increments{0};

private:

    /* when receive more up-to-date term, update current_term & be a follower*/
    void update_current_term(size_t term) {
        /* write log before change term; */
        update_term_info(term, null);
        become_follower();
    }

    void become_follower() {
        switch(this->state) {
            case State::Follower: this->election_timer.restart(); break;
            case State::Candidate: this->election_timer.restart(); break;
            case State::Leader: {
                start_election_timer();
            }; break;
            default: throw runtime_error("Server has no such state.");
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
    //TODO: make sure only one thread is applying to state machine, like use condition_variable to notice applying.
    void update_commit_index(size_t value) {
        bool should_notify = false;
        {
            std::unique_lock<std::mutex> lock(this->commit_index_mutex);
            if (value > this->commit_index) {
                this->commit_index = value;
                should_notify = true;
            }
        }
        if (should_notify) {
            apply_cv.notify_one();
        }
    }

    void append_log_simulate() {
        size_t start, end;
        {
            std::unique_lock<std::shared_mutex> lock(logs_mutex);
            start = this->logs.back().index + 1;
            this->logs.emplace_back(this->current_term, this->logs.back().index + 1, "Hello");
            end = this->logs.back().index + 1;
        }
        write_append_log(start, end);
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
        int l;
        {
            std::unique_lock<std::mutex> read_commit_lock(this->commit_index_mutex);
            l = this->commit_index;
        }

        int r = get_last_log_index();
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

    size_t get_last_log_index() {
        std::shared_lock<std::shared_mutex> get_last_log_index(this->logs_mutex);
        return this->logs.back().index;
    }

    size_t get_last_log_term() {
        std::shared_lock<std::shared_mutex> get_last_log_index(this->logs_mutex);
        return this->logs.back().term;
    }

    size_t get_log_term(size_t index) {
        std::shared_lock<std::shared_mutex> get_log_term_lock(this->logs_mutex);
        size_t start_index = this->logs[0].index;
        return logs[index - start_index].term;
    }

    /* get the index of log array. Please make sure you get the lock of logs when you call it. */
    size_t get_index(size_t absolute_index) {
        return absolute_index - logs[0].index + 0;
    }

    size_t get_total_log_size() {
        std::shared_lock<std::shared_mutex> get_total_size_lock(this->logs_mutex);
        return logs[0].index + logs.size();
    }


    bool match_prev_log_term(size_t index, size_t term) {
        return get_total_log_size() > index && get_log_term(index) == term;
    }


    bool log_conflict(size_t index, size_t term)  {
        return get_log_term(index) != term;
    }

    void remove_conflict_logs(size_t index)  {
        std::unique_lock<std::shared_mutex> remove_conflict_logs_lock(this->logs_mutex);
        this->logs.resize(index - logs[0].index + 1);
    }

    void append_logs(const vector<LogEntry>& entries);


    bool snapshot_condition() {
        //TODO: optimize the principle of install snapshot.
        return this->last_applied > 0 &&  this->last_applied % 10 == 0;
    }

    void apply_state_machine() {
        while (true) {
            std::unique_lock<std::mutex> apply_lock(this->last_applied_mutex);
            apply_cv.wait(apply_lock, [this] ()->bool {
                return this->last_applied < this->commit_index;
            });
            size_t current_commit_index;
            {
                std::unique_lock<std::mutex> get_commit_index_lock(commit_index_mutex);
                current_commit_index = this->commit_index;
            }
            vector<string> commands = get_logs_command(this->last_applied + 1, current_commit_index + 1);
            for (const auto& command: commands) {
                //TODO: authority check before pass it to higher layer(application layer).
                apply(command);
            }
            this->last_applied += commands.size();
            if (snapshot_condition()) {
                auto ret = fork();
                if (ret == -1) {
                    throw runtime_error("fork error ! ");
                } else if (ret == 0) { //Copy-On-Write: make child process install snapshot.
                    //TODO: make rewrite log two steps:
                    // 1. write to temp file
                    // 2. rename temp file to real file.

                    //TODO: use PIC to avoid more than one process install snapshot.
                    this->install_snapshot(get_snapshot_filename());
                    write_last_include_index(this->get_log_term(this->last_applied), this->last_applied);
                }
            }
        }
    }

    std::string get_term_info_file_name() {
        return "term_info/term_info" + to_string(this->id) + ".txt";
    }

    void update_term_info(size_t term, size_t vote_for) {
        write_term_info(term, vote_for);
        this->current_term = term;
        this->vote_for = vote_for;
    }


    void write_term_info(size_t term, size_t vote_for) {
        //TODO: make rewrite log two steps:
        // 1. write to temp file 
        // 2. rename temp file to real file.
        const string file_name = get_term_info_file_name();
        ofstream log_writer(file_name);
        {
            log_writer << term << " " << vote_for;
        }
        log_writer.close();
    }

    void load_term_info() {
        const string file_name = get_term_info_file_name();
        ifstream log_loader(file_name);
        {
            log_loader >> this->current_term >> this->vote_for;
        }
        log_loader.close();
        printf("- term&vote_for done.\n");
    }


    string get_log_file_name() {
        return "log/log" + to_string(this->id) + ".txt";
    }


    void rewrite_log() {
        //TODO: make rewrite log two steps:
        // 1. write to temp file
        // 2. rename temp file to real file.
        size_t last_include_index = this->get_last_include_info(get_last_include_info_filename()).second;
        std::ofstream clean_text(get_log_file_name(), std::ofstream::out | std::ofstream::trunc);
        clean_text.close();
        write_append_log(last_include_index + 1, this->get_last_log_index() + 1);
    }

    void write_append_log(size_t committed_log_start_index, size_t committed_log_end_index) {
        const string file_name = get_log_file_name();

        ofstream log_writer(file_name, ifstream::app);
        {
            std::shared_lock<std::shared_mutex> write_log_lock(this->logs_mutex);
            size_t start_index = get_index(committed_log_start_index);
            size_t end_index = get_index(committed_log_end_index);
            //append write is atomic.
            for (size_t i = start_index; i < end_index; ++i) {
                log_writer << logs[i];
            }
        }
        log_writer.close();
    }

    void load_log() {
        const string file_name = get_log_file_name();
        ifstream  log_loader(file_name);
        {
            std::unique_lock<std::shared_mutex> load_log_lock(this->logs_mutex);
            LogEntry temp;
            while (log_loader >> temp) {
                this->logs.emplace_back(temp);
            }
        }
        log_loader.close();
        printf("- logs done.\n");
    }
};


#endif //MIT6_824_C_RAFT_HPP
