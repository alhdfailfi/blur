#include "rendering_frame.h"
#include "rendering.h"
#include "config_app.h"
#include "config_blur.h"
#include "blur.h"

#include <tl/expected.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <sstream>
#include <thread>
#include <chrono>
#include <ranges>
#include <format>

tl::expected<RenderCommands, std::string> FrameRender::build_render_commands(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_path,
    const BlurSettings& settings,
    const GlobalAppSettings& app_settings
) {
    std::filesystem::path blur_script_path = (blur.resources_path / "lib/blur.py");

    auto settings_json = settings.to_json();
    if (!settings_json)
        return tl::unexpected(settings_json.error());

    auto app_settings_json = app_settings.to_json();
    if (!app_settings_json)
        return tl::unexpected(app_settings_json.error());

    settings_json->update(*app_settings_json); // 从应用设置添加新键（并覆盖重复项）

#if defined(__linux__)
    bool vapoursynth_plugins_bundled = std::filesystem::exists(blur.resources_path / "vapoursynth-plugins");
#endif

    RenderCommands commands;

    std::wstring path_string = input_path.wstring();
    std::ranges::replace(path_string, '\\', '/');

    // 构建 vspipe 命令
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

    // 构建 ffmpeg 命令
    // clang-format off
    commands.ffmpeg = {
        L"-loglevel",
        L"error",
        L"-hide_banner",
        L"-stats",
        L"-ss",
        L"00:00:00.200", // 向前跳过一点，因为模糊需要上下文 todo: 这个值可以设多低？
        L"-y",
        L"-i",
        L"-", // 从视频脚本管道输出
        L"-vframes",
        L"1", // 渲染1帧
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
            DEBUG_LOG("VSPipe 命令: {} {}", blur.vspipe_path, u::tostring(u::join(render_commands.vspipe, L" ")));
            DEBUG_LOG("FFmpeg 命令: {} {}", blur.ffmpeg_path, u::tostring(u::join(render_commands.ffmpeg, L" ")));
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

        // 先声明为局部变量，然后移动或赋值
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
                DEBUG_LOG("帧渲染: 提前终止进程");
                m_to_kill = false;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if (vspipe_stderr_thread.joinable()) {
            vspipe_stderr_thread.join();
        }

        if (settings.advanced.debug)
            u::log(
                "vspipe 退出代码: {}, ffmpeg 退出代码: {}", vspipe_process.exit_code(), ffmpeg_process.exit_code()
            );

        if (ffmpeg_process.exit_code() != 0) { // || vspipe_process.exit_code() != 0;
                                               // todo: 检查为什么vspipe没有返回0
            remove_temp_path();
            return tl::unexpected(vspipe_stderr_output.str());
        }

        return {};
    }
    catch (const boost::system::system_error& e) {
        u::log_error("进程错误: {}", e.what());
        return tl::unexpected(e.what());
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
        return tl::unexpected("Blur 未初始化");

    if (!std::filesystem::exists(input_path)) {
        return tl::unexpected("输入路径不存在");
    }

    if (!create_temp_path()) {
        u::log("创建临时路径失败");
        return tl::unexpected("创建临时路径失败");
    }

    std::filesystem::path output_path = m_temp_path / "render.jpg";

    // 渲染
    auto render_commands = build_render_commands(input_path, output_path, settings, app_settings);
    if (!render_commands)
        return tl::unexpected(render_commands.error());

    auto render_res = do_render(*render_commands, settings);
    if (!render_res)
        return tl::unexpected(render_res.error());

    return output_path;
}