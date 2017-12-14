#ifndef __DATABASE_HPP_
#define __DATABASE_HPP_
# include <occi.h>
# include <iomanip>
# include <stdlib.h>
# include <mutex>
# include <iostream>
# include <pugixml.hpp>
# include <boost/algorithm/string.hpp>
# include <ctime>
# include <sstream>

# include "server_log.hpp"
# include "csv.hpp"
# include "segitseg.hpp" 

  using namespace oracle::occi;
  using namespace std;

/*
    Oracle Connection Struktura, hogy ne kelljen szivni az allando szetvagdosassal.
*/
class OracleConnectionString {
    public:
        OracleConnectionString(string usr, string pw, string s): username(usr), password(pw), sid(s){};
        OracleConnectionString(string connection_string);
        void write();
        string username, 
               password, 
               sid;
};

/*
    Oracle Adatbazis kapcsolat OCCI-val. 
*/
class Database
{
  public:
    Database(string u, string pass, string d);
    Database(const string connection_string);
    Connection * getConnection();
    virtual ~Database();
    bool isConnected();
    bool update(const string command);
    ResultSet * select(const string command);
    string error();
  private:
    Environment *env;
    Connection *con;
    string user, password, db, error_message;
};

class OracleElement {
  public:
    string  type,
            name,
            value,
            result;
    string write();
};

/*
    Az XML fajlokhoz tartozo konfiguracios beallitasok az abbol torteno olvasashoz.
*/

class OracleConfig {
  private:
    vector<OracleElement> values;
  public:
    OracleConfig();
    OracleConfig(const string configFile);
    bool dbload(pugi::xml_document & doc, string filename, string result_filename, string connection_string);
    std::vector<OracleElement> get() const;
    void write();
};

/*
    Regisztráció és unregisztráció az adatbázisokhoz.
*/
bool unregisterMyself(std::string const & connection_string, std::string const & application_name);
bool registerMyself(std::string const & connection_string, std::string const & application_name);
std::ostream& operator<<(std::ostream & os, const OracleConfig & o);
# endif