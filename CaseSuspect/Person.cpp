#include "Person.h"
#include <iostream>

Person::Person() : name("Unknown"), age(0), occupation("Unknown") {}

Person::Person(const std::string& name, int age, const std::string& occupation)
    : name(name), age(age), occupation(occupation) {}

Person::~Person() {}

std::string Person::getName() const { return name; }
int Person::getAge() const { return age; }
std::string Person::getOccupation() const { return occupation; }

void Person::setName(const std::string& n) { name = n; }
void Person::setAge(int a) { age = a; }
void Person::setOccupation(const std::string& o) { occupation = o; }

void Person::display() const {
    std::cout << "Name: " << name << " | Age: " << age
              << " | Occupation: " << occupation << "\n";
}
