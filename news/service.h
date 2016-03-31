#include "connection.h"
#include "database.h"
#include "message.h"

class Service {
public:
    Service();

    void list_ng(Message& msg);
    void create_ng(Message& msg);
    void delete_ng(Message& msg);

    void list_art(Message& msg);
    void create_art(Message& msg);
    void delete_art(Message& msg);
    void get_art(Message& msg);

    void process(std::shared_ptr<Connection>& conn);

private:
    Database* db;
};