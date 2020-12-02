#!/usr/bin/env bash

# Extract our command line arguments
COMPILER=$1

# Convert our compiler into a list of packages it needs
if [[ $COMPILER == gcc5 ]]; then
    PACKAGES=g++-5 g++-5-multilib g++-multilib
elif [[ $COMPILER == gcc6 ]]; then
    PACKAGES=g++-6 g++-6-multilib g++-multilib
elif [[ $COMPILER == gcc7 ]]; then
    PACKAGES=g++-7 g++-7-multilib g++-multilib
elif [[ $COMPILER == gcc8 ]]; then
    PACKAGES=g++-8 g++-8-multilib g++-multilib
elif [[ $COMPILER == gcc9 ]]; then
    PACKAGES=g++-9 g++-9-multilib g++-multilib
elif [[ $COMPILER == gcc10 ]]; then
    PACKAGES=g++-10 g++-10-multilib g++-multilib
fi

# Install the packages we need
sudo -E apt-add-repository -y "ppa:ubuntu-toolchain-r/test";
sudo -E apt-get -yq update;
sudo -E apt-get -yq --no-install-suggests --no-install-recommends --force-yes install $PACKAGES;
