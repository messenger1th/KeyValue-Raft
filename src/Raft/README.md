# Raft

## Flow

![image-20221023195122319](./README/image-20221023195122319.png)

### Starts Up

be a follower when set_callback up, steps as follows.

1. read configuration from configuration file.
   1. all servers in cluster and their IP address & Port information.
2. set_callback a timer for election, run a RPC server in thread for vote request, append entry requests.



### Election: As Follower

do nothing but response to RPC request like voting  and append entry and restart election timer every time it receive RPC request from leader or candidate.



### Election: As Candidate

after being a candidate, steps  as follows.

revert to follower state anytime receives a valid append entry which means a new leader win the vote. 

1. set a voting timer, if election timeout, set_callback a new election.
2. increase  current  term, otherwise voting request will be rejected.
3. vote for self
4. if timeout, restart elec set_callback a new election.
5. sent `VoteRequest` RPC in parallel, become leader if granted by majority, finish this election before election timer set_callback a new election. 



### Restart From Crash

#### Restart: As Follower

#### Restart: As Candidate

#### Restart: As Leader



### Add a new Server into Cluster



## Details not Mentioned in Paper

### When to set `voteFor` to `null` ? 

if RPC request or response contains term T > current term,  set it `null`.

when a server receives a `RequestVote` RPC with a term higher than its own, it should update the term to the number observed **and also restart the `votedFor` to `null`** (meaning that in this case, it will always vote for the requesting server).

Link: https://stackoverflow.com/questions/50425312/in-raft-distributed-consensus-what-do-i-set-votedfor-to



### Will a candidate with huge current term break current term?

Yes, as mentioned above, anytime leader receive a term higher than its current term, will update its term and convert to a follower.

Link: https://stackoverflow.com/questions/71230789/raft-will-term-increasing-all-the-time-if-partitioned





### When should a follower set its election timer?

1. before checking the log matching property
2. Follower decides to grant its vote to that Candidate

Link: https://stackoverflow.com/questions/66944088/when-should-a-raft-follower-record-an-rpc



### what is `matchIndex` used to ? 

`matchIndex`record the accurate replicated log index of follower, so it's used to  locate  current commit index.

Link: https://stackoverflow.com/questions/46376293/what-is-lastapplied-and-matchindex-in-raft-protocol-for-volatile-state-in-server



### When should a leader set voteFor to null while receiving a voteRequest with higher term in Raft?

> For example, if you have already voted in the current term, and an incoming RequestVote RPC has a higher term that you, you should first step down and adopt their term (thereby resetting votedFor), and then handle the RPC, which will result in you granting the vote!

Namely when receiving a `voteReqest` RPC with higher term, set `voteFor` to `null` and operate later log check, rather than just set `voteFor` to `null` and return false.

Link: [Raft Q&A](https://thesquareplanet.com/blog/raft-qa/)



### Should a leader hold a timer? 



## Problem&How to Solve

### How to understand ? 

> A leader is not allowed to update `commitIndex` to somewhere in a *previous* term (or, for that matter, a future term). Thus, as the rule says, you specifically need to check that `log[N].term == currentTerm`. This is because Raft leaders cannot be sure an entry is actually committed (and will not ever be changed in the future) if it’s not from their current term. This is illustrated by Figure 8 in the paper.

Actually, committing log wit a previous term, which  means possibly be applied to state machine,  will causing inconsistency because these logs maybe overwritten in some condition like Figure 8 mentioned in paper.

Other Answer: https://stackoverflow.com/questions/60397950/confusion-about-raft-algorithm



### How does Raft deals with delayed replies in AppendEntries RPC?

https://stackoverflow.com/questions/56677690/how-does-raft-deals-with-delayed-replies-in-appendentries-rpc



## Preference

1. Paper: https://raft.github.io/raft.pdf

2. MIT6.824 Lab : https://pdos.csail.mit.edu/6.824/

3. Blog: https://thesquareplanet.com/blog/students-guide-to-raft/

   

### FAQ Links

1. https://thesquareplanet.com/blog/raft-qa/
2. https://stackoverflow.com/questions/60397950/confusion-about-raft-algorithm
