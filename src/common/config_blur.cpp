#include "config_blur.h"
#include "config_base.h"

std::string config_blur::generate_config_string(const BlurSettings& settings, bool concise) {
	std::ostringstream output;

	output << "[模糊设置 v" << BLUR_VERSION << "]" << "\n";

	// Blur section
	if (!concise || settings.blur) {
		output << "\n";
		output << "- 模糊设置" << "\n";
		output << "模糊: " << (settings.blur ? "true" : "false") << "\n";
		if (!concise || settings.blur) {
			output << "模糊量: " << settings.blur_amount << "\n";
			output << "模糊输出帧率: " << settings.blur_output_fps << "\n";
			output << "模糊权重: " << settings.blur_weighting << "\n";
			output << "模糊伽马: " << settings.blur_gamma << "\n";
		}
	}

	// Interpolation section
	if (!concise || settings.interpolate) {
		output << "\n";
		output << "- 插值设置" << "\n";
		output << "插值: " << (settings.interpolate ? "true" : "false") << "\n";
		if (!concise || settings.interpolate) {
			output << "插值帧率: " << settings.interpolated_fps << "\n";
			output << "插值方法: " << settings.interpolation_method << "\n";
		}
	}

	// Pre-interpolation section
	if (!concise || settings.pre_interpolate) {
		output << "\n";
		output << "- 预插值设置" << "\n";
		output << "预插值: " << (settings.pre_interpolate ? "true" : "false") << "\n";
		if (!concise || settings.pre_interpolate) {
			output << "预插值帧率: " << settings.pre_interpolated_fps << "\n";
		}
	}

	// Deduplication section
	if (!concise || settings.deduplicate) {
		output << "\n";
		output << "- 去重设置" << "\n";
		output << "去重: " << (settings.deduplicate ? "true" : "false") << "\n";
		if (!concise || settings.deduplicate) {
			output << "去重方法: " << settings.deduplicate_method << "\n";
		}
	}

	// Rendering section (always included)
	output << "\n";
	output << "- 渲染设置" << "\n";
	output << "编码预设: " << settings.encode_preset << "\n";
	output << "质量: " << settings.quality << "\n";
	if (!concise || settings.preview) {
		output << "预览: " << (settings.preview ? "true" : "false") << "\n";
	}
	if (!concise || settings.detailed_filenames) {
		output << "详细文件名: " << (settings.detailed_filenames ? "true" : "false") << "\n";
	}
	if (!concise || settings.copy_dates) {
		output << "复制日期: " << (settings.copy_dates ? "true" : "false") << "\n";
	}

	// GPU acceleration section
	if (!concise || settings.gpu_decoding || settings.gpu_interpolation || settings.gpu_encoding) {
		output << "\n";
		output << "- GPU加速" << "\n";
		output << "GPU解码: " << (settings.gpu_decoding ? "true" : "false") << "\n";
		output << "GPU插值: " << (settings.gpu_interpolation ? "true" : "false") << "\n";
		output << "GPU编码: " << (settings.gpu_encoding ? "true" : "false") << "\n";
	}

	// Timescale section
	if (!concise || settings.timescale) {
		output << "\n";
		output << "- 时间缩放" << "\n";
		output << "时间缩放: " << (settings.timescale ? "true" : "false") << "\n";
		if (!concise || settings.timescale) {
			output << "输入时间缩放: " << settings.input_timescale << "\n";
			output << "输出时间缩放: " << settings.output_timescale << "\n";
			if (!concise || settings.output_timescale_audio_pitch) {
				output << "调整时间缩放的音频音高: "
					   << (settings.output_timescale_audio_pitch ? "true" : "false") << "\n";
			}
		}
	}

	// Filters section
	if (!concise || settings.filters) {
		output << "\n";
		output << "- 滤镜" << "\n";
		output << "滤镜: " << (settings.filters ? "true" : "false") << "\n";
		if (!concise || settings.filters) {
			output << "亮度: " << settings.brightness << "\n";
			output << "饱和度: " << settings.saturation << "\n";
			output << "对比度: " << settings.contrast << "\n";
		}
	}

	// Advanced section
	if (!concise || settings.override_advanced) {
		output << "\n";
		output << "- 高级设置" << "\n";
		output << "高级设置: " << (settings.override_advanced ? "true" : "false") << "\n";

		if (!concise || settings.override_advanced) {
			output << "\n";
			output << "- 高级去重设置" << "\n";
			output << "去重范围: " << settings.advanced.deduplicate_range << "\n";
			output << "去重阈值: " << settings.advanced.deduplicate_threshold << "\n";

			output << "\n";
			output << "- 高级渲染设置" << "\n";
			output << "视频容器: " << settings.advanced.video_container << "\n";
			if (!concise || !settings.advanced.ffmpeg_override.empty()) {
				output << "自定义FFmpeg滤镜: " << settings.advanced.ffmpeg_override << "\n";
			}
			if (!concise || settings.advanced.debug) {
				output << "调试: " << (settings.advanced.debug ? "true" : "false") << "\n";
			}

			output << "\n";
			output << "- 高级模糊设置" << "\n";
			output << "模糊权重高斯标准差: " << settings.advanced.blur_weighting_gaussian_std_dev << "\n";
			output << "模糊权重高斯均值: " << settings.advanced.blur_weighting_gaussian_mean << "\n";
			output << "模糊权重高斯边界: " << settings.advanced.blur_weighting_gaussian_bound << "\n";

			output << "\n";
			output << "- 高级插值设置" << "\n";
			output << "SVP插值预设: " << settings.advanced.svp_interpolation_preset << "\n";
			output << "SVP插值算法: " << settings.advanced.svp_interpolation_algorithm << "\n";
			output << "插值块大小: " << settings.advanced.interpolation_blocksize << "\n";
			output << "插值遮罩区域: " << settings.advanced.interpolation_mask_area << "\n";
			output << "RIFE模型: " << settings.advanced.rife_model << "\n";

			if (!concise || settings.advanced.manual_svp) {
				output << "\n";
				output << "- 手动SVP覆盖" << "\n";
				output << "手动SVP: " << (settings.advanced.manual_svp ? "true" : "false") << "\n";
				if (!concise || settings.advanced.manual_svp) {
					output << "超级字符串: " << settings.advanced.super_string << "\n";
					output << "向量字符串: " << settings.advanced.vectors_string << "\n";
					output << "平滑字符串: " << settings.advanced.smooth_string << "\n";
				}
			}
		}
	}

	std::string result = output.str();

	// remove final newline if concise
	if (concise && !result.empty() && result.back() == '\n')
		result.pop_back();

	return result;
}

