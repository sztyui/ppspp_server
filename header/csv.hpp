# ifndef __CSV_HPP_INCLUDED_
# define __CSV_HPP_INCLUDED_

# include <vector>
# include <fstream>
# include <sstream>
# include <string>
# include <boost/algorithm/string.hpp>

# include "server_log.hpp"

class csv {
  private:
    std::vector<std::vector<std::string> > result;
    char delimiter;
    char comment_tag;
  public:
    csv();
    csv(const std::string filename, char deli, char comm);
    ~csv();
    void writeOut() const;
    std::vector<std::vector<std::string> > get() const;
    void set(const std::string filename, char deli, char comm);
};


std::ostream& 
operator<<(std::ostream & os, const csv & c);

# endif