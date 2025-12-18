﻿#include "rendering_frame.h"
#include "rendering.h"
#include "config_app.h"

// 注意：根据头文件，build_render_commands是静态成员函数
// 但在你的实现中，你把它当作非静态成员函数实现了
// 这里需要统一：要么在头文件中移除static，要么在实现中加上static
// 根据头文件，它应该是静态的，所以实现应该匹配

tl::expected<RenderCommands, std::string> FrameRender::build_render_commands(
	const std::filesystem::path& input_path,
	const std::filesystem::path& output_path,
	const BlurSettings& settings,
	const GlobalAppSettings& app_settings
) {
	std::filesystem::path blur_script_path = (blur.resources_path / "lib/blur.py");

	auto settings_json = settings.to_json();
	if (!settings_json)
		return tl::unexpected<std::string>(settings_json.error());

	auto app_settings_json = app_settings.to_json();
	if (!app_settings_json)
		return tl::unexpected<std::string>(app_settings_json.error());

	settings_json->update(*app_settings_json); // adds new keys from app settings (and overrides dupes)

#if defined(__linux__)
	bool vapoursynth_plugins_bundled = std::filesystem::exists(blur.resources_path / "vapoursynth-plugins");
#endif

	RenderCommands commands;

	std::wstring path_string = input_path.wstring();
	std::ranges::replace(path_string, L'\\', L'/');

	// Build vspipe command
	commands.vspipe = { L"-p",
		                L"-c",
		                L"y4m",
		                L"-a",
		                L"video_path=" + path_string,
		                L"-a",
		                L"settings=" + u::towstring(settings_json->dump()),
#if defined(__APPLE__)
		                L"-a",
		                std::format(L"macos_bundled={}", blur.used_installer ? L"true" : L"false"),
#endif
#if defined(_WIN32)
		                L"-a",
		                L"enable_lsmash=true",
#endif
#if defined(__linux__)
		                L"-a",
		                std::format(L"linux_bundled={}", vapoursynth_plugins_bundled ? L"true" : L"false"),
#endif
		                blur_script_path.wstring(),
		                L"-" };

	// Build ffmpeg command
	// clang-format off
	commands.ffmpeg = {
		L"-loglevel",
		L"error",
		L"-hide_banner",
		L"-stats",
		L"-ss",
		L"00:00:00.200", // skip forward a bit because blur needs context todo: how low can this go?
		L"-y",
		L"-i",
		L"-", // piped output from video script
		L"-vframes",
		L"1", // render 1 frame
		L"-q:v",
		L"2",
		L"-y",
		output_path.wstring(),
	};
	// clang-format on

	return commands;
}

tl::expected<void, std::string> FrameRender::do_render(RenderCommands render_commands, const BlurSettings& settings) {
	namespace bp = boost::process;

	std::ostringstream vspipe_stderr_output;

	try {
		boost::asio::io_context io_context;
		bp::pipe vspipe_stdout;
		bp::ipstream vspipe_stderr;
		bp::ipstream ffmpeg_stderr;

#ifndef _DEBUG
		if (settings.advanced.debug) {
#endif
			DEBUG_LOG("VSPipe command: {} {}", blur.vspipe_path, u::tostring(u::join(render_commands.vspipe, L" ")));
			DEBUG_LOG("FFmpeg command: {} {}", blur.ffmpeg_path, u::tostring(u::join(render_commands.ffmpeg, L" ")));
#ifndef _DEBUG
		}
#endif

		bp::environment env = boost::this_process::environment();

#if defined(__APPLE__)
		if (blur.used_installer) {
			env["PYTHONHOME"] = (blur.resources_path / "python").native();
			env["PYTHONPATH"] = (blur.resources_path / "python/lib/python3.12/site-packages").native();
		}
#endif

#if defined(__linux__)
		auto app_config = config_app::get_app_config();
		if (!app_config.vapoursynth_lib_path.empty()) {
			env["LD_LIBRARY_PATH"] = app_config.vapoursynth_lib_path;
			env["PYTHONPATH"] = app_config.vapoursynth_lib_path + "/python3.12/site-packages";
		}
#endif

		// Declare as local variables first, then move or assign
		auto vspipe_process = bp::child(
			boost::filesystem::path{ blur.vspipe_path },
			bp::args(render_commands.vspipe),
			bp::std_out > vspipe_stdout,
			bp::std_err > vspipe_stderr,
			env,
			io_context
#ifdef _WIN32
			,
			bp::windows::create_no_window
#endif
		);

		auto ffmpeg_process = bp::child(
			boost::filesystem::path{ blur.ffmpeg_path },
			bp::args(render_commands.ffmpeg),
			bp::std_in < vspipe_stdout,
			bp::std_out.null(),
			bp::std_err.null(),
			io_context
#ifdef _WIN32
			,
			bp::windows::create_no_window
#endif
		);

		std::thread vspipe_stderr_thread([&]() {
			std::string line;
			while (std::getline(vspipe_stderr, line)) {
				vspipe_stderr_output << line << '\n';
			}
		});

		while (vspipe_process.running() || ffmpeg_process.running()) {
			if (m_to_kill) {
				ffmpeg_process.terminate();
				vspipe_process.terminate();
				DEBUG_LOG("frame render: killed processes early");
				m_to_kill = false;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		if (vspipe_stderr_thread.joinable()) {
			vspipe_stderr_thread.join();
		}

		if (settings.advanced.debug)
			u::log(
				"vspipe exit code: {}, ffmpeg exit code: {}", vspipe_process.exit_code(), ffmpeg_process.exit_code()
			);

		if (ffmpeg_process.exit_code() != 0) { // || vspipe_process.exit_code() != 0;
			                                   // todo: check why vspipe isnt returning 0
			remove_temp_path();
			return tl::unexpected<std::string>(vspipe_stderr_output.str());
		}

		return {};
	}
	catch (const boost::system::system_error& e) {
		u::log_error("Process error: {}", e.what());
		return tl::unexpected<std::string>(e.what());
	}
}

bool FrameRender::create_temp_path() {
	static std::atomic<size_t> id = 0;
	auto temp_path = blur.create_temp_path(std::format("config-preview-{}", id++));

	if (!temp_path)
		return false;

	m_temp_path = *temp_path;
	return true;
}

bool FrameRender::remove_temp_path() {
	bool res = Blur::remove_temp_path(m_temp_path);
	if (res)
		m_temp_path.clear();
	return res;
}

tl::expected<std::filesystem::path, std::string> FrameRender::render(
	const std::filesystem::path& input_path, const BlurSettings& settings, const GlobalAppSettings& app_settings
) {
	if (!blur.initialised)
		return tl::unexpected<std::string>("Blur未初始化");

	if (!std::filesystem::exists(input_path)) {
		return tl::unexpected<std::string>("输入路径不存在");
	}

	if (!create_temp_path()) {
		u::log("创建临时路径失败");
		return tl::unexpected<std::string>("创建临时路径失败");
	}

	std::filesystem::path output_path = m_temp_path / "render.jpg";

	// render
	auto render_commands = build_render_commands(input_path, output_path, settings, app_settings);
	if (!render_commands)
		return tl::unexpected<std::string>(render_commands.error());

	auto render_res = do_render(*render_commands, settings);
	if (!render_res)
		return tl::unexpected<std::string>(render_res.error());

	return output_path;
}