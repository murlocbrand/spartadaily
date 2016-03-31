#ifndef PLEBDB_H
#define PLEBDB_H

#include "database.h"

class PlebArticle : public Article {
public:
    PlebArticle(std::string h1, std::string author, std::string txt) : 
        _author(author), _content(txt), _title(h1) {
            static int counter = 1;
            _id = counter++;
        }

    std::string& author() { return _author; }
    std::string& title() { return _title; }
    std::string& content() { return _content; }
    int id() { return _id; }
private:
    std::string _author, _content, _title;
    int _id;
};

class PlebNewsGroup : public NewsGroup {
public:
    PlebNewsGroup(std::string name) : group_name(name) {
        static int counter = 1;
        group_id = counter++;
    }

    void list_articles(std::function<void(Article*)> cb);
    void create_article(string& title, string& author, string& content);
    bool delete_article(int id);
    Article* find_article(int article_id);

    int articles() { return arts.size(); }
    int id() { return group_id; }
    std::string name() { return group_name; }
private:
    std::vector<PlebArticle> arts;
    std::string group_name;
    int group_id;
};

class PlebDB : public Database {
public:
    PlebDB() {}

    int newsgroups() { return groups.size(); }
    void list_newsgroups(std::function<void(NewsGroup*)> cb);
    NewsGroup* find_group(int id);
    bool create_newsgroup(const std::string& name);
    bool delete_newsgroup(int id);
private:
    std::vector<PlebNewsGroup> groups;
};
#endif