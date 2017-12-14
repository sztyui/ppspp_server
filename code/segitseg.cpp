# include "segitseg.hpp"

#ifdef OS_WINDOWS
   // Windows backslash karakter
   char BACKSLASH = '\\';
#else
  // Linux backslash karakter
  char BACKSLASH = '/';
#endif

Logger * _hlog = Logger::Instance();

/* Jóbarántunk a párhuzamosításhoz: mutex. :) */
std::mutex mutex_qr;

std::vector<std::string>
split_at_first(std::string str, char delimiter){
  std::vector<std::string> result;
  size_t elso = str.find_first_of('=');
  if(elso == std::string::npos) return {};  
  result.push_back(str.substr(0,elso));
  result.push_back(str.substr(elso+1, str.size() - elso));
  return result;
}

std::string 
trim(std::string str, char trimmable){
    size_t first = str.find_first_not_of(trimmable);
    size_t last = str.find_last_not_of(trimmable);
    return str.substr(first, (last-first+1));
}

std::string 
generate_random_string(size_t len) {
  std::string result = "";
  std::string chars= "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  for(size_t i = 0; i < len; i++)
    result.push_back(chars[rand()%chars.size()]);
  return result;
}

bool 
replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

std::string
replace_last_of(const std::string & str, const std::string& from, const std::string& to){
  size_t place = str.find_last_of(from);
  if(place == std::string::npos)
    return str;
  std::string retval = str;
  retval.replace(place, from.length(), to);
  return retval;
}

std::string
cut(std::string str, char trimmable_right, char trimmable_left){
  size_t first = str.find_first_of(trimmable_right),
         last = str.find_last_of(trimmable_left);
  if( last - first < 2 ) return "";
  return str.substr(first+1, (last-first-1));
}

std::string 
get_node_from_pugi(pugi::xml_document & doc, char * xpath){
  pugi::xpath_node res;
  if (!strlen(xpath))
    throw std::invalid_argument("Ures XPATH erkezett.");
  try {
    res = doc.select_single_node(xpath);
  }
  catch(const pugi::xpath_exception& e) {
    std::string error(xpath);
    throw std::invalid_argument(error);
  }
  if(res)
    return res.node().text().get();
  else {
    std::string error(xpath);
    throw std::invalid_argument("Nincs ilyen xpath a fajlban: " + error);
  }
}

/*
  Bolond sztringet lowerre rakja.
*/
inline std::string
tolower(std::string s){
  std::string re = "";
  std::locale loc;
  for(size_t i = 0; i < s.size(); i ++)
    re = re + std::tolower(s[i], loc);
  return re;
}

/*
  Három féle függvény jöhet szóba:
    1, clear(*)
    2, cut(from-to)
    3, conv(from->to)
  Input értékek:
    fuggveny: fentebb
    & value : referencia egy sztringre, amiben a kiolvasott érték van.
*/
void
qr_fuggvenyek(const std::string fuggveny, std::string & value){
  std::string zarojelben = cut(fuggveny, '(',')');
  if (tolower(fuggveny).find("clear") != std::string::npos){ 
    for(unsigned int i = 0; i < zarojelben.size(); i++)
      value.erase(std::remove(value.begin(), value.end(), zarojelben[i]), value.end());
  }
  else if (tolower(fuggveny).find("cut") != std::string::npos){
    std::vector<std::string> cut_string_values;
    boost::split(cut_string_values, zarojelben, boost::is_any_of("-"));
    if(cut_string_values.size() != 2){
      _hlog->error() << "Hiba az ertekekben: (cut) " << fuggveny << std::endl;
      return;
    }
    size_t from = std::stoi(cut_string_values[0]) - 1,
           to = std::stoi(cut_string_values[1]);
    if(to <= from || value.size() <= to){
      _hlog->error() << "Hiba az ertekekben: (cut2) "<< fuggveny << "," << std::to_string(from) << "->" << std::to_string(to) << ", size: " << value.size() << ", value: " << value << std::endl;
      return;      
    }
    value = value.substr(from,to - from - 1);
  }
  else if (tolower(fuggveny).find("conv") != std::string::npos){
    std::vector<std::string> convert_values;
    boost::split(convert_values, zarojelben, boost::is_any_of("-"));
    if(convert_values.size() != 2){
      _hlog->error() << "Hiba az ertekekben: " << fuggveny << std::endl;
      return;
    }
    size_t index = 0;
    while(true){
      /* Keressuk meg hol van a value1 szubsztring. */
      index = value.find(convert_values[0], index);
      /* Kilépek, ha a sztring végére értem.*/
      if(index == std::string::npos) break;
      /* Kicserélem amit találtam. */
      value.replace(index,convert_values[0].size(), convert_values[1]);
      /* Arrébb tolom a helyzetjelzőt. */
      index += convert_values[0].size();
    }
  }
  else return;
}

