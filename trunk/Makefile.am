sbin_PROGRAMS = telepresence

telepresence_CPPFLAGS = \
${FFMPEG_INCLUDES_FALLBACK} \
${OPENAL_INCLUDES_FALLBACK} \
${TINYSAK_CFLAGS} \
${TINYNET_CFLAGS} \
${TINYHTTP_CFLAGS} \
${TINYSIP_CFLAGS} \
${TINYDAV_CFLAGS} \
${TINYSDP_CFLAGS} \
${TINYSIGCOMP_CFLAGS} \
${TINYMEDIA_CFLAGS} \
${TINYXCAP_CFLAGS} \
${TINYSMS_CFLAGS} \
${TINYMSRP_CFLAGS} \
${TINYRTP_CFLAGS} \
${TINYIPSEC_CFLAGS} \
${LIBAVCODEC_CFLAGS} \
${LIBAVFILTER_CFLAGS} \
${LIBAVFORMAT_CFLAGS} \
${LIBAVUTIL_CFLAGS} \
${LIBSWRESAMPLE_CFLAGS} \
${LIBSWSCAL_CFLAGS}  \
${OPENAL_CFLAGS} \
-Itinywrap \
-Iinclude

telepresence_LDADD = \
${DOUBANGO_LIBS_FALLBACK} \
${FFMPEG_LIBS_FALLBACK} \
${OPENAL_LIBS_FALLBACK} \
${TINYSAK_LIBS} \
${TINYNET_LIBS} \
${TINYHTTP_LIBS} \
${TINYSIP_LIBS} \
${TINYDAV_LIBS} \
${TINYSDP_LIBS} \
${TINYSIGCOMP_LIBS} \
${TINYMEDIA_LIBS} \
${TINYXCAP_LIBS} \
${TINYSMS_LIBS} \
${TINYMSRP_LIBS} \
${TINYRTP_LIBS} \
${TINYIPSEC_LIBS} \
${LIBAVCODEC_LIBS} \
${LIBAVFILTER_LIBS} \
${LIBAVFORMAT_LIBS} \
${LIBAVUTIL_LIBS} \
${LIBSWRESAMPLE_LIBS} \
${LIBSWSCAL_LIBS} \
${OPENAL_LIBS}

telepresence_SOURCES  = \
	source/main.cc \
	source/OTBridge.cc \
	source/OTBridgeInfo.cc \
	source/OTCodec.cc \
	source/OTCodecAudio.cc \
	source/OTCodecVideo.cc \
	source/OTEngine.cc \
	source/OTFrame.cc \
	source/OTFrameAudio.cc \
	source/OTFrameVideo.cc \
	source/OTMixerMgr.cc \
	source/OTMixerMgrAudio.cc \
	source/OTMixerMgrMgr.cc \
	source/OTMixerMgrVideo.cc \
	source/OTMutex.cc \
	source/OTObject.cc \
	source/OTProcess.cc \
	source/OTProxyPlugin.cc \
	source/OTProxyPluginConsumer.cc \
	source/OTProxyPluginConsumerAudio.cc \
	source/OTProxyPluginConsumerVideo.cc \
	source/OTProxyPluginMgr.cc \
	source/OTProxyPluginProducer.cc \
	source/OTProxyPluginProducerAudio.cc \
	source/OTProxyPluginProducerVideo.cc \
	source/OTResampler.cc \
	source/OTSessionInfo.cc \
	source/OTWrap.cc
	
telepresence_SOURCES  += \
	source/cfg/OTCfg.cc \
	source/cfg/OTCfgParser.cc
	
telepresence_SOURCES  += \
	source/docstreamer/OTDocStreamer.cc \
	source/docstreamer/OTDocStreamerOpenOffice.cc

telepresence_SOURCES  += \
	source/filters/OTFilter.cc \
	source/filters/OTOverlay.cc
	
telepresence_SOURCES  += \
	source/jsoncpp/jsoncpp.cc

telepresence_SOURCES  += \
	source/mixers/OTMixer.cc \
	source/mixers/OTMixerAudio.cc \
	source/mixers/OTMixerAudio2D.cc \
	source/mixers/OTMixerAudio3D.cc \
	source/mixers/OTMixerVideo.cc \
	source/mixers/OTMixerVideo2D.cc \
	source/mixers/OTMixerVideo3D.cc
	
telepresence_SOURCES  += \
	source/nettransport/OTHttpTransport.cc \
	source/nettransport/OTNetTransport.cc

telepresence_SOURCES  += \
	source/patterns/OTPattern.cc \
	source/patterns/OTPatternVideo.cc \
	source/patterns/OTPatternVideoHangout.cc \
	source/patterns/OTPatternVideoWebEx.cc

telepresence_SOURCES  += \
	source/platforms/generic/OTEngineGen.cc \
	source/platforms/osx/OTEngineOSX.cc \
	source/platforms/win32/OTEngineWin32.cc

telepresence_SOURCES  += \
	source/presence/OTPresenceNotifer.cc \
	source/presence/OTPresenceParser.cc

telepresence_SOURCES  += \
	source/recorder/OTRecorder.cc \
	source/recorder/OTWebM.cc

telepresence_SOURCES  += \
	tinywrap/ActionConfig.cxx\
	tinywrap/AudioResampler.cxx\
	tinywrap/DDebug.cxx\
	tinywrap/MediaContent.cxx\
	tinywrap/MediaSessionMgr.cxx\
	tinywrap/Msrp.cxx\
	tinywrap/ProxyConsumer.cxx\
	tinywrap/ProxyPluginMgr.cxx\
	tinywrap/ProxyProducer.cxx\
	tinywrap/SafeObject.cxx\
	tinywrap/SipCallback.cxx\
	tinywrap/SipEvent.cxx\
	tinywrap/SipMessage.cxx\
	tinywrap/SipSession.cxx\
	tinywrap/SipStack.cxx\
	tinywrap/SipUri.cxx\
	tinywrap/SMSEncoder.cxx\
	tinywrap/Xcap.cxx


telepresence_LDFLAGS = -Wl,-rpath,$$ORIGIN


if USE_OPENOFFICE
PRJ=$(OPENOFFICE_ROOT)/sdk
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk
include $(SETTINGS)/stdtarget.mk


telepresence_CPPFLAGS += ${CC_DEFINES} -I${PRJ}/includecpp -I${PRJ}/include
telepresence_LDADD += ${CPPUHELPERLIB} ${CPPULIB} ${SALHELPERLIB} ${SALLIB}
telepresence_LDFLAGS += -Wl,-rpath,${OPENOFFICE_ROOT}/sdk/lib:${OPENOFFICE_ROOT}/program
endif

samples:
	cp -f ./telepresence.cfg ${exec_prefix}/sbin
	cp -f --parents ./fonts/truetype/freefont/* ${exec_prefix}/sbin
