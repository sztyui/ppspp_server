# include <iostream>
# include <vector>
# include <sstream>
# include <mutex>
# include <pugixml.hpp>
# include <boost/algorithm/string.hpp>
# include <unistd.h>
# include <thread>
# include <locale>
# include <csignal>

# include "rsaqrgen.h"
# include "server_log.hpp"
# include "csv.hpp"
# include "segitseg.hpp"
# include "classes.hpp"
# include "database.hpp"

  using namespace std;
  
  /* A logolóra mutató pointer. Ide lehet tolni a jó kis logolásokat. */
  Logger * _mlog = Logger::Instance();
  
  int loglevel = 2, wait_delay=10;
  unsigned number_of_threads = 1;
  std::string logfile = "./log.txt";
  std::string config_filename = "";
  std::string application_name = "PPS", global_oracle_connection_string = "";
  
  /*
    Ha rosszul indul el, kiíratjuk a helpet.
  */
  void help(){
    std::cerr << "splitter: " << std::endl;
    std::cerr << "  hasznalat:";
    std::cerr << " ./splitter -conf=<config file> -lf=<logfile> -ll=<loglevel>" << std::endl;
    std::cerr << " logfile (alap) : " << logfile << std::endl;
    std::cerr << " loglevel (alap) :" << std::to_string(loglevel) << std::endl;
  }

/*
  Feldolgozási sorokat csinál az input fájlokból.
  Hasznos, hogy többszálon tudjak futtatni.
*/  
template <typename T>
std::vector<std::vector<T> > generate_queues(const std::vector<T> &f, unsigned number_of_threads) {
  std::vector<std::vector<T> > return_queue;
  std::vector<T> line;
  typename std::vector<T>::const_iterator it;
  unsigned akt_thread = 0;
  for(it = f.begin(); it != f.end(); it++){
    line.push_back(*it);
    if(akt_thread == number_of_threads - 1 || it == f.end() - 1 ) {
      return_queue.push_back(line);
      line.clear();
      akt_thread = 0;
    }
    else {
      akt_thread++;
    }
  }
  return return_queue;
}

/*
  Signal handler
*/
void
signal_handler(int signum){
  _mlog->fatal() << "Szignal erkezett: " << std::to_string(signum) << std::endl;
  unregisterMyself(global_oracle_connection_string, application_name);
  exit(signum);
}

/* 
  forky() segít daemon módban folytatni a futást.
  forky() jó fej, legyél olyan mint forky() :) :) :D 
*/  
void
forky(){
  pid_t pid, sid;
  pid = fork();
  if(pid < 0) // Nem sikerült a forkolás, ki kell lépnem.
    throw std::runtime_error((char *)("Nem sikerult leforkolni. Pid: " + std::to_string(pid)).c_str());
  if(pid > 0)
    exit(EXIT_SUCCESS);
  umask(0);
  // A logolás megvalósítását kihagyom, mert van nekem saját logolóm.
  sid = setsid();
  if(sid < 0)
    exit(EXIT_FAILURE);
  /* Kiderítjük, hogy éppen melyik mappában kellemetlenkedünk: */
  char * path = NULL;
  path = getcwd(NULL,0);
  if(path != NULL){
    if((chdir(path)) < 0){
      std::string error_path(path);
      throw std::runtime_error((char *)("Nem sikerult az alkonyvtarra valtas: " + error_path).c_str());
    }
  }
  /* Lezárom az alapvető kimeneteket, úgy sem használom már őket. */
  // close(STDIN_FILENO);
  // close(STDOUT_FILENO);
  // close(STDERR_FILENO);
  
  /* Berakom a szignál kezelést. */
  signal(SIGINT, signal_handler);
  signal(SIGHUP, signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGFPE, signal_handler);
  signal(SIGILL, signal_handler);
  signal(SIGSEGV, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGKILL, signal_handler);
  
  return;
}

