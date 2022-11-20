# Project

## Introduction?



## Raft线性一致性

抽象定义上， 一个分布式系统里的所有进程要确定一个值v，如果这个系统满足如下几个性质， 就可以认为它解决了分布式一致性问题, 分别是:

- Termination: 所有正常的进程都会认同同一个值，不会出现一直在循环的进程。
- Agreement: 所有正常的进程选择的值都是一样的。
- Validity: 任何正常的进程确定的值v', 那么v'肯定是某个进程提交的。比如随机数生成器就不满足这个性质.

后来演变成更精炼、更常用的两个属性

* Safety：安全性
* Liveness：活性



而Raft是怎么保证的呢？

* Termination: 只要该log被commit，即可应用到state machine。即保证了Termination。
* Agreement(Safety): 由leader去除冲突的日志，并commit通过的日志。即保证了Agreement。
* Validity: 





## 项目难点

* 论文不是实现，需要自己理解把握原理。
* 分布式系统对我来说算是一个比较新的领域，接到的挺多新的原理，算法，概念等。
  * RPC
  * 需要自己实现状态机

* 环境和工具
  * 第一次在完全在Ubuntu系统下开发。
  * git，cmake和makefile进一步使用。
  * 理解了编译链接过程。

* Debug困难
  * 由于是多线程和多进程之间的通信，甚至是网络延迟的.
  * 由于网络分区的概率小，难以模拟。对于每次的模拟结果又难以复现。
* 需要自己设计架构
  * 需要自己解决race condition
  * 需要自己使用多线程优化，线程通信。
  * 当使用Linux系统提供的copy-on-write技术时，需要涉及进程通信。

* 考虑优化方面。
* 项目解耦，扩展性。
  * Callback-Timer的设计
  * Raft层和应用层的解耦




## Optimization

### Done

**`commit_index`更新优化**

* 版本1：数据排序找中点。时空复杂度$O(nlog n)$
* 版本2：二分查找找中点。时间复杂度$O(nlogm)$ 其中m为leader的commit_index - 最小的`match_index`.空间复杂度$O(1)$
* 版本3：使用增量来减少更新次数
  * 仅仅在`previous_match_index <= commit_index && previous_match_index <= this->commit_index && match_index[server_id] >= this->commit_index` 检查，并且需要满足`match_index_increments >= majority_count / 2`才更新。




**`append_entry`优化**

由follower告诉leader自己期待的下一个日志，而非由leader一个一个试。



### TODO

**`append_entries`**

* leader接到请求时，边写log边向follower复制该log。



**快照Snapshot**

1. 日志压缩时，使用copy-on-write技术。
2. 选择合适的快照生成时机。



**同步改异步优化**

* 网络分区只是小概率事件，我们可以使用异步RPC，直接返回，默认follower写入成功。
* 如果网络错误，或者跟随者返回错误。则leader重新调整nextIndex。



**log冲突仅仅删除冲突部分**





