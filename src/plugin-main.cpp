#include <obs-module.h>
#include <obs.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-audio-websocket", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "Audio WebSocket Streamer Filter";
}

struct AudioWebSocketFilter {
	obs_source_t *context;
	ix::WebSocketServer *server;
	int port;
	std::mutex server_mutex;

	AudioWebSocketFilter(obs_source_t *ctx) : context(ctx), server(nullptr), port(8080) {}

	~AudioWebSocketFilter() {
		stop_server();
	}

	void start_server(int new_port) {
		std::lock_guard<std::mutex> lock(server_mutex);
		if (server && port == new_port) return;

		stop_server_internal();

		port = new_port;
		server = new ix::WebSocketServer(port, "0.0.0.0");
		
		server->setOnClientMessageCallback([this](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg) {
			if (msg->type == ix::WebSocketMessageType::Open) {
				blog(LOG_INFO, "AudioWebSocketFilter: Client connected from %s", connectionState->getRemoteIp().c_str());
			} else if (msg->type == ix::WebSocketMessageType::Close) {
				blog(LOG_INFO, "AudioWebSocketFilter: Client disconnected from %s", connectionState->getRemoteIp().c_str());
			}
		});

		auto res = server->listen();
		if (!res.first) {
			blog(LOG_ERROR, "AudioWebSocketFilter: Failed to start WebSocket server on port %d: %s", port, res.second.c_str());
			delete server;
			server = nullptr;
		} else {
			server->start();
			blog(LOG_INFO, "AudioWebSocketFilter: WebSocket server started on port %d", port);
		}
	}

	void stop_server() {
		std::lock_guard<std::mutex> lock(server_mutex);
		stop_server_internal();
	}

	void stop_server_internal() {
		if (server) {
			server->stop();
			delete server;
			server = nullptr;
		}
	}

	void on_audio(struct obs_audio_data *audio) {
		std::lock_guard<std::mutex> lock(server_mutex);
		if (server) {
			// Get audio info to know frame size and channels
			struct obs_audio_info oai;
			if (!obs_get_audio_info(&oai)) return;
			
			// Assuming interleaved stereo 16-bit PCM for simple clients. 
			// OBS provides planar float internally.
			// To keep it simple, let's stream the raw float data from channel 0 for now.
			
			size_t frames = audio->frames;
			uint32_t bytes_per_frame = sizeof(float); // float planar
			size_t total_bytes = frames * bytes_per_frame;

			// Send Channel 0 (left or mono) raw float32 PCM data
			if (audio->data[0]) {
				std::string bin_data(reinterpret_cast<const char*>(audio->data[0]), total_bytes);
				
				// Broadcast to all connected clients
				for (auto client : server->getClients()) {
					client->sendBinary(bin_data);
				}
			}
		}
	}
};

static const char *filter_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return "Audio WebSocket Streamer";
}

static void *filter_create(obs_data_t *settings, obs_source_t *context)
{
	AudioWebSocketFilter *filter = new AudioWebSocketFilter(context);
	
	int port = (int)obs_data_get_int(settings, "port");
	if (port <= 0) port = 8080;
	
	filter->start_server(port);
	
	return filter;
}

static void filter_destroy(void *data)
{
	AudioWebSocketFilter *filter = static_cast<AudioWebSocketFilter*>(data);
	delete filter;
}

static void filter_update(void *data, obs_data_t *settings)
{
	AudioWebSocketFilter *filter = static_cast<AudioWebSocketFilter*>(data);
	int port = (int)obs_data_get_int(settings, "port");
	if (port > 0) {
		filter->start_server(port);
	}
}

static obs_properties_t *filter_properties(void *data)
{
	UNUSED_PARAMETER(data);
	obs_properties_t *props = obs_properties_create();
	obs_properties_add_int(props, "port", "WebSocket Port", 1024, 65535, 1);
	return props;
}

static void filter_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, "port", 8080);
}

static struct obs_audio_data *filter_audio(void *data, struct obs_audio_data *audio)
{
	AudioWebSocketFilter *filter = static_cast<AudioWebSocketFilter*>(data);
	filter->on_audio(audio);
	return audio;
}

struct obs_source_info audio_websocket_filter_info = {};

bool obs_module_load(void)
{
	audio_websocket_filter_info.id = "audio_websocket_filter";
	audio_websocket_filter_info.type = OBS_SOURCE_TYPE_FILTER;
	audio_websocket_filter_info.output_flags = OBS_SOURCE_AUDIO;
	audio_websocket_filter_info.get_name = filter_get_name;
	audio_websocket_filter_info.create = filter_create;
	audio_websocket_filter_info.destroy = filter_destroy;
	audio_websocket_filter_info.update = filter_update;
	audio_websocket_filter_info.get_properties = filter_properties;
	audio_websocket_filter_info.get_defaults = filter_defaults;
	audio_websocket_filter_info.filter_audio = filter_audio;

	obs_register_source(&audio_websocket_filter_info);
	return true;
}