void config_blur::create(const std::filesystem::path& filepath, const BlurSettings& current_settings) {
	std::ofstream output(filepath);
	output << generate_config_string(current_settings, false);
}

std::string config_blur::export_concise(const BlurSettings& settings) {
	return generate_config_string(settings, true);
}

tl::expected<void, std::string> config_blur::validate(BlurSettings& config, bool fix) {
	std::set<std::string> errors;

	if (!u::contains(SVP_INTERPOLATION_PRESETS, config.advanced.svp_interpolation_preset)) {
		errors.insert(
			std::format("SVP插值预设({})不是有效选项", config.advanced.svp_interpolation_preset)
		);

		if (fix)
			config.advanced.svp_interpolation_preset = DEFAULT_CONFIG.advanced.svp_interpolation_preset;
	}

	if (!u::contains(SVP_INTERPOLATION_ALGORITHMS, config.advanced.svp_interpolation_algorithm)) {
		errors.insert(
			std::format(
				"SVP插值算法({})不是有效选项", config.advanced.svp_interpolation_algorithm
			)
		);

		if (fix)
			config.advanced.svp_interpolation_algorithm = DEFAULT_CONFIG.advanced.svp_interpolation_algorithm;
	}

	if (!u::contains(INTERPOLATION_BLOCK_SIZES, config.advanced.interpolation_blocksize)) {
		errors.insert(
			std::format("插值块大小({})不是有效选项", config.advanced.interpolation_blocksize)
		);

		if (fix)
			config.advanced.interpolation_blocksize = DEFAULT_CONFIG.advanced.interpolation_blocksize;
	}

	if (!errors.empty())
		return tl::unexpected(u::join(errors, " "));

	return {};
}

