#include <windows.h>
#include <winuser.h>
#include <conio.h>
#include <iostream>
#include <filesystem>
#include "ui/text_based_player.hpp"
#include "core/logger.hpp"
#include "core/constants.hpp"

namespace fs = std::filesystem;

TextBasedPlayer::~TextBasedPlayer()
{
    m_streamingThread.join();
}

void TextBasedPlayer::printHelp()
{
    LOG_COMMAND("HELP");
    LOG(">>>> Text-based player receives command from keyboard input.");
    LOG("----------------------------------------------------------");
    LOG("-> " << BOLD("'H', '?'") << ": print help");
    LOG("-> " << BOLD("'N'     ") << ": import playlist from file");
    LOG("-> " << BOLD("'M'     ") << ": save playlist to a file");
    LOG("-> " << BOLD("'C'     ") << ": create an empty playlist");
    LOG("-> " << BOLD("'J'     ") << ": add track to the current playlist");
    LOG("-> " << BOLD("'K'     ") << ": remove a track from the current playlist");
    LOG("-> " << BOLD("'L'     ") << ": remove duplicated tracks from the current playlist");
    LOG("-> " << BOLD("'Z'     ") << ": play");
    LOG("-> " << BOLD("'X'     ") << ": pause");
    LOG("-> " << BOLD("'D'     ") << ": next track");
    LOG("-> " << BOLD("'A'     ") << ": previous track");
    LOG("-> " << BOLD("'S'     ") << ": shuffle/unshuffle");
    LOG("-> " << BOLD("'R'     ") << ": change repeat mode (none/repeat all/repeat currentsong)");
    LOG("-> " << BOLD("'I'     ") << ": current playlist info");
    LOG("-> " << BOLD("'Q'     ") << ": quit");
    LOG("----------------------------------------------------------");
}

int TextBasedPlayer::importPlaylist()
{
    std::lock_guard<decltype(m_mutex)> lock(m_mutex);
    LOG_COMMAND(CYAN("IMPORT PLAYLIST"));
    pause(true);
    std::string pathString;
    std::cout << "Enter playlist file information: ";
    std::cin >> pathString;
    fs::path currentPath = fs::current_path();
    auto path = fs::path(pathString);
    if (path.is_relative())
    {
        path = currentPath / path;
    }
    auto playlist = std::make_shared<Playlist>();
    auto count = playlist->importFromFile(path);
    if (playlist->isValid())
    {
        m_playlist.swap(playlist);
        m_currentTrack = m_playlist->resetToFirstTrack();
    }

    if (count == 0)
    {
        WARN_MSG("Empty playlist imported");
    }
    
    return count;
}

void TextBasedPlayer::createPlaylist()
{
    LOG_COMMAND(CYAN("CREATE PLAYLIST"));
    m_playlist = std::make_shared<Playlist>();
    
    std::string s;
    PROMPT("Playlist Name", s);
    m_playlist->setName(s);

    PROMPT("Playlist Description", s);
    m_playlist->setDescription(s);

    m_playlist->validate(true);
}

void TextBasedPlayer::savePlaylist()
{
    LOG_COMMAND(CYAN("SAVE PLAYLIST"));
    if (!(m_playlist && m_playlist->isValid()))
    {
        WARN_MSG("No valid playlist available");
        return;
    }

    std::string pathString;
    PROMPT("Path", pathString);
    if (fs::exists(fs::path(pathString)))
    {
        std::string answer;
        PROMPT("Playlist already exists, enter 'yes' to overwrite? ", answer);
        if (answer != std::string("yes"));
        {
            return;
        }
    }

    m_playlist->exportToFile(pathString);
}

void TextBasedPlayer::addTrack()
{
    LOG_COMMAND(CYAN("ADD TRACK"));
    
    if (!m_playlist)
    {
        WARN_MSG("No playlist available");
        return;
    }

    bool wasPlaying = m_isPlaying;
    if (wasPlaying)
    {
        pause();
    }
    std::string pathString;
    PROMPT("Path", pathString);
    if (fs::exists(fs::path(pathString)) && fs::is_regular_file(fs::path(pathString)))
    {
        TrackPtr track = std::make_shared<Track>();
        track->initFromFile(fs::path(pathString));
        m_playlist->addTrack(track);
    }
    else
    {
        WARN_MSG("File does not exist! Ignoring this command.");
    }
    
    if (wasPlaying)
    {
        play();
    }
}

