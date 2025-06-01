FROM debian:bookworm

# These ARG instructions define the build arguments that can be passed
# Default values are provided but will be overridden by docker-compose.yml
ARG UID=1000
ARG GID=1000
ARG DRIVER_REPO=https://github.com/bengibollen/ldmud.git
ARG DRIVER_BRANCH=sblib
ARG PYTHON_REPO=https://github.com/bengibollen/ldmud-python.git
ARG PYTHON_BRANCH=main

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
    build-essential ca-certificates git bison autoconf autogen automake wget \
    pkg-config libgcrypt20-dev libgnutls28-dev libpq-dev python3-dev \
    libxml2-dev zlib1g-dev libpcre3-dev libc-ares-dev python3-hunspell \
    python3-pip hunspell-de-de help2man gdb

# Create mud user/group using the provided UID/GID
RUN groupadd -g $GID mud \
 && useradd -u $UID -g mud -m mud

# Clone and build the driver from the specified repo/branch
RUN git clone ${DRIVER_REPO} --branch ${DRIVER_BRANCH} --depth 1 /build \
 && cd /build/src \
 && ./autogen.sh \
 && ./configure --with-setting=sblib --prefix=/usr/local --libdir=/mud/sblib \
 && make install-all \
 && cd / && rm -rf /build

# Clone the Python code directly to the correct location
RUN mkdir -p /mud \
 && git clone ${PYTHON_REPO} --branch ${PYTHON_BRANCH} --depth 1 /mud/ldmud-python || echo "Python repository not found or not specified" \
 && chown -R mud:mud /mud

# Set up the mud directory
RUN mkdir -p /mud/sblib && chown mud:mud /mud
COPY --chown=mud:mud . /mud/sblib/

# Clean up packages that are no longer needed
RUN apt-get clean \
 && apt-mark manual libgnutls30 libpq5 libpython3.11 libxml2 libpcre3 \
 && apt-get remove --purge -y build-essential ca-certificates git bison autoconf autogen automake wget pkg-config libgcrypt20-dev libgnutls28-dev libsqlite3-dev python3-dev libxml2-dev zlib1g-dev libpcre3-dev \
 && apt-get autoremove -y 

# Add environment variables
ENV PYTHONUNBUFFERED=1

# Add driver.sh and make it executable
ADD driver.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/driver.sh

USER mud
EXPOSE 4040 4041/udp
VOLUME /mud

CMD [ "/usr/local/bin/driver.sh" ]
