#include "diskdb.h"

#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <algorithm>

using namespace std;

void DiskNewsGroup::refresh() {
    DIR* dir = opendir(_path.c_str());
    struct dirent* entry;

    arts.resize(0);
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != 0x8 ||
            strcmp(entry->d_name, "0") == 0) continue;

        string path = _path + "/" + entry->d_name;
        ifstream in(path);

        int id;
        long len;
        string title, author, txt;
        in >> id;

        in >> len;
        in.ignore(1);
        title = std::string(len, '\0');
        in.read(&title[0], len);

        in >> len;
        in.ignore(1);
        author = std::string(len, '\0');
        in.read(&author[0], len);

        in >> len;
        in.ignore(1);
        txt = std::string(len, '\0');
        in.read(&txt[0], len);
    
        arts.emplace_back(title, author, txt, id);
        in.close();
    }

    closedir(dir);
}

void DiskNewsGroup::rmrf() {
    DIR* dir = opendir(_path.c_str());
    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL)
        if (entry->d_type == 0x8)
            remove((_path + "/" + entry->d_name).c_str());

    closedir(dir);
    rmdir(_path.c_str());
}

void DiskNewsGroup::create_article(string& title, string& author, string& content) {
    int id = _counter++;
    
    arts.emplace_back(title, author, content, id);
    Article& art = arts.back();

    string id_str = std::to_string(id);
    ofstream out(_path + "/" + id_str, std::ofstream::out | std::ofstream::trunc);
    out << art.id() << " "
        << title.length() << " " << title
        << author.length() << " " << author
        << content.length() << " " << content;
    out.flush(); out.close();

    save();
}

void DiskNewsGroup::save() {
    ofstream out(_path + "/0", std::ofstream::out | std::ofstream::trunc);
    out << _id << " " << _counter << " " << _name.length() << " " << _name;
    out.flush(); out.close();
}

bool DiskNewsGroup::delete_article(int id) {
    refresh();

    auto it = find_if(arts.begin(), arts.end(), [id](Article& art) {
        return art.id() == id;
    });

    if (it == arts.end())
        return false;
    
    string id_str = std::to_string(id);
    remove((_path + "/" + id_str).c_str());
    
    arts.erase(it);
    return true;
}

void DiskNewsGroup::list_articles(std::function<void(Article*)> cb) {
    refresh();

    for (auto& art : arts)
        cb(&art);
}

Article* DiskNewsGroup::find_article(int article_id) {
    refresh();

    auto it = std::find_if(arts.begin(), arts.end(), [article_id](Article& art) {
        return art.id() == article_id;
    });

    return it == arts.end() ? nullptr : &(*it);
}

int DiskNewsGroup::articles() {
    refresh();
    return arts.size();
}

//

void DiskDB::refresh() {
    DIR* dir = opendir(root.c_str());
    struct dirent* entry;

    groups.resize(0);
    while ((entry = readdir(dir)) != NULL)
        if (entry->d_type == 0x4 &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0)
            groups.emplace_back(root, entry);

    closedir(dir);
}

bool DiskDB::create_newsgroup(const string& name) {
    refresh();

    auto it = std::find_if(groups.begin(), groups.end(), [&](NewsGroup& ng) {
        return ng.name() == name;
    });

    if (it != groups.end())
        return false;

    groups.emplace_back(root, name, group_counter++);

    ofstream out(root + "/0", std::ofstream::out | std::ofstream::trunc);
    out << group_counter;
    out.flush(); out.close();

    return true;
}

bool DiskDB::delete_newsgroup(int id) {
    refresh();

    auto it = std::find_if(groups.begin(), groups.end(), [&](NewsGroup& ng) {
        return ng.id() == id;
    });

    if (it == groups.end())
        return false;

    it->rmrf();
    groups.erase(it);
    return true;
}

NewsGroup* DiskDB::find_group(int id) {
    refresh();

    auto it = std::find_if(groups.begin(), groups.end(), [&](NewsGroup& ng) {
        return ng.id() == id;
    });

    return it == groups.end() ? nullptr : &(*it);
}

void DiskDB::list_newsgroups(function<void(NewsGroup*)> cb) {
    refresh();
    for (auto& ng : groups)
        cb(&ng);
}

int DiskDB::newsgroups() {
    refresh();
    return groups.size();
}
