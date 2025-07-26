#include "CollisionProbabilityEstimator.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>

CollisionProbabilityEstimator::CollisionProbabilityEstimator(const std::vector<double>& state_vector_obj1,
                                                          const std::vector<double>& covariance_matrix_obj1,
                                                          const std::vector<double>& state_vector_obj2,
                                                          const std::vector<double>& covariance_matrix_obj2)
    : state_vector_obj1_(state_vector_obj1),
      covariance_matrix_obj1_(covariance_matrix_obj1),
      state_vector_obj2_(state_vector_obj2),
      covariance_matrix_obj2_(covariance_matrix_obj2) {
    if (state_vector_obj1.size() != 6) {
        throw std::invalid_argument("State vector for Object 1 must contain exactly 6 doubles.");
    }

    if (covariance_matrix_obj1.size() < 21 || covariance_matrix_obj1.size() > 45) {
        throw std::invalid_argument("Covariance matrix for Object 1 must contain between 21 and 45 doubles.");
    }

    if (state_vector_obj2.size() != 6) {
        throw std::invalid_argument("State vector for Object 2 must contain exactly 6 doubles.");
    }

    if (covariance_matrix_obj2.size() < 21 || covariance_matrix_obj2.size() > 45) {
        throw std::invalid_argument("Covariance matrix for Object 2 must contain between 21 and 45 doubles.");
    }
}

double CollisionProbabilityEstimator::estimate() const {
    // This is a fake implementation, always returning 0.01
    return 0.01;
}


