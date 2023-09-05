#include <iostream>
#include <fstream>
#include "TextQuery.h"
using namespace std;
int main()
{
    ifstream ifs("C:\\Users\\12949\\Desktop\\project\\Query\\test.txt", ifstream::in);
    if (ifs.is_open())
    {
        cout << "true" << endl;
        TextQuery tq(ifs);
        Query t("the"), it("it");
        Query ad = t | it;
        auto qr = ad.eval(tq);
        print(cout, qr)；
    }
}
