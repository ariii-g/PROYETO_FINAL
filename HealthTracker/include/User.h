#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

/**
 * @class User
 * @brief Crea el usuario y valida que sea existente
 */

class User {
public:
    int id{ -1 };
    string username;
    string password; 
    string salt;

    User();
    User(int id_, const  string& user, const  string& pass, const  string& salt_);
};

#endif
