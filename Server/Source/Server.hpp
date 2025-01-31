/*
 * Copyright (c) 2020 Andreas Pohl
 * Licensed under MIT (https://github.com/apohl79/audiogridder/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef Server_hpp
#define Server_hpp

#include <JuceHeader.h>
#include <set>
#include <thread>

#include "Worker.hpp"
#include "Defaults.hpp"
#include "ProcessorChain.hpp"
#include "Utils.hpp"
#include "json.hpp"
#include "ScreenRecorder.hpp"
#include "Sandbox.hpp"

namespace e47 {

using json = nlohmann::json;

class Server : public Thread, public LogTag {
  public:
    Server(json opts = {});
    ~Server() override;

    void initialize();
    void shutdown();
    void run() override;

    void loadConfig();
    void saveConfig();

    int getId(bool ignoreOpts = false) const;
    void setId(int i) { m_id = i; }
    const String& getName() const { return m_name; }
    void setName(const String& name);
    bool getEnableAU() const { return m_enableAU; }
    void setEnableAU(bool b) { m_enableAU = b; }
    bool getEnableVST3() const { return m_enableVST3; }
    void setEnableVST3(bool b) { m_enableVST3 = b; }
    bool getEnableVST2() const { return m_enableVST2; }
    void setEnableVST2(bool b) { m_enableVST2 = b; }
    const StringArray& getVST3Folders() const { return m_vst3Folders; }
    void setVST3Folders(const StringArray& folders) { m_vst3Folders = folders; }
    const StringArray& getVST2Folders() const { return m_vst2Folders; }
    void setVST2Folders(const StringArray& folders) { m_vst2Folders = folders; }
    bool getVSTNoStandardFolders() const { return m_vstNoStandardFolders; }
    void setVSTNoStandardFolders(bool b) { m_vstNoStandardFolders = b; }
    float getScreenQuality() const { return m_screenJpgQuality; }
    void setScreenQuality(float q) { m_screenJpgQuality = q; }
    bool getScreenDiffDetection() const { return m_screenDiffDetection; }
    void setScreenDiffDetection(bool b) { m_screenDiffDetection = b; }
    bool getScreenCapturingFFmpeg() const { return m_screenCapturingFFmpeg; }
    void setScreenCapturingFFmpeg(bool b) { m_screenCapturingFFmpeg = b; }
    ScreenRecorder::EncoderMode getScreenCapturingFFmpegEncoder() const { return m_screenCapturingFFmpegEncMode; }
    void setScreenCapturingFFmpegEncoder(ScreenRecorder::EncoderMode m) { m_screenCapturingFFmpegEncMode = m; }
    ScreenRecorder::EncoderQuality getScreenCapturingFFmpegQuality() const { return m_screenCapturingFFmpegQuality; }
    void setScreenCapturingFFmpegQuality(ScreenRecorder::EncoderQuality q) { m_screenCapturingFFmpegQuality = q; }
    bool getScreenCapturingOff() const { return m_screenCapturingOff; }
    void setScreenCapturingOff(bool b) { m_screenCapturingOff = b; }
    bool getScreenLocalMode() const { return m_screenLocalMode; }
    void setScreenLocalMode(bool b) { m_screenLocalMode = b; }
    bool getPluginWindowsOnTop() const { return m_pluginWindowsOnTop; }
    void setPluginWindowsOnTop(bool b) { m_pluginWindowsOnTop = b; }
    bool getScanForPlugins() const { return m_scanForPlugins; }
    void setScanForPlugins(bool b) { m_scanForPlugins = b; }
    bool getParallelPluginLoad() const { return m_parallelPluginLoad; }
    void setParallelPluginLoad(bool b) { m_parallelPluginLoad = b; }
    bool getSandboxing() const { return m_sandboxing; }
    void setSandboxing(bool b) { m_sandboxing = b; }
    bool getCrashReporting() const { return m_crashReporting; }
    void setCrashReporting(bool b) { m_crashReporting = b; }
    const KnownPluginList& getPluginList() const { return m_pluginlist; }
    KnownPluginList& getPluginList() { return m_pluginlist; }
    bool shouldExclude(const String& name);
    bool shouldExclude(const String& name, const std::vector<String>& include);
    auto& getExcludeList() { return m_pluginexclude; }
    void addPlugins(const std::vector<String>& names, std::function<void(bool)> fn);
    void saveKnownPluginList();

    static bool scanPlugin(const String& id, const String& format);

    void sandboxShowEditor();
    void sandboxHideEditor();

    // SandboxMaster
    void handleMessageFromSandbox(SandboxMaster&, const SandboxMessage&);
    void handleDisconnectFromSandbox(SandboxMaster&);

    // SandboxSlave
    void handleMessageFromMaster(const SandboxMessage&);
    void handleDisconnectedFromMaster();
    void handleConnectedToMaster();

    int getNumSandboxes() { return m_sandboxes.size(); }
    int getNumLoadedBySandboxes() {
        int sum = 0;
        for (auto c : m_sandboxLoadedCount) {
            sum += c;
        }
        return sum;
    }

  private:
    json m_opts;

    String m_host;
    int m_port = Defaults::SERVER_PORT;
    int m_id = 0;
    String m_name;
    StreamingSocket m_masterSocket;
    using WorkerList = Array<std::shared_ptr<Worker>>;
    WorkerList m_workers;
    KnownPluginList m_pluginlist;
    std::set<String> m_pluginexclude;
    bool m_enableAU = true;
    bool m_enableVST3 = true;
    bool m_enableVST2 = true;
    float m_screenJpgQuality = 0.9f;
    bool m_screenDiffDetection = true;
    bool m_screenCapturingFFmpeg = true;
    bool m_screenCapturingOff = false;
    bool m_screenLocalMode = false;
    bool m_pluginWindowsOnTop = false;
    ScreenRecorder::EncoderMode m_screenCapturingFFmpegEncMode = ScreenRecorder::WEBP;
    ScreenRecorder::EncoderQuality m_screenCapturingFFmpegQuality = ScreenRecorder::ENC_QUALITY_MEDIUM;
    StringArray m_vst3Folders;
    StringArray m_vst2Folders;
    bool m_vstNoStandardFolders;
    bool m_scanForPlugins = true;
    bool m_parallelPluginLoad = false;
    bool m_crashReporting = true;
    bool m_sandboxing = false;
    bool m_sandboxLogAutoclean = true;

    HashMap<String, std::shared_ptr<SandboxMaster>, DefaultHashFunctions, CriticalSection> m_sandboxes;
    Array<std::shared_ptr<SandboxMaster>> m_sandboxesForDeletion;
    std::mutex m_sandboxesForDeletionMtx;

    std::unique_ptr<SandboxSlave> m_sandboxController;

    HashMap<String, uint32, DefaultHashFunctions, CriticalSection> m_sandboxLoadedCount;

    std::atomic_bool m_sandboxReady{false};
    std::atomic_bool m_sandboxConnectedToMaster{false};
    HandshakeRequest m_sandboxConfig;
    String m_sandboxHasScreen;

    void scanNextPlugin(const String& id, const String& fmt);
    void scanForPlugins();
    void scanForPlugins(const std::vector<String>& include);

    void loadKnownPluginList();
    static void loadKnownPluginList(KnownPluginList& plist);
    static void saveKnownPluginList(KnownPluginList& plist);

    void runServer();
    void runSandbox();

    bool sendHandshakeResponse(StreamingSocket* sock, bool sandboxEnabled = false, int sandboxPort = 0);

    template <typename T>
    inline T getOpt(const String& name, T def) const {
        return jsonGetValue(m_opts, name, def);
    }

    ENABLE_ASYNC_FUNCTORS();
};

}  // namespace e47

#endif /* Server_hpp */
