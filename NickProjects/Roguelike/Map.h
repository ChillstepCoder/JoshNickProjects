#pragma once
#include <iostream>
#include <string>

using namespace std;

class Map
{
public:
    Map();
    void printMap();
    void NewMap();
    void OldMap();

private:
    int _mapcount = 0;

};
