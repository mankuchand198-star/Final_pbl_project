#ifndef WITNESS_H
#define WITNESS_H

#include "Person.h"
#include <string>

class Witness : public Person {
private:
    std::string statement;
    std::string relationToCase;

public:
    Witness();
    Witness(const std::string& name, int age, const std::string& occupation,
            const std::string& statement, const std::string& relationToCase);

    std::string getStatement() const;
    std::string getRelationToCase() const;

    void display() const override;
};

#endif
