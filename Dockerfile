FROM alpine:3.20


# Set working directory inside the container
WORKDIR /app

# Install basic tools (optional, useful for dev/debugging)
# RUN dnf -y update && \
#     dnf -y install vim curl wget git && \
#     dnf clean all\
RUN   apk add --no-cache bash vim curl wget git

# Copy your project files into container (if you have any)
COPY . /app

# Default command (interactive shell)
CMD ["/bin/bash"]
