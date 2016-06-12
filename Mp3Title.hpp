/* 
 * File:   MpgTitle.h
 * Author: markus
 *
 * Created on 28. Dezember 2015, 22:05
 */

#ifndef MP3TITLE_H
#define	MP3TITLE_H

#include <string>
#include <boost/optional.hpp>

class Id3Info {
public:
    typedef boost::optional<std::string> OptionalString;
    typedef boost::optional<int> OptionalInt;
    Id3Info (const OptionalString& title, const OptionalString& artist,
             const OptionalString& album, const OptionalInt& year,
             const OptionalString& comment, const OptionalString& genre,
             const OptionalInt& track);
    OptionalString getTitle() const;
    OptionalString getArtist() const;
    OptionalString getAlbum() const;
    OptionalInt getYear() const;
    OptionalString getComment() const;
    OptionalString getGenre() const;
    OptionalInt getTrack() const;
private:
    OptionalString _title;
    OptionalString _artist;
    OptionalString _album;
    OptionalInt _year;
    OptionalString _comment;
    OptionalString _genre;
    OptionalInt _track;
};

class Mp3Title {
public:
    Mp3Title (const boost::optional<Id3Info>& id3Info,
              const boost::optional<std::string>& filename);
    boost::optional<Id3Info> getId3Info() const;
    boost::optional<std::string> getFilename() const;
    std::string toString() const;
private:
    boost::optional<Id3Info> _id3Info;
    boost::optional<std::string> _filename;
};

#endif	/* MP3TITLE_H */

