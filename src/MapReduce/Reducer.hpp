//
// Created by epoch on 10/15/22.
//

#ifndef MIT6_824_C_REDUCER_HPP
#define MIT6_824_C_REDUCER_HPP

#include <string>
#include <list>
class Reducer {
    virtual void reduce(const std::string, const std::list<std::string>) = 0;
};


class WordAdder: public Reducer {
    void reduce(const std::string key, const std::list<std::string> values) override;
};

void WordAdder::reduce(const std::string key, const std::list<std::string> values) {

}
#endif //MIT6_824_C_REDUCER_HPP
