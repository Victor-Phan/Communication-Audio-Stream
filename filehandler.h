#pragma once
#include <iostream>
#include <cstring>
#include <QDebug>
#include <cstdio>
class FileHandler {
protected:
    std::string fileName;
    int readPointer = 0;
public:
    FileHandler();
    static bool saveDataToFile(std::string filePath, char *buffer, int bytesReceived);
    static bool fileExists(const std::string& name);

    FileHandler(const std::string& name) : fileName(name), readPointer(0) {}
    std::string readFile(int bytes);
    int readFile(int bytes, char * buf);
};

