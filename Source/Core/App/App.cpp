/*
    This file is part of Helio Workstation.

    Helio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Helio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Helio. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Common.h"
#include "App.h"
#include "AudioCore.h"
#include "UpdateManager.h"
#include "TranslationManager.h"
#include "ArpeggiatorsManager.h"
#include "AuthorizationManager.h"

#include "HelioTheme.h"
#include "ThemeSettings.h"
#include "DataEncoder.h"
#include "PluginManager.h"
#include "Config.h"
#include "Supervisor.h"
#include "InternalClipboard.h"
#include "FontSerializer.h"
#include "FileUtils.h"

#include "MainLayout.h"
#include "Document.h"
#include "SerializationKeys.h"

#include "Icons.h"
#include "ColourSchemeManager.h"
#include "ArpeggiatorsManager.h"
#include "TranslationManager.h"
#include "MainWindow.h"
#include "Workspace.h"
#include "RootTreeItem.h"

App::App()
{
}

App::~App()
{
}


//===----------------------------------------------------------------------===//
// Static
//===----------------------------------------------------------------------===//

App *App::Helio()
{
    return dynamic_cast<App *>(App::getInstance());
}

Workspace &App::Workspace()
{
    return *App::Helio()->getWorkspace();
}

MainLayout &App::Layout()
{
    return *App::Helio()->getWindow()->getWorkspaceComponent();
}

MainWindow &App::Window()
{
    return *App::Helio()->getWindow();
}

Point<double> App::getScreenInCm()
{
    Rectangle<int> screenArea = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    const double retinaFactor = Desktop::getInstance().getDisplays().getMainDisplay().scale;
    const double dpi = Desktop::getInstance().getDisplays().getMainDisplay().dpi;
    const double cmWidth = (screenArea.getWidth() / dpi) * retinaFactor * 2.54;
    const double cmHeight = (screenArea.getHeight() / dpi) * retinaFactor * 2.54;
    return Point<double>(cmWidth, cmHeight);
}

bool App::isRunningOnPhone()
{
    const auto cmSize = App::getScreenInCm();
    return (cmSize.x < 15.0 || cmSize.y < 8.0);
}

bool App::isRunningOnTablet()
{
#if HELIO_MOBILE
    return ! App::isRunningOnPhone();
#elif HELIO_DESKTOP
    return false;
#endif
}

bool App::isRunningOnDesktop()
{
#if HELIO_MOBILE
    return false;
#elif HELIO_DESKTOP
    return true;
#endif
}

String App::getAppReadableVersion()
{
    String v;
    
    if (String(APP_VERSION_REVISION).getIntValue() > 0)
    {
        v << APP_VERSION_MAJOR << "." << APP_VERSION_MINOR << "." << APP_VERSION_REVISION;
    }
    else
    {
        v << APP_VERSION_MAJOR << "." << APP_VERSION_MINOR;
    }

    if (String(APP_VERSION_NAME).isNotEmpty())
    {
        v << " (" << APP_VERSION_NAME << ")";
    }

    return v;
}

// In sql format
String App::getCurrentTime()
{
    const Time &time = Time::getCurrentTime();
    return App::getSqlFormattedTime(time);
}

// In sql format
String App::getSqlFormattedTime(const Time &time)
{
    String timeString;
    timeString << time.getYear() << '-';

    const int month = time.getMonth() + 1;
    timeString << (month < 10 ? "0" : "") << month << '-';

    const int day = time.getDayOfMonth();
    timeString << (day < 10 ? "0" : "") << day << ' ';

    const int mins = time.getMinutes();
    timeString << time.getHours() << (mins < 10 ? ":0" : ":") << mins;

    const int secs = time.getSeconds();
    timeString << (secs < 10 ? ":0" : ":") << secs;

    return timeString.trimEnd();
}

bool datesMatchByDay(const Time &date1, const Time &date2)
{
    return (date1.getYear() == date2.getYear() &&
            date1.getDayOfYear() == date2.getDayOfYear());
}

bool dateIsToday(const Time &date)
{
    const Time &now = Time::getCurrentTime();
    return datesMatchByDay(now, date);
}

bool dateIsYesterday(const Time &date)
{
    const Time &yesterday = Time::getCurrentTime() - RelativeTime::days(1);
    return datesMatchByDay(yesterday, date);
}


String App::getHumanReadableDate(const Time &date)
{
    String timeString;

    if (dateIsToday(date))
    {
        const int mins = date.getMinutes();
        timeString << date.getHours() << (mins < 10 ? ":0" : ":") << mins;

        const int secs = date.getSeconds();
        timeString << (secs < 10 ? ":0" : ":") << secs;

        return timeString.trimEnd();
    }
    if (dateIsYesterday(date))
    {
        return TRANS("common::yesterday");
    }

    const int month = date.getMonth() + 1;
    timeString << date.getYear() << '-' << (month < 10 ? "0" : "") << month << '-';

    const int day = date.getDayOfMonth();
    timeString << (day < 10 ? "0" : "") << day;

    return timeString.trimEnd();
}

void App::showTooltip(const String &text, int timeOutMs)
{
    this->getWindow()->getWorkspaceComponent()->showTooltip(text, timeOutMs);
}

void App::showTooltip(Component *newTooltip, int timeOutMs)
{
    this->getWindow()->getWorkspaceComponent()->showTooltip(newTooltip, timeOutMs);
}

void App::showTooltip(Component *newTooltip, Rectangle<int> callerScreenBounds, int timeOutMs)
{
    this->getWindow()->getWorkspaceComponent()->showTooltip(newTooltip, callerScreenBounds, timeOutMs);
}

void App::showModalComponent(Component *nonOwnedComponent)
{
    this->getWindow()->getWorkspaceComponent()->showModalNonOwnedDialog(nonOwnedComponent);
}

void App::showBlocker(Component *nonOwnedComponent)
{
    this->getWindow()->getWorkspaceComponent()->showBlockingNonModalDialog(nonOwnedComponent);
}

void App::recreateLayout()
{
    this->getWindow()->dismissLayoutComponent();
    if (TreeItem *root = this->getWorkspace()->getTreeRoot())
    {
        root->recreateSubtreePages();
    }
    this->getWindow()->createLayoutComponent();
}

void App::dismissAllModalComponents()
{
    while (Component *modal = Component::getCurrentlyModalComponent(0))
    {
        Logger::writeToLog("getNumCurrentlyModalComponents: " + String(Component::getNumCurrentlyModalComponents()));
        Logger::writeToLog("Dismissing the modal component: " + modal->getName());
        modal->exitModalState(0);
        // Unowned components may leak here, use with caution
    }
}

//===----------------------------------------------------------------------===//
// JUCEApplication
//===----------------------------------------------------------------------===//

static void handleCrash(void *)
{
    App::Helio()->getSupervisor()->trackCrash();
}

void App::initialise(const String &commandLine)
{
    this->runMode = detectRunMode(commandLine);

    if (this->runMode == App::NORMAL)
    {
        SystemStats::setApplicationCrashHandler(handleCrash);
        
        Desktop::getInstance().setOrientationsEnabled(Desktop::rotatedClockwise + Desktop::rotatedAntiClockwise);
        FileUtils::fixCurrentWorkingDirectory();
        
        Logger::setCurrentLogger(&this->logger);
        Logger::writeToLog("Helio Workstation");
        Logger::writeToLog("Ver. " + App::getAppReadableVersion());
        
        Logger::writeToLog(this->collectSomeSystemInfo());
        
        this->config = new Config();
        this->supervisor = new Supervisor();
        this->updater = new UpdateManager();

        this->theme = new HelioTheme();
        this->theme->initResources();
        LookAndFeel::setDefaultLookAndFeel(this->theme);

        this->authorizationManager = new AuthorizationManager();
        this->clipboard = new InternalClipboard();

        TranslationManager::getInstance().initialise(commandLine);
        ArpeggiatorsManager::getInstance().initialise(commandLine);
        ColourSchemeManager::getInstance().initialise(commandLine);

        this->workspace = new class Workspace();
        this->window = new MainWindow();
        
        TranslationManager::getInstance().addChangeListener(this);
        
        // Desktop versions will be initializaed by InitScreen component.
#if HELIO_MOBILE
        App::Workspace().init();
        App::Layout().init();
#endif
    }
    else if (this->runMode == App::PLUGIN_CHECK)
    {
        this->checkPlugin(commandLine);
        this->quit();
    }
    else if (this->runMode == App::FONT_SERIALIZE)
    {
        FontSerializer fs;
        fs.run(commandLine);
        this->quit();
    }
}

void App::shutdown()
{
    if (this->runMode == App::NORMAL)
    {
        TranslationManager::getInstance().removeChangeListener(this);

        Logger::writeToLog("App::shutdown");

        this->window = nullptr;
        this->workspace = nullptr;

        this->clipboard = nullptr;
        this->authorizationManager = nullptr;
        this->supervisor = nullptr;
        this->config = nullptr;
        this->theme = nullptr;

        const File tempFolder(FileUtils::getTemporaryFolder());
        if (tempFolder.exists())
        {
            tempFolder.deleteRecursively();
        }
        
        // Clear cache to avoid leak check to fire.
        Icons::clearPrerenderedCache();
        Icons::clearBuiltInImages();
        ColourSchemeManager::getInstance().shutdown();
        ArpeggiatorsManager::getInstance().shutdown();
        TranslationManager::getInstance().shutdown();
        
        Logger::setCurrentLogger(nullptr);
    }
    else if (this->runMode == App::PLUGIN_CHECK)
    {

    }
    else if (this->runMode == App::FONT_SERIALIZE)
    {

    }
}

const String App::getApplicationName()
{
    return "Helio";
}

const String App::getApplicationVersion()
{
    return App::getAppReadableVersion();
}

bool App::moreThanOneInstanceAllowed()
{
    return true; // без вариантов, мы должны как-то плагины проверять
    //return false; // debug
}

void App::anotherInstanceStarted(const String &commandLine)
{
    // This will get called if the user launches another copy of the app
    // todo read commandline && exec
    
    //this->getWindow()->toFront(true);

    Logger::outputDebugString("App::anotherInstanceStarted");
    Logger::outputDebugString(commandLine);

    //const Component *focused = Component::getCurrentlyFocusedComponent();
    //Logger::outputDebugString(focused ? focused->getName() : "");
}

void App::systemRequestedQuit()
{
    Logger::writeToLog("App::systemRequestedQuit");

    if (this->getWorkspace() != nullptr)
    {
        this->getWorkspace()->stopPlaybackForAllProjects();
        this->getWorkspace()->autosave();
    }

    App::dismissAllModalComponents();
    
    this->triggerAsyncUpdate();
}

void App::suspended()
{
    Logger::writeToLog("App::suspended");

    if (this->getWorkspace() != nullptr)
    {
        this->getWorkspace()->stopPlaybackForAllProjects();
        this->getWorkspace()->getAudioCore().mute();
        this->getWorkspace()->autosave();
    }
    
#if JUCE_ANDROID
    this->getWindow()->detachOpenGLContext();
#endif
}

void App::resumed()
{
    Logger::writeToLog("App::resumed");

    if (this->getWorkspace() != nullptr)
    {
        this->getWorkspace()->getAudioCore().unmute();
    }

#if JUCE_ANDROID
    this->getWindow()->attachOpenGLContext();
#endif
}

void App::unhandledException(const std::exception *e, const String &sourceFilename, int lineNumber)
{
    this->getSupervisor()->trackException(e, sourceFilename, lineNumber);
}


//===----------------------------------------------------------------------===//
// Accessors
//===----------------------------------------------------------------------===//

MainWindow *App::getWindow() const noexcept
{
    return this->window;
}

Workspace *App::getWorkspace() const noexcept
{
    return this->workspace;
}

Config *App::getConfig() const noexcept
{
    return this->config;
}

InternalClipboard *App::getClipboard() const noexcept
{
    return this->clipboard;
}

Supervisor *App::getSupervisor() const noexcept
{
    return this->supervisor;
}

AuthorizationManager *App::getAuthManager() const noexcept
{
    return this->authorizationManager;
}

UpdateManager *App::getUpdateManager() const noexcept
{
    return this->updater;
}

HelioTheme *App::getTheme() const noexcept
{
    return this->theme;
}


//===----------------------------------------------------------------------===//
// Private
//===----------------------------------------------------------------------===//

String App::collectSomeSystemInfo()
{
    String systemInfo;
    const Desktop::Displays::Display &dis = Desktop::getInstance().getDisplays().getMainDisplay();
    const Rectangle<int> rect(dis.totalArea);
    const double scale(dis.scale);
    const auto cmScreenSize = App::getScreenInCm();

    systemInfo
            << "Resolution: " << rect.getWidth() << ":" << rect.getHeight() << newLine
            << "Display scale: " << scale << newLine
            << "Screen area: " << String(cmScreenSize.x) << " x " << String(cmScreenSize.y) << " cm."
            
            << "User logon name: "  << SystemStats::getLogonName() << newLine
            << "Full user name: "   << SystemStats::getFullUserName() << newLine
            << "Host name: "        << SystemStats::getComputerName() << newLine

            << "Current working directory: "         << File::getCurrentWorkingDirectory().getFullPathName() << newLine
            << "User documents directory: "          << File::getSpecialLocation(File::userDocumentsDirectory).getFullPathName() << newLine
            << "User application data directory: "   << File::getSpecialLocation(File::userApplicationDataDirectory).getFullPathName() << newLine
            << "Common application data directory: " << File::getSpecialLocation(File::commonApplicationDataDirectory).getFullPathName() << newLine
            << "Temp directory: "                    << File::getSpecialLocation(File::tempDirectory).getFullPathName() << newLine
            << newLine;

    return systemInfo;
}

String App::getMacAddressList()
{
    Array <MACAddress> macAddresses;
    MACAddress::findAllAddresses(macAddresses);

    StringArray addressStrings;

    for (auto && macAddresse : macAddresses)
    {
        addressStrings.add(macAddresse.toString());
    }

    return addressStrings.joinIntoString(", ");
}

App::RunMode App::detectRunMode(const String &commandLine)
{
    if (commandLine != "")
    {
        if (commandLine.contains("-F") && commandLine.contains("-f"))
        {
            return App::FONT_SERIALIZE;
        }
        if (FileUtils::getTempSlot(commandLine).existsAsFile())
        {
            return App::PLUGIN_CHECK;
        }
    }

    return App::NORMAL;
}

void App::checkPlugin(const String &markerFile)
{
#if JUCE_MAC
    Process::setDockIconVisible(false);
#endif

    const File tempFile(FileUtils::getTemporaryFolder() + File::getSeparatorString() + markerFile);

    try
    {
        if (tempFile.existsAsFile())
        {
            if (tempFile.getSize() < 32768)
            {
                const String pluginPath = tempFile.loadFileAsString();

                // сразу удаляем файл, если плагин накосячит, хост об этом узнает
                tempFile.deleteFile();

                const File pluginFile(pluginPath);

                //if (pluginFile.existsAsFile()) // может быть и id
                {
                    KnownPluginList scanner;
                    OwnedArray<PluginDescription> typesFound;

                    AudioPluginFormatManager formatManager;
                    AudioCore::initAudioFormats(formatManager);

                    for (int i = 0; i < formatManager.getNumFormats(); ++i)
                    {
                        AudioPluginFormat *format = formatManager.getFormat(i);
                        scanner.scanAndAddFile(pluginPath, false, typesFound, *format);
                    }

                    // если мы дошли до сих пор, то все хорошо и плагин нас не обрушил
                    // так и запишем.
                    if (typesFound.size() != 0)
                    {
                        ScopedPointer<XmlElement> typesXml(new XmlElement(Serialization::Core::instrumentRoot));

                        for (auto i : typesFound)
                        {
                            typesXml->addChildElement(i->createXml());
                        }

                        DataEncoder::saveObfuscated(tempFile, typesXml);
                    }
                }
            }
        }
    }
    catch (...)
    {
        JUCEApplication::quit();
    }
}

void App::handleAsyncUpdate()
{
    this->quit();
}


void App::changeListenerCallback(ChangeBroadcaster *source)
{
    Logger::writeToLog("Reloading translations");
    this->recreateLayout();
}

#if defined TEST

#if JUCE_WINDOWS
#pragma comment(linker, "/subsystem:console")
#endif

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#else

START_JUCE_APPLICATION(App)

#endif
