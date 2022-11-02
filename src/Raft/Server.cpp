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
    for (size_t i = 1; i <= 3; ++i) {
        size_t port = i + 5000;
        if (i != this->id) {
            other_servers.emplace(i, NetAddress{"127.0.0.1", port});
            other_server_connections.emplace(i, make_unique<buttonrpc>());
            other_server_connections[i]->as_client("127.0.0.1", port);
            other_server_connections[i]->set_timeout(this->election_timer_base.count() / 2);
        }
    }
    election_timer.set_callback(&Server::as_candidate, this);
    printf("Configuration Done!\n");
}

void Server::starts_up() {
    start_election_timer();
}

VoteResult Server::request_vote(size_t term, size_t candidate_id, size_t last_log_index, size_t last_log_term) {

    printf("candidate[%lu] is calling request_vote, term[%lu], last_log-term[%lu]-index[%lu]\n", candidate_id, term, last_log_term, last_log_index);

    VoteResult res{this->current_term, false};
    /* 1. Reply false if term < currentTerm (§5.1) */
    if (this->current_term > term) {
        return res;
    } else if (this->current_term < term) {
        //TODO: should this step after set vote_for to null ? otherwise it vote for the requester if requester has a higher term with appropriate log.e
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
    std::cout << "request_vote: resetted " << std::endl;
    this->election_timer.restart();
    this->vote_for = candidate_id;
    res.vote_granted = true;
    return res;
}

AppendResult Server::append_entries(size_t term, size_t leader_id, size_t prev_log_index, size_t prev_log_term,
                                    const string &entries, size_t leader_commit) {
    AppendResult res{this->current_term, false};
    if (this->id == 3) {
        printf("Leader[%lu] append-term[%lu]-prev_log_index[%lu]-term[%lu], my-log-term[%lu]-index[%lu]-return term[%lu]-success[%d]\n", leader_id, term, prev_log_index, prev_log_term, logs.back().term, logs.back().index, res.term,res.success);
//        return res;
    };

    if (this->current_term > term) {
        printf("Leader[%lu] append-term[%lu]-prev_log_index[%lu]-term[%lu], my-log-term[%lu]-index[%lu]-return term[%lu]-success[%d]\n", leader_id, term, prev_log_index, prev_log_term, logs.back().term, logs.back().index, res.term,res.success);
        return res;
    } else if (this->current_term  < term) {
        update_current_term(term);
    }

    /* reset its election timer after get a valid term, before checking the log matching property */
    this->election_timer.restart();

    // step2: Reply false if log doesn’t contain an entry at prevLogIndex whose term matches prevLogTerm (§5.3)
    if (!match_prev_log_term(prev_log_index, prev_log_term)) {
        printf("Leader[%lu] append-term[%lu]-prev_log_index[%lu]-term[%lu], my-log-term[%lu]-index[%lu]-return term[%lu]-success[%d]\n", leader_id, term, prev_log_index, prev_log_term, logs.back().term, logs.back().index, res.term,res.success);
        return res;
    }


    /* step3: If an existing entry conflicts with a new one (same index but different terms), delete the existing entry and all that follow it (§5.3) */
    if (log_conflict(prev_log_index, prev_log_term)) {
        printf("Server[%lu]: Log[%lu] Conflicts \n", this->id, last_log_index);
        remove_conflict_logs(prev_log_index);
    }


    /* step4: Append any new entries not already in the logAppend any new entries not already in the log */
    auto appending_entries = parse_string_logs(entries);
    append_logs(appending_entries);

    /* Step5: If leaderCommit > commitIndex, set commitIndex = min(leaderCommit, index of last new entry) */
    if (leader_commit > commit_index) {
        this->commit_index = min(leader_commit, last_log_index);
        if (this->last_applied < this->commit_index) {

            this->last_applied = this->commit_index; cout << "change start a apply thread" << endl;

//            thread t(&Server::apply_entries, this);
            //TODO: for application layer, start a thread to increment & applied to state machine.
        }
    }

    res.success = true;
    printf("Leader[%lu] append-term[%lu]-prev_log_index[%lu]-term[%lu], my-log-term[%lu]-index[%lu]-return term[%lu]-success[%d]\n", leader_id, term, prev_log_index, prev_log_term, logs.back().term, logs.back().index, res.term, res.success);
    return res;
}

void Server::as_candidate() {
    if (this->id == 3) {
        cout << "You can't be candidate." << endl;
        return;
    }

    /* steps after being a candidate */
    this->state = State::Candidate; //change state.
    ++this->current_term;   // increment term.
    election_timer.restart(); // start a new election timer.
    size_t vote_count = 1;  // vote for self.
    this->vote_for = this->id;



    auto size = get_total_log_size();
    auto last_log_info = get_last_log_info();
    printf("server[%lu] I' m a candidate, term: %lu, logs-size[%lu]--last-term[%lu]-index[%lu]\n", this->id, this->current_term, size, last_log_info.first, last_log_info.second);

    //TODO: make this step in parallel
    for (const auto& [server_id, ptr]: other_server_connections) {
        auto last_log_info = get_last_log_info();
        VoteResult vote_result = ptr->call<VoteResult>("request_vote", this->current_term, this->id, last_log_info.first, last_log_info.second).val();
        vote_count += vote_result.vote_granted ? 1 : 0;
        if (vote_result.vote_granted) {
            printf("%lu vote me(Server[%lu])\n", server_id, this->id);
        }
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
    /* stop election_timer  */
    election_timer.stop();
    auto next_log_index = get_last_log_info().second + 1;
    next_index.clear();
    match_index.clear();
    for (const auto&[server_id, _]: other_server_connections) {
        next_index[server_id] = next_log_index;
        match_index[server_id] = 0;
    }

    printf("server[%lu] I' m a leader, term: %lu, log_size[%lu]-last_index[%lu]\n", this->id, this->current_term,this->logs.size(), this->logs.back().index);

    for (const auto& [server_id, ptr]: other_server_connections) {
        thread t(&Server::send_log_heartbeat, this, server_id);
        t.detach();
    }

    printf("send threads created. simulated log append. \n");

    //TODO: delete it after developed client.
    while (this->state == State::Leader) {
        append_log_simulate();
        std::this_thread::sleep_for(std::chrono::milliseconds(delay / 3 * 2));
    }
}

void Server::start_election_timer() {
    this->election_timer.reset_period(ms(election_timer_base.count() + rand() % election_timer_fluctuate.count()));
    this->election_timer.restart();
}

/* origin RPC function for debug. */
std::string Server::Hello(size_t id)  {

    this->election_timer.restart();
    printf("server[%lu] send Hello\n", id);
    return "Hello";
}


void Server::remove_conflict_logs(size_t index) {
    //TODO: binary search the logs, rather than just remove it all ?
    //TODO: update it after change way of log store.
    cout << "size: " << index - logs[0].index + 1 << endl;
    this->logs.resize(index - logs[0].index + 1);
}

void Server::append_logs(const vector<LogEntry>& entries) {
    std::unique_lock<std::shared_mutex> append_logs_lock(this->logs_mutex);
    for (const auto& entry: entries) {
        this->logs.emplace_back(entry);
    }

    // TODO 1: update it after log compaction,
    // TODO 2: use a lock to read ?
    this->last_log_term = logs.back().term;
    this->last_log_index = logs.back().index;
    printf("current_last_log_info-size[%lu]-term[%lu]-index[%lu]\n", this->logs.size(), last_log_term, last_log_index);
}

void Server::send_log_heartbeat(size_t server_id) {
    const auto& ptr = other_server_connections[server_id];
    while (this->state == State::Leader) {

        size_t send_log_size = get_send_log_size(server_id);
        std::string entries = get_log_string(next_index[server_id], next_index[server_id] + send_log_size);
        size_t prev_log_index = next_index[server_id] - 1;
        size_t prev_log_term = get_log_term(prev_log_index);

        AppendResult append_result = ptr->call<AppendResult>("append_entries", this->current_term, this->id, prev_log_index,
                                                             prev_log_term, entries, this->commit_index).val();

        if (append_result.term > current_term) {
            update_current_term(append_result.term);
            return;
        }

        if (append_result.success) {
            next_index[server_id] += send_log_size;
            //TODO: should update matchIndex ?
            match_index[server_id] = prev_log_index + send_log_size;
            printf("Term[%lu] Send append_entry to %lu, Response: %d , next_index: [%lu], matchIndex[%lu]\n", this->current_term, server_id, append_result.success, next_index[server_id], match_index[server_id].load());
            if (match_index[server_id] > commit_index) {
                //todo: finish this following step
//                update_commit_index(find_match_index_median());
            }
        } else {
            /* decrement nextIndex and retry; */
            if (append_result.term != 0) {
                printf("server[%lu]: inconsistency!\n", server_id);
                next_index[server_id] -= 1;
                assert(next_index[server_id] > 0);
                continue;
            } else {
                printf("server[%lu]: crash down! \n", server_id);
            }
        }
        /* send heartbeat periodically. */
        std::this_thread::sleep_for(std::chrono::milliseconds(delay / 3 * 2));
    }
}


LogEntry::LogEntry(size_t term, size_t index, const string &command) : term(term), index(index), command(command) {}
ostream& operator<<(ostream& out, const LogEntry& entry) {
    out << entry.term << ' ' << entry.index << ' ' << entry.command;
    return out;
}
