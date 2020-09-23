#ifndef DATFILEREADER_H
#define DATFILEREADER_H

#include <string>
#include <vector>

/*! Reads dat files with vector data. */
class DatFileReader {
public:
    void read(const std::string & filename);

    std::vector<double> x,y,u,v,length;
};

#endif // DATFILEREADER_H
