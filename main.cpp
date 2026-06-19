#include <iostream>
#include <filesystem>


int main(int, char**){
    std::cout << "Hello, from lightSim!\n";
    std::cout << std::filesystem::current_path() << std::endl;
}
