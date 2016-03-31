/* myclient.cc: sample client program */
#include "connection.h"
#include "connectionclosedexception.h"

#include "message.h"
#include "protocol.h"
#include "browser.h"

#include <ncurses.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <vector>

using namespace std;

string error_message(int code) {
    switch (code) {
    case Protocol::ERR_NG_ALREADY_EXISTS: return "Newsgroup already exists.";
    case Protocol::ERR_NG_DOES_NOT_EXIST: return "Newsgroup doesn't exist.";
    case Protocol::ERR_ART_DOES_NOT_EXIST: return "Article does not exist.";
    case Protocol::ANS_ACK: return "Success.";
    case Protocol::ANS_NAK: return "Command not acknowledged (unknown error).";
    case 255: return "Invalid group or article index.";
    default: return "Unknown protocol error.";
    }
    return "Unkown error code.";
}

// couldn't figure out how to use cin>> with echo enabled (ncurses doesn't like) 
string readstr() {
    string str = "";
    int c;
    echo();
    while ((c = getch()) != -1) {
        if (c == 13) break;
        str.push_back(c);
    }
    noecho();
    return str;
}

int readint() {
    int num = 0; 
    bool nochar = true;
    int c;

    echo();
    while ((c = getch()) != -1) {
        if (c == 13 || c < '0' || c > '9') 
            break;
        num = num * 10 + (c - '0');
        nochar = false;
    }
    noecho();
    
    if (nochar)
        return -1;

    return num;
}

void error(string reason) {
    endwin();
    cerr << reason << endl;
    exit(1);
}

void clear_display() {
    int row, col;
    getmaxyx(stdscr, row, col);

    // leave option and status bars be
    for (int y = 0; y != row - 2; ++y) {
        move(y, 0);
        clrtoeol();
    }

    refresh();
}

void draw_list(int& page, vector<Browser::Item>& items) {
    int row, col;
    getmaxyx(stdscr, row, col);
    row -= 2;

    clear_display();
    move(0, 0);

    if (page < 0)
        page = 0;
    else if (page * row > items.size())
        page = items.size() / row;

    for (int i = page * row; i != items.size(); ++i) {
        auto& item = items[i];
        printw("%i: %s", i, item.title.c_str());

        int last_line_len = item.title.length() % col;
        if (last_line_len + 1 < col)
            addstr("\n");
    }

    refresh();
}

void print_bar(int row_offset, string str) {
    int row, col;
    getmaxyx(stdscr, row, col);
    mvaddstr(row - row_offset, 0, str.c_str());
    clrtoeol();
}

void query_input(string prompt) {
    print_bar(2, prompt.c_str());
}

void options_bar(bool mk, bool ls, bool rm, bool articles, bool groups) {
    if (mk || ls || rm) {
        if (!groups)
            print_bar(1, "g = group");
        else
            print_bar(1, "a = article, g = group");            
    } else {
        string opt = "q = quit, c = create, l = list";
        if (articles || groups) 
            opt += ", d = delete";
        if (articles) 
            opt += ", r = read";
        print_bar(1, opt);
    }
}

void status_bar(bool mk, bool ls, bool rm) {
    if (ls)
        query_input("> list");
    else if (mk)
        query_input("> create");
    else if (rm)
        query_input("> delete");
    else
        query_input("> ");

    clrtoeol();
    refresh();
}

void select_group(Browser& browser, int& prev, int& page) {
    if (prev != -1)
        return;

    if (page == 0)
        draw_list(page, browser.newsgroups());
    query_input("group> "); refresh();
    prev = readint();
}

