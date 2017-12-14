# ifndef __classes_hpp_
# define __classes_hpp_

# include <iostream>
# include <vector>
# include <map>
# include <sstream>
# include <fstream>
# include <stdexcept> 
# include <dirent.h>
# include <boost/iostreams/device/mapped_file.hpp>
# include <boost/iostreams/stream.hpp>
# include <boost/algorithm/string.hpp>
# include <boost/filesystem.hpp>
# include <string>
# include <locale>
# include <csignal>

# include "csv.hpp"
# include "server_log.hpp"
# include "segitseg.hpp"
# include "database.hpp"

/*
 Globális változók.
*/
  
class Globals {
    public:
      std::string 
             input_directory, 
             input_extension,
             input_archive,
             output_directory,
             output_extension,
             output_archive,
             error_directory,
             config_file_name,
             opener_tag,
             closer_tag,
             qr_xml_places_config,
             posta_qr_conf,
             result_filename_xpath,
             oracle_connection_string,
             oracle_config_file,
             global_application_name = "PPS";
      csv qr_config_csv;
      OracleConfig global_ora;
    Globals(std::string argument);
    ~Globals();
    OracleConfig get_oracle_config_object();
    std::string print();
};

class File {
  /*
    Mi fog kelleni először is:
     - teljes fájlnév biztosan.
     - teljes eredmény fájlnév biztosan
     - megfelel-e az XML ellenőrzésen.
     - hanyadik darab volt az input fajlban.
  */
  private:
    std::string 
      input_filename,
      exbilldocno;
    size_t counter = 0;
    bool 
      xml_valid = false,
      qr_generated = false,
      oracle_write = false;
    size_t part;
  public:
    File(std::string i_name, size_t c, std::string exbilldocno = "");
    std::string get_input_name();
    std::string outname(const Globals gc);
    void set_xml_valid();
    void set_qr_generated();
    void set_oracle_write();
    bool oke();
    void set_exb(std::string exbilldocno);
    bool get_xml();
    bool get_qr();
    bool get_oracle();
};

 /* 
    Segítő függvények.
    Deklarálva a segítség.cpp-ben. 
 */
std::vector<std::string>
list_directory(std::string directory, std::string extension);

void
cutty(std::string inputfile, Globals gc);

# endif