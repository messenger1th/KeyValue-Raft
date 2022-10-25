//
// Created by epoch on 10/24/22.
//

#include "Server.hpp"

//TODO:delete it;
#include <csignal>

Server::Server(size_t id, const std::string &pair) : Server(id, pair.substr(0, pair.find(':')),
                                                            std::stoul(pair.substr(pair.find(':') + 1))) {}

Server::Server(size_t id, const std::string &IP, const size_t &port):
    id(id)
{
    //TODO
}


void Server::starts_up() {
    sleep(5);
    printf("starts up\n");
    for (size_t i = 1; i <= 3; ++i) {
        size_t port = i + 5000;
        if (i != this->id) {
            other_servers.emplace(i, NetAddress{"127.0.0.1", port});
            other_server_connections.emplace(i, make_unique<buttonrpc>());
            other_server_connections[i]->as_client("127.0.0.1", port);
            printf("config port %lu\n", port);
        }
    }
    start_election_timer();
}

VoteResult Server::request_vote(size_t term, size_t candidate_id, size_t last_log_index, size_t last_log_term) {
    this->election_timer.reset();
    printf("candidate[%llu] is calling request_voting\n", candidate_id);
    VoteResult res{this->current_term, false};
    if (this->current_state == State::Leader) {
        return res;
    }
    bool& vote_granted = res.vote_granted;
    vote_granted = this->vote_for == null || this-> vote_for == candidate_id;
    if (vote_granted) {
        this->vote_for = candidate_id;
    }
    return res;
}

AppendResult Server::append_entries(size_t term, size_t leader_id, size_t prev_log_index, size_t prev_log_term,
                                    const std::vector<LogEntry> &entries, size_t leader_commit) {
    return AppendResult();
}

void Server::as_candidate() {
    printf("server[%llu] I' m a candidate\n", this->id);

    ++this->current_term;
    start_election_timer();
    size_t vote_count = 1;
    int N = 2;
    for (const auto& [server_id, ptr]: other_server_connections) {
        ptr->set_timeout(election_timer_base.count() + rand() % election_timer_fluctuate.count());
        VoteResult vote_result = ptr->call<VoteResult>("request_vote", this->id, this->id, 0, 0).val();
        vote_count += vote_result.vote_granted ? 1 : 0;
        if (vote_result.vote_granted) {
            vote_count++;
        }
    }
    if (vote_count >= N) {
        this->current_state = State::Leader;
        as_leader();
    }
}

void Server::as_leader() {
    printf("server[%llu] I' m a leader\n", this->id);
    this->election_timer.stop();
    while (this->current_state == State::Leader) {
        sleep(1);
        for (const auto& [server_id, ptr]: other_server_connections) {
            ptr->set_timeout(election_timer_base.count() + rand() % election_timer_fluctuate.count());
            string hello = ptr->call<string>("Hello", this->id).val();
            printf("Send Hello to %d, Response: %s\n", server_id, hello.c_str());
        }
    }
    cout << "end" << endl;
}

void Server::start_election_timer() {
    election_timer.start(&Server::as_candidate, this);

}



std::string Server::Hello(size_t id)  {
    this->election_timer.reset();
    printf("server[%d] send Hello\n", id);
    return "Hello";
}
