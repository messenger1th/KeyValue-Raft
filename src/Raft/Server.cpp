//
// Created by epoch on 10/24/22.
//

#include "Server.hpp"

Server::Server(size_t id, const std::string &pair) : Server(id, pair.substr(0, pair.find(':')),
                                                            std::stoul(pair.substr(pair.find(':') + 1))) {}

Server::Server(size_t id, const std::string &IP, const size_t &port):
    id(id)
{
    //TODO
}


void Server::starts_up() {
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

    election_timer.start(&Server::as_candidate, this);
}

VoteResult Server::request_vote(size_t term, size_t candidate_id, size_t last_log_index, size_t last_log_term) {
    return VoteResult();
}

AppendResult Server::append_entries(size_t term, size_t leader_id, size_t prev_log_index, size_t prev_log_term,
                                    const std::vector<LogEntry> &entries, size_t leader_commit) {
    return AppendResult();
}

void Server::as_candidate() {
    printf("times out: I' m a candidate\n");

    for (const auto& [server_id, ptr]: other_server_connections) {
        string HelloResponse = ptr->call<string>("Hello", this->id).val();
        printf("Hello %llu, response %s\n", server_id, HelloResponse.c_str());
    }
}

void Server::as_leader() {

}
