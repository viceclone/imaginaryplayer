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

void TextBasedPlayer::currentPlaylistInfo()
{
    if (m_isPlaying)
    {
        NEWLINE();
    }
    LOG(BOLD("/////////////////// CURRENT PLAYLIST ///////////////////"));
    LOG(CYAN(BOLD("Name: " << m_playlist->name() << "")));
    LOG(BOLD("Description: ") << m_playlist->description() << "");
    for (auto track : m_playlist->tracks())
    {
        if (m_currentTrack && m_currentTrack == track)
        {
            LOG(">>>'" << track->title() 
                << "' by '" << track->artist() << "'");
        }
        else
        {
            LOG("---'" << track->title() 
                << "' by '" << track->artist() << "'");
        }
    }
    LOG(BOLD("########################################################"));
}

void TextBasedPlayer::streamCurrentSong()
{
    if (!m_currentTrack)
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
        }
        else
        {
            LOG("No track in your playlist!");
        }
        LOG("Current track: '" << m_currentTrack->title() 
            << "' by '" << m_currentTrack->artist() << "'");
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
    if (m_currentTrack)
    {
        m_currentTrack->resetCurrentContentIndex();
    }
    
    if (!autoplay)
    {
        LOG_COMMAND(CYAN("NEXT TRACK"));
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
    m_playlist = std::make_shared<Playlist>();
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
        case 'Q':
            terminate();
            break;
        default:
            break;
        }
    } while (m_isRunning);
}

