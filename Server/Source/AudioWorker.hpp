/*
 * Copyright (c) 2020 Andreas Pohl
 * Licensed under MIT (https://github.com/apohl79/audiogridder/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef AudioWorker_hpp
#define AudioWorker_hpp

#include <JuceHeader.h>
#include <thread>
#include <unordered_map>

#include "ProcessorChain.hpp"
#include "Message.hpp"
#include "Utils.hpp"
#include "ChannelMapper.hpp"

namespace e47 {

struct audio_chunk_hdr_t {
    int channels;
    int samples;
    bool isDouble;
};

class ProcessorChain;

class AudioWorker : public Thread, public LogTagDelegate {
  public:
    AudioWorker(LogTag* tag);
    virtual ~AudioWorker() override;

    void init(std::unique_ptr<StreamingSocket> s, int channelsIn, int channelsOut, int channelsSC,
              uint64 activeChannels, double rate, int samplesPerBlock, bool doublePrecission);

    void run() override;
    void shutdown();
    void clear();

    int getChannelsIn() const { return m_channelsIn; }
    int getChannelsOut() const { return m_channelsOut; }
    int getChannelsSC() const { return m_channelsSC; }

    bool addPlugin(const String& id, String& err);
    void delPlugin(int idx);
    void exchangePlugins(int idxA, int idxB);
    std::shared_ptr<AGProcessor> getProcessor(int idx) const { return m_chain->getProcessor(idx); }
    int getSize() const { return static_cast<int>(m_chain->getSize()); }
    int getLatencySamples() const { return m_chain->getLatencySamples(); }
    void update() { m_chain->update(); }
    bool isSidechainDisabled() const { return m_chain->isSidechainDisabled(); }

    float getParameterValue(int idx, int paramIdx) { return m_chain->getParameterValue(idx, paramIdx); }

    struct ComparablePluginDescription : PluginDescription {
        ComparablePluginDescription(const PluginDescription& other) : PluginDescription(other) {}
        bool operator==(const ComparablePluginDescription& other) const { return isDuplicateOf(other); }
    };

    using RecentsListType = Array<ComparablePluginDescription>;
    String getRecentsList(String host) const;
    void addToRecentsList(const String& id, const String& host);

  private:
    std::unique_ptr<StreamingSocket> m_socket;
    int m_channelsIn;
    int m_channelsOut;
    int m_channelsSC;
    ChannelSet m_activeChannels;
    ChannelMapper m_channelMapper;
    double m_rate;
    int m_samplesPerBlock;
    bool m_doublePrecission;
    std::shared_ptr<ProcessorChain> m_chain;
    static std::unordered_map<String, RecentsListType> m_recents;
    static std::mutex m_recentsMtx;

    AudioBuffer<float> m_procBufferF;
    AudioBuffer<double> m_procBufferD;

    template <typename T>
    AudioBuffer<T>* getProcBuffer();

    template <>
    AudioBuffer<float>* getProcBuffer() {
        return &m_procBufferF;
    }

    template <>
    AudioBuffer<double>* getProcBuffer() {
        return &m_procBufferD;
    }

    template <typename T>
    void processBlock(AudioBuffer<T>& buffer, MidiBuffer& midi);

    ENABLE_ASYNC_FUNCTORS();
};

}  // namespace e47

#endif /* AudioWorker_hpp */
