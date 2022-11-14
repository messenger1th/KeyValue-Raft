# Project

## Opitimzation

**`append_entry`冲突优化**

由follower告诉leader自己期待的下一个日志，而非一个一个试。







**`commit_index`更新优化**

* 版本1：数据排序找中点。时空复杂度$O(nlog n)$
* 版本2：二分查找找中点。时间复杂度$O(nlogm)$ 其中m为leader的commit_index - 最小的`match_index`.空间复杂度$O(1)$
* 版本3：使用增量来减少更新次数
  * 仅仅在`previous_match_index <= commit_index && previous_match_index <= this->commit_index && match_index[server_id] >= this->commit_index` 检查，并且需要满足`match_index_increments >= majority_count / 2`才更新。




