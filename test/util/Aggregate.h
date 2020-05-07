#pragma once

#include "rapidcheck.h"

namespace rc {
  struct Aggregate {
    int x;
    int y;
  };

  template<>
  struct Arbitrary<Aggregate> {
    static Gen<Aggregate> arbitrary() {
      return gen::construct<Aggregate>(gen::arbitrary<int>(),
                                       gen::arbitrary<int>());
    }
  };

  struct NestedAggregate {
    Aggregate a;
    int z;
  };
} // namespace rc
