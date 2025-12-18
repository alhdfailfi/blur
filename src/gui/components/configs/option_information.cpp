#include "configs.h"

#include "../../ui/ui.h"
#include "../../render/render.h"

namespace configs = gui::components::configs;

void configs::option_information(ui::Container& container) {
	const static std::unordered_map<std::string, std::vector<std::string>> option_explanations = {
		// Blur settings
		// { "section blur checkbox",
		//   {
		// 	  "Enable motion blur",
		//   }, },
		{
			"blur amount",
			{
				"运动模糊量",
				"(0 = 无模糊, 1 = 完全混合所有帧, >1 = 混合额外帧（重影）)",
			},
		},
		// { "output fps",
		//   {
		// 	  "FPS of the output video",
		//   }, },
		{
			"blur gamma",
			{
				"模糊前视频变暗的程度。使高光更突出",
			},
		},
		{
			"blur weighting gaussian std dev slider",
			{
				"高斯模糊权重的标准差",
			},
		},
		{
			"blur weighting triangle reverse checkbox",
			{
				"反转三角形加权的方向",
			},
		},
		{
			"blur weighting bound input",
			{
				"加权边界，用于更广泛地分布权重",
			},
		},

		// Interpolation settings
		// { "section interpolation checkbox",
		//   {
		// 	  "Enable interpolation to a higher FPS before blurring",
		//   }, },
		{
			"interpolate scale checkbox",
			{
				"使用FPS插值的倍数而非设定FPS",
			},
		},
		{
			"interpolated fps mult",
			{
				"FPS插值的倍数",
				"输入视频将插值到此FPS（模糊前）",
			},
		},
		{
			"interpolated fps",
			{
				"将输入视频插值到的FPS（模糊前）",
			},
		},
		{
			"interpolation method dropdown",
			{
				"质量：rife > svp",
				"速度：svp > rife",
			},
		},
		// pre-interp settings
		{
			"section pre-interpolation checkbox",
			{
				"启用预插值，使用更准确但更慢的AI模型进行主插值前的处理",
			},
		},
		{
			"pre-interpolated fps mult",
			{
				"FPS预插值的倍数",
				"输入视频将插值到此FPS（主插值和模糊前）",
			},
		},
		{
			"pre-interpolated fps",
			{
				"将输入视频预插值到的FPS（模糊前）",
			},
		},
		{
			"SVP interpolation preset dropdown",
			{
				"请查看blur GitHub获取更多信息",
			},
		},
		{
			"SVP interpolation algorithm dropdown",
			{
				"请查看blur GitHub获取更多信息",
			},
		},
		{
			"interpolation block size dropdown",
			{
				"插值的块大小",
				"(更高 = 准确性降低，速度更快；更低 = 准确性提高，速度更慢)",
			},
		},
		{
			"interpolation mask area slider",
			{
				"插值的遮罩量",
				"(更高可减少静态对象的模糊但可能影响平滑度)",
			},
		},

		// Rendering settings
		{
			"deduplicate checkbox",
			{
				"移除重复帧并用插值帧替换",
				"(修复由录制中的卡顿引起的'不流畅'输出)",
			},
		},
		{
			"deduplicate range",
			{
				"去重时在当前帧之外查找唯一帧的范围",
				"如果您的素材FPS低于应有值，请调高此值，例如以240fps录制的卡顿120fps游戏画面",
				"如果模糊后的素材开始模糊静态元素（如菜单屏幕），请降低此值",
			},
		},
		{
			"deduplicate threshold input",
			{
				"触发去重的运动阈值",
				"在高级设置中启用调试并渲染视频以嵌入显示每帧运动的文本",
			},
		},
		{
			"deduplicate method dropdown",
			{
				"质量：rife > svp",
				"速度：old > svp > rife",
			},
		},
		{
			"preview checkbox",
			{
				"渲染时显示预览",
			},
		},
		{
			"detailed filenames checkbox",
			{
				"将模糊设置添加到生成的文件名中",
			},
		},

		// Timescale settings
		// {
		// 	"section timescale checkbox",
		// 	{
		// 		"Enable video timescale manipulation",
		// 	},
		// },
		// {
		// 	"input timescale",
		// 	{
		// 		"Timescale of the input video file",
		// 	},
		// },
		// {
		// 	"output timescale",
		// 	{
		// 		"Timescale of the output video file",
		// 	},
		// },
		{
			"adjust timescaled audio pitch checkbox",
			{
				"加速或减速视频时对音频进行音高校正",
			},
		},

		// Filters
		// { "section filters checkbox", { "Enable video filters", }, },
		// { "brightness", { "Adjusts brightness of the output video", }, },
		// { "saturation", { "Adjusts saturation of the output video", }, },
		// { "contrast", { "Adjusts contrast of the output video", }, },

		// Advanced rendering
		// { "gpu interpolation checkbox", { "Uses GPU for interpolation", }, },
		// { "gpu encoding checkbox", { "Uses GPU for rendering", }, },
		// { "gpu encoding type dropdown", { "Select GPU type", }, },
		{
			"video container text input",
			{
				"输出视频容器格式",
			},
		},
		{
			"custom ffmpeg filters text input",
			{
				"用于渲染的自定义FFmpeg过滤器",
				"(覆盖GPU和质量选项)",
			},
		},
		// { "debug checkbox", { "Shows debug window and prints commands used by blur", } }
		{
			"copy dates checkbox",
			{
				"从输入文件复制修改日期",
			},
		},
	};

	std::string hovered = ui::get_hovered_id();

	if (hovered.empty())
		return;

	if (!option_explanations.contains(hovered))
		return;

	ui::add_hint("hovered option info", container, option_explanations.at(hovered), gfx::Color::white(), fonts::dejavu);
}