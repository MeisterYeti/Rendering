/*
	This file is part of the Rendering library.
	Copyright (C) 2007-2013 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2007-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2007-2012 Ralf Petring <ralf@petring.net>
	Copyright (C) 2019-2020 Sascha Brandt <sascha@brandt.graphics>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "ShaderObjectInfo.h"
#include "../Helper.h"
#include "../Core/Device.h"
#include <Util/IO/FileUtils.h>
#include <Util/IO/FileLocator.h>
#include <Util/Macros.h>
#include <cstddef>
#include <vector>
#include <memory>

#include <shaderc/shaderc.hpp>
#include <spirv_cross.hpp>
#include <vulkan/vulkan.hpp>

#include <iostream>

namespace Rendering {

//-------------

static std::string toString(ShaderStage stage) {
	switch(stage) {
		case ShaderStage::Vertex: return "Vertex";
		case ShaderStage::TesselationControl: return "TesselationControl";
		case ShaderStage::TesselationEvaluation: return "TesselationEvaluation";
		case ShaderStage::Geometry: return "Geometry";
		case ShaderStage::Fragment: return "Fragment";
		case ShaderStage::Compute: return "Compute";
	}
	return "";
}

//-------------

static std::string toString(ShaderResourceType type) {
	switch(type) {
		case ShaderResourceType::Input: return "Input";
		case ShaderResourceType::InputAttachment: return "InputAttachment";
		case ShaderResourceType::Output: return "Output";
		case ShaderResourceType::Image: return "Image";
		case ShaderResourceType::ImageSampler: return "ImageSampler";
		case ShaderResourceType::ImageStorage: return "ImageStorage";
		case ShaderResourceType::Sampler: return "Sampler";
		case ShaderResourceType::BufferUniform: return "BufferUniform";
		case ShaderResourceType::BufferStorage: return "BufferStorage";
		case ShaderResourceType::PushConstant: return "PushConstant";
		case ShaderResourceType::SpecializationConstant: return "SpecializationConstant";
	}
	return "";
}

//-------------

std::string ShaderResource::toString() const {
	return name + ": (" 
		+ "stage " + Rendering::toString(stages) + ", "
		+ "type " + Rendering::toString(type) + ", "
		+ "set " + std::to_string(set) + ", "
		+ "binding " + std::to_string(binding) + ", "
		+ "location " + std::to_string(location) + ", "
		+ "input_attachment_index " + std::to_string(input_attachment_index) + ", "
		+ "vec_size " + std::to_string(vec_size) + ", "
		+ "columns " + std::to_string(columns) + ", "
		+ "array_size " + std::to_string(array_size) + ", "
		+ "offset " + std::to_string(offset) + ", "
		+ "size " + std::to_string(size) + ", "
		+ "constant_id " + std::to_string(constant_id) + ", "
		+ "dynamic " + std::to_string(dynamic) + ")";
}

//-------------
	
const uint32_t ShaderObjectInfo::SHADER_STAGE_VERTEX = static_cast<uint32_t>(ShaderStage::Vertex);
const uint32_t ShaderObjectInfo::SHADER_STAGE_FRAGMENT = static_cast<uint32_t>(ShaderStage::Fragment);
const uint32_t ShaderObjectInfo::SHADER_STAGE_GEOMETRY = static_cast<uint32_t>(ShaderStage::Geometry);
const uint32_t ShaderObjectInfo::SHADER_STAGE_TESS_CONTROL = static_cast<uint32_t>(ShaderStage::TesselationControl);
const uint32_t ShaderObjectInfo::SHADER_STAGE_TESS_EVALUATION = static_cast<uint32_t>(ShaderStage::TesselationEvaluation);
const uint32_t ShaderObjectInfo::SHADER_STAGE_COMPUTE = static_cast<uint32_t>(ShaderStage::Compute);
	
//-------------

class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
public:
	using Ptr = std::unique_ptr<shaderc::CompileOptions::IncluderInterface>;

	ShaderIncluder(const Util::FileName& file) {
		locator.addSearchPath(file.getPath());
	}

	// Handles shaderc_include_resolver_fn callbacks.
	virtual shaderc_include_result* GetInclude(const char* requested_source,
																							shaderc_include_type type,
																							const char* requesting_source,
																							size_t include_depth);

	// Handles shaderc_include_result_release_fn callbacks.
	virtual void ReleaseInclude(shaderc_include_result* data) { delete data; }

	Util::FileLocator locator;
};

//-------------

shaderc_include_result* ShaderIncluder::GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) {
	auto* result = new shaderc_include_result;
	result->content = nullptr;
	result->content_length = 0;
	result->source_name = nullptr;
	result->source_name_length = 0;
	result->user_data = nullptr;
	auto includeFile = locator.locateFile(Util::FileName(requested_source));
	WARN("ShaderObjectInfo: #include not supported. Include file: '" + std::string(requested_source) + "' " + (includeFile.first ? "found" : "not found"));
	return result; 
}

//-------------

ShaderObjectInfo::ShaderObjectInfo(ShaderStage stage, std::string _code) : stage(stage), code(std::move(_code)) { }

//-------------

ShaderObjectInfo::ShaderObjectInfo(ShaderStage stage, std::vector<uint32_t> _spirv) : stage(stage), spirv(std::move(_spirv)) { }

//-------------

ShaderModuleHandle ShaderObjectInfo::compile(const DeviceRef& device) {
	if(code.empty()) {
		WARN("ShaderObjectInfo: Cannot compile empty code.");
		return nullptr;
	}

	vk::Device vkDevice(device->getApiHandle());

	if(!spirv.empty()) {
		// Don't recompile
		return {vkDevice.createShaderModule({{}, spirv.size() * sizeof(uint32_t), spirv.data()}), vkDevice};
	}
	
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	options.SetGenerateDebugInfo();
	options.SetOptimizationLevel(shaderc_optimization_level_performance);
	options.SetAutoMapLocations(true);
	options.SetAutoBindUniforms(true);
	
	ShaderIncluder::Ptr includer(new ShaderIncluder(filename));
	options.SetIncluder(std::move(includer));

	std::string name = filename.toString();
	shaderc_shader_kind kind;
	switch(stage) {
		case ShaderStage::Vertex:
			name = "Vertex"; 
			options.AddMacroDefinition("SG_VERTEX_SHADER");
			kind = shaderc_glsl_vertex_shader;
			break;
		case ShaderStage::TesselationControl:
			name = "TesselationControl"; 
			options.AddMacroDefinition("SG_TESSELATIONCONTROL_SHADER");
			kind = shaderc_glsl_tess_control_shader;
			break;
		case ShaderStage::TesselationEvaluation:
			name = "TesselationEvaluation"; 
			options.AddMacroDefinition("SG_TESSELATIONEVALUATION_SHADER");
			kind = shaderc_glsl_tess_evaluation_shader;
			break;
		case ShaderStage::Geometry:
			name = "Geometry"; 
			options.AddMacroDefinition("SG_GEOMETRY_SHADER");
			kind = shaderc_glsl_geometry_shader;
			break;
		case ShaderStage::Fragment:
			name = "Fragment"; 
			options.AddMacroDefinition("SG_FRAGMENT_SHADER");
			kind = shaderc_glsl_fragment_shader;
			break;
		case ShaderStage::Compute:
			name = "Compute"; 
			options.AddMacroDefinition("SG_COMPUTE_SHADER");
			kind = shaderc_glsl_compute_shader;
			break;
	}
	for(auto& define : defines)
		options.AddMacroDefinition(define.key, define.value);

	if(!filename.empty())
		name = filename.toString();
	
	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(code, kind, name.c_str(), options);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		if(filename.empty())
			WARN(std::string("Shader compile error:\n") + result.GetErrorMessage() + "\nShader code:\n" + code);
		else
			WARN(std::string("Shader compile error:\n") + result.GetErrorMessage() + "\nin shader file: " + filename.toShortString() + "\n");
		return nullptr;
	}
	spirv = { result.cbegin(), result.cend() };

	return {vkDevice.createShaderModule({{}, spirv.size() * sizeof(uint32_t), spirv.data()}), vkDevice};
}

//-------------

ShaderModuleHandle ShaderObjectInfo::compile() {
	return compile(Device::getDefault());
}

//-------------

static ShaderResource readPushConstant(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource, ShaderStage stage) {
	ShaderResource result{resource.name, stage, ShaderResourceType::PushConstant};
	const auto& spirvType = compiler.get_type_from_variable(resource.id);
	result.size = compiler.get_declared_struct_size_runtime_array(spirvType, 0); // TODO: specify runtime array size
	result.offset = std::numeric_limits<std::uint32_t>::max();
	for(auto i=0u; i < spirvType.member_types.size(); ++i) 
		result.offset = std::min(result.offset, compiler.get_member_decoration(spirvType.self, i, spv::DecorationOffset));
	result.size -= result.offset;
	return result;
}

//-------------

static ShaderResource readSpecializationConstant(spirv_cross::Compiler& compiler, spirv_cross::SpecializationConstant& resource, ShaderStage stage) {
	ShaderResource result{compiler.get_name(resource.id), stage, ShaderResourceType::SpecializationConstant};
	const auto& spirvValue = compiler.get_constant(resource.id);
	const auto& spirvType = compiler.get_type(spirvValue.constant_type);
	switch (spirvType.basetype) {
		case spirv_cross::SPIRType::BaseType::Boolean:
		case spirv_cross::SPIRType::BaseType::Char:
		case spirv_cross::SPIRType::BaseType::Int:
		case spirv_cross::SPIRType::BaseType::UInt:
		case spirv_cross::SPIRType::BaseType::Float:
			result.size = 4;
			break;
		case spirv_cross::SPIRType::BaseType::Int64:
		case spirv_cross::SPIRType::BaseType::UInt64:
		case spirv_cross::SPIRType::BaseType::Double:
			result.size = 8;
			break;
		default:
			result.size = 0;
			break;
	}
	result.offset = 0;
	result.constant_id = resource.constant_id;
	return result;
}

//-------------

static ShaderResource readShaderResource(spirv_cross::Compiler& compiler, spirv_cross::Resource& resource, ShaderStage stage, ShaderResourceType type) {
	ShaderResource result{resource.name, stage, type};
	result.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
	result.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
	result.location = compiler.get_decoration(resource.id, spv::DecorationLocation);
	result.input_attachment_index = compiler.get_decoration(resource.id, spv::DecorationInputAttachmentIndex);	
	const auto& spirvType = compiler.get_type_from_variable(resource.id);
	result.size = compiler.get_declared_struct_size_runtime_array(spirvType, 0); // TODO: specify runtime array size
	result.array_size = spirvType.array.size() ? spirvType.array[0] : 1;
	result.vec_size = spirvType.vecsize;
	result.columns = spirvType.columns;
	return result;
}

//-------------

std::vector<ShaderResource> ShaderObjectInfo::reflect() {
	if(spirv.empty()) {
		WARN("ShaderObjectInfo: Cannot reflect shader code. Please compile first.");
		return {};
	}
	std::vector<ShaderResource> resources;
	spirv_cross::Compiler compiler(spirv);	
	auto spvResources = compiler.get_shader_resources();
	for(auto& res : spvResources.stage_inputs)
		resources.emplace_back(readShaderResource(compiler, res, stage, ShaderResourceType::Input));
	for(auto& res : spvResources.subpass_inputs)
		resources.emplace_back(readShaderResource(compiler, res, stage, ShaderResourceType::InputAttachment));
	for(auto& res : spvResources.stage_outputs)
		resources.emplace_back(readShaderResource(compiler, res, stage, ShaderResourceType::Output));
	for(auto& res : spvResources.separate_images)
		resources.emplace_back(readShaderResource(compiler, res, stage, ShaderResourceType::Image));
	for(auto& res : spvResources.sampled_images)
		resources.emplace_back(readShaderResource(compiler, res, stage, ShaderResourceType::ImageSampler));
	for(auto& res : spvResources.storage_images)
		resources.emplace_back(readShaderResource(compiler, res, stage, ShaderResourceType::ImageStorage));
	for(auto& res : spvResources.separate_samplers)
		resources.emplace_back(readShaderResource(compiler, res, stage, ShaderResourceType::Sampler));
	for(auto& res : spvResources.uniform_buffers)
		resources.emplace_back(readShaderResource(compiler, res, stage, ShaderResourceType::BufferUniform));
	for(auto& res : spvResources.storage_buffers)
		resources.emplace_back(readShaderResource(compiler, res, stage, ShaderResourceType::BufferStorage));
	for(auto& res : spvResources.push_constant_buffers)
		resources.emplace_back(readPushConstant(compiler, res, stage));
	for(auto& res : compiler.get_specialization_constants())
		resources.emplace_back(readSpecializationConstant(compiler, res, stage));
	
	return resources;
}

//-------------

ShaderObjectInfo ShaderObjectInfo::createVertex(const std::vector<uint32_t>& spirv) {
	return ShaderObjectInfo(ShaderStage::Vertex, spirv);
}

//-------------

ShaderObjectInfo ShaderObjectInfo::createFragment(const std::vector<uint32_t>& spirv) {
	return ShaderObjectInfo(ShaderStage::Fragment, spirv);
}

//-------------

ShaderObjectInfo ShaderObjectInfo::createGeometry(const std::vector<uint32_t>& spirv) {
	return ShaderObjectInfo(ShaderStage::Geometry, spirv);
}

//-------------

ShaderObjectInfo ShaderObjectInfo::createCompute(const std::vector<uint32_t>& spirv) {
	return ShaderObjectInfo(ShaderStage::Compute, spirv);
}

//-------------

ShaderObjectInfo ShaderObjectInfo::createVertex(const std::string & code) {
	return ShaderObjectInfo(ShaderStage::Vertex, code);
}

//-------------

ShaderObjectInfo ShaderObjectInfo::createFragment(const std::string & code) {
	return ShaderObjectInfo(ShaderStage::Fragment, code);
}

//-------------

ShaderObjectInfo ShaderObjectInfo::createGeometry(const std::string & code) {
	return ShaderObjectInfo(ShaderStage::Geometry, code);
}

//-------------

ShaderObjectInfo ShaderObjectInfo::createCompute(const std::string & code) {
	return ShaderObjectInfo(ShaderStage::Compute, code);
}

//-------------

ShaderObjectInfo ShaderObjectInfo::loadVertex(const Util::FileName & file) {
	return createVertex(Util::FileUtils::getParsedFileContents(file)).setFileName(file);
}

//-------------

ShaderObjectInfo ShaderObjectInfo::loadFragment(const Util::FileName & file) {
	return createFragment(Util::FileUtils::getParsedFileContents(file)).setFileName(file);
}

//-------------

ShaderObjectInfo ShaderObjectInfo::loadGeometry(const Util::FileName & file) {
	return createGeometry(Util::FileUtils::getParsedFileContents(file)).setFileName(file);
}

//-------------

ShaderObjectInfo ShaderObjectInfo::loadCompute(const Util::FileName & file) {
	return createCompute(Util::FileUtils::getParsedFileContents(file)).setFileName(file);
}

//-------------

}

//-------------
