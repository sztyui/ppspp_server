# include "classes.hpp"

#ifdef OS_WINDOWS
   // Windows backslash karakter
   char BACKSLASH2 = '\\';
#else
  // Linux backslash karakter
  char BACKSLASH2 = '/';
#endif

/* Class Global variables: */
Logger * _clog = Logger::Instance();

Globals::Globals(std::string argument){
  if(argument.size() == 0) throw std::invalid_argument("Hibas argomentum: " + argument); // Nem sikerült meghatározni a fájl nevét.
  this->config_file_name = argument;
  std::ifstream ifs(this->config_file_name.c_str());
  if(ifs.fail()) throw std::invalid_argument("Nincs ilyen fajl: '" + argument + "'"); // Nem sikerült beolvasni a fájlt.
  std::string parameter="", value="", tmp="";
  
  /* function_map deklarációk */
  
  while(getline(ifs,tmp))
  {
    if(tmp.size() == 0) continue;
    std::string line = trim(tmp,' ');
    if(line[0] == '#') continue;
    std::vector<std::string> values = split_at_first(line, '=');
    if(values.size() != 2) throw std::invalid_argument("Hibas argomentum: (valuesize) " + tmp);
    parameter = trim(values[0],' ');
    value = trim(values[1],' ');
    if(parameter == "input_directory"){
      this->input_directory = trim(value,'"');
    }
    else if(parameter == "input_extension"){
      this->input_extension = trim(value,'"');
    }
    else if(parameter == "output_directory"){
      this->output_directory = trim(value,'"');
    }
    else if (parameter == "output_extension"){
      this->output_extension = trim(value,'"');
    }
    else if (parameter == "opener_string"){
      this->opener_tag = trim(value,'"');
    }
    else if (parameter == "closer_string"){
      this->closer_tag = trim(value,'"');
    }
    else if (parameter == "input_archive"){
      this->input_archive = trim(value,'"');
    }
    else if (parameter == "output_archive"){
      this->output_archive = trim(value,'"');
    }
    else if (parameter == "error_directory"){
      this->error_directory = trim(value,'"');
    }
    else if (parameter == "qr_xml_places"){
      this->qr_xml_places_config = trim(value, '"');
      qr_config_csv.set(this->qr_xml_places_config,';','#');
      _clog->debug() << qr_config_csv << std::endl;
    }
    else if (parameter == "posta_qr_conf"){
      this->posta_qr_conf == trim(value, '"');
    }
    else if (parameter == "filename"){
      this->result_filename_xpath = trim(value,'"');
    }
    else if (parameter == "connection_string"){
      this->oracle_connection_string = trim(value,'"');
    }
    else if (parameter == "oracle_config"){
      this->oracle_config_file = trim(value, '"');
      this->global_ora = OracleConfig(this->oracle_config_file);
    }
    else {
      throw std::invalid_argument("Hibas argomentum: " + tmp);
    }
  }
}
 
Globals::~Globals(){}

std::string Globals::print() {
  return "Input dir: " + input_directory + ", Output dir: " + output_directory + ", Input extension: " + input_extension + ", Output extension: " + output_extension + ", Opener: " + opener_tag + ", Closer: " + closer_tag; 
}

OracleConfig 
Globals::get_oracle_config_object(){
  return this->global_ora;
}

std::string File::outname(const Globals gc) {
  std::string output_filename;
  if(!exbilldocno.empty())
    output_filename = { gc.output_extension[0] == '.' ? exbilldocno + gc.output_extension : exbilldocno + "." + gc.output_extension };
  else {
    std::string tmp = input_filename;
    output_filename = replace(tmp, gc.input_extension, "") ;
    output_filename = { gc.output_extension[0] == '.' ? output_filename + "_" + std::to_string(counter) + gc.output_extension : output_filename + "_" + std::to_string(counter) + "." + gc.output_extension };
  }
  return output_filename;
}

void File::set_xml_valid(){ 
  this->xml_valid = ! this->xml_valid;
}

void File::set_qr_generated(){
  this->qr_generated = ! this->qr_generated;
}

void File::set_oracle_write(){
  this->oracle_write = ! this->oracle_write;
}

bool File::oke(){
  return xml_valid && qr_generated && oracle_write;
}

void File::set_exb(std::string exbilldocno){
  this->exbilldocno = exbilldocno;
}

bool File::get_xml(){
  return this->xml_valid;
}

bool File::get_qr(){
  return this->qr_generated;
}

bool File::get_oracle(){
  return this->oracle_write;
}

File::File(std::string i_name, size_t c, std::string exbilldocno){
  this->input_filename = i_name;
  this->exbilldocno = exbilldocno;
  this->counter = c;
}

std::string File::get_input_name() { return input_filename; }
    
