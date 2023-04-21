# Shield Link

[![GitHub issues](https://img.shields.io/github/issues/io-ing/shieldlink)](https://github.com/io-ing/shieldlink/issues)
[![GitHub forks](https://img.shields.io/github/forks/io-ing/shieldlink)](https://github.com/io-ing/shieldlink/network)
[![GitHub stars](https://img.shields.io/github/stars/io-ing/shieldlink)](https://github.com/io-ing/shieldlink/stargazers)
[![GitHub license](https://img.shields.io/github/license/io-ing/shieldlink)](https://github.com/io-ing/shieldlink/blob/master/LICENSE)

ShieldLink is a program written by C++, which provides a secure encrypted communication solution. Through different levels of key negotiation between the server and the client, asymmetric encryption is used to ensure the confidentiality of data transmission in the communication process. At the same time, the program can also record the key information to the database for subsequent use and management. The program also provides an external interface, which provides encryption protection for the communication of other programs.

# Installation

## Dependencies

- C++ 11 or higher
- OpenSSL library
- Protobuf library
- JSON library
- Oracle client 11gR2

## Environment

- Windows 10 or Linux
- Visual Studio or GCC

## Steps

1. Clone the project to local `git clone https://github.com/io-ing/shieldlink.git`

2. Enter the project directory `cd shieldlink`

3. Compile the project

   1. ```
      cd ServerSeckey/ServerSeckey
      g++ *.cpp *.cc -ljson -lprotobuf -lcrypto -std=c++11
      ```

   2. ```
      cd ClientSecKey/ClientSecKey
      g++ *.cpp *.cc -ljson -lprotobuf -lcrypto -std=c++11
      ```

4. Run the project

# Usage

## Client

Copy configuration file, edit configuration file, run the client program, Follow the prompts.

```
$ cp client.json ClientSecKey/ClientSecKey
$ cd ClientSecKey/ClientSecKey
$ vim client.json
$ ./shieldlink_client
  /=============================================================/
  /=============================================================/
  /*     1.seckeyAgree                                         */
  /*     2.seckeyCheck                                         */
  /*     3.seckeyLogoff                                        */
  /*     0.exit                                                */
  /=============================================================/
  /=============================================================/
```

## Server

Copy configuration file, edit configuration file, run the server program, then wait for client connection and message.

```
$ cp server.json ServerSeckey/ServerSeckey
$ cd ServerSeckey/ServerSeckey
$ vim server.json
$ ./shieldlink_server
```

# Advanced

Provide interfaces for third-party programs.

1. Compile the Interface

   1. ```
      cd Interface\Interface
      g++ -c *.cpp -fpic -std=c++11
      g++ -shared *.o -o libinterface.so
      cp libInterface.so /usr/lib
      echo export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib >> ∼/.bashrc
      . ~/.bashrc
      ```

2. Run the Test program

   1. ```
      cd socket-test\TcpServer\TcpServer
      ./TcpServer
      ```

   2. ```
      cd socket-test\TcpClient\TcpClient
      ./TcpClient
      ```

# License

This project is licensed under the Mozilla Public License 2.0. See [LICENSE](LICENSE) file for more information.

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-brightgreen.svg)](https://opensource.org/licenses/MPL-2.0)