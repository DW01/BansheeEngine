//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanRenderTexture.h"
#include "BsVulkanFramebuffer.h"
#include "BsVulkanTexture.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"

namespace bs
{
	VulkanRenderTexture::VulkanRenderTexture(const RENDER_TEXTURE_DESC& desc)
		:RenderTexture(desc), mProperties(desc, false)
	{ 

	}

	namespace ct
	{
	VulkanRenderTexture::VulkanRenderTexture(const RENDER_TEXTURE_DESC& desc, UINT32 deviceIdx)
		:RenderTexture(desc, deviceIdx), mProperties(desc, false), mDeviceIdx(deviceIdx), mFramebuffer(nullptr)
	{
		
	}

	VulkanRenderTexture::~VulkanRenderTexture()
	{
		mFramebuffer->destroy();
	}

	void VulkanRenderTexture::initialize()
	{
		RenderTexture::initialize();

		VULKAN_FRAMEBUFFER_DESC fbDesc;
		fbDesc.width = mProperties.getWidth();
		fbDesc.height = mProperties.getHeight();
		fbDesc.layers = mProperties.getNumSlices();
		fbDesc.numSamples = mProperties.getMultisampleCount() > 1 ? mProperties.getMultisampleCount() : 1;
		fbDesc.offscreen = true;

		for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; ++i)
		{
			if (mColorSurfaces[i] == nullptr)
				continue;

			const SPtr<TextureView>& view = mColorSurfaces[i];
			VulkanTexture* texture = static_cast<VulkanTexture*>(mDesc.colorSurfaces[i].texture.get());

			VulkanImage* image = texture->getResource(mDeviceIdx);
			if (image == nullptr)
				continue;

			TextureSurface surface;
			surface.mipLevel = view->getMostDetailedMip();
			surface.numMipLevels = view->getNumMips();

			if (texture->getProperties().getTextureType() == TEX_TYPE_3D)
			{
				if(view->getFirstArraySlice() > 0)
					LOGERR("Non-zero array slice offset not supported when rendering to a 3D texture.");

				if (view->getNumArraySlices() > 1)
					LOGERR("Cannot specify array slices when rendering to a 3D texture.");

				surface.arraySlice = 0;
				surface.numArraySlices = mProperties.getNumSlices();

				fbDesc.color[i].baseLayer = 0;
			}
			else
			{
				surface.arraySlice = view->getFirstArraySlice();
				surface.numArraySlices = view->getNumArraySlices();

				fbDesc.color[i].baseLayer = view->getFirstArraySlice();
				fbDesc.layers = view->getNumArraySlices();
			}

			fbDesc.color[i].image = image;
			fbDesc.color[i].surface = surface;
			fbDesc.color[i].format = VulkanUtility::getPixelFormat(texture->getProperties().getFormat(),
																   texture->getProperties().isHardwareGammaEnabled());
		}

		if(mDepthStencilSurface != nullptr)
		{
			const SPtr<TextureView>& view = mDepthStencilSurface;
			VulkanTexture* texture = static_cast<VulkanTexture*>(mDesc.depthStencilSurface.texture.get());

			VulkanImage* image = texture->getResource(mDeviceIdx);
			if (image != nullptr)
			{
				TextureSurface surface;
				surface.mipLevel = view->getMostDetailedMip();
				surface.numMipLevels = view->getNumMips();

				if (texture->getProperties().getTextureType() == TEX_TYPE_3D)
				{
					if (view->getFirstArraySlice() > 0)
						LOGERR("Non-zero array slice offset not supported when rendering to a 3D texture.");

					if (view->getNumArraySlices() > 1)
						LOGERR("Cannot specify array slices when rendering to a 3D texture.");

					surface.arraySlice = 0;
					surface.numArraySlices = 1;

					fbDesc.depth.baseLayer = 0;
				}
				else
				{
					surface.arraySlice = view->getFirstArraySlice();
					surface.numArraySlices = view->getNumArraySlices();

					fbDesc.depth.baseLayer = view->getFirstArraySlice();
					fbDesc.layers = view->getNumArraySlices();
				}

				fbDesc.depth.image = image;
				fbDesc.depth.surface = surface;
				fbDesc.depth.format = VulkanUtility::getPixelFormat(texture->getProperties().getFormat(),
																	texture->getProperties().isHardwareGammaEnabled());
				fbDesc.depth.baseLayer = view->getFirstArraySlice();
			}
		}

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::instance());
		SPtr<VulkanDevice> device = rapi._getDevice(mDeviceIdx);

		mFramebuffer = device->getResourceManager().create<VulkanFramebuffer>(fbDesc);
	}

	void VulkanRenderTexture::getCustomAttribute(const String& name, void* data) const
	{
		if (name == "FB")
		{
			VulkanFramebuffer** fb = (VulkanFramebuffer**)data;
			*fb = mFramebuffer;
			return;
		}
	}		
	}
}