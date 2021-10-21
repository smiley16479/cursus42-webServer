#include <iostream>
#include <string>
using namespace std;

int main(int argc, char const *argv[])
{
    for (int i(0);cout << "test : " << i << endl && i <= 2; cout << "condition : " << ++i << endl) 
        cout << "ds la boucle : " << i << endl;
    return 0;
}
