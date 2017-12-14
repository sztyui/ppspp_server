# include "database.hpp"

  using namespace oracle::occi;
  using namespace std;

Logger * database_log = Logger::Instance();
std::mutex ora_m;

bool registerMyself(std::string const & connection_string, std::string const & application_name) {
  OracleConnectionString ora(connection_string);
  Database pData(ora.username, ora.password, ora.sid);
  ResultSet *rs = pData.select("select fut from twd_module where module_id = '" + application_name + "'");
  rs->next();
  if(rs->getString(1) == "1") {
    return false;
  }
  else {
    rs = pData.select("select count(module_id) from twd_module where module_id = '" + application_name + "'");
    rs->next();
    if(atoi(rs->getString(1).c_str()) == 0) {
      std::stringstream ss; ss << (int) getpid();
      if(pData.update("insert into twd_module values ('" + application_name+"'," + ss.str() + ", 1)")) {
        return true;
      }
      else 
        return false;
    }
    else {
      std::stringstream ss; ss << (int) getpid();
      if(pData.update("update twd_module set fut = 1, pid = '" + ss.str() + "' where module_id = '" + application_name + "'")) {
        return true;
      }
      else 
        return false;
    }
  }
  return true;
}

bool unregisterMyself(std::string const & connection_string, std::string const & application_name) {
  OracleConnectionString ora(connection_string);
  Database pData(ora.username, ora.password, ora.sid);
  ResultSet *rs = pData.select("select fut from twd_module where module_id = '" + application_name + "'");
  rs->next();
  if(rs->getString(1) == "1") {
    if (pData.update("update twd_module set fut = 0, pid = 0 where module_id = '" + application_name + "'")) {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
  return true;
}
  
/*
  Constructor :)
*/

Database::Database(const string connection_string) {
  try {
    OracleConnectionString ora(connection_string);
    env = Environment::createEnvironment(Environment::DEFAULT);
    try
    {
      ora_m.lock();
      con = env->createConnection(ora.username, ora.password, ora.sid);
    }
    catch (SQLException& ex)
    {
      ora_m.unlock();
      env = NULL;
      database_log->error() << "Hiba a kapcsolodaskor" << std::endl; 
      database_log->error() << "Hibauzenet: " << ex.getMessage();
      throw (char *) ("CONNECTIONERROR - " + ex.getMessage()).c_str();
    } 
  }
  catch(const char * e){
    database_log->error() << e << std::endl;
    throw e; 
  }
}

Database::Database(std::string u,std::string pass, std::string d){
  if(u.empty() || pass.empty() || d.empty()) {
    env = NULL;
    error_message = "CONFIGERROR - Valamelyik parameter ures :( (username, password, database) :" + u + "," + pass + "," + d;
    throw (char *) error_message.c_str();
  }
  user = u; 
  password = pass; 
  db = d;
  env = Environment::createEnvironment(Environment::DEFAULT);
  try {
    ora_m.lock();
    con = env->createConnection(user, password, db);
  }
  catch (SQLException& ex){
    ora_m.unlock();
    env = NULL;
    error_message = ex.getMessage();
    throw (char *) ex.getMessage().c_str();
  }
}

/*
  Destructor :(
*/
Database::~Database()
{
  if(env != NULL) {
    ora_m.unlock();
    env->terminateConnection(con);
    Environment::terminateEnvironment(env);
  }
  else {
    delete env;
  }
}

bool Database::isConnected() {
  if(env == NULL) {
    return false;
  }
  return true;
}

Connection * Database::getConnection() {
  if(con != NULL) {
    return con;
  }
  return NULL;
}

std::string 
Database::error() {
  std::string tmp = error_message;
  if(!error_message.empty()){
    error_message = "";
  }
  return tmp;
}

bool Database::update(const std::string command) {
  try {
    Statement *stmt = con->createStatement(command);
    stmt->executeQuery();
    con->commit();
    delete stmt;
  }
  catch (SQLException& ex) {
    std::string error_message = "ORAERROR - Adatbazis iras hiba ::update kozben: " + ex.getMessage() + ", input: " + command;
    throw (char *) error_message.c_str();
  }  
  return true;
}

ResultSet * 
Database::select(const std::string command) {
 ResultSet * rs = NULL;
 try {
  Statement * stmt = con->createStatement(command);
  rs = stmt->executeQuery();
  return rs;
 } 
 catch (SQLException& ex) {
  std::string error_message = "ORAERROR - Adatbazis iras hiba ::update kozben: " + ex.getMessage() + ", input: " + command;
  throw (char *) error_message.c_str();
 }
 return NULL;
}


namespace database {
  /* Megmondja egy elemről, hogy üres-e? */
  struct is_empty
  {
    bool operator()(const std::string& s){
      return s.empty();
    }  
  };
}

/*
  ORACLE CONFIG
*/
OracleConfig::OracleConfig(){} /* Ez nem csinál semmit, csak létrehoz egy üres objektumot, 
                                  amit majd egyenlővé teszek a Globals konstruktorában létrehozottal.
                                  */
OracleConfig::OracleConfig(const std::string configFile) {
  csv config(configFile, ':', '#'); //'
  std::vector<std::vector<std::string> > c = config.get();
  for(auto line : c) {
    if(line.size()==0) continue;
    if(line.size() != 2){
      database_log->error() << "Rossz sort talaltam: (" << configFile << ") :" << line.size() << std::endl;
      for(auto element: line) database_log->error() << element << ",";
      database_log->error() << std::endl;
      continue;
    }
    OracleElement ora_tmp;
    std::vector<std::string> line_tmp;
    boost::split(line_tmp, line[0], boost::is_any_of(" "));
    if(line_tmp.size() != 2) {
      database_log->error() << "Hiba az egyenlosegjel elott a sorban." << std::endl;
      throw (char *) ("CONFIGERROR - Hiba az egyenlosegjel elotti sorban: " + line_tmp[0]).c_str();
      continue;
    }
    ora_tmp.type = line_tmp[0];
    ora_tmp.name = line_tmp[1];
    ora_tmp.value = line[1];
    values.push_back(ora_tmp);
  }
}

std::string 
fullpath(std::string in, size_t place, char character){
  return in;
}

std::string 
filename(std::string in, size_t place, char character){
  std::vector<std::string> parts;
  boost::split(parts, in, boost::is_any_of(std::string(1,character)));
  if(parts.size() < place) {
    return "";
  }
  if(place == 0) place = 1; 
  return parts[place-1];
}

std::vector<std::string (*)(std::string, size_t, char)> function_vectors = {fullpath,filename};
std::vector<std::string> function_names = {"fullpath(", "filename("};

std::string 
oracle_config_functions(std::string function, std::string input_filename, std::string result_filename) {
  boost::to_lower(function);
  size_t place = 0;
  for(auto funcname : function_names) {
    if(function.find(funcname) != string::npos) {
      size_t 
        bal = function.find_first_of('('), 
        jobb = function.find_last_of(')');
      std::string v1 = function.substr(bal+1, jobb-bal-1); 
      std::vector<std::string> v_e;
      boost::split(v_e, v1,boost::is_any_of(","));
      size_t elso = 0; char masodik = ' ';
      if(v_e.size() == 2) {
        elso = atoi(v_e[1].c_str());
        masodik = v_e[0][0];
      }
      if(place == 0){
        return result_filename;
      }
      else return function_vectors[place](input_filename,elso,masodik);
    }
    place++;
  }
  return "";
}

bool 
OracleConfig::dbload(pugi::xml_document & doc, std::string result_filename, std::string input_filename, std::string connection_string){
  std::vector<OracleElement> tmp_result;
  for(auto line: values){
    boost::to_lower(line.type);
    OracleElement tmp_element;
    if(line.type == "xpath"){
      tmp_element.type = line.type;
      tmp_element.name = line.name;
      tmp_element.value = line.value;
      tmp_element.result = get_node_from_pugi(doc, (char *) (line.value).c_str());
    }
    else if(line.type == "const") {
      tmp_element.type = line.type;
      tmp_element.name = line.name;
      tmp_element.value = line.value;
      tmp_element.result = line.value;
    }
    else if(line.type == "func") {
      tmp_element.type = line.type;
      tmp_element.name = line.name;
      tmp_element.value = line.value;
      tmp_element.result = oracle_config_functions(line.value, input_filename, result_filename);
    }
    else {
      database_log->error() << "Ismeretlen ertek a konfigban: " << line.type << std::endl;
      throw (char *) ("CONFIGERROR - Ismeretlen ertek: " + line.type).c_str();
    }
    tmp_result.push_back(tmp_element);
  }
  values = tmp_result;
  std::string oracle_insert = "INSERT INTO tbl_ppsw (EXBILLDOCNO, SYS_ID, MANDT_ID, SAP_DEVICE_NAME, DOC_TYPE, SAP_SPOOL_ID, DOCUMENT_STATUS, TIMESTAMP_ACTION, TMP_FILEPATH) VALUES (";
  for(auto line: values) {
    boost::to_lower(line.type);
    if(line.result.empty()) {
      database_log->error() << "Ures <result> erteket talaltam: " << line.write() << std::endl;
    }
    if(line.type == "xpath" || line.type == "func")
      oracle_insert = oracle_insert + "'" + line.result + "',";
    else if(line.type == "const")
      oracle_insert = oracle_insert + line.result + ",";
    else
      database_log->error() << "Ilyen ertek tipust nem ismerek:" << line.type << std::endl;
  }
  if(oracle_insert[oracle_insert.size() - 1] == ',') oracle_insert = oracle_insert.substr(0,oracle_insert.size()-1);
  oracle_insert = oracle_insert+")";
  Database loaddb(connection_string);
  if(!loaddb.update(oracle_insert)) {
    throw (char *) ("ORAERROR - Hiba adodott a beirasban: " + oracle_insert).c_str();
    return false;
  }
return true;
}

std::vector<OracleElement> 
OracleConfig::get() const {
  return this->values;
}

std::ostream&
operator<<(std::ostream & os, const OracleConfig & o){
  os << "Oracle konfig ertekek:" << std::endl;
  for (auto line : o.get()){
    os << "\ttype: " << line.type << ", name: " << line.name << ", value: " << line.value << ", result: " << line.result << std::endl;
  }
  return os;
}

std::string 
OracleElement::write(){
  return "type: " + type + ", name: " + name + ", value: " + value + ", result: " + result;
}

/*
    Oracle Connection String class implementation:
*/
OracleConnectionString::OracleConnectionString(std::string connection_string){
    std::vector<std::string> first;
    boost::split(first,connection_string,boost::is_any_of("/@"));
    if(first.size() != 3){
      database_log->error() << "Hiba a kapcsolodaskor. Nem megfelelo kapcsolati sztring parameter jott: " << connection_string << std::endl;
      throw (char *) ("CONNECTIONERROR - String: " + connection_string).c_str();
    }
    username = first[0];
    password = first[1];
    sid = first[2];
}

void 
OracleConnectionString::write() {
    std::cout << "Username: " << username << ", Password: " << password << ", SID: " << sid << std::endl;
}