int main(int argc, char* argv[]) {
    initscr(); cbreak(); noecho();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);

    if (argc != 3)
        error("Usage: reader host-name port-number");
    
    int port = -1;
    try {
        port = stoi(argv[2]);
    } catch (exception& e) {
        error("Invalid port number.");
    }
    
    std::shared_ptr<Connection> conn = make_shared<Connection>(argv[1], port);
    if (!conn->isConnected())
        error("Connection attempt failed");

    bool mk = false, ng = false, art = false, 
         ls = false, rm = false, read = false;

    options_bar(mk, ls, rm, false, false);
    status_bar(mk, ls, rm);

    int opt;
    int page = 0;
    int last_group = 0;

    Message msg(conn);
    Browser browser(msg);
    while ((opt = getch()) != -1) {
        switch (static_cast<unsigned char>(opt)) {
        case 'q': endwin(); exit(0); break;
        case 'c': mk = true; break; // create <group/article>
        case 'l': ls = true; break; // list <group/article>
        case 'd': rm = true; break; // delete <group/article>
        case 'g': ng = true; break; // select group
        case 'a': art = true; break; // select article
        case 'r': read = true; break; // read article
        case 2: page++; draw_list(page, browser.selection()); break;
        case 3: page--; draw_list(page, browser.selection()); break;
        default: continue; // skip invalid chars
        }
        
        options_bar(mk, ls, rm, browser.have_articles(), browser.have_groups());
        status_bar(mk, ls, rm);

        // run query if we want to create/delete/remove group/article or read
        bool exec = ((art || ng) && (ls || rm || mk)) || (read);
        if (!exec) continue;

        try {
            if ((art && !browser.have_groups()) ||
                (rm && ng && !browser.have_groups()) ||
                (rm && art && !browser.have_articles()) ||
                (read && !browser.have_articles()) ||
                (read && last_group == -1)) {
                // Catch user trying to do something stupid, like creating an
                // article (must select group) when no groups have been listed.
            } else if (ls && art) {
                // list articles
                select_group(browser, last_group, page);
                int ack = browser.list_articles(last_group);
                if (ack != Protocol::ANS_ACK)
                    mvaddstr(0, 0, error_message(ack).c_str());
                else
                    draw_list(page = 0, browser.selection());
            } else if (ls && ng) {
                // list groups, invalidates old group index
                last_group = -1;
                int ack = browser.list_groups();
                if (ack != Protocol::ANS_ACK)
                    mvaddstr(0, 0, error_message(ack).c_str());
                else
                    draw_list(page = 0, browser.selection());
            } else if (mk && ng) {
                // create group
                query_input("group name> "); refresh();
                string title = readstr();

                int ok = browser.create_group(title);
                mvaddstr(0, 0, error_message(ok).c_str());
            } else if (mk && art) {
                // create article
                select_group(browser, last_group, page);
                clear_display();

                int x = 0, y = 0;
                query_input("title> "); refresh();
                string title = readstr();
                mvaddstr(y, x, title.c_str());
                getyx(stdscr, y, x);

                query_input("author> "); refresh();
                string author = readstr();
                mvaddstr(y, x, " - ");
                addstr(author.c_str());
                addstr("\n");
                getyx(stdscr, y, x);

                query_input("write content, finish with double ENTER");
                move(y + 2, x);
                string content = "";
                int oc = 0, c;
                getyx(stdscr, y, x);

                echo();// nl();
                while ((c = getch()) != -1) {
                    // exit on double enter
                    if (c == 13 && oc == 13) break;
                    // pressing enter seems to trigger carriage return
                    if (c == 13) {
                        content.push_back('\n');
                        mvaddstr(y, 0, content.c_str());
                    } else
                        content.push_back(c);
                    oc = c;
                }
                noecho();// nonl();

                clear_display();
                int ok = browser.create_article(last_group, title, author, content);
                mvaddstr(0, 0, error_message(ok).c_str());
            } else if (rm && ng) {
                // remove group, invalidates old group index
                last_group = -1;
                draw_list(page = 0, browser.newsgroups());
                query_input("group> "); refresh();
                int group_id = readint();

                clear_display();
                
                int ok = browser.delete_group(group_id);
                mvaddstr(0, 0, error_message(ok).c_str()); 
            } else if (rm && art) {
                // remove article
                select_group(browser, last_group, page);

                draw_list(page = 0, browser.news());
                query_input("article> "); refresh();
                int article_id = readint();

                clear_display();
                
                int ok = browser.delete_article(last_group, article_id);
                mvaddstr(0, 0, error_message(ok).c_str()); 
            } else if (read) {
                // read article
                if (page == 0)
                    draw_list(page, browser.news());
                query_input("article> "); refresh();
                int article_id = readint();
                
                clear_display();
                
                int ok = browser.get_article(last_group, article_id);
                if (ok != Protocol::ANS_ACK) {
                    mvaddstr(0, 0, error_message(ok).c_str()); 
                } else {
                    // title - author
                    //
                    // content
                    mvaddstr(0, 0, browser.art_title().c_str());
                    addstr(" - ");
                    addstr(browser.art_author().c_str());
                    addstr("\n\n");
                    addstr(browser.art_content().c_str());
                }
            }
            mk = ng = art = ls = rm = read = false;
            options_bar(mk, ls, rm, browser.have_articles(), browser.have_groups());
            status_bar(mk, ls, rm);
        } catch (ConnectionClosedException&) {
            error("No reply from server. Exiting.");
        }
    }
}
