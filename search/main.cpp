#include <iostream> 
#include <cstring>
#include <fstream>
#include <iomanip>

#include "search_engine.h"
#include "search_utils.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Failed to start search. Bad arguments" << std::endl;
        std::cout << "Usage: search text_file.txt" << std::endl;
        exit(-1);
    }
    SearchEngine search(argv[1], std::cout, std::cerr);
    search.init();
    std::string s;
    std::cout << std::setprecision(5);
    std::cout << std::fixed;
    while (std::getline(std::cin, s)) {
         if (!s.empty()) {
            Timer timer;
            search.processRequest(s);
            std::cout << "Processing time: " << timer.elapsed() << std::endl;
         }
    }
    return 0;
}