/*
  Feldolgozza a beérkezett argomentumokat.
  Feljebb a globális deklarációknál ott vannak az alap értékek.
*/
void args_process(std::vector<std::string> a){
  for(auto i: a){
    std::vector<std::string> tmp;
    boost::split(tmp,i,boost::is_any_of("="));
    if(tmp.size()!=2){
      _mlog->fatal() << "Hibas parameter: " << i << std::endl;
      help();
      _mlog->fatal() << "A futas leall." << std::endl;
      exit(EXIT_FAILURE);
    }
    if(tmp[0] == "-conf"){
      config_filename = trim(tmp[1],' ');
    }
    else if(tmp[0] == "-lf"){
      logfile = trim(tmp[1], ' ');
      if(logfile != "cout")
        _mlog->set_file(logfile);
    }
    else if(tmp[0] == "-ll"){
      int tmp_loglevel = std::stoi(trim(tmp[1], ' '));
      if (tmp_loglevel > 5 || tmp_loglevel < 0){
        _mlog->warning() << "loglevel max 5! most: " << tmp_loglevel << std::endl;
        tmp_loglevel = 5;
      }
      loglevel = tmp_loglevel;
      _mlog->set_level(loglevel);
    }
    else if(tmp[0] == "-dl"){
      wait_delay = std::stoi(trim(tmp[1],' '));
    }
    else if(tmp[0] == "-t"){
      std::locale loc;
      bool digit = true;
      for(auto i : trim(tmp[1], ' ')){
        if(!std::isdigit(i, loc))
          digit = false;
      }
      if(digit){
        int tmp_not = std::stoi(trim(tmp[1], ' '));
        if(tmp_not < 0 || tmp_not > 12){
          _mlog->error() << std::to_string(tmp_not) << " darab szalat adtal meg. Ez igy nem jo. 0 es 12 koze kellene esnie." << std::endl;
        }
        else {
          number_of_threads = tmp_not;
        }
      }
    }
  }
  _mlog->debug() << "logfile: " << logfile << ", loglevel: " << loglevel << std::endl;
}

/*
  Mi fog kelleni bemenetnek?
    -> input mappa
    -> input kiterjesztés
    -> output mappa
    -> output kiterjesztés
*/
int
main(int argc, char** argv) {
  std::vector<std::string> arguments(argv + 1, argv + argc);
  if(arguments.size() < 1){
    cerr << "Nem megfelelo parameterek." << endl;
    help();
  }
  /* Itt kell leforkolni.  */
  try {
    forky();
  }
  catch (const std::runtime_error & re){
    _mlog->fatal() << "Nem sikerult a forkolas: " << re.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  /* Feldolgozási folyamat kezdődik. */
  args_process(arguments);
  try {
    Globals gc(config_filename);
    global_oracle_connection_string = gc.oracle_connection_string;
    try {
      _mlog->debug() << "Oracle connection string: " << global_oracle_connection_string << std::endl;
      if(!registerMyself(global_oracle_connection_string, application_name)){
        _mlog->fatal() << "Mar be van jegyezve a PPS LP a twd_module tablaba." << std::endl;
        raise(SIGABRT);
      }
    }
    catch (const char * e){
      _mlog->fatal() << "Hiba: registerMyself: " << e << std::endl;
      raise(SIGABRT);
    }
    /* Ennek kellene futnia a végtelen ciklusban. :) */
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    while(true){
      std::vector<std::string> a = list_directory(gc.input_directory, gc.input_extension);
      _mlog->debug() << "Input files found: " << a.size() << std::endl;
      std::vector<std::vector<std::string> > que = generate_queues(a, number_of_threads);
      unsigned int actual_row = 0;
      for(auto row: que){
        std::vector<std::thread> thread_vector;
        unsigned int actual_col = 0;
        for(auto col: row){
          _mlog->debug() << "(" << actual_row << "," << actual_col++ << ") file: " << col << std::endl;
           try {
             thread_vector.push_back(std::thread(cutty, col, gc));
           }
           catch(std::exception& e){
             /* Itt még mozgasd is majd el az error-ba a hibás állományt.*/
             _mlog->debug() << "Hiba a cut fuggvenyben: " << e.what() << std::endl;
           }
        }
        for(auto& t: thread_vector) t.join(); /* Itt kell joinolni a threadet b*meg, nem a felső ciklusban, mert össze-szarja, hogyozza magát... */
        _mlog->debug() << std::to_string(actual_row++) << ". feldolgozasi lista vege." << std::endl;
        raise(SIGABRT);
      }
      sleep(wait_delay);
    }
    /* Eddig tart a végtelen ciklus. (Ugye az is véget ér egyszer, csak kellően sok erőforrásra van szükség.)*/
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  }
  catch (const std::invalid_argument& e ){
      _mlog->fatal() << "Meg a main-ban valami hiba tortent: " << e.what() << endl;
      raise(SIGABRT);
  }
  return 0;
}