// 辅助函数：尝试用多个键名提取值
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

BlurSettings config_blur::parse(const std::string& config_content) {
	std::istringstream stream(config_content);
	auto config_map = config_base::read_config_map(stream);
	return parse_from_map(config_map);
}

BlurSettings config_blur::parse(const std::filesystem::path& config_filepath) {
	std::ifstream file_stream(config_filepath);
	auto config_map = config_base::read_config_map(file_stream);
	return parse_from_map(config_map, config_filepath);
}

BlurSettings config_blur::parse_from_map(
	const std::map<std::string, std::string>& config_map, const std::optional<std::filesystem::path>& config_filepath
) {
	BlurSettings settings;

	// 使用辅助函数提取值，支持中英文键名
	extract_config_bool_with_fallback(config_map, "模糊", "blur", settings.blur);
	extract_config_value_with_fallback(config_map, "模糊量", "blur amount", settings.blur_amount);
	extract_config_value_with_fallback(config_map, "模糊输出帧率", "blur output fps", settings.blur_output_fps);
	extract_config_string_with_fallback(config_map, "模糊权重", "blur weighting", settings.blur_weighting);
	extract_config_value_with_fallback(config_map, "模糊伽马", "blur gamma", settings.blur_gamma);

	extract_config_bool_with_fallback(config_map, "插值", "interpolate", settings.interpolate);
	extract_config_string_with_fallback(config_map, "插值帧率", "interpolated fps", settings.interpolated_fps);
	extract_config_string_with_fallback(config_map, "插值方法", "interpolation method", settings.interpolation_method);

	extract_config_bool_with_fallback(config_map, "预插值", "pre-interpolate", settings.pre_interpolate);
	extract_config_string_with_fallback(config_map, "预插值帧率", "pre-interpolated fps", settings.pre_interpolated_fps);

	extract_config_bool_with_fallback(config_map, "去重", "deduplicate", settings.deduplicate);
	extract_config_value_with_fallback(config_map, "去重方法", "deduplicate method", settings.deduplicate_method);

	extract_config_value_with_fallback(config_map, "编码预设", "encode preset", settings.encode_preset);
	extract_config_value_with_fallback(config_map, "质量", "quality", settings.quality);
	extract_config_bool_with_fallback(config_map, "预览", "preview", settings.preview);
	extract_config_bool_with_fallback(config_map, "详细文件名", "detailed filenames", settings.detailed_filenames);
	extract_config_bool_with_fallback(config_map, "复制日期", "copy dates", settings.copy_dates);

	extract_config_bool_with_fallback(config_map, "GPU解码", "gpu decoding", settings.gpu_decoding);
	extract_config_bool_with_fallback(config_map, "GPU插值", "gpu interpolation", settings.gpu_interpolation);
	extract_config_bool_with_fallback(config_map, "GPU编码", "gpu encoding", settings.gpu_encoding);

	extract_config_bool_with_fallback(config_map, "时间缩放", "timescale", settings.timescale);
	extract_config_value_with_fallback(config_map, "输入时间缩放", "input timescale", settings.input_timescale);
	extract_config_value_with_fallback(config_map, "输出时间缩放", "output timescale", settings.output_timescale);
	extract_config_bool_with_fallback(config_map, "调整时间缩放的音频音高", "adjust timescaled audio pitch", settings.output_timescale_audio_pitch);

	extract_config_bool_with_fallback(config_map, "滤镜", "filters", settings.filters);
	extract_config_value_with_fallback(config_map, "亮度", "brightness", settings.brightness);
	extract_config_value_with_fallback(config_map, "饱和度", "saturation", settings.saturation);
	extract_config_value_with_fallback(config_map, "对比度", "contrast", settings.contrast);

	extract_config_bool_with_fallback(config_map, "高级设置", "advanced", settings.override_advanced);

	if (settings.override_advanced) {
		extract_config_value_with_fallback(config_map, "去重范围", "deduplicate range", settings.advanced.deduplicate_range);
		extract_config_string_with_fallback(config_map, "去重阈值", "deduplicate threshold", settings.advanced.deduplicate_threshold);

		extract_config_value_with_fallback(config_map, "视频容器", "video container", settings.advanced.video_container);
		extract_config_string_with_fallback(config_map, "自定义FFmpeg滤镜", "custom ffmpeg filters", settings.advanced.ffmpeg_override);
		extract_config_bool_with_fallback(config_map, "调试", "debug", settings.advanced.debug);

		extract_config_value_with_fallback(config_map, "模糊权重高斯标准差", "blur weighting gaussian std dev", settings.advanced.blur_weighting_gaussian_std_dev);
		extract_config_value_with_fallback(config_map, "模糊权重高斯均值", "blur weighting gaussian mean", settings.advanced.blur_weighting_gaussian_mean);
		extract_config_string_with_fallback(config_map, "模糊权重高斯边界", "blur weighting gaussian bound", settings.advanced.blur_weighting_gaussian_bound);

		extract_config_string_with_fallback(config_map, "SVP插值预设", "svp interpolation preset", settings.advanced.svp_interpolation_preset);
		extract_config_string_with_fallback(config_map, "SVP插值算法", "svp interpolation algorithm", settings.advanced.svp_interpolation_algorithm);
		extract_config_string_with_fallback(config_map, "插值块大小", "interpolation block size", settings.advanced.interpolation_blocksize);
		extract_config_value_with_fallback(config_map, "插值遮罩区域", "interpolation mask area", settings.advanced.interpolation_mask_area);
		extract_config_string_with_fallback(config_map, "RIFE模型", "rife model", settings.advanced.rife_model);
		extract_config_bool_with_fallback(config_map, "手动SVP", "manual svp", settings.advanced.manual_svp);
		extract_config_string_with_fallback(config_map, "超级字符串", "super string", settings.advanced.super_string);
		extract_config_string_with_fallback(config_map, "向量字符串", "vectors string", settings.advanced.vectors_string);
		extract_config_string_with_fallback(config_map, "平滑字符串", "smooth string", settings.advanced.smooth_string);
	}

	u::verify_gpu_encoding(settings);
	u::set_fastest_rife_gpu(settings);

	if (config_filepath) {
		// rewrite config with proper structure and default values
		create(*config_filepath, settings);
	}

	return settings;
}

