#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    std::string line;

    // read the contents of the file line by line
    while (std::getline(file, line)) {
        std::stringstream line_stream(line);
        std::string cell;
        int first_value, second_value;

        // split the line into two values
        std::getline(line_stream, cell, ',');
        first_value = std::stoi(cell);
        std::getline(line_stream, cell, ',');
        second_value = std::stoi(cell);

        // print the values to the console in the desired format
        std::cout << first_value << " --> " << second_value << std::endl;
    }

    return 0;
}