/*
  Na vajon mire lesz szükség a QR generáláshoz?
    -> PUGIXML dokumentum.
    -> A konfigurációs fájl.
    -> A qrgenerator.conf fájl ahhoz a buzi postaqr generátorhoz.
*/
bool
qr_generator(pugi::xml_document & document, const csv & config_file, const std::string posta_qr_konfig_fajl){
  std::string result = "", qrplace="";
  int sor = 0;
  for(auto line: config_file.get()){
    sor ++;
    if(line.size() < 2){
      throw (char *) ("CONFIGERROR - Hibas sor a konfigban: " + std::to_string(sor)).c_str();
    }
    /* Felolvasom a konfigból a sorokat, és visszaszerzem az értékeiket. */
    if (line[0] == "CONST")
      result = result + "\"" + line[1] + "\";";
    else if (line[0] == "XPATH"){
      try {
        std::string xml_value = get_node_from_pugi(document, (char *) line[1].c_str());
        if(line.size() == 3){
          for(auto j: line) 
          qr_fuggvenyek(line[2], xml_value);
        }
        result = result + '"' + xml_value + "\";";
      }
      catch (std::invalid_argument & a){
        std::string error(a.what());
        throw (char *) ("NOQR - Nem talaltam meg ezt az erteket: " + error).c_str();
      }
    }
    else if (line[0] == "QRPLC")
      qrplace = line[1];
    else 
      throw (char *) ("CONFIGERROR - Ismeretlen erteket talaltam a configban a " + std::to_string(sor) + ". sorban: " + line[0]).c_str();
  }
  result = result.substr(0,result.size()-1);
  _hlog->debug() << "QR Input: " << result << std::endl;
  if(result.size() == 0)
    throw (char *) "QRERROR - Nem tudtam kiolvasni az ertekeket.";
  
  /* Lemutexelem a párhuzamosításhoz. */
  mutex_qr.lock();
  /* PostaQR Crypto inicializálás */
  char r[256], e[256], qrcode[2048];
  rsaQRInitCrypto( (char *) posta_qr_konfig_fajl.c_str(), r);
  if(strlen(r) != 0)
    _hlog->debug() << "r erteke: " << r << std::endl;  /* Ezt majd logold ki debugba.*/
  rsaQRGetError(e);
  if(strlen(e) != 0){
    std::string error(e);
    _hlog->error() << "Hiba a QR generator Crypto betoltesenel: " << error << std::endl;
    throw (char *) ("CONFIGERROR - Hiba a Crypto betoltesenel. Hibauzenet: " + error).c_str();
  }
  /* QR generálás. */
  memset(e,0, sizeof e);
  rsaQREncryptCsvInvoice((char *) result.c_str(), qrcode);
  rsaQRGetError(e);
  if(strlen(e) != 0){
    std::string error(e);
    _hlog->error() << "Hiba a QR kod generalasanal: " << error << ", value: " << result << std::endl;
    throw (char *) ("QRERROR - Hiba a QR generalasanal. Hibauzenet: " + error + " value: " + result).c_str();
  }
  _hlog->debug() << "QR: " << (std::string) qrcode << std::endl;
  mutex_qr.unlock();
  
  /* Az elkészült QR kódot hozzáadom a dokumentumhoz. */
  pugi::xpath_node to;
  try {
    to = document.select_single_node((char *) qrplace.c_str());
    to.node().text().set(qrcode);
  }
  catch(pugi::xpath_exception &e) {
    std::string error(e.what()); 
    throw (char *) ("QRERROR - Hiba a QR beirasanal. Hibauzenet: " + error).c_str();
  }
  
  return true;
}

/*
  Ez megmondja hogy két érték között van-e középső.
  Ahhoz kell, hogy ki tudja találni, hogy hibás az xml a cut() fügvényben.
*/
bool 
has_middle(const std::vector<size_t> &in, const size_t &smaller, const size_t &bigger) {
  for(auto element : in) {
    if(element > smaller && element < bigger) return true;
  }
  return false;
}

/* 
  Python-ból az os.path.join után szabadon.
  Fájlnévből és path-ból csinál egészet. 
*/
std::string
f_join(const std::string & folder, const std::string & file){
  return folder[folder.size()-1] == BACKSLASH ? folder + file : folder + BACKSLASH + file; 
}

/* Ez megnezi a merevlemezen, hogy a szoban forgo fajl letezik-e. */
bool 
f_exists(const std::string & name){
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}