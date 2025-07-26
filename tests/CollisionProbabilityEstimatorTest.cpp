#include "gtest/gtest.h"
#include "CollisionProbabilityEstimator.h"

// Test fixture for CollisionProbabilityEstimator
class CollisionProbabilityEstimatorTest : public ::testing::Test {
protected:
    std::vector<double> default_state_vector_obj1 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    std::vector<double> default_covariance_matrix_obj1;
    std::vector<double> default_state_vector_obj2 = {7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    std::vector<double> default_covariance_matrix_obj2;

    void SetUp() override {
        default_covariance_matrix_obj1.resize(36, 0.0);
        for (int i = 0; i < 6; ++i) {
            default_covariance_matrix_obj1[i * 6 + i] = 1.0;
        }
        default_covariance_matrix_obj2.resize(36, 0.0);
        for (int i = 0; i < 6; ++i) {
            default_covariance_matrix_obj2[i * 6 + i] = 2.0;
        }
    }
};

// Test the constructor
TEST_F(CollisionProbabilityEstimatorTest, ConstructorInitializesMembers)
{
    CollisionProbabilityEstimator estimator(default_state_vector_obj1, default_covariance_matrix_obj1, default_state_vector_obj2, default_covariance_matrix_obj2);
    EXPECT_EQ(0.01, estimator.estimate());
}

// Test the estimate method (with the fake implementation)
TEST_F(CollisionProbabilityEstimatorTest, EstimateReturnsFakeValue)
{
    CollisionProbabilityEstimator estimator(default_state_vector_obj1, default_covariance_matrix_obj1, default_state_vector_obj2, default_covariance_matrix_obj2);
    EXPECT_EQ(0.01, estimator.estimate());
}

// Test constructor with empty state vector for Object 1
TEST_F(CollisionProbabilityEstimatorTest, ConstructorThrowsOnEmptyStateVectorObj1)
{
    std::vector<double> empty_state_vector = {};
    EXPECT_THROW(
        CollisionProbabilityEstimator(empty_state_vector, default_covariance_matrix_obj1, default_state_vector_obj2, default_covariance_matrix_obj2),
        std::invalid_argument
    );
}

// Test constructor with empty covariance matrix for Object 1
TEST_F(CollisionProbabilityEstimatorTest, ConstructorThrowsOnEmptyCovarianceMatrixObj1)
{
    std::vector<double> empty_covariance_matrix = {};
    EXPECT_THROW(
        CollisionProbabilityEstimator(default_state_vector_obj1, empty_covariance_matrix, default_state_vector_obj2, default_covariance_matrix_obj2),
        std::invalid_argument
    );
}

// Test constructor with incorrect state vector size for Object 1
TEST_F(CollisionProbabilityEstimatorTest, ConstructorThrowsOnIncorrectStateVectorSizeObj1)
{
    std::vector<double> incorrect_state_vector = {1.0, 2.0, 3.0};
    EXPECT_THROW(
        CollisionProbabilityEstimator(incorrect_state_vector, default_covariance_matrix_obj1, default_state_vector_obj2, default_covariance_matrix_obj2),
        std::invalid_argument
    );
}

// Test constructor with incorrect covariance matrix size for Object 1
TEST_F(CollisionProbabilityEstimatorTest, ConstructorThrowsOnIncorrectCovarianceMatrixSizeObj1)
{
    std::vector<double> incorrect_covariance_matrix = {1.0, 2.0, 3.0, 4.0};
    EXPECT_THROW(
        CollisionProbabilityEstimator(default_state_vector_obj1, incorrect_covariance_matrix, default_state_vector_obj2, default_covariance_matrix_obj2),
        std::invalid_argument
    );
}

// Test constructor with empty state vector for Object 2
TEST_F(CollisionProbabilityEstimatorTest, ConstructorThrowsOnEmptyStateVectorObj2)
{
    std::vector<double> empty_state_vector = {};
    EXPECT_THROW(
        CollisionProbabilityEstimator(default_state_vector_obj1, default_covariance_matrix_obj1, empty_state_vector, default_covariance_matrix_obj2),
        std::invalid_argument
    );
}

// Test constructor with empty covariance matrix for Object 2
TEST_F(CollisionProbabilityEstimatorTest, ConstructorThrowsOnEmptyCovarianceMatrixObj2)
{
    std::vector<double> empty_covariance_matrix = {};
    EXPECT_THROW(
        CollisionProbabilityEstimator(default_state_vector_obj1, default_covariance_matrix_obj1, default_state_vector_obj2, empty_covariance_matrix),
        std::invalid_argument
    );
}

// Test constructor with incorrect state vector size for Object 2
TEST_F(CollisionProbabilityEstimatorTest, ConstructorThrowsOnIncorrectStateVectorSizeObj2)
{
    std::vector<double> incorrect_state_vector = {1.0, 2.0, 3.0};
    EXPECT_THROW(
        CollisionProbabilityEstimator(default_state_vector_obj1, default_covariance_matrix_obj1, incorrect_state_vector, default_covariance_matrix_obj2),
        std::invalid_argument
    );
}

// Test constructor with incorrect covariance matrix size for Object 2
TEST_F(CollisionProbabilityEstimatorTest, ConstructorThrowsOnIncorrectCovarianceMatrixSizeObj2)
{
    std::vector<double> incorrect_covariance_matrix = {1.0, 2.0, 3.0, 4.0};
    EXPECT_THROW(
        CollisionProbabilityEstimator(default_state_vector_obj1, default_covariance_matrix_obj1, default_state_vector_obj2, incorrect_covariance_matrix),
        std::invalid_argument
    );
}