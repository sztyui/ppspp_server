# ifndef __segitseg_hpp_
# define __segitseg_hpp_

# include <iostream>
# include <string>
# include <string.h>
# include <vector>
# include <pugixml.hpp>
# include <mutex>
#include <sys/stat.h>
# include <boost/algorithm/string.hpp>

# include "csv.hpp"
# include "rsaqrgen.h"
# include "server_log.hpp"



/* 
  Ez azért kellett mert a buzi xpath-ra többrészre szplitteli a konfigfájlt,
  az meg nekem nem annyira jó.
  Lényeg: megkeresi az első delimitert és ott vágja el. 
*/
std::vector<std::string>
split_at_first(std::string str, char delimiter);

/* Trimming a string from character: */
std::string 
trim(std::string str, char trimmable);

/* Random sztring generátor, mert nem találtam egy normálisat a neten. */
std::string 
generate_random_string(size_t len);

/* Szubsztring csere. */
bool 
replace(std::string& str, const std::string& from, const std::string& to);

/* Kicseréle az input from értékének utolsó előfordulását. */
std::string
replace_last_of(const std::string & str, const std::string& from, const std::string& to);

/* Megkeresi jobbról - balról az első trimmable-t és a közte lévő értéket adja vissza. */
std::string
cut(std::string str, char trimmable_right, char trimmable_left);

/*
  Kiszedi a pugixml dokumentumból ami köll.
  Nem szaroz...
*/
std::string 
get_node_from_pugi(pugi::xml_document & doc, char * xpath);

/*
  Bolond sztringet lowerre rakja.
*/ 
std::string
tolower(std::string s);

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
qr_fuggvenyek(const std::string fuggveny, std::string & value);

/*
  Na vajon mire lesz szükség a QR generáláshoz?
    -> PUGIXML dokumentum.
    -> A konfigurációs fájl.
    -> A qrgenerator.conf fájl ahhoz a buzi postaqr generátorhoz.
*/
bool
qr_generator(pugi::xml_document & document, const csv & config_file, const std::string posta_qr_konfig_fajl);

/*
  Ez megmondja hogy két érték között van-e középső.
  Ahhoz kell, hogy ki tudja találni, hogy hibás az xml a cut() fügvényben.
*/
bool 
has_middle(const std::vector<size_t> &in, const size_t &smaller, const size_t &bigger);

/* 
  Python-ból az os.path.join után szabadon.
  Fájlnévből és path-ból csinál egészet. 
*/
std::string
f_join(const std::string & folder, const std::string & file);

/* Ez megnezi a merevlemezen, hogy a szoban fogo fajl letezik-e. */
bool 
f_exists(const std::string & file);

# endif