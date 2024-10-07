## Dependency:

### protobuf:
```sh
### Installation
$ sudo apt update
$ sudo apt install protobuf-compiler
### verify the installation
$ protoc
```

### goby: [reference](https://goby.software/3.0/)
```sh
# To install release packages on Ubuntu
$ echo "deb http://packages.gobysoft.org/ubuntu/release/ `lsb_release -c -s`/" | sudo tee /etc/apt/sources.list.d/gobysoft_release.list
$ sudo apt-key adv --recv-key --keyserver keyserver.ubuntu.com 19478082E2F8D3FE
$ sudo apt update
# minimal
sudo apt install libgoby3-dev goby3-apps
# full
sudo apt install libgoby3-dev libgoby3-gui-dev goby3-apps goby3-gui goby3-doc goby3-test libgoby3-moos-dev goby3-moos
```