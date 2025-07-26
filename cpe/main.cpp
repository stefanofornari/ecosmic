#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "CollisionProbabilityEstimator.h" // Include the new header

#include "utils.h"

int main() {

    std::string norad_id_obj1;
    std::string state_vector_line_obj1;
    std::string covariance_matrix_line_obj1;
    std::vector<double> state_vector_obj1;
    std::vector<double> covariance_matrix_obj1;

    std::string norad_id_obj2;
    std::string state_vector_line_obj2;
    std::string covariance_matrix_line_obj2;
    std::vector<double> state_vector_obj2;
    std::vector<double> covariance_matrix_obj2;

    // Read NORAD ID for Object 1 (line 1)
    std::getline(std::cin, norad_id_obj1);

    // Read state vector for Object 1 (line 2)
    std::getline(std::cin, state_vector_line_obj1);
    if (!parseDoubles(state_vector_line_obj1, state_vector_obj1)) {
        std::cerr << "Error: Invalid numeric input encountered in state vector line for Object 1: \"" << state_vector_line_obj1 << "\"" << std::endl;
        return 1; // Exit if parsing failed
    }

    // Read covariance matrix for Object 1 (line 3)
    std::getline(std::cin, covariance_matrix_line_obj1);
    if (!parseDoubles(covariance_matrix_line_obj1, covariance_matrix_obj1)) {
        std::cerr << "Error: Invalid numeric input encountered in covariance matrix line for Object 1: \"" << covariance_matrix_line_obj1 << "\"" << std::endl;
        return 1; // Exit if parsing failed
    }

    // Read NORAD ID for Object 2 (line 4)
    std::getline(std::cin, norad_id_obj2);

    // Read state vector for Object 2 (line 5)
    std::getline(std::cin, state_vector_line_obj2);
    if (!parseDoubles(state_vector_line_obj2, state_vector_obj2)) {
        std::cerr << "Error: Invalid numeric input encountered in state vector line for Object 2: \"" << state_vector_line_obj2 << "\"" << std::endl;
        return 1; // Exit if parsing failed
    }

    // Read covariance matrix for Object 2 (line 6)
    std::getline(std::cin, covariance_matrix_line_obj2);
    if (!parseDoubles(covariance_matrix_line_obj2, covariance_matrix_obj2)) {
        std::cerr << "Error: Invalid numeric input encountered in covariance matrix line for Object 2: \"" << covariance_matrix_line_obj2 << "\"" << std::endl;
        return 1; // Exit if parsing failed
    }

    // Instantiate the CollisionProbabilityEstimator object with four parameters
    try {
        CollisionProbabilityEstimator estimator(state_vector_obj1, covariance_matrix_obj1, state_vector_obj2, covariance_matrix_obj2);

        // Run estimate() and print the result
        double result = estimator.estimate();
        std::cout << std::fixed << std::setprecision(2) << result << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}