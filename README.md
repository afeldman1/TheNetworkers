TheNetworkers
=============

This readme is for the group project for Fall 2013 CPE-490.

On Windows
==========
On Windows, open the project in Visual Studio (or any IDE), and compile as normal.
To run it, run the output executable.

On OSX
======
On OSX, compile/run server/client with this command:

g++ chat_server.cpp -std=c++11 -stdlib=libstdc++ -o chat_server
./chat_server

g++ chat_client.cpp -std=c++11 -stdlib=libstdc++ -o chat_client
./chat_client

On Linux
========
On Linux, compile/run server/client with this command:

g++ chat_server.cpp -std=c++11 -lpthread -o chat_server
./chat_server

g++ chat_client.cpp -std=c++11 -lpthread -o chat_client
./chat_client