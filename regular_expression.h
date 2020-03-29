//
// Created by zawawy on 3/27/20.
//

#ifndef LEXGEN_REGULAR_EXPRESSION_H
#define LEXGEN_REGULAR_EXPRESSION_H

#include <string>
#include <vector>

class RegularExpression {
private:
    std::string name_;
    std::string value_;
    bool isReservedCharacter(char character);
public:
    RegularExpression(const std::string& name, const std::string& value);
    const std::string& getName() const;
    const std::string& getValue() const;
    void applyRangeOperationIfExists();
    std::vector<std::string> extractInputSymbols();
};

#endif //LEXGEN_REGULAR_EXPRESSION_H