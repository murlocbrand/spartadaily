#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include <string>
#include <functional>

using namespace std;

class Article {
public:
    virtual string& author() = 0;
    virtual string& content() = 0;
    virtual string& title() = 0;
    virtual int id() = 0;
};

class NewsGroup {
public:
    virtual int articles() = 0;
    virtual void list_articles(function<void(Article*)> cb) = 0;
    virtual void create_article(string& title, string& author, string& content) = 0;
    virtual bool delete_article(int id) = 0;
    virtual Article* find_article(int article_id) = 0;

    virtual int id() = 0;
    virtual string name() = 0;
};

class Database {
public:
    virtual int newsgroups() = 0;
    virtual void list_newsgroups(function<void(NewsGroup*)> cb) = 0;
    virtual NewsGroup* find_group(int id) = 0;
    virtual bool create_newsgroup(const string& name) = 0;
    virtual bool delete_newsgroup(int id) = 0;
};

#endif