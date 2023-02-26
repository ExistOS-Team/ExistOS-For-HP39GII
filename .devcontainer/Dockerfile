ARG VARIANT="ubuntu-22.04"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

ARG USERNAME=vscode

#
# Update apt package list and install packages
#
RUN apt-get update \
    && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install \
        build-essential \
        cmake \
        gdb \
        git \
        gcc-arm-none-eabi \
        clangd \
        libcrypto++8 \
        libcrypto++-dev \
        libusb-1.0.0-dev 

RUN SNIPPET="export PROMPT_COMMAND='history -a' && export HISTFILE=/commandhistory/.bash_history" \
    && mkdir /commandhistory \
    && touch /commandhistory/.bash_history \
    && chown -R $USERNAME /commandhistory \
    && echo "$SNIPPET" >> "/home/$USERNAME/.bashrc"