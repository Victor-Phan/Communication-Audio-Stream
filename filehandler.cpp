/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: 	filehandler.cpp - Contains all logic relating to file IO.
--
--
-- PROGRAM: 		Communication Audio Program
--
-- FUNCTIONS:
--                  bool fileExists(const std::string& name)
--                  std::string readFile(int bytes)
--                  int readFile(int bytes, char * buf)
--                  bool saveDataToFile(std::string filePath, char *buffer, int numBytes)
--
-- DATE: 			March 20, 2020
--
-- REVISIONS:
--
-- DESIGNER: 		Victor Phan
--
-- PROGRAMMER: 		Victor Phan
--
-- NOTES:
--                  FileHandler class pertains logic such as writing to a file of any type, reading from
-                   any file, and checking if a file exists.
--------------------------------------------------------------------------------------------------------------------*/
#include "filehandler.h"

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	saveDataToFile
--
-- DATE:		March 18, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	bool saveDataToFile(std::string filePath, char *buffer, int numBytes)
--                                  filePath - path to file
--                                  buffer - data to be written to the file
--                                  numBytes - number of bytes to append to file
--
--
-- RETURNS:     Return true if writing to file is successful
--
-- NOTES:
--              This function will open the file and write the character buffer to it.
--
-------------------------------------------------------------------------------------------------------------------*/
bool FileHandler::saveDataToFile(std::string filePath, char *buffer, int numBytes) {
    FILE* fp = fopen(filePath.c_str(), "ab+");
    if(fp != NULL) {
        fwrite(buffer, 1, numBytes, fp);
        fclose(fp);
        return true;
    } else {
        return false;
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	fileExists
--
-- DATE:		March 18, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	bool fileExists(const std::string& name)
--                  name - the filename path that is checked for existing
--
-- RETURNS:     Returns true if the file exists.
--
-- NOTES:
--              Opens the file and closes it in order to check if the specified file exists.
--
-------------------------------------------------------------------------------------------------------------------*/
bool FileHandler::fileExists(const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "rb")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	readFile
--
-- DATE:		March 18, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	std::string readFile(int bytes)
--                  bytes - the number of bytes to rad from the file
--
-- RETURNS:     Returns the data that was read. Returns empty string if no data was read.
--
-- NOTES:
--              Opens the file and reads bytes bytes from the file. Then closes the file
--
-------------------------------------------------------------------------------------------------------------------*/
std::string FileHandler::readFile(int bytes) {
    char buffer[bytes];
    std::string data;
    int startFilePointer = readPointer;
    memset(buffer,0,bytes);
    FILE *file = fopen(fileName.c_str(), "rb");
    fseek(file,readPointer,SEEK_SET);
    if(!feof(file)) {
        readPointer += fread(buffer, 1, sizeof(buffer), file);
        data.append(buffer,bytes);
    }

    fclose(file);
    if(startFilePointer == readPointer) {
        return "";
    }
    return data;
}

/*-----------------------------------------------------------------------------------------------------------------
-- Function:	readFile
--
-- DATE:		March 18, 2020
--
-- REVISIONS:
--
-- DESIGNER: 	Victor Phan
--
-- PROGRAMMER: 	Victor Phan
--
-- INTERFACE:	std::string readFile(int bytes, char * buf)
--                  bytes - the number of bytes to rad from the file
--                  buf - buffer to save the file data to
--
-- RETURNS:     Returns the number of bytes read from the file
--
-- NOTES:
--              Opens the file and reads bytes bytes from the file. Then closes the file
--
-------------------------------------------------------------------------------------------------------------------*/
int FileHandler::readFile(int bytes, char * buf) {
    int bytesRead = 0;
    memset(buf,0,bytes);
    FILE *file = fopen(fileName.c_str(), "rb");
    fseek(file,readPointer,SEEK_SET);
    if(!feof(file)) {
        bytesRead = fread(buf, 1, bytes, file);
        readPointer += bytesRead;
    }
    fclose(file);
    return bytesRead;
}
