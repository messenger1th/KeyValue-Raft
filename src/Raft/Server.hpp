//
// Created by epoch on 10/24/22.
//

#ifndef MIT6_824_C_SERVER_HPP
#define MIT6_824_C_SERVER_HPP
#include <vector>
#include <cstddef>
#include <string>

#include "Timer.hpp"
#include "buttonrpc.hpp"


/* debug */
#include <iostream>
using std::cout, std::endl;
/*TODO: delete above*/

using ms = std::chrono::milliseconds;
using s = std::chrono::seconds;

constexpr size_t null = 0;

struct LogEntry {
    size_t term;
    size_t index;
    std::string command;
};

struct VoteResult {
    size_t term;
    bool vote_granted;
};

struct AppendResult {
    size_t term;
    bool success;
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
    AppendResult append_entries(size_t term, size_t leader_id, size_t prev_log_index, size_t prev_log_term, const std::vector<LogEntry>& entries, size_t leader_commit);


    /* be a candidate */
    void as_candidate();

    /* be a leader */
    void as_leader();


    /* help function*/
    void starts_up();

private:
    /* persistent part */
    size_t current_term{0};
    size_t vote_for{null};
    std::vector<LogEntry> log;

    /* volatile part */
    size_t commit_index{0};
    size_t last_applied{0};

    /* extra information*/
    std::atomic<State> current_state{State::Follower};

private:
    /* timer */
    ms election_timer_base{1000};
    ms election_timer_fluctuate{1000};
    Timer<ms> election_timer{2500};

    void start_election_timer();

    /* id -> ip:port */
    size_t id;
    std::unordered_map<size_t, NetAddress> other_servers;
    std::unordered_map<size_t, unique_ptr<buttonrpc>> other_server_connections;

    /* extra information */
};


#endif //MIT6_824_C_SERVER_HPP
