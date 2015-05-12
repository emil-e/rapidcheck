#include <rapidcheck.h>

#include "MapParser.h"

using namespace rc;

int main() {
  rc::check("serializing and then parsing should yield original map",
            [](const std::map<std::string, std::string> &map) {
              RC_PRE(map.find("") == map.end());
              RC_ASSERT(parseMap(mapToString(map)) == map);
            });
}
