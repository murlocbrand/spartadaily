#include "protocol.h"
#include "message.h"
#include <string>
#include <iostream>

using namespace std;

Connection& operator<<(Connection& conn, int num) {
    conn.write((num >> 24) & 0xFF);
    conn.write((num >> 16) & 0xFF);
    conn.write((num >> 8)  & 0xFF);
    conn.write(num & 0xFF);
    return conn;
}

Connection& operator>>(Connection& conn, int& num) {
    unsigned char byte1 = conn.read();
    unsigned char byte2 = conn.read();
    unsigned char byte3 = conn.read();
    unsigned char byte4 = conn.read();
    num = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
    return conn;
}

Connection& operator<<(Connection& conn, string str) {
    for (auto c : str)
        conn.write(c);
    return conn;
}

Message& Message::operator>>(int& num) {
    unsigned char type = line->read();
    
    if (type != Protocol::PAR_NUM) {
        num = -1;
        return *this;
    }

    *line >> num;
    return *this;
}

unsigned char Message::byte() { return line->read(); }

int Message::num() { 
    int tmp;
    *this >> tmp;
    return tmp; 
}

Message& Message::operator<<(unsigned char cmd) {
    line->write(cmd);
    return *this;
}

Message& Message::operator>>(string& str){
    unsigned char type = line->read();
    
    if (type != Protocol::PAR_STRING) {
        str = "";
        return *this;
    }

    int len;
    *line >> len;

    str.resize(len);
    for (int i = 0; i < len; ++i)
        str[i] = line->read();

    return *this;
}

Message& Message::operator<<(int num) {
    line->write(Protocol::PAR_NUM);
    *line << num;
    return *this;
}

Message& Message::operator<<(string& str) {
    line->write(Protocol::PAR_STRING);
    *line << str.length() << str;
    return *this;
}

Message& Message::operator<<(string&& str) {
    line->write(Protocol::PAR_STRING);
    *line << str.length() << str;
    return *this;
}