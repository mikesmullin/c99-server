# Use the latest Alpine Linux edge image for newer packages
FROM alpine:edge

# Install Clang, musl-dev (for musl libc headers), build tools, lldb (includes lldb-server) for remote debugging, and tini for signal handling
RUN apk add --no-cache clang musl-dev build-base lldb tini && \
    adduser -D app && \
    mkdir -p /app/build && \
    chown -R app:app /app

# Set working directory
WORKDIR /app

# Switch to the non-root user
USER app

# Expose port for lldb-server
EXPOSE 2345

# Use tini as entrypoint to properly forward signals
ENTRYPOINT ["tini", "--"]

# Arguments for test compilation
ARG IN_FILE=src/main.c
ARG OUT_BINARY=main
# Persist ARG value in an environment variable
#ENV OUT_BINARY=${OUT_BINARY}

# Copy the C source code into the container (will be created later)
COPY . .

# Compile the C program with Clang
RUN clang @clang_options.rsp -O0 -gdwarf-4 -DDEBUG_SLOW "$(echo "${IN_FILE}" | sed 's/\\/\//g')" -o build/${OUT_BINARY} && \
    echo "lldb-server g *:2345 -- build/${OUT_BINARY}" > entrypoint.sh

# Command to run the compiled program with lldb-server for remote debugging
# CMD ["lldb-server", "platform", "--listen", "*:2345", "--server"]
# CMD ["lldb-server", "g", "*:2345", "--", "build/$OUT_BINARY"]
# CMD ["./build/main"]
CMD ["sh", "./entrypoint.sh"]
