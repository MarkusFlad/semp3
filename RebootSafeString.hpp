/* 
 * File:   RebootSafeString.hpp
 * Author: markus
 *
 * Created on 14. März 2017, 15:30
 */

#ifndef REBOOTSAFESTRING_HPP
#define	REBOOTSAFESTRING_HPP

#include <boost/filesystem/path.hpp>
#include <fstream>
#include <string>

/**
 * Class that can be used to store a string in a power-off safe way, persistent
 * way. Uses two files to store the string. One file contains the previous one
 * the old string value. If the new file is missing because of a power-loss
 * the old string value is used.
 */
class RebootSafeString {
public:
    /** Default value. Constructs an invalid reboot safe string.*/
    RebootSafeString();
    /** Read the string value from the either the file that carries the latest
      * or the file that carries the previous value.
      * @param parentPath The path where the files are stored.
      * @param baseFileName The common name of the two files. */
    RebootSafeString(const boost::filesystem::path& parentPath,
                     const std::string& baseFileName);
    /** Store the string value in the file that contains the oldest value.
      * @param oldString The object that maintains the previous reboot safe
     *                   string.
      * @param value The new string value to be made persistent. */
    RebootSafeString(const RebootSafeString& oldString,
                     const std::string& value);
    /**
     * Check if the reboot safe string is valid.
     * @return False if the default constructor has been called, else true.
     */
    bool isValid() const;
    /**
     * Get the path where the files are stored that are used to implement the
     * boot safe persistency.
     * @return Parent path of the persistency files.
     */
    boost::filesystem::path getParentPath() const;
    /**
     * Get common name of the files that are used to implement the boot safe
     * persistency.
     * @return Common part of the persistency files.
     */
    std::string getBaseFileName() const;
    /**
     * Get the actual value of the boot safe string.
     * @return The string that is stored in this boot safe string.
     */
    std::string getValue() const;

private:
    std::ifstream getInputStream (const std::string& prefix) const;
    std::ofstream getOutputStream (const std::string& prefix) const;
    int getSerialNumber (const std::string& prefix) const;
    std::string getValue (const std::string& prefix) const;

    bool _valid;
    boost::filesystem::path _parentPath;
    std::string _baseFileName;
    unsigned int _serialNumber;
    std::string _value;
    static const std::string ODD_PREFIX;
    static const std::string EVEN_PREFIX;
};

#endif	/* REBOOTSAFESTRING_HPP */

