# Asynchronous Execution of CPE Service

The C++ Collision Probability Estimator (CPE) algorithm, with an average runtime of 10 minutes, must be called asynchronously from a Python FastAPI service. The FastAPI service will receive a CCSDS CDM file, parse it, and then trigger the C++ algorithm, which in turn produces an output CCSDS CDM file.

## Asynchronous Execution Options in AWS

For long-running tasks like the 10-minute C++ algorithm, direct synchronous calls from a web server are not ideal. Asynchronous patterns are necessary to prevent timeouts and resource blocking.

1.  **SQS (Simple Queue Service) + Lambda/EC2 Workers:**
    *   **FastAPI Service:** Receives the request, parses the CDM, extracts necessary inputs, and sends a message to an SQS queue. The message would contain data or a reference (e.g., S3 path) to the parsed inputs.
    *   **SQS Queue:** Acts as a reliable buffer for tasks.
    *   **Lambda/EC2 Workers:**
        *   **Lambda:** If the C++ algorithm can be packaged as a Lambda layer or custom runtime and consistently completes within Lambda's 15-minute execution limit, it's a serverless and scalable option.
        *   **EC2 Instances (Auto Scaling Group):** For more control, longer execution times, or specific hardware, EC2 instances running a worker process (polling SQS) are suitable.
    *   **Output Handling:** The C++ algorithm would write its output CDM to an S3 bucket. The FastAPI service could then provide a polling endpoint or use a notification service (e.g., SNS, WebSockets) for result retrieval.

2.  **AWS Step Functions:**
    *   **FastAPI Service:** Triggers a Step Functions state machine execution.
    *   **Step Functions State Machine:** Orchestrates the workflow. This could involve:
        *   A Lambda function for initial CDM parsing and input preparation.
        *   An ECS/Fargate task (containerized C++ application) for the long-running CPE execution. Step Functions can directly invoke and wait for these tasks.
        *   Another Lambda function for post-processing the output CDM (e.g., storing metadata, sending notifications).
    *   **Benefits:** Provides visual workflows, built-in error handling, retries, and parallel execution, making it excellent for complex, multi-step processes.

3.  **AWS Batch:**
    *   **FastAPI Service:** Submits a job to AWS Batch.
    *   **AWS Batch:** Manages a fleet of EC2 instances (or Fargate) to run containerized batch jobs. The C++ algorithm would be containerized and executed as a job.
    *   **Benefits:** Optimized for batch computing, handling scaling, scheduling, and job management.

## Local Prototyping on a Laptop

To prototype this asynchronous flow locally, you can simulate the queue and worker components:

1.  **Simulating the Queue:**
    *   **Local File System:** The FastAPI service can write the C++ algorithm inputs (e.g., parsed data or paths to input files) to a designated local directory.
    *   **Local Database (e.g., SQLite):** For more structured task management, a local SQLite database could store task information.

2.  **Simulating the Worker:**
    *   **Separate Python Script:** A dedicated Python script would act as the worker. It would continuously monitor the local task queue (e.g., poll the directory or database for new tasks).
    *   **Execute C++ via `subprocess`:** When a new task is detected, the Python worker script would use Python's `subprocess` module to execute the compiled C++ CPE executable. Inputs would be passed (e.g., via stdin, command-line arguments, or temporary files), and outputs captured.
    *   **Output Storage:** The output of the C++ executable (the generated CDM file) would be stored in another local directory.

**Example Local Prototype Flow:**

*   **FastAPI (`app.py`):**
    *   Receives the CDM file via a POST request.
    *   Generates a unique `task_id`.
    *   Saves the input CDM to a `local_task_queue` directory.
    *   Creates a JSON file in `local_task_queue` with `task_id`, input path, and expected output path.
    *   Provides a GET endpoint (`/get_result/{task_id}`) to check task status and retrieve results from a `local_results` directory.

*   **Worker Script (`worker.py`):**
    *   Continuously polls the `local_task_queue` directory for new JSON task files.
    *   When a task is found, it reads the task information.
    *   Executes the C++ `ecosmic` executable using `subprocess.run()`, passing the necessary inputs (e.g., reading from the input CDM file and piping to C++ stdin).
    *   Captures the C++ executable's stdout (the result) and writes it to the specified output CDM path in the `local_results` directory.
    *   Deletes the processed task file and input CDM.

This setup allows for local development and testing of the asynchronous interaction between the FastAPI service and the C++ CPE algorithm.
