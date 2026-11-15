#ifndef PERSON_H
#define PERSON_H

#include <string>

class Person {
protected:
    std::string name;
    int age;
    std::string occupation;

public:
    Person();
    Person(const std::string& name, int age, const std::string& occupation);
    virtual ~Person();

    std::string getName() const;
    int getAge() const;
    std::string getOccupation() const;

    void setName(const std::string& n);
    void setAge(int a);
    void setOccupation(const std::string& o);

    virtual void display() const;
};

#endif
