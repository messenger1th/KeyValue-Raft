//
// Created by epoch on 10/24/22.
//

#include "Raft.hpp"

//TODO:delete it;
#include <stdbool.h>
#include <csignal>

LogEntry::LogEntry(size_t term, size_t index, const string &command) : term(term), index(index), command(command) {}




Raft::Raft(size_t id, const std::string &pair) : Raft(id, pair.substr(0, pair.find(':')),
                                                      std::stoul(pair.substr(pair.find(':') + 1))) {}

Raft::Raft(size_t id, const std::string &IP, const size_t &port):
    id(id),
    ip(IP),
    port(port)
{
    //TODO: do something at constructor.
}


void Raft::load_connection_configuration() {
    for (size_t i = 1; i <= 3; ++i) {
        size_t port = i + 5000;
        if (i != this->id) {
            other_servers.emplace(i, NetAddress{"127.0.0.1", port});
            other_server_connections.emplace(i, make_unique<buttonrpc>());
            other_server_connections[i]->as_client("127.0.0.1", port);
            other_server_connections[i]->set_timeout(this->election_timer_base.count() / 2);
        }
    }
    printf("- server connection done.\n");
}

void Raft::load_persistent_value() {
    load_connection_configuration();
    load_term_info();
    load_snapshot();
    load_log();
}

void Raft::set_default_value() {
    if (this->logs.empty()) {
        logs.emplace_back();
    }
    this->commit_index = logs.back().index;
}

void Raft::configure() {
    printf("---- Configuring... \n");
    load_persistent_value(); /* load persistent value from log. */
    set_default_value();     /* set default config. */
    election_timer.set_callback(&Raft::become_candidate, this); /* set election callback function. */
    thread apply_thread(&Raft::apply_state_machine, this);
    apply_thread.detach();
    printf("---- Configuring done!\n");
}


/* will block here*/
void Raft::start_service() {
    configure();

    buttonrpc server_rpc;
    server_rpc.as_server(this->port);
    server_rpc.bind("Hello", &Raft::Hello, this);
    server_rpc.bind("request_vote", &Raft::request_vote, this);
    server_rpc.bind("append_entries", &Raft::append_entries, this);

    start_election_timer();

    /* will block here*/
    server_rpc.run();
}

VoteResult Raft::request_vote(size_t term, size_t candidate_id, size_t last_log_index, size_t last_log_term) {

    printf("candidate[%lu] is calling request_vote, term[%lu], last_log-term[%lu]-index[%lu]\n", candidate_id, term, last_log_term, last_log_index);

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
    const auto last_log_info = get_last_log_info();
    const auto& my_last_log_term = last_log_info.first, &my_last_log_index = last_log_info.second;
    if (my_last_log_term > last_log_term || (my_last_log_term == last_log_term && my_last_log_index > last_log_index)) {
        return res;
    }

    this->election_timer.restart();
    update_term_info(this->current_term, candidate_id);
    res.vote_granted = true;
    return res;
}

AppendResult Raft::append_entries(size_t term, size_t leader_id, size_t prev_log_index, size_t prev_log_term,
                                  const string &entries, size_t leader_commit) {
    AppendResult res{this->current_term, false, this->get_last_log_index() + 1};
    if (this->current_term > term) {
        printf("1Leader[%lu] append-term[%lu]-prev_log_index[%lu]-term[%lu], my-log-term[%lu]-index[%lu]-return term[%lu]-success[%d]\n", leader_id, term, prev_log_index, prev_log_term, logs.back().term, logs.back().index, res.term,res.success);
        return res;
    } else if (this->current_term  < term) {
        update_current_term(term);
    }

    /* reset its election timer after get a valid term, before checking the log matching property */
    this->election_timer.restart();

    // step2: Reply false if log doesn’t contain an entry at prevLogIndex whose term matches prevLogTerm (§5.3)
    if (!match_prev_log_term(prev_log_index, prev_log_term)) {
        printf("2Leader[%lu] append-term[%lu]-prev_log_index[%lu]-term[%lu], my-log-term[%lu]-index[%lu]-return term[%lu]-success[%d]\n", leader_id, term, prev_log_index, prev_log_term, logs.back().term, logs.back().index, res.term,res.success);
        return res;
    }


    /* step3: If an existing entry conflicts with a new one (same index but different terms), delete the existing entry and all that follow it (§5.3) */
    if (log_conflict(prev_log_index, prev_log_term)) {
        printf("Server[%lu]: Log[%lu] Conflicts \n", this->id, prev_log_term);
        remove_conflict_logs(prev_log_index);
        rewrite_log();
    }


    /* step4: Append any new entries not already in the logAppend any new entries not already in the log */
    auto appending_entries = parse_string_logs(entries);
    append_logs(appending_entries);


    /* Step5: If leaderCommit > commitIndex, set commitIndex = min(leaderCommit, index of last new entry) */
    if (leader_commit > commit_index) {
        update_commit_index(std::min(leader_commit, this->get_last_log_index()));
        //TODO: notify to apply to state machine after commit.
    }

    res.success = true;
    this->state = State::Follower;
    printf("sLeader[%lu] append-term[%lu]-prev_log_index[%lu]-term[%lu], my-log-term[%lu]-index[%lu]-return term[%lu]-success[%d]\n", leader_id, term, prev_log_index, prev_log_term, logs.back().term, logs.back().index, res.term, res.success);
    printf("My log size: %lu\n", this->logs.size());
    return res;
}

