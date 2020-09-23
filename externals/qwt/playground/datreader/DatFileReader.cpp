#include "DatFileReader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

#define PI 3.141592653589793238

size_t explode(const std::string& str, std::vector<std::string>& tokens, char delim) {
    std::string tmp;
    tokens.clear();
    for (std::string::const_iterator it=str.begin(); it!=str.end(); ++it) {
        if (*it!=delim)
            tmp+=*it;
        else {
            if (tmp.empty()) continue;
            tokens.push_back(tmp);
            tmp.clear();
        }
    }
    if (tmp.size())
        tokens.push_back(tmp);
    return tokens.size();
}

void DatFileReader::read(const std::string & filename) {
    std::ifstream in(filename.c_str());
    if (!in) {
        std::cerr << filename << " not found." << std::endl;
        return;
    }

    //  TITLE="Delphin 6 Flux Output - Summation flux for balance equation Heat [J/m2s]"
    //  VARIABLES= "X",  "Y", "Time [d]",  "Summation flux for balance equation Heat [J/m2s] X-Flux", "Summation flux for balance equation Heat [J/m2s] Y-Flux",  "Summation flux for balance equation Heat [J/m2s] Total-Flux"
    //  ZONE I=36666, T="Time 0", DATAPACKING=POINT

    std::string line;
    std::getline(in, line); // title
    std::getline(in, line); // variables
    std::getline(in, line); // zoneid

    std::vector<std::string> tokens;
    explode(line, tokens, ',');
    std::string::size_type pos = tokens[0].find("=");
    unsigned int nSamples = atoi(tokens[0].substr(pos+1).c_str());

    x.resize(nSamples);
    y.resize(nSamples);
    length.resize(nSamples);
    u.resize(nSamples);
    v.resize(nSamples);

    std::cout << line << std::endl;
    // read next n lines
    for (int zone=0; zone<3; ++zone) {
        double t;
        for (unsigned int i=0; i<nSamples; ++i) {
            std::getline(in, line);
            std::stringstream strm(line);
            if (zone == 0)
                strm >> x[i] >> y[i] >> t >> u[i] >> v[i] >> length[i];
            else
                strm >> t >> u[i] >> v[i]  >> length[i];
        }
        std::getline(in, line);
        if (!line.empty())
            return; // expected empty line
        std::getline(in, line);
        std::cout << line << std::endl;
    }
}


#if 0
void angle( double u, double v ) 
{
    double l = std::sqrt( u * u + v * v );

    double cosBeta = u / l;
    double beta = std::acos(cosBeta);
    if (v < 0)
        beta = 2 * M_PI - beta;

    double betaDeg = beta / M_PI * 180;
    std::cout << u << " " << v << " " << l << " " << betaDeg << std::endl;
}
#endif

//  angle(0,0);
//  angle(1,0);
//  angle(1,1);
//  angle(0,1);
//  angle(-1,1);
//  angle(-1,0);
//  angle(-1,-1);
//  angle(0,-1);
//  angle(1,-1);