void TextBasedPlayer::removeTrack()
{
    LOG_COMMAND(CYAN("REMOVE TRACK"));
    if (!m_playlist)
    {
        WARN_MSG("No playlist available");
        return;
    }
    bool wasPlaying = m_isPlaying;
    if (wasPlaying)
    {
        pause();
    }
    
    currentPlaylistInfo();
    std::string indexStr;
    PROMPT("Song index", indexStr);
    auto index = std::stoi(indexStr);
    if (index <= m_playlist->size() && index >= 1)
    {
        if (m_playlist->removeTrack(index-1))
        {
            LOG("Track removed successfully!");
            return;
        }
    }
    else
    {
        WARN_MSG("Track index out of bound!");
    }
    
    if (wasPlaying)
    {
        play();
    }
}

void TextBasedPlayer::removeDuplicate()
{
    LOG_COMMAND(CYAN("REMOVE DUPLICATE"));
    if (!m_playlist)
    {
        WARN_MSG("No playlist available");
        return;
    }

    m_playlist->removeDuplicate();
}

void TextBasedPlayer::currentPlaylistInfo()
{
    if (m_isPlaying)
    {
        NEWLINE();
    }

    LOG(BOLD("/////////////////// CURRENT PLAYLIST ///////////////////"));
    if (!m_playlist || !m_playlist->isValid())
    {
        LOG(RED("NO PLAYLIST AVAILABLE"));
        LOG(BOLD("########################################################"));
        return;
    }
    
    LOG(CYAN(BOLD("Name: " << m_playlist->name() << "")));
    LOG(BOLD("Description: ") << m_playlist->description() << "");
    int count = 0;
    for (auto track : m_playlist->tracks())
    {   
        count++;
        if (m_currentTrack && m_currentTrack == track)
        {
            LOG(">>> " << count << ". '" << track->title() 
                << "' by '" << track->artist() << "'");
        }
        else
        {
            LOG("--- " << count << ". '"  << track->title() 
                << "' by '" << track->artist() << "'");
        }
    }
    LOG(BOLD("########################################################"));
}

void TextBasedPlayer::currentTrackInfo()
{
    if (m_isPlaying)
    {
        NEWLINE();
    }
    LOG(BOLD("/////////////////// CURRENT TRACK ///////////////////"));

    if (!m_playlist)
    {
        WARN_MSG("No playlist available");
        return;
    }

    if (!m_currentTrack)
    {
        LOG(RED("NO CURRENT TRACK IS SELECTED"));
    }
    else
    {
        LOG(CYAN(BOLD("Title: " << m_currentTrack->title() << "")));
        LOG(BOLD("Artist: ") << m_currentTrack->artist() << "");
        LOG(BOLD("Codec: ") << m_currentTrack->codec() << "");
    }
    LOG(BOLD("########################################################"));
}

void TextBasedPlayer::streamCurrentSong()
{
    if (!m_currentTrack || !m_playlist || !m_playlist->isValid())
    {
        return;
    }

    while (m_currentTrack && !m_currentTrack->endOfTrack() && m_isRunning)
    {
        if (!m_isPlaying)
        {
            std::unique_lock<decltype(m_mutex)> lock(m_mutex);
            m_cv.wait(lock);
        }
        else
        {
            std::cout << m_currentTrack->streamCurrentContent();
            std::this_thread::sleep_for(DelayBetweenContent);
        }
    }

    if (!m_isRunning)
    {
        LOG("Quitting the application!");
        return;
    }

    if (!next(true /*autoplay*/))
    {
        m_isPlaying = false;
        LOG("Press "<< GREEN("PLAY") << " to replay to the current playlist!");
    }
}

void TextBasedPlayer::play()
{
    std::lock_guard<decltype(m_mutex)> lock(m_mutex);
    LOG_COMMAND(GREEN("PLAY"));
    if (!m_playlist || !m_playlist->isValid())
    {
        LOG("No playlist is currently available!");
        return;
    }

    if (!m_isPlaying)
    {
        if (!m_currentTrack)
        {
            m_currentTrack = m_playlist->resetToFirstTrack();
        }
        else
        {
            m_currentTrack = m_playlist->currentTrack();
        }
        
        if (m_currentTrack)
        {
            m_isPlaying = true;
            LOG("Current track: '" << m_currentTrack->title() 
            << "' by '" << m_currentTrack->artist() << "'");
        }
        else
        {
            LOG("No track in your playlist!");
        }
    }
    m_cv.notify_one();
}

void TextBasedPlayer::pause(bool autopause)
{
    if (!autopause)
    {
        std::lock_guard<decltype(m_mutex)> lock(m_mutex);
        LOG_COMMAND(YELLOW("PAUSE"));
        m_isPlaying = false;
    }
    else
    {
        m_isPlaying = false;
    }
    
}