BlurSettings config_blur::parse_global_config() {
	return parse(get_global_config_path());
}

std::filesystem::path config_blur::get_global_config_path() {
	return blur.settings_path / CONFIG_FILENAME;
}

std::filesystem::path config_blur::get_config_filename(const std::filesystem::path& video_folder) {
	return video_folder / CONFIG_FILENAME;
}

BlurSettings config_blur::get_global_config() {
	return config_base::load_config<BlurSettings>(get_global_config_path(), create, parse);
}

config_blur::ConfigRes config_blur::get_config(const std::filesystem::path& config_filepath, bool use_global) {
	bool local_cfg_exists = std::filesystem::exists(config_filepath);

	auto global_cfg_path = get_global_config_path();
	bool global_cfg_exists = std::filesystem::exists(global_cfg_path);

	ConfigRes res;
	std::filesystem::path cfg_path;

	if (use_global && !local_cfg_exists && global_cfg_exists) {
		cfg_path = global_cfg_path;

		if (blur.verbose)
			u::log("Using global config");
	}
	else {
		// check if the config file exists, if not, write the default values
		if (!local_cfg_exists) {
			create(config_filepath);

			u::log("Configuration file not found, default config generated at {}", config_filepath);
		}

		cfg_path = config_filepath;
	}

	res.config = parse(cfg_path);
	res.is_global = (cfg_path == global_cfg_path);

	return res;
}

