#include "render/vulkan/DescriptorSet.hpp"
#include "render/vulkan/DescriptorWriter.hpp"

#include <unordered_map>
#include <stdexcept>
#include <cassert>

namespace rnd::render::vulkan{

	static inline VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment){
		if (minOffsetAlignment > 0) {
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		}
		return instanceSize;
	}

	DescriptorSet::~DescriptorSet(){
		if (device){
			delete[] sets;
			delete[] buffers;
			if (buffer) delete[] buffer;
		}
	}

	void DescriptorSet::init(DescriptorSetBuilder &builder){
		assert(builder.device != nullptr && "cannot create a descriptor set without a valid device");
		device = builder.device;
		sets = new VkDescriptorSet[builder.descriptorSetCount];
		buffers = new BufferRange[builder.descriptorSetCount];

		descriptorSetCount = builder.descriptorSetCount;

		createPool(builder);
		createSetLayout(builder);
		createBuffers(builder);
		createDescriptorSets(builder);

		this->builder = builder;
	}

	void DescriptorSet::createPool(DescriptorSetBuilder &builder){
		DescriptorPoolBuilder poolBuilder;

		poolBuilder.setMaxSets(builder.descriptorSetCount);
		poolBuilder.setDevice(builder.device);

		std::unordered_map<VkDescriptorType, uint32_t> typeToCount;
		for (auto &d : builder.descriptors){
			typeToCount[d.type] += d.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ? d.imageCount : 1;
		}

		for (auto &t : typeToCount){
			poolBuilder.addPoolSize(t.first, t.second * builder.descriptorSetCount);
		}

		pool.init(poolBuilder);
	}

	void DescriptorSet::createSetLayout(DescriptorSetBuilder &builder){
		DescriptorSetLayoutBuilder layoutBuilder;

		layoutBuilder.setDevice(builder.device);

		for (auto &d : builder.descriptors){
			layoutBuilder.addBinding(d.binding, d.type, d.stage, d.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ? d.imageCount : 1);
		}

		layout.init(layoutBuilder);
	}

	void DescriptorSet::createBuffers(DescriptorSetBuilder &builder){
		uint32_t offset = 0;
		for (uint32_t i=0; i<builder.descriptors.size(); i++){
			auto &d = builder.descriptors[i];

			if (d.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER){
				buffers[i].size = d.bufferSize;
				buffers[i].alignement = getAlignment(d.bufferSize, device->getPhysicalDevice()->getProperties().limits.minUniformBufferOffsetAlignment);

				buffers[i].offset = offset;

				offset += buffers[i].alignement;
			}
		}

		if (offset){
			buffer = new Buffer[descriptorSetCount];

			for (uint32_t i=0; i<descriptorSetCount; i++){
				buffer[i].init(builder.device);
				buffer[i].alloc(offset, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				buffer[i].map();
			}

			offset = 0;
			for (uint32_t i=0; i<builder.descriptors.size(); i++){
				auto &d = builder.descriptors[i];

				if (d.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER){
					VkDescriptorBufferInfo info = {};
					info.offset = offset;
					info.range = d.bufferSize;
					
					d.bufferInfo = info;
					offset += d.bufferSize;
				} 
			}
		}
	}

	void DescriptorSet::createDescriptorSets(DescriptorSetBuilder &builder){
		for (uint32_t i=0; i<descriptorSetCount; i++){
			DescriptorWriter writer(device, &layout, &pool);
			
			for (auto &d : builder.descriptors){

				switch (d.type){
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:{
						d.bufferInfo.buffer = buffer[i].getBuffer();
						writer.writeBuffer(d.binding, &d.bufferInfo); 
						break;
					}
					case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: writer.writeImages(d.binding, d.imageCount, d.imageInfos.data()); break;
					default: throw "invalid descriptor type"; break;
				}
			}

			writer.build(sets[i]);
		}
	}

	void DescriptorSet::setDescriptorImage(uint32_t setIndex, uint32_t binding, uint32_t index, VkDescriptorImageInfo info){
		builder.descriptors[binding].imageInfos[index] = info;

		DescriptorWriter writer(device, &layout, &pool);

		auto &d = builder.descriptors[binding];
		writer.writeImages(d.binding, d.imageCount, d.imageInfos.data());

		writer.overwrite(sets[setIndex]);
	}

	DescriptorSetLayout& DescriptorSet::getLayout(){
		return layout;
	}

	DescriptorPool& DescriptorSet::getPool(){
		return pool;
	}

	void* DescriptorSet::getBuffer(uint32_t setIndex, uint32_t binding){
		auto range = buffers[binding];
		auto &b = buffer[setIndex];
		char *ptr = reinterpret_cast<char*>(b.getMappedMemory());
		ptr += range.offset;
		return ptr;
	}

	void DescriptorSet::writeBuffer(uint32_t setIndex, uint32_t binding, void* data){
		auto range = buffers[binding];
		auto &b = buffer[setIndex];
		char *ptr = reinterpret_cast<char*>(b.getMappedMemory());
		ptr += range.offset;
		memcpy(ptr, data, range.size);
		b.flush(range.alignement, range.offset);
	}

	VkDescriptorSet* DescriptorSet::getSets(){
		return sets;
	}

	VkDescriptorSet DescriptorSet::getSet(uint32_t index){
		return sets[index];
	}
}