#include "config_app.h"
#include "config_base.h"

// 辅助函数：尝试用多个键名提取值（与 config_blur.cpp 保持一致）
namespace {
template<typename T>
bool extract_config_value_with_fallback(const std::map<std::string, std::string>& config_map, 
                                       const std::string& cn_key, const std::string& en_key, T& value) {
    auto it = config_map.find(cn_key);
    if (it != config_map.end()) {
        try {
            std::stringstream ss(it->second);
            ss >> value;
            return !ss.fail();
        } catch (...) {
            // 忽略转换错误
        }
    }
    
    it = config_map.find(en_key);
    if (it != config_map.end()) {
        try {
            std::stringstream ss(it->second);
            ss >> value;
            return !ss.fail();
        } catch (...) {
            // 忽略转换错误
        }
    }
    
    return false;
}

bool extract_config_string_with_fallback(const std::map<std::string, std::string>& config_map,
                                        const std::string& cn_key, const std::string& en_key, std::string& value) {
    auto it = config_map.find(cn_key);
    if (it != config_map.end()) {
        value = it->second;
        return true;
    }
    
    it = config_map.find(en_key);
    if (it != config_map.end()) {
        value = it->second;
        return true;
    }
    
    return false;
}

bool extract_config_bool_with_fallback(const std::map<std::string, std::string>& config_map,
                                      const std::string& cn_key, const std::string& en_key, bool& value) {
    auto it = config_map.find(cn_key);
    if (it != config_map.end()) {
        std::string val_str = it->second;
        std::transform(val_str.begin(), val_str.end(), val_str.begin(), ::tolower);
        if (val_str == "true" || val_str == "1") {
            value = true;
            return true;
        } else if (val_str == "false" || val_str == "0") {
            value = false;
            return true;
        }
    }
    
    it = config_map.find(en_key);
    if (it != config_map.end()) {
        std::string val_str = it->second;
        std::transform(val_str.begin(), val_str.end(), val_str.begin(), ::tolower);
        if (val_str == "true" || val_str == "1") {
            value = true;
            return true;
        } else if (val_str == "false" || val_str == "0") {
            value = false;
            return true;
        }
    }
    
    return false;
}
} // 匿名命名空间

void config_app::create(const std::filesystem::path& filepath, const GlobalAppSettings& settings) {
	std::ofstream output(filepath);

	output << "[blur v" << BLUR_VERSION << "]" << "\n";

	output << "\n";
	output << "- PC特定模糊设置" << "\n";
	output << "输出前缀: " << settings.output_prefix << "\n";
	output << "显卡类型 (nvidia/amd/intel): " << settings.gpu_type << "\n";
	output << "RIFE显卡编号: " << settings.rife_gpu_index << "\n";

	output << "\n";
	output << "- 图形界面" << "\n";
	output << "模糊数量与FPS关联: " << (settings.blur_amount_tied_to_fps ? "true" : "false") << "\n";

	output << "\n";
	output << "- 桌面通知" << "\n";
	output << "渲染成功通知: " << (settings.render_success_notifications ? "true" : "false") << "\n";
	output << "渲染失败通知: " << (settings.render_failure_notifications ? "true" : "false") << "\n";

	output << "\n";
	output << "- 更新" << "\n";
	output << "检查更新: " << (settings.check_updates ? "true" : "false") << "\n";
	output << "包含测试版更新: " << (settings.check_beta ? "true" : "false") << "\n";

	output << "\n";
	output << "- 杂项" << "\n";
	output << "通知配置覆盖: " << (settings.notify_about_config_override ? "true" : "false") << "\n";

#ifdef __linux__
	output << "\n";
	output << "- linux" << "\n";
	output << "VapourSynth库路径: " << settings.vapoursynth_lib_path << "\n";
#endif
}

GlobalAppSettings config_app::parse(const std::filesystem::path& config_filepath) {
	std::ifstream file_stream(config_filepath);
	auto config_map = config_base::read_config_map(file_stream);

	GlobalAppSettings settings;

	// 使用辅助函数提取值，支持中英文键名
	extract_config_string_with_fallback(config_map, "输出前缀", "output_prefix", settings.output_prefix);
	extract_config_string_with_fallback(config_map, "显卡类型 (nvidia/amd/intel)", "gpu_type", settings.gpu_type);
	extract_config_value_with_fallback(config_map, "RIFE显卡编号", "rife_gpu_index", settings.rife_gpu_index);

	extract_config_bool_with_fallback(config_map, "模糊数量与FPS关联", "blur_amount_tied_to_fps", settings.blur_amount_tied_to_fps);
	extract_config_bool_with_fallback(config_map, "渲染成功通知", "render_success_notifications", settings.render_success_notifications);
	extract_config_bool_with_fallback(config_map, "渲染失败通知", "render_failure_notifications", settings.render_failure_notifications);
	extract_config_bool_with_fallback(config_map, "检查更新", "check_updates", settings.check_updates);
	extract_config_bool_with_fallback(config_map, "包含测试版更新", "check_beta", settings.check_beta);
	extract_config_bool_with_fallback(config_map, "通知配置覆盖", "notify_about_config_override", settings.notify_about_config_override);

#ifdef __linux__
	extract_config_string_with_fallback(config_map, "VapourSynth库路径", "vapoursynth_lib_path", settings.vapoursynth_lib_path);
#endif

	// 使用解析后的值重新创建配置文件（保持良好格式）
	create(config_filepath, settings);

	return settings;
}

std::filesystem::path config_app::get_app_config_path() {
	return blur.settings_path / APP_CONFIG_FILENAME;
}

GlobalAppSettings config_app::get_app_config() {
	return config_base::load_config<GlobalAppSettings>(get_app_config_path(), create, parse);
}

tl::expected<nlohmann::json, std::string> GlobalAppSettings::to_json() const {
	nlohmann::json j;

	j["gpu_type"] = this->gpu_type;
	j["rife_gpu_index"] = this->rife_gpu_index;

	return j;
}