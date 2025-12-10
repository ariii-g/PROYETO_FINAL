#include "User.h"

User::User() : id(-1) {}

User::User(int id_, const  string& user, const  string& pass, const  string& salt_)
    : id(id_), username(user), password(pass), salt(salt_) {}
