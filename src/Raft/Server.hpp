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


constexpr auto delay = 1000;
constexpr size_t null = 0;

struct LogEntry {
    size_t term{0};
    size_t index{0};
    std::string command{""};
};

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
    AppendResult append_entries(size_t term, size_t leader_id, size_t prev_log_index, size_t prev_log_term, const std::vector<LogEntry>& entries, size_t leader_commit);


    /* be a candidate */
    void as_candidate();

    /* be a leader */
    void as_leader();

    /* help function*/
    void read_config();
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
    size_t majority_count{2};
    std::atomic<State> state{State::Follower};

private:
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

    /* extra information */
    std::atomic<size_t> last_log_term{0};
    std::atomic<size_t> last_log_index{0};


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
            this->election_timer.reset();
        }
    }
};


#endif //MIT6_824_C_SERVER_HPP