std::vector<std::string>
list_directory(std::string directory, std::string extension){
  std::vector<std::string> return_vector;
  DIR *dir;
  struct dirent *ent;
  if((dir = opendir(directory.c_str())) != NULL){
    while((ent = readdir(dir)) != NULL){
      std::string s(ent->d_name);
      if(s.size() < extension.size()) continue; 
      if(s.substr(s.size() - extension.size(), s.size()-1) == extension) 
        return_vector.push_back({directory[directory.size()-1] == BACKSLASH2 ? directory + s : directory + BACKSLASH2 + s});
    }
    closedir(dir);
  }
  return return_vector;
}

template<typename T>
std::vector<size_t> 
find_occurences(const T& line, const T& occurence) {
  std::vector<size_t> occurences_vector;
  size_t actual_position = line.find(occurence);
  while(actual_position != std::string::npos) {
    occurences_vector.push_back(actual_position);
    actual_position = line.find(occurence, actual_position + 1);
  }
  return occurences_vector;
}

void 
cutty(std::string inputfile, Globals gc){
  boost::iostreams::mapped_file_source file(inputfile.c_str());
  boost::iostreams::stream<boost::iostreams::mapped_file_source> inSource(file, std::ios::binary);
  std::stringstream input;
  std::string temple;
  while(getline(inSource, temple)) 
    input << temple;
  while(getline(input, temple)){
    std::vector<size_t> opener_occurences = find_occurences(temple, gc.opener_tag);
    std::vector<size_t> closer_occurences = find_occurences(temple, gc.closer_tag);
    if(opener_occurences.size() == closer_occurences.size()) {}
    else if(opener_occurences.size() > closer_occurences.size()) {
      for(size_t i = 0; i < opener_occurences.size() - 1; i++) {
        if(!has_middle(closer_occurences, opener_occurences[i], opener_occurences[i+1])) {
          size_t salala;
          (opener_occurences[i+1] - 1 < 0) ? salala = 0 : salala = opener_occurences[i+1] -1;
          closer_occurences.insert(closer_occurences.begin() + i,salala);
        }
        else
          continue;
      }
    }
    else if(closer_occurences.size() > opener_occurences.size()) {
      for(size_t i = 1; i != closer_occurences.size(); i++) {
        if(!has_middle(opener_occurences, closer_occurences[i-1], closer_occurences[i])) {
        size_t salala;
        (closer_occurences[i-1] + 1 > temple.length()) ? salala = temple.length() : salala = closer_occurences[i-1] + 1;
          opener_occurences.insert(opener_occurences.begin() + i,salala);
        }
        else continue;
      }
    }
    else {
      throw std::out_of_range("Opener Occurences hiba: " + inputfile);
    }
    /* Part 2 */
    if(opener_occurences.size() == 0 || closer_occurences.size() == 0) {
      throw std::out_of_range("Zero occurences hiba: " + inputfile);
    }
    /* 
      Itt kezdődik el a dokumentumok QR-kód generálása és az ORACLE írás is. 
      Úgy vigyázz...
    */
    for(size_t fasz = 0; fasz < opener_occurences.size(); fasz++) { /* Ne is kérdezd, valamiért ez a változó név jutott eszembe, és lusta voltam átírni. */
      std::stringstream file;
      file << temple.substr(opener_occurences[fasz], closer_occurences[fasz] - opener_occurences[fasz] + gc.closer_tag.length()); /* Kiveszem a streamből a fájlt. */
      File file_parameters(inputfile,fasz);
      /* XML dokumentum betöltése a memóriába. */
      pugi::xml_document doc;
      pugi::xml_parse_result xml_parse_result = doc.load(file);
      if(! xml_parse_result){
        _clog->error() << "Nem tudtam megnyitni a fajlt: " << inputfile << ", hely: " << std::to_string(fasz) << ", oka: " << xml_parse_result.description() <<std::endl;
        /* Logold ki, hogy nem tudtad megnyitni a fájlt, aztán tedd ki az error-ba.*/
        std::ofstream of (f_join(gc.error_directory, replace_last_of(file_parameters.outname(gc),".","_" + generate_random_string(5) + "_xmlerror.")));
        of << file << std::endl;
        of.close();
        continue; /* Itt ugrunk egyet, úgy sem tudunk mit kezdeni egy nem valid xml-el. */
      }
      file_parameters.set_xml_valid();
      /* Ha sikerült megnyitni, kiolvasom az exbilldocno-t, mint egyedi fájlnevet. */
      if(gc.result_filename_xpath.size() != 0)
        file_parameters.set_exb(get_node_from_pugi(doc,(char *) gc.result_filename_xpath.c_str()));
      
      /* ----------------------------------------------------------------------------------------------------------------------------------*/
      /* QR Generálás. */
      try{
        /* Ha a try végigfut, a QR kód generálás sikeres. */
        qr_generator(doc, gc.qr_config_csv, gc.posta_qr_conf);
        _clog->debug() << "Fajlnev (sikeres QR): " << file_parameters.outname(gc) << std::endl;
        //doc.save_file((char *) (f_join(gc.output_directory, file_parameters.outname(gc))).c_str(), "\t"); /* ??? */
        file_parameters.set_qr_generated();
      }
      catch (const char* msg){
        std::string error_code(msg);
        if(error_code.find("CONFIGERROR") != std::string::npos){
          /* A QR konfigban van valami hiba, abbahagyjuk a futást. */
          _clog->error() << "Kivetel ekezett: " << error_code << std::endl;
          /* És végül a program kilép. */
          _clog->error() << "A program most kilep." << std::endl;
          /* A signal_handler majd kiírja magát az adatbázisból. */
          raise(SIGABRT);
        }
        else if (error_code.find("NOQR") != std::string::npos){
          /* Ez a fájl bizony nem QR kódos. */
          _clog->debug() << "A fajl nem qr-es: " << f_join(gc.output_directory, file_parameters.outname(gc)) << std::endl;
          _clog->debug() << "Uzenet: " << error_code << std::endl;
          //doc.save_file((char *) (f_join(gc.output_directory, file_parameters.outname(gc))).c_str(), "\t");
          file_parameters.set_qr_generated();
        }
        else if (error_code.find("QRERROR") != std::string::npos){
          /* Hiba történt a QR kód generálása közben, így error-ba tesszük. */
          _clog->error() << "Fajlnev: " << replace_last_of(file_parameters.outname(gc),".", "_error.") << std::endl;
          _clog->error() << "Uzenet: " << error_code << std::endl;
          doc.save_file((char *) (f_join(gc.error_directory, replace_last_of(file_parameters.outname(gc),".","_" + generate_random_string(5) + "_qrerror."))).c_str(), "\t");
          continue;
        }
        else {
          _clog->error() << "Ismeretlen kivétel érkezett QR generáláskor: " << error_code << std::endl;
        }
      }
      //* ----------------------------------------------------------------------------------------------------------------------------------*/      
      /*  
          Ellenőriznem kell, hogy a fájl objektum QR értéke megfelelő-e.
          Ha CONFIGERROR volt akkor már úgy is leállt az egész, úgyhogy mindegy mi van. 
      */

      OracleConfig dbora = gc.get_oracle_config_object();
      //OracleConfig dbora(gc.oracle_config_file);
      try {
        _clog->debug() << "Oracle eredmeny: " << dbora.dbload(doc, f_join(gc.output_directory, file_parameters.outname(gc)), inputfile, gc.oracle_connection_string) << std::endl; //f_join(gc.output_directory, file_parameters.outname(gc))
        file_parameters.set_oracle_write();
      }
      catch (const char * e){
        std::string error_code(e);
        if(error_code.find("CONFIGERROR") != std::string::npos){
          _clog->fatal() << "Adatbazis konfiguracios hiba: " << error_code << std::endl;
          /* El ne felejtsd unregisztrálni magadat. */
          raise(SIGABRT);
        }
        else if (error_code.find("ORAERROR") != std::string::npos){
          _clog->error() << "Hiba az adatbazis ertekek olvasasa kozben: " << error_code << std::endl;          
          doc.save_file((char *) (f_join(gc.error_directory, replace_last_of(file_parameters.outname(gc),".","_" + generate_random_string(5) + "_oraerror."))).c_str(), "\t");
        }
        else {
          _clog->error() << "Ismeretlen hiba: " << error_code << std::endl;
        }
        continue;
      }

      _clog->debug() << "Mentés veszi kezdetét. nev: " << file_parameters.outname(gc) << " xml: " << file_parameters.get_xml() << ", qr: " << file_parameters.get_qr() << ", oracle: " << file_parameters.get_oracle() << std::endl;
      /* Ha minden sikeres, lementem a fajlt a merevlemezre. */
      if(file_parameters.get_xml() && file_parameters.get_qr() && file_parameters.get_oracle()){
        std::string tmp_last_save_file_name = f_join(gc.output_directory, file_parameters.outname(gc)); 
        try {
          if(!f_exists(tmp_last_save_file_name)) 
            doc.save_file((char *) tmp_last_save_file_name.c_str(), "\t");
          else {
            replace_last_of(tmp_last_save_file_name,".","_" + generate_random_string(5) + ".");
            doc.save_file((char *) tmp_last_save_file_name.c_str(), "\t");
          }
        }
        catch (const std::exception & e){
          _clog->error() << e.what() << std::endl;
          continue;
        }
      }
      else {
        _clog->debug() << "Valamelyik ertek hibas lett: (xml,qr,oracle)" << file_parameters.get_xml() << ", " << file_parameters.get_qr() << ", " << file_parameters.get_oracle() << std::endl;
        continue;
      }
    } // a for ciklus vége.
  } // A stream felolvasó while vége.
  return;
}