# include <iostream>
# include <thread>
# include <vector>

# include "classes.hpp"

static const int number_of_threads = 4;

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

void call_from_thread(std::string filename){
  std::cout << "\tHello thread: "<< filename << std::endl;
}

int main(){
  unsigned int cycle_meter = 100000;
  std::vector<std::string> input_files;
  std::string tmp_filename;
  while(getline(std::cin, tmp_filename))
    input_files.push_back(tmp_filename);
  std::vector<std::vector<std::string> > que = generate_queues(input_files, number_of_threads);
  unsigned int akt_row = 0;
  for(auto row: que){
    std::cerr << "row: " << std::to_string(akt_row++) << std::endl;
    std::vector<std::thread> thread_vector;
    unsigned int akt_col = 0;
    for(auto col: row){
      thread_vector.push_back(std::thread(cut, col, gc));
      std::cerr << "\tcol: " << std::to_string(akt_col++) << ", value: " << col << std::endl;
    }
    
    for(std::vector<std::thread>::iterator it = thread_vector.begin(); it != thread_vector.end(); it++)
      (*it).join();
  }
  
  return 0;
}

