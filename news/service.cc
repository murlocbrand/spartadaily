#include "service.h"
#include "plebdb.h"
#include "protocol.h"
#include <iostream>
using namespace std;

Service::Service() : db(new PlebDB()) {}
//Service::Service() : db(new DiskDatabase()) {}

void Service::list_ng(Message& msg) {
    // COM_LIST_NG COM_END
    // ANS_LIST_NG num_p [num_p string_p]* ANS_END
    msg << db->newsgroups();

    db->list_newsgroups([&](NewsGroup* ng) {
        msg << ng->id() << ng->name();
    });
}

void Service::create_ng(Message& msg) {
    // COM_CREATE_NG string_p COM_END
    // ANS_CREATE_NG [ANS_ACK | ANS_NAK ERR_NG_ALREADY_EXISTS] ANS_END
    string title;
    msg >> title;

    if (db->create_newsgroup(title)) {
        msg << static_cast<unsigned char>(Protocol::ANS_ACK);
        return;
    }

    msg << static_cast<unsigned char>(Protocol::ANS_NAK)
        << static_cast<unsigned char>(Protocol::ERR_NG_ALREADY_EXISTS);
}

void Service::delete_ng(Message& msg) {
    // COM_DELETE_NG num_p COM_END
    // ANS_DELETE_NG [ANS_ACK | ANS_NAK ERR_NG_DOES_NOT_EXIST] ANS_END
    int group_id = msg.num();

    if (db->delete_newsgroup(group_id)) {
        msg << static_cast<unsigned char>(Protocol::ANS_ACK);
        return;
    }

    msg << static_cast<unsigned char>(Protocol::ANS_NAK)
        << static_cast<unsigned char>(Protocol::ERR_NG_DOES_NOT_EXIST);
}

void Service::list_art(Message& msg) {
    // COM_LIST_ART num_p COM_END
    // ANS_LIST_ART [ANS_ACK num_p [num_p string_p]* |
    //               ANS_NAK ERR_NG_DOES_NOT_EXIST] ANS_END
    int group_id = msg.num();

    NewsGroup* ng = db->find_group(group_id);
    if (ng == nullptr) {
        msg << static_cast<unsigned char>(Protocol::ANS_NAK)
            << static_cast<unsigned char>(Protocol::ERR_NG_DOES_NOT_EXIST);
        return;
    }

    msg << static_cast<unsigned char>(Protocol::ANS_ACK);
    msg << ng->articles();
    ng->list_articles([&](Article* art) {
        msg << art->id() << art->title();
    });
}

void Service::create_art(Message& msg) {
    // COM_CREATE_ART num_p string_p string_p string_p COM_END
    // ANS_CREATE_ART [ANS_ACK | ANS_NAK ERR_NG_DOES_NOT_EXIST] ANS_END
    int group_id;
    string title, author, content;
    msg >> group_id >> title >> author >> content;

    NewsGroup* ng = db->find_group(group_id);
    if (ng == nullptr) {
        msg << static_cast<unsigned char>(Protocol::ANS_NAK)
            << static_cast<unsigned char>(Protocol::ERR_NG_DOES_NOT_EXIST);
        return;
    }

    ng->create_article(title, author, content);
    msg << static_cast<unsigned char>(Protocol::ANS_ACK);
}

void Service::delete_art(Message& msg) {
    // COM_DELETE_ART num_p num_p COM_END
    // ANS_DELETE_ART [ANS_ACK |
    //                 ANS_NAK [ERR_NG_DOES_NOT_EXIST | ERR_ART_DOES_NOT_EXIST]] ANS_END
    int group_id, art_id;
    msg >> group_id >> art_id;

    NewsGroup* ng = db->find_group(group_id);
    if (ng == nullptr) {
        msg << static_cast<unsigned char>(Protocol::ANS_NAK)
            << static_cast<unsigned char>(Protocol::ERR_NG_DOES_NOT_EXIST);
        return;
    }

    if (!ng->delete_article(art_id)) {
        msg << static_cast<unsigned char>(Protocol::ANS_NAK)
            << static_cast<unsigned char>(Protocol::ERR_ART_DOES_NOT_EXIST);
        return;
    }

    msg << static_cast<unsigned char>(Protocol::ANS_ACK);
}   

void Service::get_art(Message& msg) {
    // COM_GET_ART num_p num_p COM_END
    // ANS_GET_ART [ANS_ACK string_p string_p string_p |
    //              ANS_NAK [ERR_NG_DOES_NOT_EXIST | ERR_ART_DOES_NOT_EXIST]] ANS_END
    int group_id, art_id;
    msg >> group_id >> art_id;

    NewsGroup* ng = db->find_group(group_id);
    if (ng == nullptr) {
        msg << static_cast<unsigned char>(Protocol::ANS_NAK)
            << static_cast<unsigned char>(Protocol::ERR_NG_DOES_NOT_EXIST);
        return;
    }

    Article* art = ng->find_article(art_id);
    if (art == nullptr) {
        msg << static_cast<unsigned char>(Protocol::ANS_NAK)
            << static_cast<unsigned char>(Protocol::ERR_ART_DOES_NOT_EXIST);
        return;
    }
    
    unsigned char ack = Protocol::ANS_ACK;
    msg << ack << art->title() << art->author() << art->content();
}

void Service::process(shared_ptr<Connection>& conn) {
    unsigned char cmd = conn->read();
    Message msg(conn);

    // ANS
    conn->write(cmd + 19);

    switch (cmd) {
    case Protocol::COM_LIST_NG:     list_ng(msg); break;
    case Protocol::COM_CREATE_NG:   create_ng(msg); break;
    case Protocol::COM_DELETE_NG:   delete_ng(msg); break;
    case Protocol::COM_LIST_ART:    list_art(msg); break;
    case Protocol::COM_CREATE_ART:  create_art(msg); break;
    case Protocol::COM_DELETE_ART:  delete_art(msg); break;
    case Protocol::COM_GET_ART:     get_art(msg); break;
    default:                        msg << Protocol::ANS_NAK;
    }
    
    // CMD_END
    if (conn->read() != Protocol::COM_END)
        cout << "SHIT" << endl;

    conn->write(Protocol::ANS_END);
}
