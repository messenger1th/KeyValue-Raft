# Raft

## Flow

![image-20221023195122319](./README/image-20221023195122319.png)

### Starts Up

be a follower when start up, steps as follows.

1. read configuration from configuration file.
   1. all servers in cluster and their IP address & Port information.
2. start a timer for election, run a RPC server in thread for vote request, append entry requests.



### Election: As Follower

do nothing but response to RPC request like voting  and append entry and reset election timer every time it receive RPC request from leader or candidate.



### Election: As Candidate

after being a candidate, steps  as follows.

revert to follower state anytime received a valid append entry which means a new leader win the vote. 

1. set a voting timer, if election timeout, start a new election.
2. increase  current  term, otherwise voting request will be rejected.



### Restart From Crash

#### Restart: As Follower

#### Restart: As Candidate

#### Restart: As Leader





## Problem&How to Solve

