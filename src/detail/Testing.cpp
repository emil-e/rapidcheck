#include "Testing.h"

namespace rc {
namespace detail {
namespace {

int sizeFor(const TestParams &params, int i) {
  // We want sizes to be evenly spread, even when maxSuccess is not an even
  // multiple of the number of sizes (i.e. maxSize + 1). Another thing is that
  // we always want to ensure that the maximum size is actually used.

  const auto numSizes = params.maxSize + 1;
  const auto numRegular = (params.maxSuccess / numSizes) * numSizes;
  if (i < numRegular) {
    return i % numSizes;
  }

  const auto numRest = params.maxSuccess - numRegular;
  if (numRest == 1) {
    return 0;
  } else {
    return ((i % numSizes) * params.maxSize) / (numRest - 1);
  }
}

} // namespace

SearchResult searchProperty(const Property &property,
                            const TestParams &params,
                            TestListener &listener) {
  SearchResult searchResult;
  searchResult.type = SearchResult::Type::Success;
  searchResult.numSuccess = 0;
  searchResult.numDiscarded = 0;
  searchResult.tags.reserve(params.maxSuccess);

  const auto maxDiscard = params.maxDiscardRatio * params.maxSuccess;

  auto recentDiscards = 0;
  auto r = Random(params.seed);
  while (searchResult.numSuccess < params.maxSuccess) {
    const auto size =
        sizeFor(params, searchResult.numSuccess) + (recentDiscards / 10);
    const auto random = r.split();

    auto shrinkable = property(random, size);
    auto caseDescription = shrinkable.value();
    listener.onTestCaseFinished(caseDescription);
    const auto &result = caseDescription.result;

    switch (result.type) {
    case CaseResult::Type::Failure:
      searchResult.type = SearchResult::Type::Failure;
      searchResult.failure =
          SearchResult::Failure(std::move(shrinkable), size, random);
      return searchResult;

    case CaseResult::Type::Discard:
      searchResult.numDiscarded++;
      recentDiscards++;
      if (searchResult.numDiscarded > maxDiscard) {
        searchResult.type = SearchResult::Type::GaveUp;
        searchResult.failure =
            SearchResult::Failure(std::move(shrinkable), size, random);
        return searchResult;
      }
      break;

    case CaseResult::Type::Success:
      searchResult.numSuccess++;
      recentDiscards = 0;
      if (!caseDescription.tags.empty()) {
        searchResult.tags.push_back(std::move(caseDescription.tags));
      }
      break;
    }
  }

  return searchResult;
}

std::pair<Shrinkable<CaseDescription>, std::vector<std::size_t>>
shrinkTestCase(const Shrinkable<CaseDescription> &shrinkable,
               TestListener &listener) {
  std::vector<std::size_t> path;
  Shrinkable<CaseDescription> best = shrinkable;

  auto shrinks = shrinkable.shrinks();
  std::size_t i = 0;
  while (auto shrink = shrinks.next()) {
    auto caseDescription = shrink->value();
    bool accept = caseDescription.result.type == CaseResult::Type::Failure;
    listener.onShrinkTried(caseDescription, accept);
    if (accept) {
      best = std::move(*shrink);
      shrinks = best.shrinks();
      path.push_back(i);
      i = 0;
    } else {
      i++;
    }
  }

  return std::make_pair(std::move(best), std::move(path));
}

} // namespace detail
} // namespace rc