bool TextBasedPlayer::next(bool autoplay)
{
    std::lock_guard<decltype(m_mutex)> lock(m_mutex);
    
    if (!m_playlist)
    {
        WARN_MSG("No playlist available");
        return false;
    }

    if (m_currentTrack)
    {
        m_currentTrack->resetCurrentContentIndex();
    }
    
    if (!autoplay)
    {
        LOG_COMMAND(CYAN("NEXT TRACK"));
    }
    
    if (m_playlist->getRepeatMode() == RepeatMode::RepeatCurrentSong)
    {
        LOG("Switching back repeat mode to " << YELLOW(BOLD("WHOLE PLAYLIST")));
    }

    m_currentTrack = m_playlist->nextTrack(autoplay);
    NEWLINE();
    if (m_currentTrack)
    {
        LOG("Switching to the next song '" << m_currentTrack->title() 
                    << "' by '" << m_currentTrack->artist() << "'");
        return true;
    }
    else
    {
        LOG("No track available!");
        return false;
    }
}

bool TextBasedPlayer::previous()
{
    std::lock_guard<decltype(m_mutex)> lock(m_mutex);
    LOG_COMMAND(CYAN("PREVIOUS TRACK"));
    if (m_currentTrack)
    {
        m_currentTrack->resetCurrentContentIndex();
    }
    
    m_currentTrack = m_playlist->previousTrack();
    NEWLINE();
    if (m_currentTrack)
    {
        LOG("Switching to the previous song '" << m_currentTrack->title() 
                    << "' by '" << m_currentTrack->artist() << "'");
        return true;
    }
    else
    {
        LOG("No track available!");
        return false;
    }
}

void TextBasedPlayer::shuffle()
{
    std::lock_guard<decltype(m_mutex)> lock(m_mutex);
    if (!m_playlist->isShuffled())
    {
        LOG_COMMAND(CYAN("SHUFFLE"));
        m_playlist->shuffle();
    }
    else
    {
        LOG_COMMAND(CYAN("UNSHUFFLE"));
        m_playlist->unshuffle();
    }
}

void TextBasedPlayer::repeat()
{
    std::lock_guard<decltype(m_mutex)> lock(m_mutex);
    LOG_COMMAND(CYAN("REPEAT"));
    m_playlist->repeat();
    switch (m_playlist->getRepeatMode())
    {
    case RepeatMode::NoRepeat:
        LOG("Switching repeat mode to " << YELLOW(BOLD("NONE")));
        break;
    case RepeatMode::RepeatWholePlaylist:
        LOG("Switching repeat mode to " << YELLOW(BOLD("WHOLE PLAYLIST")));
        break;
    case RepeatMode::RepeatCurrentSong:
        LOG("Switching repeat mode to " << YELLOW(BOLD("CURRENT SONG")));
        break;
    default:
        break;
    }
}

void TextBasedPlayer::init()
{
    std::lock_guard<decltype(m_mutex)> lock(m_mutex);
}

void TextBasedPlayer::terminate()
{
    std::lock_guard<decltype(m_mutex)> lock(m_mutex);
    LOG_COMMAND(RED("TERMINATE"));
    m_isRunning = false;
    m_cv.notify_all();
}

void TextBasedPlayer::run()
{
    LOG(BOLD("*********************** WELCOME TO THE IMAGINARY PLAYER BY HUY ***********************"));
    LOG(BOLD(">>>> Press 'N' to import your playlist from file <<<<"));
    LOG(BOLD(">>>> Press 'H' or '?' to get help <<<<"));
    m_isRunning = true;
    m_streamingThread = std::thread([this]
    {
        while (m_isRunning)
        {
            streamCurrentSong();
            std::this_thread::sleep_for(DelayBetweenTracks);
        }
    });

    startCommandHandler();
}

void TextBasedPlayer::startCommandHandler()
{
    int charCommand;
    do
    {
        charCommand = _getch();
        charCommand = toupper(charCommand);
        if (m_isPlaying)
        {
            NEWLINE();
        }
        switch (charCommand)
        {
        case 'H':
        case '?':
            printHelp();
            break;
        case 'N':
            importPlaylist();
            break;
        case 'M':
            savePlaylist();
            break;
        case 'C':
            createPlaylist();
            break;
        case 'J':
            addTrack();
            break;
        case 'K':
            removeTrack();
            break;
        case 'L':
            removeDuplicate();
            break;
        case 'Z':
            play();
            break;
        case 'X':
            pause();
            break;
        case 'D':
            next();
            break;
        case 'A':
            previous();
            break;
        case 'S':
            shuffle();
            break;
        case 'R':
            repeat();
            break;
        case 'I':
            currentPlaylistInfo();
            break;
        case 'U':
            currentTrackInfo();
            break;
        case 'Q':
            terminate();
            break;
        default:
            break;
        }
    } while (m_isRunning);
}

