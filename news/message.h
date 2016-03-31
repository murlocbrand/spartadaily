#ifndef MESSAGE_H
#define MESSAGE_H

#include "connection.h"
#include "connectionclosedexception.h"

#include <string>

using namespace std;

Connection& operator<<(Connection& conn, int num);
Connection& operator>>(Connection& conn, int& i);
Connection& operator<<(Connection& conn, string str);

class Message {
public:
    Message(const std::shared_ptr<Connection>& conn) : line(conn) {};

    unsigned char byte();
    int num();

    Message& operator>>(int& num);
    Message& operator>>(string& str);

    Message& operator<<(int num);
    Message& operator<<(unsigned char cmd);
    Message& operator<<(string& str);
    Message& operator<<(string&& str);

private:
    const std::shared_ptr<Connection>& line;
};

#endif