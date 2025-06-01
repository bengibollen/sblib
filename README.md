# Silver Bucket Mudlib

## Docker Setup

This repository contains Docker configurations to build and run the Silver Bucket mudlib with the LDMud driver.

### Requirements

- Docker
- Docker Compose

### Configuration

Before starting, you may want to adjust the following settings in `docker-compose.yml`:

- `DRIVER_REPO`: URL to the LDMud driver repository
- `DRIVER_BRANCH`: Branch to use for the driver
- `PYTHON_REPO`: URL to the Python startup code repository
- `PYTHON_BRANCH`: Branch to use for the Python code

### Running the MUD

To build and start the MUD and database:

```bash
# Build and start the containers
docker-compose up -d

# View logs
docker-compose logs -f

# Stop the containers
docker-compose down
```

### Volumes

- The mudlib directory (this repository) is mounted at `/mud/sblib` inside the container
- PostgreSQL data is stored in a Docker volume named `pgdata`

### Ports

- MUD server: 4040 (TCP)
- UDP port: 4041 (UDP)
