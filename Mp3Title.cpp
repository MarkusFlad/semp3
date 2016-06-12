/* 
 * File:   MpgTitle.cpp
 * Author: markus
 * 
 * Created on 28. Dezember 2015, 22:05
 */

#include "Mp3Title.hpp"
#include "Button.hpp"

using std::string;
using boost::optional;

Id3Info::Id3Info (const optional<string>& title, const optional<string>& artist,
                  const optional<string>& album, const optional<int>& year,
                  const optional<string>& comment,
                  const optional<string>& genre, const optional<int>& track)
: _title (title),
  _artist (artist),
  _album (album),
  _year (year),
  _comment (comment),
  _genre (genre),
  _track (track) {
}
optional<string> Id3Info::getTitle() const {
    return _title;
}
optional<string> Id3Info::getArtist() const {
    return _artist;
}
optional<string> Id3Info::getAlbum() const {
    return _album;
}
optional<int> Id3Info::getYear() const {
    return _year;
}
optional<string> Id3Info::getComment() const {
    return _comment;
}
optional<string> Id3Info::getGenre() const {
    return _genre;
}
optional<int> Id3Info::getTrack() const {
    return _track;
}

Mp3Title::Mp3Title (const optional<Id3Info>& id3Info,
                    const optional<string>& filename)
: _id3Info (id3Info)
, _filename (filename) {
}
optional<Id3Info> Mp3Title::getId3Info() const {
    return _id3Info;
}
optional<std::string> Mp3Title::getFilename() const {
    return _filename;
}
string Mp3Title::toString() const {
    if (_id3Info) {
        const Id3Info& id3Info = *_id3Info;
        if (id3Info.getArtist() && id3Info.getTitle()) {
            return *id3Info.getArtist() + " - " + *id3Info.getTitle();
        } else if (id3Info.getTitle()) {
            return *id3Info.getTitle();
        } else {
            return "Unknown Title";
        }
    } else if (_filename) {
        return *_filename;
    } else {
        return "Unknown Title";
    }
}

