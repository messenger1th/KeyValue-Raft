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

    printf("candidate[%llu] is calling request_voting\n", candidate_id);

    /* If RPC request or response contains term T > currentTerm: set currentTerm = T, convert to follower (§5.1)*/
    if (this->current_term < term) {
        this->state = State::Follower;
        this->current_term = term;
    }

    VoteResult res{this->current_term, false};
    /* 1. Reply false if term < currentTerm (§5.1) */
    if (this->current_term > term) {
        return res;
    }

    /* 2. Reply false, if votedFor is not null or candidateId, */
    if (this->vote_for != null && this->vote_for != candidate_id) {
        return res;
    }

    /* 3. Reply false, if candidate’s log is not at least as up-to-date as receiver’s log (§5.2, §5.4)*/
    if (this->last_log_term > last_log_term || (this->last_log_term == last_log_term && this->last_log_index > last_log_index)) {
        return res;
    }

    res.vote_granted = true;
    return res;
}

AppendResult Server::append_entries(size_t term, size_t leader_id, size_t prev_log_index, size_t prev_log_term,
                                    const std::vector<LogEntry> &entries, size_t leader_commit) {

    AppendResult res{this->current_term, false};
    if (this->current_term > term) {
        return res;
    } else if (this->current_term  == term) {
        /* If RPC request or response contains term T > currentTerm: set currentTerm = T, convert to follower (§5.1)*/
        this->state = State::Follower;
        this->current_term = term;
    }


    if (this->current_term < term) {
        return res;
    }
    //TODO step2: Reply false if log doesn’t contain an entry at prevLogIndex whose term matches prevLogTerm (§5.3)

    //TODO step3: If an existing entry conflicts with a new one (same index
    //but different terms), delete the existing entry and all that
    //follow it (§5.3)

    //TODO step4: Append any new entries not already in the logAppend any new entries not already in the log

    //TODO step5: If leaderCommit > commitIndex, set commitIndex = min(leaderCommit, index of last new entry)

    res.success = true;
    return res;
}

void Server::as_candidate() {

    /* steps after being a candidate */
    this->state = State::Candidate; //change state.
    ++this->current_term;   // increment term.
    start_election_timer(); // start a new election timer.
    size_t vote_count = 1;  // vote for self.

    printf("server[%llu] I' m a candidate, term: %d\n", this->id, this->current_term);

    //TODO: make this step in parallel
    for (const auto& [server_id, ptr]: other_server_connections) {
        ptr->set_timeout(election_timer_base.count() + rand() % election_timer_fluctuate.count());
        VoteResult vote_result = ptr->call<VoteResult>("request_vote", this->id, this->id, 0, 0).val();
        vote_count += vote_result.vote_granted ? 1 : 0;
        if (vote_result.vote_granted) {
            vote_count++;
        }
    }

    if (vote_count >= majority_count) {
        this->state = State::Leader;
        as_leader();
    }
}

void Server::as_leader() {
    //TODO: create leader unique property.

    //TODO: create leader timer for send heartbeat periodically.


    printf("server[%llu] I' m a leader, term: %d \n", this->id, this->current_term);

    this->election_timer.stop();
    while (this->state == State::Leader) {
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
