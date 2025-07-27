# Ecosmic - Collision Probability Estimation Service

This service provides collision probability estimations for satellite operators.

## Disclaimer

This assessment has been conducted in a collaborative environment with an AI assistant. This collaboration significantly enhanced the efficiency and breadth of the work, resulting in higher quality and timely delivery of the artifacts. I have, however, driven, supervised, reviewed, checked, and tested all the work presented.

## Deliverables

* Architecture and architecture diagram
  * [Architecture description](#cloud-native-architecture)
  * [AWS Architecture Diagram (PDF)](docs/aws%20architecture.pdf)
* Infrastructure-as-code stub
  * See [Infrastructure as Code](#infrastructure-as-code)
* Development roadmap
  * [Monthly roadmap](https://github.com/users/stefanofornari/projects/1/views/1)
  * [Project repository](https://github.com/stefanofornari/ecosmic)
  * [User Stories and bugs](https://github.com/stefanofornari/ecosmic/issues)
  * [Project backlog](https://github.com/users/stefanofornari/projects/1/views/3)
* FastAPI & CLI prototype with minimal logic and test coverage 
  * See [Proof of Concept (PoC) Setup](#proof-of-concept-poc-setup)

## How to Use the Service

### Submitting an Estimate

To submit a Collision Data Message (CDM) for estimation, use the `POST /api/cpe` endpoint. The `owner` is required as a query parameter, and the CDM file content should be sent in the request body.

Example using `curl`:

```bash
curl -X POST "http://YOUR_SERVER_ADDRESS/api/cpe?owner=YOUR_OWNER_ID" --data-binary "@docs/example.cdm"
```

Replace `YOUR_SERVER_ADDRESS` with the actual address of your FastAPI application (e.g., `localhost:8000`) and `YOUR_OWNER_ID` with the relevant OWNER ID. The response will be a UUID, which is the `cpe_id` for tracking the job.

**Possible Status Codes:**
*   `200 OK`: Job successfully submitted. The response body contains the `cpe_id` (UUID) for the submitted job.
*   `400 Bad Request`: No content provided in the request body.
*   `403 Forbidden`: Job submission not allowed (e.g., `owner` is missing or the provided one is not allowed).
*   `500 Internal Server Error`: An unexpected error occurred during processing.

### Retrieving an Estimate

To retrieve the status or result of a previously submitted estimation job, use the `GET /api/cpe` endpoint with the `owner` and `cpe_id`.

Example using `curl`:

```bash
curl -X GET "http://YOUR_SERVER_ADDRESS/api/cpe?owner=YOUR_OWNER_ID&cpe_id=YOUR_CPE_ID"
```

Replace `YOUR_SERVER_ADDRESS`, `YOUR_OWNER_ID`, and `YOUR_CPE_ID` with the appropriate values.

**Possible Status Codes:**
*   `200 OK`: The CPE result is available. The response body contains the content of the `.cpe` file.
*   `400 Bad Request`: `cpe_id` is missing.
*   `403 Forbidden`: CPE status not allowed (e.g., `owner` is missing or the provided one is not allowed).
*   `404 Not Found`: The CPE result is not yet available or the `cpe_id` does not exist for the given `owner`.
*   `500 Internal Server Error`: An unexpected error occurred during retrieval.

Replace `YOUR_SERVER_ADDRESS`, `YOUR_OWNER_ID`, and `YOUR_CPE_ID` with the appropriate values.

### Authentication

A common and relatively simple approach for authentication is to use API keys. This method is suitable for many scenarios, especially for internal services, as it balances security with ease of implementation.
Initially, we can implement basic API key authentication. In the future, the authentication mechanism can be extended to a more robust token-based approach (e.g., OAuth 2.0's authorization code flow), where a long-lived authorization key is used to acquire a short-lived access token..

## Cloud Native Architecture

The service is designed as a cloud native service hosted on AWS infrastrucure.

### Sizing and Scalability

We expect to onboard: 

* 3 customers in 3 months, each with ~10 satellites and an average of 1 run/day/satellite, with an SLA defining a maximum time-to-result of 1 hour 
* A large customer in month 4 with ~1000 satellites and an average of 5 runs/day/satellite, with an SLA defining a maximum time-to-result of 1 hour 

Given an average processing time of 10 minutes per run, our system can process 6 requests per hour per instance. Assuming the runs are, on average, evenly distributed throughout the 24-hour day, our planning indicates the following resource needs:

* First 3 months: 30 runs per day, which translates to 1.25 runs per hour. This requires 2 CPE processing instances (for redundancy).
* From month 4 onwards: 5030 runs per day, which translates to 210 runs per hour. This requires 35 CPE processing instances."

### Observability

To understanding the health, performance, and runtime behavior of the service a mix of AWS Services and specific features have been takedn into account.

#### AWS Services:

* All containerized applications (FastAPI service, CPE Consumer service) running on AWS Fargate will automatically send their standard output and standard error streams to Amazon CloudWatch Logs. API Gateway and ALB also integrate with CloudWatch Logs for access logs and error logs
  * FastAPI Service: Request logs (incoming API calls, HTTP methods, paths, status codes), application-level logs (e.g., parsing errors, S3 upload confirmations, SQS message sends, DynamoDB updates).
  * CPE Consumer Service: Logs related to SQS message polling, S3 file downloads, execution of the C++ cpe program (including its stdout/stderr), S3 output uploads, and DynamoDB status updates.
  * API Gateway/ALB: Access logs detailing incoming requests, latency, and any errors at the gateway/load balancer level.
* CloudWatch Logs allows for centralized log collection, searching, filtering, and setting up alarms based on specific log patterns (e.g., "ERROR" messages, "Key not found" exceptions)
* AWS services automatically emit a wide range of metrics to Amazon CloudWatch Metrics
  * API Gateway: Count (total requests), Latency, 4XXError, 5XXError.
  * ALB: RequestCount, HTTPCode_Target_2XX_Count, HTTPCode_Target_5XX_Count, TargetConnectionErrorCount, HealthyHostCount.
  * ECS (Fargate): CPUUtilization, MemoryUtilization for tasks and services, RunningTaskCount.
  * SQS: NumberOfMessagesSent, NumberOfMessagesReceived, ApproximateNumberOfMessagesVisible (queue depth), ApproximateNumberOfMessagesNotVisible (in-flight messages), ApproximateAgeOfOldestMessage.
  * DynamoDB: ConsumedReadCapacityUnits, ConsumedWriteCapacityUnits, SuccessfulRequestLatency, ThrottledRequests.
  * S3: BucketSizeBytes, NumberOfObjects, AllRequests, GetRequests, PutRequests, 4xxErrors, 5xxErrors.
* AWS X-Ray to trace the entire flow

#### Observability Features/Requirements

Application specific observability requriements will be implemented. They are added to the user project user stories and labeled as observability. 

* Log library to properly log to the stdout so to be captured by CloudWatch
* Log main events in each application
* Correlation id/X-ray tracing
* CloudWatch Dashboard

See [the backlog](https://github.com/users/stefanofornari/projects/1/views/3) for details. 

### AWS Architecture

For a visual representation of the AWS architecture, refer to the following diagrams:

*   [AWS Architecture Diagram (PDF)](docs/aws%20architecture.pdf)
*   [AWS Architecture Diagram (OpenOffice Draw)](docs/aws%20architecture.odg)

### Infrastructure as Code

The entire infrastrucure required to operate the service is coded in Terraform descriptors. The required code can be fund in the iac project subdirectory. It covers:

1.  Foundational networking components (VPC, subnets, gateways, route tables, and basic security groups)
1.  Core data services (S3, SQS, DynamoDB)
1.  ECS Cluster, necessary IAM roles and policies, CloudWatch Log Groups
1.  ECS Services themselves and ECS Task Definitions for both the FastAPI and CPE Consumer services

What is missing:
1. IAM Roles and Policies
1. ALB Configuration

## Development

### Project Management and Tracking

The requirements are collected as User Stories or Tasks in the [Ecosmic Collision Probability Estimation Issue Tracker](https://github.com/stefanofornari/ecosmic/issues) and can be more easily viewed in the [Ecosmic Collision Probability Estimation Backlog](https://github.com/users/stefanofornari/projects/1/views/3). For a roadmap view, check out the [Ecosmic Collision Probability Estimation Monthly Roadmap](https://github.com/users/stefanofornari/projects/1/views/1).

### Project Summary and Structure

To clone the repository and its submodules, use the following commands:

```bash
git clone https://github.com/stefanofornari/ecosmic
cd ecosmic
git submodule init
git submodule update
```

The `ecosmic` project is organized into several top-level directories, each serving a specific purpose:

*   **`api/`**: Houses the Python FastAPI application.
*   **`build/`**: Contains compiled executables and build artifacts.
*   **`cpe/`**: Holds the core C++ Collision Probability Estimator logic.
*   **`dequeuer/`**: Contains the bash script for processing queue files.
*   **`docs/`**: Stores project documentation and examples.
*   **`iac/`**: Defines the AWS infrastructure using Terraform.
*   **`queue/`**: Implements the file-based queue system for job processing.
*   **`scripts/`**: Contains various utility shell scripts.
*   **`tests/`**: Holds the C++ unit tests for the project.
*   **`third_party/`**: Includes external dependencies like Google Test.


## Proof of Concept (PoC) Setup

### Implementation Notes

1. Due to some ambiguity in the requirements the current implementation does not implement the following, which has been tracked by requirement [CPE result format](https://github.com/stefanofornari/ecosmic/issues/3):
   > The algorithm shall produce as output a CCSDS CDM file, containing the same 
     information of the one provided by the user. The fields "ORIGINATOR" and 
     "COLLISION_PROBABILITY" shall be updated, namely, with "Ecosmic" and the result 
     of the algorithm. The delivery mechanism is up to you to decide
1. The following requirement has not been implemented yet and tracked in [CPE request authorization](https://github.com/stefanofornari/ecosmic/issues/2):
   > The endpoint shall be usable only by customers who own the space object identified
     as primary in the CCSDS CDM.

### Project Setup

To set up and run the PoC, follow these steps:

#### 1. Build the Collision Probability Estimator (CPE) Program

From the project root directory, create a `build` directory and compile the C++ `cpe` program using CMake:

```bash
mkdir -p build
cd build
cmake ..
make
cd ..
```

This will create the `cpe` executable at `build/cpe/cpe`.

To run the `cpe` program directly from the command line, you can pipe the content of a CDM file into its standard input. For example:

```bash
cat docs/example.cpe | ./build/cpe/cpe
```

This command will feed the content of `docs/example.cpe` to the `cpe` program, and its output (the estimated collision probability) will be printed to your terminal.

To run the C++ tests:

```bash
./build/tests/runTests
```

#### 2. Install Python Dependencies

Navigate to the `api` directory and install the required Python packages:

```bash
pip install -r api/requirements.txt
```

#### 3. Start the API Process

From the project root directory, start the FastAPI application. This will run the API server, typically on `http://localhost:8000`.

```bash
python3 -m uvicorn api.main:app
```

#### 4. Start the Dequeuer Process

In a separate terminal, from the project root directory, start the dequeuer script. This script monitors the input queue, processes CDM files, and moves them to the appropriate output or failed directories.

```bash
./dequeuer/dequeuer.sh "./build/cpe/cpe" "queue"
```

This command starts the dequeuer, using the compiled C++ `cpe` executable to process the file content.

Once all processes are running, you can submit CDM files via the API, and the dequeuer will pick them up for processing.

### PoC Queue

The PoC simulates a queue system using a simple directory with the following structure:

- queue
  - in
    - owner id 1
    - owner id 2
    - ...
  - out
    - owner id 1
    - owner id 2
    - ...
  - done
    - owner id 1
    - owner id 2
    - ...
  - failed
    - owner id 1
    - owner id 2
    - ...

Note that, as a simple access-control mechanism, the API accepts requests only for owners that have a directory created under queue/in.

### E2E example

Being in the project directory:

1. create queue/in/123abc
1. start the API
   ```bash
   python3 -m uvicorn api.main:app
   ```
1. run the dequeuer
   ```bash
   ./dequeuer/dequeuer.sh "./build/cpe/cpe" "queue"
   ```
1. submit a cpe request
   ```bash
   curl -X POST "http://127.0.0.1:8000/api/cpe?owner=123abc" -H "Content-Type: text/plain" --data-binary "@docs/example.cdm"
   ```
   take note of the returned CPE_ID
1. check the status of the request/retrieve the estimate
   ```bash
   curl "http://127.0.0.1:8000/api/cpe?owner=123abc&cpe_id=<CPE_ID>"
   ```