tl::expected<nlohmann::json, std::string> BlurSettings::to_json() const {
	nlohmann::json j;

	j["blur"] = this->blur;
	j["blur_amount"] = this->blur_amount;
	j["blur_output_fps"] = this->blur_output_fps;
	j["blur_weighting"] = this->blur_weighting;
	j["blur_gamma"] = this->blur_gamma;

	j["interpolate"] = this->interpolate;
	j["interpolated_fps"] = this->interpolated_fps;
	j["interpolation_method"] = this->interpolation_method;

	j["pre_interpolate"] = this->pre_interpolate;
	j["pre_interpolated_fps"] = this->pre_interpolated_fps;

	j["deduplicate"] = this->deduplicate;
	j["deduplicate_method"] = this->deduplicate_method;

	j["timescale"] = this->timescale;
	j["input_timescale"] = this->input_timescale;
	j["output_timescale"] = this->output_timescale;
	j["output_timescale_audio_pitch"] = this->output_timescale_audio_pitch;

	j["filters"] = this->filters;
	j["brightness"] = this->brightness;
	j["saturation"] = this->saturation;
	j["contrast"] = this->contrast;

	j["encode preset"] = this->encode_preset;
	j["quality"] = this->quality;
	j["preview"] = this->preview;
	j["detailed_filenames"] = this->detailed_filenames;
	// j["copy_dates"] = this->copy_dates;

	j["gpu_decoding"] = this->gpu_decoding;
	j["gpu_interpolation"] = this->gpu_interpolation;
	j["gpu_encoding"] = this->gpu_encoding;

	j["filters"] = this->filters;
	j["brightness"] = this->brightness;
	j["saturation"] = this->saturation;
	j["contrast"] = this->contrast;

	// advanced
	j["deduplicate_range"] = this->advanced.deduplicate_range;
	j["deduplicate_threshold"] = this->advanced.deduplicate_threshold;

	// j["video_container"] = this->advanced.video_container;
	// j["ffmpeg_override"] = this->advanced.ffmpeg_override;
	j["debug"] = this->advanced.debug;

	j["blur_weighting_gaussian_std_dev"] = this->advanced.blur_weighting_gaussian_std_dev;
	j["blur_weighting_gaussian_mean"] = this->advanced.blur_weighting_gaussian_mean;
	j["blur_weighting_gaussian_bound"] = this->advanced.blur_weighting_gaussian_bound;

	j["svp_interpolation_preset"] = this->advanced.svp_interpolation_preset;
	j["svp_interpolation_algorithm"] = this->advanced.svp_interpolation_algorithm;
	j["interpolation_blocksize"] = this->advanced.interpolation_blocksize;
	j["interpolation_mask_area"] = this->advanced.interpolation_mask_area;

	auto rife_model_path = get_rife_model_path();
	if (!rife_model_path)
		return tl::unexpected(rife_model_path.error());

	j["rife_model"] = *rife_model_path;

	j["manual_svp"] = this->advanced.manual_svp;
	j["super_string"] = this->advanced.super_string;
	j["vectors_string"] = this->advanced.vectors_string;
	j["smooth_string"] = this->advanced.smooth_string;

	return j;
}

BlurSettings::BlurSettings() {
	u::verify_gpu_encoding(*this);
}

// NOLINTBEGIN(readability-convert-member-functions-to-static) other platforms need it
tl::expected<std::filesystem::path, std::string> BlurSettings::get_rife_model_path() const {
	// NOLINTEND(readability-convert-member-functions-to-static)
	std::filesystem::path rife_model_path;

#if defined(_WIN32)
	rife_model_path = u::get_resources_path() / "lib/models" / this->advanced.rife_model;
#elif defined(__linux__)
	rife_model_path = u::get_resources_path() / "models" / this->advanced.rife_model;
#elif defined(__APPLE__)
	rife_model_path = u::get_resources_path() / "models" / this->advanced.rife_model;
#endif

	if (!std::filesystem::exists(rife_model_path))
		return tl::unexpected(std::format("RIFE模型'{}'未找到", this->advanced.rife_model));

	return rife_model_path;
}