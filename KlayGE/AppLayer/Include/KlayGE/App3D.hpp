// App3D.hpp
// KlayGE App3D类 头文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2003-2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 增加了MakeWindow (2007.6.26)
//
// 3.1.0
// 增加了OnResize (2005.11.20)
//
// 3.0.0
// 增加了多遍更新 (2005.8.16)
//
// 2.7.0
// 增加了Quit (2005.6.28)
//
// 2.0.0
// 初次建立 (2003.7.10)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _APP3D_HPP
#define _APP3D_HPP

#define KLAYGE_LIB_NAME KlayGE_AppLayer
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/RenderSettings.hpp>

namespace KlayGE
{
	// 一个用于创建3D应用程序框架的基类。建立一个3D应用程序需要继承这个类，
	//			然后重载以下函数:
	//
	//			InitObjects()			- 初始化3D设备
	//			NumPasses()				- 返回场景需要绘制的遍数
	//			Update()				- 刷新场景
	//			DelObjects()			- 清除3D场景
	/////////////////////////////////////////////////////////////////////////////////
	class App3DFramework
	{
		friend class SceneManager;

	public:
		App3DFramework(std::string const & name, RenderSettings const & settings);
		virtual ~App3DFramework();

		virtual void Create();

		WindowPtr MakeWindow(std::string const & name, int32_t left, int32_t top,
			uint32_t width, uint32_t height);
		WindowPtr MainWnd() const
		{
			return main_wnd_;
		}

		Camera const & ActiveCamera() const;
		Camera& ActiveCamera();

		float FPS() const;

		void Run();
		void Quit();

		virtual void OnResize(uint32_t width, uint32_t height);

	protected:
		void LookAt(float3 const & eye, float3 const & lookAt, float3 const & up = float3(0, 1, 0));
		void Proj(float nearPlane, float farPlane);

	protected:
		virtual void InitObjects()
		{
		}

		virtual uint32_t NumPasses() const
		{
			return 1;
		}
		void Update(uint32_t pass);
		virtual void DoUpdate(uint32_t pass) = 0;
		void UpdateStats();

		virtual void RenderOver()
		{
		}

		virtual void DelObjects()
		{
		}

	protected:
		std::string name_;
		RenderSettings settings_;

		// Stats
		float	fps_;
		float	frame_time_;
		float	accumulate_time_;
		uint32_t num_frames_;

		Timer timer_;

		WindowPtr main_wnd_;
	};
}

#endif		// _APP3D_HPP
