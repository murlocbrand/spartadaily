#ifndef BROWSER_H
#define BROWSER_H

#include <vector>
#include <string>

#include "message.h"

using namespace std;

class Browser {
public:
    struct Item {
        Item() : title(""), id(-1) {}
        Item(string title, int id) : title(title), id(id) {}
    
        string title;
        int id;
    };

    Browser(Message& msg) : msg(msg), selector(true),
        title(""), author(""), content("") {}

    unsigned char list_groups();
    unsigned char create_group(string name);
    unsigned char delete_group(int group_id);

    unsigned char list_articles(int group_id);
    unsigned char create_article(int group_id, string title, string author, string text);
    unsigned char delete_article(int gid, int aid);
    unsigned char get_article(int gid, int aid);

    bool have_groups() { return groups.size() > 0; }
    bool have_articles() { return articles.size() > 0; }

    vector<Browser::Item>& newsgroups() { return groups; }
    vector<Browser::Item>& news() { return articles; }

    // returns item list depending on if last_groups or list_articles was run
    vector<Browser::Item>& selection() { return selector ? groups : articles; }

    string& art_author() { return author; }
    string& art_title() { return title; }
    string& art_content() { return content; }
    
private:
    Message& msg;
    bool selector;
    vector<Item> groups, articles;

    string title, author, content;
};

#endif