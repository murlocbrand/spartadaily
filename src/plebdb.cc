#include "plebdb.h"
#include <algorithm>

void PlebNewsGroup::list_articles(std::function<void(Article*)> cb) {
    for (auto& art : arts)
        cb(&art);
}

void PlebNewsGroup::create_article(string& title, string& author, string& content) {
    arts.emplace_back(title, author, content);

}
bool PlebNewsGroup::delete_article(int id) {
    auto it = find_if(arts.begin(), arts.end(), [id](Article& art) {
        return art.id() == id;
    });

    if (it == arts.end())
        return false;
    
    arts.erase(it);
    return true;
}

Article* PlebNewsGroup::find_article(int article_id) {
    auto it = std::find_if(arts.begin(), arts.end(), [article_id](Article& art) {
        return art.id() == article_id;
    });
    return it == arts.end() ? nullptr : &(*it);
}

bool PlebDB::create_newsgroup(const std::string& name) {
    auto it = std::find_if(groups.begin(), groups.end(), [&](NewsGroup& ng) {
        return ng.name() == name;
    });

    if (it != groups.end())
        return false;

    groups.emplace_back(name);
    return true;
}

bool PlebDB::delete_newsgroup(int id) { 
    auto it = std::find_if(groups.begin(), groups.end(), [&](NewsGroup& ng) {
        return ng.id() == id;
    });

    if (it == groups.end())
        return false;

    groups.erase(it);
    return true;
}

NewsGroup* PlebDB::find_group(int id) {
    auto it = std::find_if(groups.begin(), groups.end(), [&](NewsGroup& ng) {
        return ng.id() == id;
    });
    return it == groups.end() ? nullptr : &(*it);
}

void PlebDB::list_newsgroups(std::function<void(NewsGroup*)> cb) {
    for (auto& ng : groups)
        cb(&ng);
}
