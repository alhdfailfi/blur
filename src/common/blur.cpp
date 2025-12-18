#include "blur.h"

#include "utils.h"
#include "rendering.h"
#include "updates.h"
#include "config_blur.h"
#include "config_app.h"
#include "config_presets.h"

tl::expected<void, std::string> Blur::initialise(bool _verbose, bool _using_preview) {
	resources_path = u::get_resources_path();
	settings_path = u::get_settings_path();

	auto global_blur_config_path = config_blur::get_global_config_path();
	if (!std::filesystem::exists(global_blur_config_path))
		config_blur::create(global_blur_config_path, config_blur::DEFAULT_CONFIG);

	auto app_config_path = config_app::get_app_config_path();
	if (!std::filesystem::exists(app_config_path))
		config_app::create(app_config_path, GlobalAppSettings{});

	auto preset_config_path = config_presets::get_preset_config_path();
	if (!std::filesystem::exists(preset_config_path))
		config_presets::create(preset_config_path, PresetSettings{});

#if defined(_WIN32)
	used_installer = std::filesystem::exists(resources_path / "lib\\vapoursynth\\vspipe.exe") &&
	                 std::filesystem::exists(resources_path / "lib\\ffmpeg\\ffmpeg.exe");
#elif defined(__linux__)
	// todo
	used_installer = false;
#elif defined(__APPLE__)
	used_installer = std::filesystem::exists(resources_path / "vapoursynth/vspipe") &&
	                 std::filesystem::exists(resources_path / "ffmpeg/ffmpeg");
#endif

	if (used_installer) {
#if defined(_WIN32)
		vspipe_path = (blur.resources_path / "lib\\vapoursynth\\vspipe.exe");
		ffmpeg_path = (blur.resources_path / "lib\\ffmpeg\\ffmpeg.exe");
		ffprobe_path = (blur.resources_path / "lib\\ffmpeg\\ffprobe.exe");
#elif defined(__linux__)
		// todo
#elif defined(__APPLE__)
		vspipe_path = (blur.resources_path / "vapoursynth/vspipe");
		ffmpeg_path = (blur.resources_path / "ffmpeg/ffmpeg");
		ffprobe_path = (blur.resources_path / "ffmpeg/ffprobe");
#endif

		const static std::string manual_troubleshooting_info = "请尝试重新下载最新安装程序。";

		// didn't use installer, check if dependencies are installed
		if (!std::filesystem::exists(ffmpeg_path)) {
			return tl::unexpected("未找到 FFmpeg。 " + manual_troubleshooting_info);
		}

		if (!std::filesystem::exists(ffprobe_path)) {
			return tl::unexpected("未找到 FFprobe。 " + manual_troubleshooting_info);
		}

		if (!std::filesystem::exists(vspipe_path)) {
			return tl::unexpected("未找到 VapourSynth。 " + manual_troubleshooting_info);
		}
	}
	else {
		const static std::string manual_troubleshooting_info =
			"如果不确定这是什么意思，请尝试使用安装程序。";

		// didn't use installer, check if dependencies are installed
		if (auto _ffmpeg_path = u::get_program_path("ffmpeg")) {
			ffmpeg_path = *_ffmpeg_path;
		}
		else {
			return tl::unexpected("FFmpeg could not be found. " + manual_troubleshooting_info);
		}

		if (auto _ffprobe_path = u::get_program_path("ffprobe")) {
			ffprobe_path = *_ffprobe_path;
		}
		else {
			return tl::unexpected("FFprobe could not be found. " + manual_troubleshooting_info);
		}

		if (auto _vspipe_path = u::get_program_path("vspipe")) {
			vspipe_path = *_vspipe_path;
		}
		else {
			return tl::unexpected("VapourSynth could not be found. " + manual_troubleshooting_info);
		}
	}

	verbose = _verbose;
	using_preview = _using_preview;

	setup_signal_handlers();

	int atexit_res = std::atexit([] {
		blur.in_atexit = true; // spdlog's already shut down or smth. Cancer
		blur.cleanup();
	});

	if (atexit_res != 0)
		DEBUG_LOG("注册 atexit 失败");

	initialise_base_temp_path();

	initialised = true;

	std::thread([this] {
		initialise_rife_gpus();
	}).detach();

	return {};
}

void Blur::initialise_base_temp_path() {
	temp_path = std::filesystem::temp_directory_path() / APPLICATION_NAME;
	int i = 0;
	while (true) {
		if (std::filesystem::exists(temp_path)) {
			temp_path = std::filesystem::temp_directory_path() / std::format("{}-{}", APPLICATION_NAME, ++i);
			continue;
		}

		std::filesystem::create_directory(temp_path);
		break;
	}
}

void Blur::cleanup() {
	// prevent multiple cleanup calls
	if (cleanup_performed.exchange(true))
		return;

	u::log("开始应用程序清理...");

	exiting = true;

	// stop renders
	rendering.stop_renders_and_wait();

	// remove temp dirs
	DEBUG_LOG( "正在移除临时路径 {}", temp_path);
	std::filesystem::remove_all(temp_path); // todo: is this unsafe lol

	u::log("应用程序清理完成");
}

std::optional<std::filesystem::path> Blur::create_temp_path(const std::string& folder_name) const {
	auto temp_dir = temp_path / folder_name;

	if (std::filesystem::exists(temp_dir)) {
		u::log("临时目录 {} 已存在，正在清除并重新创建", temp_path);
		remove_temp_path(temp_dir);
	}

	u::log("尝试创建临时目录 {}", temp_dir);

	if (!std::filesystem::create_directory(temp_dir))
		return {};

	u::log("已创建临时目录 {}", temp_dir);

	return temp_dir;
}

bool Blur::remove_temp_path(const std::filesystem::path& temp_path) {
	if (temp_path.empty())
		return false;

	if (!std::filesystem::exists(temp_path))
		return false;

	try {
		std::filesystem::remove_all(temp_path);
		u::log("已移除临时目录 {}", temp_path);

		return true;
	}
	catch (const std::filesystem::filesystem_error& e) {
		u::log_error("移除临时路径时出错: {}", e.what());
		return false;
	}
}

tl::expected<updates::UpdateCheckRes, std::string> Blur::check_updates() {
	auto config = config_app::get_app_config();
	if (!config.check_updates)
		return updates::UpdateCheckRes{};

	return updates::is_latest_version(config.check_beta);
}

void Blur::update(
	const std::string& tag,
	const std::optional<std::function<void(const std::string& text, bool done)>>& progress_callback
) {
	updates::update_to_tag(tag, progress_callback);
}

void Blur::initialise_rife_gpus() {
	rife_gpus = u::get_rife_gpus();

	std::ranges::copy(
		std::ranges::transform_view(
			rife_gpus,
			[](const auto& pair) {
				return pair.second;
			}
		),
		std::back_inserter(rife_gpu_names)
	);

	initialised_rife_gpus = true;
}

void cleanup_handler(int signal) {
	// Restore default handler immediately to prevent re-entry
	std::signal(signal, SIG_DFL);

	blur.cleanup();

	// Re-raise the signal for proper exit code
	std::raise(signal);
}

void Blur::setup_signal_handlers() {
	std::signal(SIGINT, cleanup_handler);
	std::signal(SIGTERM, cleanup_handler);
#ifndef _WIN32
	std::signal(SIGHUP, cleanup_handler);
#endif
}
