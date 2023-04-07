#include <windows.h>
#include <winuser.h>
#include <conio.h>
#include <iostream>
#include <filesystem>
#include "ui/text_based_player.hpp"
#include "core/logger.hpp"

namespace fs = std::filesystem;

TextBasedPlayer::~TextBasedPlayer()
{
    m_streamingThread.join();
}

int TextBasedPlayer::importPlaylist()
{
    std::string pathString;
    std::cout << "Enter folder containing playlist information: ";
    std::cin >> pathString;
    fs::path currentPath = fs::current_path();
    auto path = currentPath / pathString;
    if (!fs::is_directory(path))
    {
        std::cerr << "Invalid path!" << std:: endl;
        return 0;
    }
   
    m_playlist = std::make_shared<Playlist>();
    return m_playlist->importFromFolder(path);
}

void TextBasedPlayer::streamCurrentSong()
{
    if (!m_currentTrack)
    {
        return;
    }

    std::cout << "Current track: '" << m_currentTrack->title() 
              << "' by '" << m_currentTrack->artist() << "'" << std::endl;
    while (!m_currentTrack->endOfTrack() && m_isRunning)
    {
        if (!m_isPlaying)
        {
            std::unique_lock<decltype(m_mutex)> lock(m_mutex);
            m_cv.wait(lock);
        }
        else
        {
            std::cout << m_currentTrack->streamCurrentContent();
            Sleep(1000 /*milliseconds*/);
        }
    }
    std::cout << std::endl;

    if (!m_isRunning)
    {
        std::cout << "Quitting the application!" << std::endl;
        return;
    }

    m_currentTrack->resetCurrentContentIndex();
    if (!next())
    {
        m_isPlaying = false;
        std::cout << "No more track available. Press PLAY to relisten to the curren playlist!" << std::endl;
    }
}

void TextBasedPlayer::play()
{
    if (!m_isPlaying)
    {
        {
            std::unique_lock<decltype(m_mutex)> lock(m_mutex);
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
                std::cout << "No track in your playlist!" << std::endl;
            }
        }
        m_cv.notify_one();
    }    
}

void TextBasedPlayer::pause()
{
    std::unique_lock<decltype(m_mutex)> l(m_mutex);
    m_isPlaying = false;
}

bool TextBasedPlayer::next()
{
    m_currentTrack = m_playlist->nextTrack();
    return m_currentTrack != nullptr;
}

bool TextBasedPlayer::previous()
{
    return true;
}

void TextBasedPlayer::shuffle()
{

}

void TextBasedPlayer::unshuffle()
{

}

void TextBasedPlayer::repeatAll()
{

}

void TextBasedPlayer::repeatOne()
{

}

void TextBasedPlayer::init()
{
    m_playlist = std::make_shared<Playlist>();
}

void TextBasedPlayer::run()
{
    m_isRunning = true;
    auto trackCount = importPlaylist();
    m_currentTrack = m_playlist->resetToFirstTrack();
    std::cout << trackCount << " tracks have been imported" << std::endl;
    m_streamingThread = std::thread([this]
    {
        while (m_isRunning)
        {
            streamCurrentSong();
            Sleep(10);
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
        switch (charCommand)
        {
        case 'Z':
            std::cout << std::endl << ">>>>> " << GREEN(BOLD("PLAY")) << " <<<<<" << std::endl;
            play();
            break;
        case 'X':
            std::cout << std::endl << ">>>>> " << YELLOW(BOLD("PAUSE")) << " <<<<<" << std::endl;
            pause();
            break;
        case 'Q':
            std::cout << std::endl << ">>>>> " << RED(BOLD("QUIT")) << " <<<<<" << std::endl;
            terminate();
            break;
        default:
            break;
        }
    } while (m_isRunning);
    
}

void TextBasedPlayer::terminate()
{
    m_isRunning = false;
    m_cv.notify_all();
}