# Pushprox-client
An OpenWRT package with a small client for Prometheus [PushProx](https://github.com/prometheus-community/PushProx). 
The official PushProx client is written in Go, and after compiling, the binary size is too large to be installed on 
routers such as the TP-Link Archer C7 v5.

# How to build
1.  Install all [required packages](https://openwrt.org/docs/guide-developer/toolchain/install-buildsystem)
to build OpenWRT example for Ubuntu:
```
sudo apt update
sudo apt install build-essential clang flex bison g++ gawk \
gcc-multilib g++-multilib gettext git libncurses-dev libssl-dev \
python3-distutils python3-setuptools rsync swig unzip zlib1g-dev file wget
```
2. Download OpenWRT SDK for your router architecture. For example, the SDK for the TP-Link Archer C7 v5 router can be 
found [here](https://mirror-03.infra.openwrt.org/releases/22.03.6/targets/ath79/generic/).
```commandline
wget https://mirror-03.infra.openwrt.org/releases/22.03.6/targets/ath79/generic/openwrt-sdk-22.03.6-ath79-generic_gcc-11.2.0_musl.Linux-x86_64.tar.xz 
```
3. Navigate to the package dir and clone this repo.
4. From SDK root dir execute the following command to build the package.
```commandline
make package/pushprox-client/compile
```
5. Copy package to the router and install it.
```commandline
scp bin/packages/mips_24kc/base/pushprox-client_1.0.0-1_mips_24kc.ipk root@192.168.1.1:~

opkg update
opkg install pushprox-client_1.0.0-1_mips_24kc.ipk
```

# Usage
```commandline
pushprox-client [pushprox-proxy-url] [wan_ip]

#Example 
pushprox-client http://pushproxy-adress.com 192.168.50.12
```
