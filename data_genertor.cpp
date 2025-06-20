#include<bits/stdc++.h>
using namespace std;

int main()
{
    ofstream out(".\\sample\\special.in");
    out << "1\n";
    out << "1 1 1000\n";
    out << "500\n";
    for(int i = 1; i <= 500; i++)
        out << "0 60000 6000\n";

    for(int i = 1; i <= 500; i++)
        out << "20 ";

    out << "\n20 200";
    out.close();
}