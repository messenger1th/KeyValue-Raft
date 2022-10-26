//
// Created by epoch on 10/24/22.
//

#include "Server.hpp"

//TODO:delete it;
#include <stdbool.h>
#include <csignal>

Server::Server(size_t id, const std::string &pair) : Server(id, pair.substr(0, pair.find(':')),
                                                            std::stoul(pair.substr(pair.find(':') + 1))) {}

Server::Server(size_t id, const std::string &IP, const size_t &port):
    id(id)
{
    //TODO
}


void Server::read_config() {
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

    election_timer.set_callback(&Server::as_candidate, this);
}

void Server::starts_up() {
//    sleep(3);
    start_election_timer();
}

VoteResult Server::request_vote(size_t term, size_t candidate_id, size_t last_log_index, size_t last_log_term) {

    printf("candidate[%llu] is calling request_vote, term[%llu]\n", candidate_id, term);

    VoteResult res{this->current_term, false};
    /* 1. Reply false if term < currentTerm (§5.1) */
    if (this->current_term > term) {
        return res;
    } else if (this->current_term < term) {
        update_current_term(term);
    }

    /* 2. Reply false, if votedFor is not null or candidateId, */
    if (this->vote_for != null && this->vote_for != candidate_id) {
        return res;
    }

    /* 3. Reply false, if candidate’s log is not at least as up-to-date as receiver’s log (§5.2, §5.4)*/
    if (this->last_log_term > last_log_term || (this->last_log_term == last_log_term && this->last_log_index > last_log_index)) {
        return res;
    }

    this->election_timer.reset();
    res.vote_granted = true;
    return res;
}

AppendResult Server::append_entries(size_t term, size_t leader_id, size_t prev_log_index, size_t prev_log_term,
                                    const std::vector<LogEntry> &entries, size_t leader_commit) {

    AppendResult res{this->current_term, false};
    if (this->current_term > term) {
        return res;
    } else if (this->current_term  < term) {
        update_current_term(term);
    }


    if (this->current_term < term) {
        return res;
    }
    //TODO step2: Reply false if log doesn’t contain an entry at prevLogIndex whose term matches prevLogTerm (§5.3)

    /* after checking term, in this time point, the RPC requester would be a valid leader. */
    this->election_timer.reset();

    //TODO step3: If an existing entry conflicts with a new one (same index
    //but different terms), delete the existing entry and all that
    //follow it (§5.3)

    //TODO step4: Append any new entries not already in the logAppend any new entries not already in the log

    //TODO step5: If leaderCommit > commitIndex, set commitIndex = min(leaderCommit, index of last new entry)
    this->election_timer.reset();
    res.success = true;
    return res;
}

void Server::as_candidate() {

    /* steps after being a candidate */
    this->state = State::Candidate; //change state.
    ++this->current_term;   // increment term.
    election_timer.reset(); // start a new election timer.
    size_t vote_count = 1;  // vote for self.

    printf("server[%llu] I' m a candidate, term: %d\n", this->id, this->current_term);

    //TODO: make this step in parallel
    for (const auto& [server_id, ptr]: other_server_connections) {
        ptr->set_timeout(election_timer_base.count() / 2 );
        VoteResult vote_result = ptr->call<VoteResult>("request_vote", this->id, this->id, 0, 0).val();
        vote_count += vote_result.vote_granted ? 1 : 0;
        if (this->current_term < vote_result.term) {
            update_current_term(this->current_term);
            return;
        }
    }

    if (vote_count >= majority_count) {
        this->state = State::Leader;
        as_leader();
    }
}

void Server::as_leader() {
    election_timer.stop();

    //TODO: create leader unique property.

    //TODO: create leader timer for send heartbeat periodically.


    printf("server[%llu] I' m a leader, term: %d \n", this->id, this->current_term);

    while (this->state == State::Leader) {
        for (const auto& [server_id, ptr]: other_server_connections) {
            ptr->set_timeout(this->election_timer_base.count() / 2);
            AppendResult append_result = ptr->call<AppendResult>("append_entries", this->current_term, this->id, 0, 0, vector<LogEntry>{}, 0).val();
            printf("Term[%llu] Send append_entry to %llu, Response: %d \n", this->current_term, server_id, append_result.success);
            if (append_result.term > current_term) {
                update_current_term(append_result.term);
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delay / 2));
    }
}

void Server::start_election_timer() {
    this->election_timer.reset_period(ms(election_timer_base.count() + rand() % election_timer_fluctuate.count()));
    this->election_timer.reset();
    this->election_timer.run();
}


std::string Server::Hello(size_t id)  {

    this->election_timer.reset();
    printf("server[%d] send Hello\n", id);
    return "Hello";
}

