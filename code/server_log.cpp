#include "server_log.hpp"

Logger* Logger::log = NULL;

Logger* Logger::Instance(){
  if(!log)
    log = new Logger();
  return log;
}
  
Logger& Logger::operator=(Logger &log){
		if ( this != &log )
		{
			this->_os = log._os;
			this->_level = log._level;
		}
		return *this;
	}

void Logger::set_file(const std::string &log_file){
		_os = new std::ofstream(log_file.c_str(), std::ios_base::app);
		if(NULL == _os)
			_os = &std::cerr;
    logFileName = log_file;
	}

std::ostream& Logger::debug(){
		if(_level < DEBUG)
			return _ns;
		*_os<<get_time()<<LOGSTR[DEBUG] << " ";
		return *_os;
	}
  
std::ostream& Logger::info(){
		if(_level < INFO)
			return _ns;
		*_os<<get_time()<<LOGSTR[INFO] << " ";
		return *_os;
	}
  
std::ostream& Logger::warning(){
		if(_level < WARNING)
			return _ns;
		*_os<<get_time()<<LOGSTR[WARNING]<< " ";
		return *_os;	
	}
  
std::ostream& Logger::error(){
		if(_level < ERROR)
			return _ns;
		*_os<<get_time()<<LOGSTR[ERROR]<< " ";
		return *_os;
	}
  
std::ostream& Logger::fatal(){
		if(_level < FATAL)
			return _ns;
		*_os<<get_time()<<LOGSTR[FATAL] << " ";
		return *_os;
	}
  
std::string Logger::get_time(){
  time_t rawtime; struct tm * timeinfo;
  time(&rawtime); timeinfo = localtime(&rawtime);
  char buffer[80]; strftime(buffer, 80, "%Y.%m.%d %H:%M:%S ", timeinfo);
  return std::string(buffer);
	}
  
void Logger::set_level(int v){
    if(v>5) v=5;
		_level = static_cast<LogLevel>(v);
    logLevel = v;
	}