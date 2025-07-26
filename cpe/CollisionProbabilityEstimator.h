#ifndef COLLISION_PROBABILITY_ESTIMATOR_H
#define COLLISION_PROBABILITY_ESTIMATOR_H

#include <vector>
#include <string>
#include <stdexcept> // For std::invalid_argument

// Define the CollisionProbabilityEstimator class
class CollisionProbabilityEstimator {
public:
    CollisionProbabilityEstimator(const std::vector<double>& state_vector_obj1,
                                  const std::vector<double>& covariance_matrix_obj1,
                                  const std::vector<double>& state_vector_obj2,
                                  const std::vector<double>& covariance_matrix_obj2);

    double estimate() const;

private:
    std::vector<double> state_vector_obj1_;
    std::vector<double> covariance_matrix_obj1_;
    std::vector<double> state_vector_obj2_;
    std::vector<double> covariance_matrix_obj2_;
};



#endif // COLLISION_PROBABILITY_ESTIMATOR_H
