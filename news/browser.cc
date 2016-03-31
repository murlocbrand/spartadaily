#include "browser.h"
#include "protocol.h"
#include "message.h"

using namespace std;

unsigned char Browser::list_groups() {
    msg << static_cast<unsigned char>(Protocol::COM_LIST_NG)
        << static_cast<unsigned char>(Protocol::COM_END);

    int ans = msg.byte();
    if (ans != Protocol::ANS_LIST_NG)
        return ans;

    int nog = msg.num();
    groups.resize(0);
    for (int i = 0; i < nog; ++i) {
        int id; string title;
        msg >> id >> title;
        groups.emplace_back(title, id);
    }

    int end = msg.byte();
    if (end != Protocol::ANS_END)
        return end;

    selector = true;
    return Protocol::ANS_ACK;
}

unsigned char Browser::create_group(string name) {
    msg << static_cast<unsigned char>(Protocol::COM_CREATE_NG)
        << name
        << static_cast<unsigned char>(Protocol::COM_END);

    int ans = msg.byte();
    if (ans != Protocol::ANS_CREATE_NG)
        return ans;

    int ok = msg.byte();

    // consume potential error message
    if (ok == Protocol::ANS_NAK)
        ok = msg.byte();

    int end = msg.byte();
    if (end != Protocol::ANS_END)
        return end;

    list_groups();
    return ok;
}
unsigned char Browser::delete_group(int idx) {
    if (idx < 0 || idx >= groups.size())
        return 255;

    msg << static_cast<unsigned char>(Protocol::COM_DELETE_NG)
        << groups[idx].id
        << static_cast<unsigned char>(Protocol::COM_END);

    int ans = msg.byte();
    if (ans != Protocol::ANS_DELETE_NG)
        return ans;

    int ok = msg.byte();

    // consume potential error message
    if (ok == Protocol::ANS_NAK) 
        ok = msg.byte();

    int end = msg.byte();
    if (end != Protocol::ANS_END)
        return end;

    list_groups();
    return ok;
}

unsigned char Browser::list_articles(int idx) {
    if (idx < 0 || idx >= groups.size())
        return 255;

    msg << static_cast<unsigned char>(Protocol::COM_LIST_ART)
        << groups[idx].id
        << static_cast<unsigned char>(Protocol::COM_END);

    int ans = msg.byte();
    if (ans != Protocol::ANS_LIST_ART)
        return ans;

    int ok = msg.byte();
    if (ok == Protocol::ANS_ACK) {
        int noa = msg.num();

        articles.resize(0);
        while (noa--) {
            int id; string title;
            msg >> id >> title;
            articles.emplace_back(title, id);
        }
    } else
        ok = msg.byte();
    
    int end = msg.byte();
    if (end != Protocol::ANS_END)
        return end;

    selector = false;
    return ok;
}

unsigned char Browser::create_article(int idx, string title, string author, string text) {
    if (idx < 0 || idx >= groups.size())
        return 255;

    msg << static_cast<unsigned char>(Protocol::COM_CREATE_ART)
        << groups[idx].id << title << author << text
        << static_cast<unsigned char>(Protocol::COM_END);

    int ans = msg.byte();
    if (ans != Protocol::ANS_CREATE_ART)
        return ans;

    int ack = msg.byte();
    if (ack != Protocol::ANS_ACK)
        ack = msg.byte();

    int end = msg.byte();
    if (end != Protocol::ANS_END)
        return end;

    return ack;
}

unsigned char Browser::delete_article(int gid, int aid) {
    if (gid < 0 || gid >= groups.size() ||
        aid < 0 || aid >= articles.size())
        return 255;

    msg << static_cast<unsigned char>(Protocol::COM_DELETE_ART)
        << groups[gid].id << articles[aid].id
        << static_cast<unsigned char>(Protocol::COM_END);

    int ans = msg.byte();
    if (ans != Protocol::ANS_DELETE_ART)
        return ans;

    int ack = msg.byte();
    if (ack != Protocol::ANS_ACK)
        ack = msg.byte();

    int end = msg.byte();
    if (end != Protocol::ANS_END)
        return end;

    return ack;
}

unsigned char Browser::get_article(int gid, int aid) {
    if (gid < 0 || gid >= groups.size() ||
        aid < 0 || aid >= articles.size())
        return 255;

    msg << static_cast<unsigned char>(Protocol::COM_GET_ART)
        << groups[gid].id << articles[aid].id
        << static_cast<unsigned char>(Protocol::COM_END);

    int ans = msg.byte();
    if (ans != Protocol::ANS_GET_ART)
        return ans;

    int ack = msg.byte();
    if (ack == Protocol::ANS_ACK)
        msg >> title >> author >> content;
    else
        ack = msg.byte();

    int end = msg.byte();
    if (end != Protocol::ANS_END)
        return end;

    return ack;
}