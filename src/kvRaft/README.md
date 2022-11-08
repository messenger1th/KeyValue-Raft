# Skip List



## Interface

* `insert(K, V)`
* `erase(K)`
* `erase_all(K)`
* `erase_range(Lower, Upper)`

**Display**

* `print_list()`
* `print_list_level()`

**Other**: Some Getter





## Development Log

### Version 1.0: Template & Single Thread

**Feature**

1. Template for any type of Key & Value 
2. support same value insertion
3. support single key erase and erase_all key
4. support range erase
5. use `shared_ptr` for resource manage. 



### Version 2.0: Support Multi-Task

**Feature**

1.  Use `shared_mutex` (Since C++17) support for multi thread read & write.
2.  Use C++ standard library for cross-platform



## Performance
### Single Thread 

Parameters: 
* Skip List Level : 20


| Operation \ Data Size(k) |    10     |   100    |  1000   |
| :----------------------: | :-------: | :------: | :-----: |
|          insert          | 0.0198597 | 0.188613 | 1.98124 |
|          search          | 0.0105348 | 0.189374 | 1.21571 |
|          erase           | 0.0123729 | 0.164983 | 1.26743 |




### Multi-Thread

Parameters: 

* Skip List Level : 20
* Worker Thread Count : 8

| Operation \ Data Size(k) |     10     |    100    |   1000   |
| :----------------------: | :--------: | :-------: | :------: |
|          insert          | 0.0318995  | 0.417403  | 3.57796  |
|          search          | 0.00314293 | 0.0659986 | 0.371875 |
|          erase           | 0.0307055  | 0.437583  | 3.48913  |



## Todo\Wish List
1. Use Thread Pool (Based On C++14) for Unit Test, which is more practicable.
2. Use More Modern Feature of C++, such as `package_task`, 
3. Use more small granularity lock for better Read & Write performance.
4. Use makefile for better extension & compile performance.
5. Use More systematic test frame for Unit Test: Like Docker ?  
