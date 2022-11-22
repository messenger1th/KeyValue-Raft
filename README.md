# Key-Value Database Raft  Distributed System

## Prerequisites

* Timer: [Callback-Timer](https://github.com/messenger1th/Callback-Timer)
* RPC: Third-Party library [buttonrpc](https://github.com/button-chen/buttonrpc_cpp14) 
* State Machine: [Key-Value DataBase](https://github.com/messenger1th/SkipList)



## Features

* Leader election
* Replication and recovery 
* Snapshot and log compaction.



## Optimization

* Follower tell its expect next log rather than try.
* design multi-thread architecture to accelerate `RquestVote` and `AppendEntries`
* set  `log_increment` variable and binary search to optimize `matchIndex` updating.
* use copy-on-write technique (fork in Linux) for snapshot.



## Run

### prerequisite

* cmake
* make

### build

```shell
./make.sh
```

if you want to **remove log and other temp file and build**

```shell
./clearmake.sh
```

or just want to  **remove log and other temp file**

```shell
./rmtemp.sh
```

### Run

```cpp
bin/kvRaft/kvRaft [id] #e.g. bin/kvRaft/kvRaft 1
```

then you will see configure steps and run.

```shell
---- Configuring... 
- server connection done.
- term&vote_for done.
- snapshot warning: no snapshot found. 
- logs done.
---- Configuring done!
candidate[2] is calling request_vote, term[1], last_log-term[0]-index[0]
```



## Make Your Distributed System

In Raft class, there are three virtual method, just inherit these to make your Distributed System.

```cpp
class Raft {
    virtual void apply(const string& command); //apply command to state machine.
    virtual void install_snapshot(const string& filename); //install snapshot to file.
    virtual void load_snapshot(const string& filename); //load snapshot from file.
}
```





## Implementation Details

architecture and more implementation description see in [Wiki](https://github.com/messenger1th/KeyValue-Raft/wiki) 




## TODO

* feature: member change.
* snapshot: 
  * leader must occasionally send snapshots to followers that lag behind
  * let follower rather leader create snapshot  send to other .



