FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install conan && conan profile detect --force

WORKDIR /app

COPY conanfile.txt .
RUN conan install . --output-folder=build --build=missing -s build_type=Release

COPY . .

WORKDIR /app/build
RUN cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release \
    && ninja

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/phonebook-service .
COPY --from=builder /app/swagger-res ./swagger-res

EXPOSE 8000

CMD ["./phonebook-service"]