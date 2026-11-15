#include "Witness.h"
#include <iostream>

Witness::Witness() : Person(), statement(""), relationToCase("Unknown") {}

Witness::Witness(const std::string& name, int age, const std::string& occupation,
                  const std::string& statement, const std::string& relationToCase)
    : Person(name, age, occupation), statement(statement),
      relationToCase(relationToCase) {}

std::string Witness::getStatement() const { return statement; }
std::string Witness::getRelationToCase() const { return relationToCase; }

void Witness::display() const {
    std::cout << "---------------------------------------------\n";
    std::cout << "WITNESS: " << name << "\n";
    std::cout << "  Age             : " << age << "\n";
    std::cout << "  Occupation      : " << occupation << "\n";
    std::cout << "  Relation to Case: " << relationToCase << "\n";
    std::cout << "  Statement       : \"" << statement << "\"\n";
}
