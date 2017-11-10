#include <iostream>
#include <iomanip>
#include <functional>
#include <string>
#include <unordered_set>

using namespace std;

int main()
{

    /*std::string str = "Meet the new boss...";
    std::size_t str_hash = std::hash<std::string>{}(str);
    std::cout << "hash(" << str << ") = " << str_hash << '\n';*/

    float str = 1.32543543;
    int str_hash = std::hash<float>{}(str);
    std::cout << "hash(" << str << ") = " << str_hash%11 << '\n';
    /*S obj = { "Hubert", "Farnsworth"};
    std::cout << "hash(" << std::quoted(obj.first_name) << ',' 
        << std::quoted(obj.last_name) << ") = "
        << MyHash{}(obj) << " (using MyHash)\n                           or "
        << std::hash<S>{}(obj) << " (using std::hash) " << '\n';*/
}
