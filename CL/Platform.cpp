/*
	This file is part of the Rendering library.
	Copyright (C) 2014 Sascha Brandt <myeti@mail.upb.de>

	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifdef RENDERING_HAS_LIB_OPENCL
#include "Platform.h"

#include "Device.h"


#include <Util/Macros.h>

#include <CL/cl.hpp>

namespace Rendering {
namespace CL {

Platform::Platform() = default;

Platform::Platform(cl::Platform* platform) : platform(new cl::Platform(*platform)) { }

Platform::~Platform() = default;

Platform::Platform(const Platform& platform) : platform(new cl::Platform(*platform.platform.get())) { }

Platform::Platform(Platform&& platform) = default;

Platform& Platform::operator=(Platform&&) = default;

std::string Platform::getExtensions() const {
	return platform->getInfo<CL_PLATFORM_EXTENSIONS>();
}

std::string Platform::getName() const {
	return platform->getInfo<CL_PLATFORM_NAME>();
}

std::string Platform::getProfile() const {
	return platform->getInfo<CL_PLATFORM_PROFILE>();
}

std::string Platform::getVendor() const {
	return platform->getInfo<CL_PLATFORM_VENDOR>();
}

std::string Platform::getVersion() const {
	return platform->getInfo<CL_PLATFORM_VERSION>();
}

std::vector<Device> Platform::getDevices() const {
	std::vector<Device> out;
	std::vector<cl::Device> devices;
	platform->getDevices(CL_DEVICE_TYPE_ALL, &devices);
	for(auto device : devices)
		out.push_back(&device);
	return out;
}

std::vector<Platform> Platform::get() {
	std::vector<Platform> out;
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	for(auto pf : platforms)
		out.push_back(&pf);
	return out;
}

} /* namespace CL */
} /* namespace Rendering */
#endif /* RENDERING_HAS_LIB_OPENCL */
