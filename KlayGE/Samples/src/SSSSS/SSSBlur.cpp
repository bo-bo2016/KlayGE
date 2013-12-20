#include <KlayGE/KlayGE.hpp>
#include <KlayGE/RenderEffect.hpp>

#include "SSSBlur.hpp"

using namespace KlayGE;

SSSBlurPP::SSSBlurPP()
		: PostProcess(L"SSSBlurPP")
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	mrt_support_ = (caps.max_simultaneous_rts > 1);

	input_pins_.push_back(std::make_pair("color_tex", TexturePtr()));
	input_pins_.push_back(std::make_pair("depth_tex", TexturePtr()));

	output_pins_.push_back(std::make_pair("output", TexturePtr()));

	params_.push_back(std::make_pair("strength", RenderEffectParameterPtr())); 
	params_.push_back(std::make_pair("correction", RenderEffectParameterPtr()));

	RenderEffectPtr effect = SyncLoadRenderEffect("SSS.fxml");
	blur_x_tech_ = effect->TechniqueByName("BlurX");
	if (mrt_support_)
	{
		std::string blur_y_name = "BlurY0";
		for (uint32_t i = 0; i < 6; ++ i)
		{
			blur_y_name[5] = static_cast<char>('0' + i);
			blur_y_techs_[i] = effect->TechniqueByName(blur_y_name);
		}
	}
	else
	{
		blur_y_techs_[0] = effect->TechniqueByName("BlurYWOMRT");
		std::string accum_name = "Accum0";
		for (uint32_t i = 0; i < 6; ++ i)
		{
			accum_name[5] = static_cast<char>('0' + i);
			accum_techs_[i] = effect->TechniqueByName(accum_name);
		}
	}
	this->Technique(blur_x_tech_);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	blur_x_fb_ = rf.MakeFrameBuffer();

	if (!mrt_support_)
	{
		blur_y_fb_ = rf.MakeFrameBuffer();
	}

	color_tex_param_ = technique_->Effect().ParameterByName("color_tex");
	step_param_ = technique_->Effect().ParameterByName("step");
}

void SSSBlurPP::InputPin(uint32_t index, TexturePtr const & tex)
{
	PostProcess::InputPin(index, tex);

	if (0 == index)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		blur_x_tex_ = rf.MakeTexture2D(tex->Width(0), tex->Height(0), 1, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		blur_y_tex_ = rf.MakeTexture2D(tex->Width(0), tex->Height(0), 1, 1, tex->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);

		blur_x_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blur_x_tex_, 0, 1, 0));
		blur_x_fb_->Attach(FrameBuffer::ATT_DepthStencil, frame_buffer_->Attached(FrameBuffer::ATT_DepthStencil));
		if (mrt_support_)
		{
			frame_buffer_->Attach(FrameBuffer::ATT_Color1, rf.Make2DRenderView(*blur_y_tex_, 0, 1, 0));
		}
		else
		{
			blur_y_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*blur_y_tex_, 0, 1, 0));
			blur_y_fb_->Attach(FrameBuffer::ATT_DepthStencil, frame_buffer_->Attached(FrameBuffer::ATT_DepthStencil));
		}
	}
}

void SSSBlurPP::Apply()
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	float sss_strength;
	params_[0].second->Value(sss_strength);
	for (uint32_t i = 0; i < 6; ++ i)
	{
		re.BindFrameBuffer(blur_x_fb_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color, Color(0.0f, 0.0f, 0.0f, 0), 1.0f, 0);
		technique_ = blur_x_tech_;
		*color_tex_param_ = (0 == i) ? this->InputPin(0) : blur_y_tex_;
		*step_param_ = float2(i * sss_strength / frame_buffer_->Width(), 0);
		this->Render();

		if (!mrt_support_)
		{
			re.BindFrameBuffer(blur_y_fb_);
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color, Color(0.0f, 0.0f, 0.0f, 0), 1.0f, 0);
			technique_ = blur_y_techs_[0];
			*color_tex_param_ = blur_x_tex_;
			*step_param_ = float2(0, i * sss_strength / frame_buffer_->Height());
			this->Render();

			re.BindFrameBuffer(frame_buffer_);
			technique_ = accum_techs_[i];
			*color_tex_param_ = blur_y_tex_;
			this->Render();
		}
		else
		{
			re.BindFrameBuffer(frame_buffer_);
			technique_ = blur_y_techs_[i];
			*color_tex_param_ = blur_x_tex_;
			*step_param_ = float2(0, i * sss_strength / frame_buffer_->Height());
			this->Render();
		}
	}
}