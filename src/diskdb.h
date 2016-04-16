#ifndef DISKDB_H
#define DISKDB_H

#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>

#include "database.h"

using namespace std;

class DiskArticle : public Article {
public:
    DiskArticle() : _author(""), _content(""), _title(""), _id(-1) {}
    
    DiskArticle(string h1, string author, string txt, int id) : 
        _author(author), _content(txt), _title(h1), _id(id) {}

    string& author() { return _author; }
    string& title() { return _title; }
    string& content() { return _content; }
    int id() { return _id; }
private:
    string _author, _content, _title;
    int _id;
};

class DiskNewsGroup : public NewsGroup {
public:
    DiskNewsGroup() : _name(""), _path(""), _id(-1), _counter(-1) {}

    DiskNewsGroup(string root, string name, int id) : 
        _name(name), _id(id), _counter(1) {
        string id_str = std::to_string(id);
        _path = root + "/" + id_str;
        mkdir(_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        save();
    }

    DiskNewsGroup(string root, dirent* entry) {
        _path = root + "/" + entry->d_name;
        ifstream id_file(_path + "/0");
        
        int len;
        id_file >> _id >> _counter >> len;
        
        id_file.ignore(1);
        _name = std::string(len, '\0');
        id_file.read(&_name[0], len);

        id_file.close();
    }
    
    void rmrf();
    void refresh();
    void save();

    void list_articles(function<void(Article*)> cb);
    void create_article(string& title, string& author, string& content);
    bool delete_article(int id);
    Article* find_article(int article_id);

    int articles();
    int id() { return _id; }
    string name() { return _name; }
private:
    vector<DiskArticle> arts;
    string _name, _path;
    int _id, _counter;
};

class DiskDB : public Database {
public:
    DiskDB(string path) : root(path), group_counter(1) {
        ifstream id_file(path + "/0");
        if (id_file)
            id_file >> group_counter;
        id_file.close();
    }

    void refresh();

    int newsgroups();
    void list_newsgroups(function<void(NewsGroup*)> cb);
    NewsGroup* find_group(int id);
    bool create_newsgroup(const string& name);
    bool delete_newsgroup(int id);

private:
    string root;
    int group_counter;
    vector<DiskNewsGroup> groups;
};
#endif