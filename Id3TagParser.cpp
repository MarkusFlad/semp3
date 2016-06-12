/* 
 * File:   Id3TagParser.cpp
 * Author: markus
 * 
 * Created on 2. Januar 2016, 21:39
 */

#include "Id3TagParser.hpp"
#include <boost/algorithm/string/trim.hpp>
#include <sstream>
#include <iostream>

using std::istringstream;
using std::string;
using boost::algorithm::trim_copy;
using boost::optional;

Id3TagParser::Id3TagParser()
: _parsingStarted (false) {
}
bool Id3TagParser::parse (const string& tag) {
    _parsingStarted = true;
    size_t pos = tag.find ("ID3");
    if (pos == string::npos || tag.length() < 4) {
        if (_mp3Filename) {
            return false;
        }
        _mp3Filename = tag;
        return true;
    }
    pos += 3;
    size_t posMarker;
    if ((posMarker = tag.find (':', pos)) != string::npos &&
        posMarker == pos + 1 && tag.length() > posMarker + 128) {
        if (_id3v1Mp3Title) {
            return false;
        }
        pos = posMarker + 1;
        string title = trim_copy (tag.substr (pos, 30));
        pos += 30;
        string artist = trim_copy (tag.substr (pos, 30));
        pos += 30;
        string album = trim_copy (tag.substr (pos, 30));
        pos += 30;
        istringstream issYear (tag.substr (pos, 4));
        pos += 4;
        int year = 0;
        issYear >> year;
        string comment = trim_copy (tag.substr (98, 30));
        string genre = trim_copy (tag.substr (128));
        Id3Info id3Info (title, artist, album, year, comment, genre,
                         optional<int>() /* track is not given. */);
        _id3v1Mp3Title = Mp3Title (id3Info, optional<string>());
        return true;
    } if ((posMarker = tag.find ('.', pos)) != string::npos) {
        if (posMarker == pos) {
            // The dot is just after ID3 -> ID3.
            if ((pos = tag.find ("genre:")) != string::npos) {
                istringstream issGenre (tag.substr(pos + 6));
                int genre;
                issGenre >> genre;
                _id3v1Genre = genre;
            } else if ((pos = tag.find ("track:")) != string::npos) {
                istringstream issTrack (tag.substr(pos + 6));
                int track;
                issTrack >> track;
                _id3v1Track = track;
            } else {
                return false;
            }
        } else if ((posMarker = tag.find ("v2.")) != string::npos &&
                   posMarker == pos) {
            // The string v2 is between the dot and ID3 -> ID3v2.
            if ((pos = tag.find ("title:")) != string::npos) {
                _id3v2Title = tag.substr (pos + 6);
            } else if ((pos = tag.find ("artist:")) != string::npos) {
                _id3v2Artist = tag.substr (pos + 7);
            } else if ((pos = tag.find ("album:")) != string::npos) {
                _id3v2Album = tag.substr (pos + 6);
            } else if ((pos = tag.find ("year:")) != string::npos) {
                istringstream issYear (tag.substr(pos + 5));
                int year;
                issYear >> year;
                _id3v2Year = year;
            } else if ((pos = tag.find ("comment:")) != string::npos) {
                _id3v2Comment = tag.substr (pos + 8);
            } else if ((pos = tag.find ("genre:")) != string::npos) {
                _id3v2Genre = tag.substr (pos + 6);
            }
        } else {
            return false;
        }
    }
}
Mp3Title Id3TagParser::getMp3Title() const {
    if (_id3v1Mp3Title) {
        const Mp3Title& idv1Mp3Title = _id3v1Mp3Title.get();
        if (idv1Mp3Title.getId3Info()) {
            const Id3Info& id3Info = *idv1Mp3Title.getId3Info();
            optional<string> title = id3Info.getTitle();
            optional<string> artist = id3Info.getArtist();
            optional<string> album = id3Info.getAlbum();
            optional<int> year = id3Info.getYear();
            optional<string> comment = id3Info.getComment();
            optional<string> genre = id3Info.getGenre();
            optional<int> track = id3Info.getTrack();
            
            if (_id3v2Title) {
                title = *_id3v2Title;
            }
            if (_id3v2Album) {
                album = *_id3v2Album;
            }
            if (_id3v2Artist) {
                artist = *_id3v2Artist;
            }
            if (_id3v2Year) {
                year = *_id3v2Year;
            }
            if (_id3v2Comment) {
                comment = *_id3v2Comment;
            }
            if (_id3v2Genre) {
                genre = *_id3v2Genre;
            }
            if (_id3v1Genre && !genre) {
                genre = getId3v1Genre (_id3v1Genre.get());
            }
            Id3Info newId3Info (title, artist, album, year, comment, genre,
                                _id3v1Track);
            return Mp3Title (newId3Info, _mp3Filename);
        }
    } else { // No _id3v1Mp3Title given
        Id3Info id3Info (_id3v2Title, _id3v2Artist, _id3v2Album, _id3v2Year,
                         _id3v2Comment, _id3v2Genre, _id3v1Track);
        return Mp3Title (id3Info, _mp3Filename);
    }
}
string Id3TagParser::getId3v1Genre (int genreId) {
    switch (genreId) {
        case  0: return "Blues";
        case  1: return "Classic Rock";
        case  2: return "Country";
        case  3: return "Dance";
        case  4: return "Disco";
        case  5: return "Funk";
        case  6: return "Grunge";
        case  7: return "Hip-Hop";
        case  8: return "Jazz";
        case  9: return "Metal";
        case 10: return "New Age";
        case 11: return "Oldies";
        case 12: return "Other";
        case 13: return "Pop";
        case 14: return "R&B";
        case 15: return "Rap";
        case 16: return "Reggae";
        case 17: return "Rock";
        case 18: return "Techno";
        case 19: return "Industrial";
        case 20: return "Alternative";
        case 21: return "Ska";
        case 22: return "Death Metal";
        case 23: return "Pranks";
        case 24: return "Soundtrack";
        case 25: return "Euro-Techno";
        case 26: return "Ambient";
        case 27: return "Trip-Hop";
        case 28: return "Vocal";
        case 29: return "Jazz+Funk";
        case 30: return "Fusion";
        case 31: return "Trance";
        case 32: return "Classical";
        case 33: return "Instrumental";
        case 34: return "Acid";
        case 35: return "House";
        case 36: return "Game";
        case 37: return "Sound Clip";
        case 38: return "Gospel";
        case 39: return "Noise";
        case 40: return "AlternRock";
        case 41: return "Bass";
        case 42: return "Soul";
        case 43: return "Punk";
        case 44: return "Space";
        case 45: return "Meditative";
        case 46: return "Instrumental Pop";
        case 47: return "Instrumental Rock";
        case 48: return "Ethnic";
        case 49: return "Gothic";
        case 50: return "Darkwave";
        case 51: return "Techno-Industrial";
        case 52: return "Electronic";
        case 53: return "Pop-Folk";
        case 54: return "Eurodance";
        case 55: return "Dream";
        case 56: return "Southern Rock";
        case 57: return "Comedy";
        case 58: return "Cult";
        case 59: return "Gangsta";
        case 60: return "Top 40";
        case 61: return "Christian Rap";
        case 62: return "Pop/Funk";
        case 63: return "Jungle";
        case 64: return "Native American";
        case 65: return "Cabaret";
        case 66: return "New Wave";
        case 67: return "Psychadelic";
        case 68: return "Rave";
        case 69: return "Showtunes";
        case 70: return "Trailer";
        case 71: return "Lo-Fi";
        case 72: return "Tribal";
        case 73: return "Acid Punk";
        case 74: return "Acid Jazz";
        case 75: return "Polka";
        case 76: return "Retro";
        case 77: return "Musical";
        case 78: return "Rock & Roll";
        case 79: return "Hard Rock";
        case 80: return "Folk";
        case 81: return "Folk-Rock";
        case 82: return "National Folk";
        case 83: return "Swing";
        case 84: return "Fast Fusion";
        case 85: return "Bebob";
        case 86: return "Latin";
        case 87: return "Revival";
        case 88: return "Celtic";
        case 89: return "Bluegrass";
        case 90: return "Avantgarde";
        case 91: return "Gothic Rock";
        case 92: return "Progressive Rock";
        case 93: return "Psychedelic Rock";
        case 94: return "Symphonic Rock";
        case 95: return "Slow Rock";
        case 96: return "Big Band";
        case 97: return "Chorus";
        case 98: return "Easy Listening";
        case 99: return "Acoustic";
        case 100: return "Humour";
        case 101: return "Speech";
        case 102: return "Chanson";
        case 103: return "Opera";
        case 104: return "Chamber Music";
        case 105: return "Sonata";
        case 106: return "Symphony";
        case 107: return "Booty Bass";
        case 108: return "Primus";
        case 109: return "Porn Groove";
        case 110: return "Satire";
        case 111: return "Slow Jam";
        case 112: return "Club";
        case 113: return "Tango";
        case 114: return "Samba";
        case 115: return "Folklore";
        case 116: return "Ballad";
        case 117: return "Power Ballad";
        case 118: return "Rhythmic Soul";
        case 119: return "Freestyle";
        case 120: return "Duet";
        case 121: return "Punk Rock";
        case 122: return "Drum Solo";
        case 123: return "A capella";
        case 124: return "Euro-House";
        case 125: return "Dance Hall";
        default: return "Unknown";
    }
}