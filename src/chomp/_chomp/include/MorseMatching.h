/// MorseMatching.h
/// Shaun Harker
/// 2017-07-19
/// MIT LICENSE

#pragma once

#include <memory>
#include <unordered_set>
#include <vector>

#include "Integer.h"
#include "Chain.h"
#include "Complex.h"
#include "Fibration.h"

class MorseMatching {
public:
  /// MorseMatching
  MorseMatching ( std::shared_ptr<Complex> complex_ptr ) {
    Complex const& complex = *complex_ptr;
    Integer N = complex.size();
    mate_.resize(N,-1);
    priority_.resize(N);
    Integer num_processed = 0;
    std::vector<Integer> boundary_count (N);
    std::unordered_set<Integer> coreducible;
    std::unordered_set<Integer> ace_candidates;

    for ( auto x : complex ) {
      boundary_count[x] = complex.boundary({x}).size();
      switch ( boundary_count[x] ) {
        case 0: ace_candidates.insert(x); break;
        case 1: coreducible.insert(x); break;
      }
    }

    auto process = [&](Integer y){
      priority_[y] = num_processed ++;
      coreducible.erase(y);
      ace_candidates.erase(y);
      for ( auto x : complex.coboundary({y}) ) {
        boundary_count[x] -= 1;
        switch ( boundary_count[x] ) {
          case 0: coreducible.erase(x); ace_candidates.insert(x); break;
          case 1: coreducible.insert(x); break;
        }
      }
    };

    while ( num_processed < N ) {
      if ( not coreducible.empty() ) {
        Integer K, Q;
        // Extract K
        auto it = coreducible.begin(); K = *it; coreducible.erase(it); // pop from unordered_set
        // Find mate Q
        for ( auto x : complex.boundary({K}) ) if ( mate_[x] == -1 ) { Q = x; break; }
        mate_[K] = Q; mate_[Q] = K;
        process(Q); process(K);
      } else {
        Integer A;
        auto it = ace_candidates.begin(); A = *it; ace_candidates.erase(it); // pop from unordered_set
        mate_[A] = A;
        process(A);
      }
    }
  }

  // DRY mistake -- only a few lines differ. 
  MorseMatching ( std::shared_ptr<Fibration> fibration_ptr ) {
    Fibration const& fibration = *fibration_ptr;
    Complex const& complex = *fibration.complex();
    Integer N = complex.size();
    mate_.resize(N,-1);
    priority_.resize(N);
    Integer num_processed = 0;
    std::vector<Integer> boundary_count (N);
    std::unordered_set<Integer> coreducible;
    std::unordered_set<Integer> ace_candidates;

    auto bd = [&](Integer x) {
      Chain result;
      auto x_val = fibration.value(x);
      for ( auto y : complex.boundary({x}) ) {
        auto y_val = fibration.value(y);
        if ( y_val > x_val ) {
          throw std::logic_error("fibration closure property failed line MorseMatching line 87");
        }
        if ( x_val == y_val ) result += y;
      }
      return result;
    };

    auto cbd = [&](Integer x) {
      Chain result;
      auto x_val = fibration.value(x);
      for ( auto y : complex.coboundary({x}) ) {
        if ( x_val == fibration.value(y) ) result += y;
      }
      return result;
    };

    for ( auto x : complex ) {
      boundary_count[x] = bd(x).size();
      switch ( boundary_count[x] ) {
        case 0: ace_candidates.insert(x); break;
        case 1: coreducible.insert(x); break;
      }
    }

    auto process = [&](Integer y){
      priority_[y] = fibration.value(y)*complex.size() + num_processed ++;
      coreducible.erase(y);
      ace_candidates.erase(y);
      for ( auto x : cbd(y) ) {
        boundary_count[x] -= 1;
        switch ( boundary_count[x] ) {
          case 0: coreducible.erase(x); ace_candidates.insert(x); break;
          case 1: coreducible.insert(x); break;
        }
      }
    };

    while ( num_processed < N ) {
      if ( not coreducible.empty() ) {
        Integer K, Q;
        // Extract K
        auto it = coreducible.begin(); K = *it; coreducible.erase(it); // pop from unordered_set
        // Find mate Q
        for ( auto x : bd(K) ) if ( mate_[x] == -1 ) { Q = x; break; }
        if ( fibration.value(K) != fibration.value(Q) ) {
          throw std::logic_error("fibration error -- memory bug? MorseMatching line 132");
        }
        mate_[K] = Q; mate_[Q] = K;
        process(Q); process(K);
      } else {
        Integer A;
        auto it = ace_candidates.begin(); A = *it; ace_candidates.erase(it); // pop from unordered_set
        mate_[A] = A;
        process(A);
      }
    }
  }

  /// mate
  Integer
  mate ( Integer x ) const { 
    return mate_[x];
  }

  /// priority
  Integer
  priority ( Integer x ) const { 
    return priority_[x];
  }

private:
  std::vector<Integer> mate_;
  std::vector<Integer> priority_;
};

/// Python Bindings

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

inline void
MorseMatchingBinding(py::module &m) {
  py::class_<MorseMatching, std::shared_ptr<MorseMatching>>(m, "MorseMatching")
    .def(py::init<std::shared_ptr<Complex>>())
    .def(py::init<std::shared_ptr<Fibration>>())    
    .def("mate", &MorseMatching::mate)
    .def("priority", &MorseMatching::priority);
}