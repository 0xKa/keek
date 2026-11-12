#include <iostream>
#include <string>
#include <vector>

struct Cat {
    std::string name;
    int napsPerDay = 16;
    bool knocksThingsOver = true;
};

int main()
{
    const std::vector<Cat> cats{{"Biscuit"}, {"Professor Whiskers", 22, false}};
    for (const Cat &cat : cats) {
        std::cout << cat.name << " naps " << cat.napsPerDay << " times a day";
        std::cout << (cat.knocksThingsOver ? " and breaks things\n" : "\n");
    }
    return 0;
}
