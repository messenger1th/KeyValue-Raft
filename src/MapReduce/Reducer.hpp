//
// Created by epoch on 10/15/22.
//

#ifndef MIT6_824_C_REDUCER_HPP
#define MIT6_824_C_REDUCER_HPP

#include <string>
#include <list>
#include <string>
#include <fstream>

class Reducer {
    virtual void reduce(const std::string, const std::list<std::string>) = 0;
};


class WordAdder: public Reducer {
public:
    WordAdder(size_t id);
    void start_work();
    void reduce(const std::string key, const std::list<std::string> values) override;

private:
    std::ofstream writer;
    std::ifstream reader;
    size_t id;
};


#endif //MIT6_824_C_REDUCER_HPP
