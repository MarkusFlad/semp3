/* 
 * File:   Id3TagParser.hpp
 * Author: markus
 *
 * Created on 2. Januar 2016, 21:39
 */

#ifndef ID3TAGPARSER_HPP
#define	ID3TAGPARSER_HPP

#include "Mp3Title.hpp"
#include <boost/optional.hpp>
#include <string>

class Id3TagParser {
public:
    Id3TagParser();
    inline bool isParsingStarted() const;
    bool parse (const std::string& tag);
    Mp3Title getMp3Title() const;
    static std::string getId3v1Genre (int genreId);
private:
    bool _parsingStarted;
    boost::optional<std::string> _mp3Filename;
    boost::optional<Mp3Title> _id3v1Mp3Title;
    boost::optional<int> _id3v1Genre;
    boost::optional<int> _id3v1Track;
    boost::optional<std::string> _id3v2Title;
    boost::optional<std::string> _id3v2Artist;
    boost::optional<std::string> _id3v2Album;
    boost::optional<int> _id3v2Year;
    boost::optional<std::string> _id3v2Comment;
    boost::optional<std::string> _id3v2Genre;
};

bool Id3TagParser::isParsingStarted() const {
    return _parsingStarted;
}

#endif	/* ID3TAGPARSER_HPP */
