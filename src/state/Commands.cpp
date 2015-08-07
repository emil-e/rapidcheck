#include "rapidcheck/state/Commands.hpp"

namespace rc {
namespace state {
namespace detail {

std::vector<std::vector<int>> commandIndeciesPermutations(int s1count, int s2count)
{
   // Create a vector with the desired number of zeros and ones
   std::vector<int> indecies(s1count, 0);
   std::vector<int> ones(s2count, 1);
   indecies.insert(indecies.end(), ones.begin(), ones.end());

   std::vector<std::vector<int>> permutations;
   permutations.push_back(indecies);

   while(std::next_permutation(indecies.begin(), indecies.end()))
   {
      permutations.push_back(indecies);
   }

   return permutations;
}

} // namespace detail
} // namespace state
} // namespace rc
