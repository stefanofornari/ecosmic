
# --- ECS Cluster ---

resource "aws_ecs_cluster" "ecosmic_cluster" {
  name = "ecosmic-cluster"
  tags = {
    Name = "ecosmic-cluster"
  }
}

# --- IAM Roles for ECS Tasks ---

# ECS Task Execution Role (for Fargate to pull images and publish logs)
resource "aws_iam_role" "ecs_task_execution_role" {
  name = "ecosmic-ecs-task-execution-role"

  assume_role_policy = jsonencode({
    Version = "2012-10-17",
    Statement = [
      {
        Action = "sts:AssumeRole",
        Effect = "Allow",
        Principal = {
          Service = "ecs-tasks.amazonaws.com"
        }
      }
    ]
  })
  tags = {
    Name = "ecosmic-ecs-task-execution-role"
  }
}

resource "aws_iam_role_policy_attachment" "ecs_task_execution_role_policy" {
  role       = aws_iam_role.ecs_task_execution_role.name
  policy_arn = "arn:aws:iam::aws:policy/service-role/AmazonECSTaskExecutionRolePolicy"
}

# FastAPI Task Role (for S3, SQS, DynamoDB access)
resource "aws_iam_role" "fastapi_task_role" {
  name = "ecosmic-fastapi-task-role"

  assume_role_policy = jsonencode({
    Version = "2012-10-17",
    Statement = [
      {
        Action = "sts:AssumeRole",
        Effect = "Allow",
        Principal = {
          Service = "ecs-tasks.amazonaws.com"
        }
      }
    ]
  })
  tags = {
    Name = "ecosmic-fastapi-task-role"
  }
}

resource "aws_iam_policy" "fastapi_access_policy" {
  name        = "ecosmic-fastapi-access-policy"
  description = "Policy for FastAPI ECS task to access S3, SQS, DynamoDB"

  policy = jsonencode({
    Version = "2012-10-17",
    Statement = [
      {
        Effect = "Allow",
        Action = [
          "s3:PutObject",
          "s3:GetObject",
          "s3:ListBucket"
        ],
        Resource = [
          aws_s3_bucket.ecosmic_input_bucket.arn,
          "${aws_s3_bucket.ecosmic_input_bucket.arn}/*"
        ]
      },
      {
        Effect = "Allow",
        Action = [
          "sqs:SendMessage"
        ],
        Resource = aws_sqs_queue.ecosmic_cpe_queue.arn
      },
      {
        Effect = "Allow",
        Action = [
          "dynamodb:PutItem",
          "dynamodb:GetItem",
          "dynamodb:UpdateItem"
        ],
        Resource = aws_dynamodb_table.ecosmic_cpe_jobs.arn
      }
    ]
  })
}

resource "aws_iam_role_policy_attachment" "fastapi_access_policy_attachment" {
  role       = aws_iam_role.fastapi_task_role.name
  policy_arn = aws_iam_policy.fastapi_access_policy.arn
}

# CPE Consumer Task Role (for S3, SQS, DynamoDB access)
resource "aws_iam_role" "cpe_consumer_task_role" {
  name = "ecosmic-cpe-consumer-task-role"

  assume_role_policy = jsonencode({
    Version = "2012-10-17",
    Statement = [
      {
        Action = "sts:AssumeRole",
        Effect = "Allow",
        Principal = {
          Service = "ecs-tasks.amazonaws.com"
        }
      }
    ]
  })
  tags = {
    Name = "ecosmic-cpe-consumer-task-role"
  }
}

resource "aws_iam_policy" "cpe_consumer_access_policy" {
  name        = "ecosmic-cpe-consumer-access-policy"
  description = "Policy for CPE Consumer ECS task to access S3, SQS, DynamoDB"

  policy = jsonencode({
    Version = "2012-10-17",
    Statement = [
      {
        Effect = "Allow",
        Action = [
          "s3:GetObject",
          "s3:PutObject",
          "s3:ListBucket"
        ],
        Resource = [
          aws_s3_bucket.ecosmic_input_bucket.arn,
          "${aws_s3_bucket.ecosmic_input_bucket.arn}/*",
          aws_s3_bucket.ecosmic_output_bucket.arn,
          "${aws_s3_bucket.ecosmic_output_bucket.arn}/*"
        ]
      },
      {
        Effect = "Allow",
        Action = [
          "sqs:ReceiveMessage",
          "sqs:DeleteMessage",
          "sqs:GetQueueAttributes"
        ],
        Resource = aws_sqs_queue.ecosmic_cpe_queue.arn
      },
      {
        Effect = "Allow",
        Action = [
          "dynamodb:GetItem",
          "dynamodb:UpdateItem"
        ],
        Resource = aws_dynamodb_table.ecosmic_cpe_jobs.arn
      }
    ]
  })
}

resource "aws_iam_role_policy_attachment" "cpe_consumer_access_policy_attachment" {
  role       = aws_iam_role.cpe_consumer_task_role.name
  policy_arn = aws_iam_policy.cpe_consumer_access_policy.arn
}

# --- CloudWatch Log Groups ---

resource "aws_cloudwatch_log_group" "fastapi_log_group" {
  name              = "/ecs/ecosmic-fastapi"
  retention_in_days = 7 # Adjust retention as needed
  tags = {
    Name = "ecosmic-fastapi-log-group"
  }
}

