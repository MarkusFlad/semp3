/* 
 * File:   RebootSafeString.cpp
 * Author: markus
 * 
 * Created on 14. März 2017, 15:28
 */

#include "RebootSafeString.hpp"
#include <boost/filesystem/path.hpp>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <string>
#include <streambuf>
#include <climits>

using boost::filesystem::path;
using std::string;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::istreambuf_iterator;

const string RebootSafeString::ODD_PREFIX ("a-");
const string RebootSafeString::EVEN_PREFIX ("b-");

RebootSafeString::RebootSafeString()
: _valid (false)
, _serialNumber (0) {
}
RebootSafeString::RebootSafeString(const path& parentPath,
                                   const string& baseFileName)
: _valid (true)
, _parentPath (parentPath)
, _baseFileName (baseFileName) {
    int oddSerialNumber = getSerialNumber(ODD_PREFIX);
    int evenSerialNumber = getSerialNumber(EVEN_PREFIX);
    _serialNumber = std::max (oddSerialNumber, evenSerialNumber);
    if (_serialNumber == 0) {
        if (oddSerialNumber == 0) {
            _serialNumber = evenSerialNumber;
        } else if (evenSerialNumber == 0) {
            _serialNumber = oddSerialNumber;
        }
    }
    if (_serialNumber != 0) {
        if (_serialNumber % 2) {
            _value = getValue(ODD_PREFIX);
        } else {
            _value = getValue(EVEN_PREFIX);
        }
    }
}
RebootSafeString::RebootSafeString(const RebootSafeString& oldString,
                                   const string& value)
: _valid (true)
, _parentPath (oldString._parentPath)
, _baseFileName (oldString._baseFileName)
/* Note: If we reach uint max we continue with an even serial number since */
/*       uint max is always odd least significant bit is 1.  */
, _serialNumber ((oldString._serialNumber == UINT_MAX) ? 2 :
                 (oldString._serialNumber + 1))
, _value (value) {
    if (_serialNumber != 0) {
        ofstream stream;
        if (_serialNumber % 2) {
            stream = getOutputStream(ODD_PREFIX);
        } else {
            stream = getOutputStream(EVEN_PREFIX);
        }
        stream << _value;
        stream << '\n';
        stream << '<';
        stream << std::dec << std::setw(9) << std::setfill('0');
        stream << _serialNumber;
        stream << '>';
        stream.close();
    }
}
bool RebootSafeString::isValid() const {
    return _valid;
}
path RebootSafeString::getParentPath() const {
    return _parentPath;
}
string RebootSafeString::getBaseFileName() const {
    return _baseFileName;
}
string RebootSafeString::getValue() const {
    return _value;
}
ifstream RebootSafeString::getInputStream (const string& prefix) const {
    path fileName (_parentPath);
    fileName /= prefix + _baseFileName;
    ifstream stream (fileName.c_str());
    return stream;
}
ofstream RebootSafeString::getOutputStream (const string& prefix) const {
    path fileName (_parentPath);
    fileName /= prefix + _baseFileName;
    ofstream stream (fileName.c_str());
    return stream;
}
int RebootSafeString::getSerialNumber (const string& prefix) const {
    ifstream stream = getInputStream (prefix);
    if (stream.seekg(-12, std::ios::end).good()) {
        char header ('h');
        unsigned int serialNumber (0);
        char trailer ('t');
        stream >> header;
        stream >> serialNumber;
        stream >> trailer;
        stream.close();
        if (header == '<' && trailer == '>') {
            return serialNumber;
        }
    }
    return 0;
}
string RebootSafeString::getValue (const string& prefix) const {
    ifstream stream = getInputStream (prefix);
    if (stream.seekg(0, std::ios::beg).good()) {
        string value ((istreambuf_iterator<char>(stream)),
                       istreambuf_iterator<char>());   
        stream.close();
        if (value.length() > 12) {
            return value.substr(0, value.length() - 12);
        } else {
            return "";
        }
    }
}
