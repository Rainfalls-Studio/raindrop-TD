#pragma once

#include <vulkan/vulkan.h>
#include "../LogicalDevice.hpp"

#include <vector>
#include <initializer_list>

namespace rnd::render::vulkan{
	struct FramebufferAttachments{
		struct Attachment{
			VkFormat format;
			VkImage image = VK_NULL_HANDLE;
			VkImageView imageView = VK_NULL_HANDLE;
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
			VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
			VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		};

		FramebufferAttachments() = default;
		FramebufferAttachments(std::initializer_list<Attachment> attachments){
 			this->attachments.resize(attachments.size());
			size_t i=0;
			for (auto &a : attachments){
				this->attachments[i] = a;
			}
		}

		FramebufferAttachments& operator=(const FramebufferAttachments &other){
			attachments.resize(other.attachments.size());
			for (int i=0; i<static_cast<int>(attachments.size()); i++){
				attachments[i] = other.attachments[i];
			}
			return *this;
		}

		std::vector<Attachment> attachments;
	};

	class FramebufferBuilder{
		friend class Framebuffer;
		friend class RenderTarget;
		friend class RenderTargetBuilder;
		public:
			FramebufferBuilder() = default;

			void setAttachments(FramebufferAttachments attachments);
			void addAttachmentImage(uint8_t attachmentIndex, VkImage image);
			void addAttachmentImageView(uint8_t attachmentIndex, VkImageView imageView);
			void addRenderPass(VkRenderPass renderPass);
			void setExtent(VkExtent2D extent);
			void setLogicalDevice(LogicalDevice* device);

			void enableDepthAttachment(VkFormat depthFormat, bool enabled = true);
			void addDepthAttachmentImage(VkImage depthImage);
			void addDepthAttachmentImageView(VkImageView dephtImageView);
			
		private:
			LogicalDevice* device = nullptr;
			VkExtent2D extent;
			VkRenderPass renderPass = VK_NULL_HANDLE;
			FramebufferAttachments attachments{};

			bool depthBufferEnabled = false;
			VkFormat depthFormat;
			VkImage depthImage = VK_NULL_HANDLE;
			VkImageView depthImageView = VK_NULL_HANDLE;
			VkMemoryPropertyFlags depthMemoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

			VkAttachmentLoadOp depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			VkAttachmentStoreOp depthStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			VkAttachmentLoadOp depthStencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			VkAttachmentStoreOp depthStencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	};
}