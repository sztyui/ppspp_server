# include "csv.hpp"

Logger * _csvlog = Logger::Instance();

  using namespace std;

/* Kiveszi a kommenteket a sorok közül. */
std::string 
clear_line_from_comments(const std::string line, char comment_tag) {
  std::string result = "";
  if(line[0] == comment_tag)
    return result;
  result = line[0];
  for(size_t i = 1; i < line.size(); i++) {
    if(line[i] == comment_tag && line[i-1] != '\\')
      return result;
    else
      result = result + line[i];
  }
  return result;
}

/* kitörli az idézőjeleket */
std::string 
remove_quotes(const std::string input) {
  std::string tmp = input;
  boost::algorithm::trim(tmp);
  if((tmp[0] == '\'' || tmp[0] == '"') && (tmp[tmp.size()-1] == '\'' || tmp[tmp.size()-1] == '"')) {
    return tmp.substr(1, tmp.size()-2);
  }
  return tmp;
}

/* Üres konstruktor. */
csv::csv(){}

/* CSV osztály konstruktora */
csv::csv(const std::string filename, char deli = ';', char comm = '\\') {   //' <- ez azert kellett ide, mert a buzi visual studio azt hiszi, hogy escapeltem a commentet.
  delimiter = deli; comment_tag = comm;
  ifstream input(filename.c_str());
  if(input) {
    std::string akt_line;
    while(getline(input, akt_line)){
      if(akt_line.size() == 0) continue;
      std::string tmp = clear_line_from_comments(akt_line, comment_tag);
      if(tmp.size() == 0) continue;
      std::vector<std::string> akt_csv_line;
      boost::split(akt_csv_line,tmp,boost::is_any_of(std::string(1,delimiter)));
      if(akt_csv_line.size() > 0) {
        std::vector<std::string> tmp_csv_line;
        for(auto field : akt_csv_line) {
          boost::algorithm::trim(field);
          if(!field.empty()){
            tmp_csv_line.push_back(remove_quotes(field));
          }
          else continue;
        }
        result.push_back(tmp_csv_line);
      }
    }
  }
}

csv::~csv(){}

void csv::writeOut() const {
  size_t number_of_line = 0;
  for(auto line : result) {
    std::cout << number_of_line++ << ":\t";
    for(auto field : line) {
      std::cout << field << "\t";
    }
    std::cout << std::endl;
  }
}

void
csv::set(const std::string filename, char deli=';', char comm='\\'){ //'
  csv a(filename, deli, comm);
  *this = a;
}

std::vector<std::vector<std::string> > 
csv::get() const {
  return this->result;
}

std::ostream& 
operator<<(std::ostream & os, const csv & c){
  size_t number_of_line = 0;
  auto result = c.get();
  for(auto line : result) {
    os << number_of_line++ << ":\t";
    for(auto field : line) {
      os << field << "\t";
    }
    os << std::endl;
  } 
  return os;
}