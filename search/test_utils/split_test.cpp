#include <iostream>
#include <fstream>

#include "../search_utils.h"

int main(int argc, char *argv[]) {
    std::fstream in(argv[1], std::ios_base::in);
    std::fstream out("output.txt", std::ios_base::out);
    TextSplitter splitter(in);
    size_t count;
    splitter.getSentences(out, count);
    std::cout << count << std::endl;
    return 0;
}