resource "aws_cloudwatch_log_group" "cpe_consumer_log_group" {
  name              = "/ecs/ecosmic-cpe-consumer"
  retention_in_days = 7 # Adjust retention as needed
  tags = {
    Name = "ecosmic-cpe-consumer-log-group"
  }
}

# --- ECS Task Definitions ---

resource "aws_ecs_task_definition" "fastapi_task_definition" {
  family                   = "ecosmic-fastapi-task"
  cpu                      = "256"    # 0.25 vCPU
  memory                   = "512"    # 0.5 GB
  network_mode             = "awsvpc"
  requires_compatibilities = ["FARGATE"]
  execution_role_arn       = aws_iam_role.ecs_task_execution_role.arn
  task_role_arn            = aws_iam_role.fastapi_task_role.arn

  container_definitions = jsonencode([
    {
      name        = "fastapi-app"
      image       = "your-fastapi-image:latest" # REPLACE with your FastAPI Docker image
      cpu         = 256
      memory      = 512
      essential   = true
      portMappings = [
        {
          containerPort = 8000
          hostPort      = 8000
          protocol      = "tcp"
        }
      ]
      logConfiguration = {
        logDriver = "awslogs"
        options = {
          "awslogs-group"         = aws_cloudwatch_log_group.fastapi_log_group.name
          "awslogs-region"        = "us-east-1" # REPLACE with your AWS region
          "awslogs-stream-prefix" = "fastapi"
        }
      }
      environment = [
        {
          name  = "BASE_QUEUE_DIR",
          value = "/app/queue" # Assuming your container has this path
        }
      ]
    }
  ])
  tags = {
    Name = "ecosmic-fastapi-task-definition"
  }
}

resource "aws_ecs_task_definition" "cpe_consumer_task_definition" {
  family                   = "ecosmic-cpe-consumer-task"
  cpu                      = "256"    # 0.25 vCPU
  memory                   = "512"    # 0.5 GB
  network_mode             = "awsvpc"
  requires_compatibilities = ["FARGATE"]
  execution_role_arn       = aws_iam_role.ecs_task_execution_role.arn
  task_role_arn            = aws_iam_role.cpe_consumer_task_role.arn

  container_definitions = jsonencode([
    {
      name        = "cpe-consumer"
      image       = "your-cpe-consumer-image:latest" # REPLACE with your CPE Consumer Docker image
      cpu         = 256
      memory      = 512
      essential   = true
      logConfiguration = {
        logDriver = "awslogs"
        options = {
          "awslogs-group"         = aws_cloudwatch_log_group.cpe_consumer_log_group.name
          "awslogs-region"        = "us-east-1" # REPLACE with your AWS region
          "awslogs-stream-prefix" = "cpe-consumer"
        }
      }
      environment = [
        {
          name  = "SQS_QUEUE_URL",
          value = aws_sqs_queue.ecosmic_cpe_queue.id
        },
        {
          name  = "S3_INPUT_BUCKET",
          value = aws_s3_bucket.ecosmic_input_bucket.bucket
        },
        {
          name  = "S3_OUTPUT_BUCKET",
          value = aws_s3_bucket.ecosmic_output_bucket.bucket
        },
        {
          name  = "DYNAMODB_TABLE_NAME",
          value = aws_dynamodb_table.ecosmic_cpe_jobs.name
        }
      ]
    }
  ])
  tags = {
    Name = "ecosmic-cpe-consumer-task-definition"
  }
}

# --- ECS Services ---

resource "aws_ecs_service" "fastapi_service" {
  name            = "ecosmic-fastapi-service"
  cluster         = aws_ecs_cluster.ecosmic_cluster.id
  task_definition = aws_ecs_task_definition.fastapi_task_definition.arn
  desired_count   = 1 # Start with 1, can be scaled later
  launch_type     = "FARGATE"

  network_configuration {
    subnets         = [aws_subnet.private_subnet_1.id, aws_subnet.private_subnet_2.id]
    security_groups = [aws_security_group.ecs_tasks_sg.id]
    assign_public_ip = false # Fargate tasks in private subnets don't need public IPs
  }

  # Uncomment and configure if you set up an ALB in the next step
  # load_balancer {
  #   target_group_arn = aws_lb_target_group.fastapi_tg.arn
  #   container_name   = "fastapi-app"
  #   container_port   = 8000
  # }

  depends_on = [
    aws_iam_role_policy_attachment.ecs_task_execution_role_policy,
    aws_iam_role_policy_attachment.fastapi_access_policy_attachment
  ]
  tags = {
    Name = "ecosmic-fastapi-service"
  }
}

resource "aws_ecs_service" "cpe_consumer_service" {
  name            = "ecosmic-cpe-consumer-service"
  cluster         = aws_ecs_cluster.ecosmic_cluster.id
  task_definition = aws_ecs_task_definition.cpe_consumer_task_definition.arn
  desired_count   = 1 # Start with 1, can be scaled later
  launch_type     = "FARGATE"

  network_configuration {
    subnets         = [aws_subnet.private_subnet_1.id, aws_subnet.private_subnet_2.id]
    security_groups = [aws_security_group.ecs_tasks_sg.id]
    assign_public_ip = false # Fargate tasks in private subnets don't need public IPs
  }

  depends_on = [
    aws_iam_role_policy_attachment.ecs_task_execution_role_policy,
    aws_iam_role_policy_attachment.cpe_consumer_access_policy_attachment
  ]
  tags = {
    Name = "ecosmic-cpe-consumer-service"
  }
}
