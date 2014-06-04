#include <iostream>
#include <map>

#include <rapidcheck.h>

using namespace rc;

DESCRIBE("stuff")
{
    it("does things the right way",
       [](const std::vector<std::map<std::string, int>> &x) {
           for (auto &y : x) {
               if (y.find("ab") != y.end())
                   return false;
           }

           return true;
       });

    it("does some other stuff",
       [](const std::vector<std::map<int, int>> &x) {
           for (auto &y : x) {
               for (auto &pair : y) {
                   if (pair.first == pair.second && y.size() > 10)
                       return false;
               }
           }

           return true;
       });
}

DESCRIBE("other things")
{
    it("does things the right way",
       [](const std::vector<std::map<std::string, int>> &x) {
           for (auto &y : x) {
               if (y.find("ab") != y.end())
                   return false;
           }

           return true;
       });

    it("does some other stuff",
       [](const std::vector<std::map<int, int>> &x) {
           for (auto &y : x) {
               for (auto &pair : y) {
                   if (pair.first == pair.second && y.size() > 10)
                       return false;
               }
           }

           return true;
       });
}

int main(int argc, char **argv)
{
    rapidcheck(argc, argv);
    return 0;
}
