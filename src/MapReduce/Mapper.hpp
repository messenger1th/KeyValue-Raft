//
// Created by epoch on 10/15/22.
//

#ifndef MIT6_824_C_MAPPER_HPP
#define MIT6_824_C_MAPPER_HPP

#include <string>
#include <unordered_map>
#include <fstream>

class Mapper {
public:
    virtual void start_work() = 0;
    virtual void map(const std::string& key, const std::string& value) = 0;
};


class WordCounter: public Mapper{
public:
    WordCounter(size_t id);
    void start_work() override;
    void map(const std::string &key, const std::string &value) override;

private:
    size_t id;
    std::ofstream writer;
    std::ifstream reader;
};



#endif //MIT6_824_C_MAPPER_HPP