void Raft::become_candidate() {

    /* steps after being a candidate */
    this->state = State::Candidate; //change state.
    election_timer.restart(); // start a new election timer.
    size_t vote_count = 1;  // vote for self.
    update_term_info(this->current_term + 1, this->id); //increment term and set vote_for self.



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
            update_current_term(vote_result.term);
            return;
        }
    }

    if (vote_count >= majority_count && this->state == State::Candidate) {
        this->state = State::Leader;
        become_leader();
    } else {
        become_follower();
    }
}

void Raft::become_leader() {
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
        thread t(&Raft::send_log_heartbeat, this, server_id);
        t.detach();
    }

    printf("send threads created. simulated log append. \n");

    //TODO: delete it after developed client.
    // simulate client request.
    while (this->state == State::Leader) {
        append_log_simulate();
        std::this_thread::sleep_for(std::chrono::milliseconds(delay / 3 * 2));
    }
}

void Raft::start_election_timer() {
    this->election_timer.reset_period(ms(election_timer_base.count() + rand() % election_timer_fluctuate.count()));
    this->election_timer.restart();
}

/* origin RPC function for debug. */
std::string Raft::Hello(size_t id)  {

    this->election_timer.restart();
    printf("server[%lu] send Hello\n", id);
    return "Hello";
}


void Raft::append_logs(const vector<LogEntry>& entries) {
    size_t last_index, current_index;
    {
        std::unique_lock<std::shared_mutex> append_logs_lock(this->logs_mutex);
        last_index = this->logs.back().index;
        for (const auto& entry: entries) {
            this->logs.emplace_back(entry);
        }
        current_index = this->logs.back().index;
    }
    write_append_log(last_index, current_index);
}



void Raft::send_log_heartbeat(size_t server_id) {
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
            size_t previous_match_index = match_index[server_id];
            match_index[server_id] = prev_log_index + send_log_size;

            printf("Term[%lu] Send append_entry to %lu, Response: %d , next_index: [%lu], matchIndex[%lu]\n", this->current_term, server_id, append_result.success, next_index[server_id], match_index[server_id].load());
            printf("previous_match_index[%lu]: %lu, this->commit_index: %lu, match_index[%lu]: %lu\n", server_id, previous_match_index, this->commit_index, server_id, match_index[server_id].load());
            if (previous_match_index <= this->commit_index && match_index[server_id] >= this->commit_index) {
                this->match_index_increments += send_log_size;
                if (this->match_index_increments >= majority_count / 2) {
                    this->match_index_increments = this->match_index_increments % (majority_count / 2);
                    size_t new_commit_index = find_match_index_median();
                    /* only commit log in its term. */
                    if (this->get_log_term(new_commit_index) == this->current_term) {
                        update_commit_index(new_commit_index);
                    }
                }
            } else {
                this->match_index_increments += send_log_size;
            }
        } else {
            /* decrement nextIndex and retry; */
            if (append_result.term != 0) {
                printf("server[%lu]: inconsistency! expect_index: %lu \n ", server_id, append_result.expect_index);
                next_index[server_id] = min(next_index[server_id] - 1, append_result.expect_index);
                assert(next_index[server_id] > 0);
                continue;
            } else {
                printf("server[%lu]: crash down! \n", server_id);
            }
        }
        /* send heartbeat periodically. */
        std::this_thread::sleep_for(std::chrono::milliseconds(delay / 2));
    }
}




ostream& operator<<(ostream& out, const LogEntry& entry) {
    string oneline = to_string(entry.term) + ' ' + to_string(entry.index) + ' ' + entry.command + '\n';
    out.write(oneline.c_str(), oneline.size());
    return out;
}

istream& operator>>(istream& in, LogEntry& entry) {
    in >> entry.term >> entry.index >> entry.command;
    return in;
}
