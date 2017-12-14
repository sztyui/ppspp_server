# ifndef __SERVER_LOG_HPP
# define __SERVER_LOG_HPP

# include <string>
# include <iostream>
# include <fstream>
# include <ctime>
# include <sstream>
# include <iomanip>
# include <stddef.h> 
# include <iostream>
# include <string>
# include <fstream>
# include <mutex>

/**
 * @brief       streambuf for null stream
 * @author      amadeu zou
 */
class NullStreamBuffer : public std::streambuf
{
public:
	NullStreamBuffer() : _pos(0), _size(32){
		_buf= new char[_size];
	}
	~NullStreamBuffer(){
		delete [] _buf;
	}
	
protected:
	// central output function
	virtual int_type overflow(int_type c){
		if (c != EOF){
			if(_pos >= _size)
				_pos %= _size;
			if(c == '\n')
				_pos = 0; 
			else
				_buf[_pos++] = c;
		}
		return c;
	}
	
private:
	char* _buf;    
	int _pos;
	int _size;  
};

/**
 * @brief       null stream
 * @author      amadeu zou
 */
class NullStream : public std::ostream
{
public:
    NullStream() : std::ios(0), std::ostream(new NullStreamBuffer())  {}
    ~NullStream() { delete rdbuf(); }
};

enum LogLevel {
  FATAL = 0, ERROR = 1, WARNING = 2, INFO = 3, NONE = 4, DEBUG = 5
};

// log info header
const std::string LOGSTR[6] = {
		"[FATAL]", "[ERROR]", "[WARNING]", "[INFO]", "[NONE]", "[DEBUG]"
	};
  
class Logger
{
  public:
    static Logger* Instance();
    void set_file(const std::string &log_file);
    std::ostream& debug() ;
    std::ostream& info() ;
    std::ostream& warning();
    std::ostream& error();
    std::ostream& fatal();
    std::string get_time();
    void set_level(int v);
    std::ostream& hello();
    friend std::ostream& operator<<(std::ostream &os, Logger &t){
      os << *t._os;
      return os;
    }
  private:
    std::string logFileName;
    std::ostream *_os;
    time_t _time;
    LogLevel _level;
    NullStream _ns;
    int logLevel;
  protected:
    static Logger* log;
    Logger() :_os(&std::cerr), _level(DEBUG){};
    ~Logger()	{ _os = NULL;	delete log;}
    Logger(const Logger &log) :_os(log._os), _level(log._level){};
    Logger& operator=(Logger &log);
};

# endif