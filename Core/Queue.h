/*
	This file is part of the Rendering library.
	Copyright (C) 2019-2020 Sascha Brandt <sascha@brandt.graphics>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef RENDERING_CORE_QUEUE_H_
#define RENDERING_CORE_QUEUE_H_

#include "Common.h"

#include <Util/ReferenceCounter.h>

#include <memory>

namespace Rendering {
class Device;
class CommandBuffer;
class CommandPool;
class Swapchain;
using DeviceRef = Util::Reference<Device>;
using CommandBufferRef = Util::Reference<CommandBuffer>;
using CommandPoolRef = Util::Reference<CommandPool>;
using SwapchainRef = Util::Reference<Swapchain>;

class Queue : public Util::ReferenceCounter<Queue> {
public:
	using Ref = Util::Reference<Queue>;
	
	~Queue() = default;
	
	bool submit(const CommandBufferRef& commands);
	bool present();
	
	CommandBufferRef requestCommandBuffer(bool primary=true);
	const CommandPoolRef& getCommandPool() const;

	const QueueHandle& getApiHandle() const { return handle; }
	uint32_t getIndex() const { return index; }
	uint32_t getFamilyIndex() const { return familyIndex; }
	bool supports(QueueFamily type) const;
private:
	friend class Device;
	explicit Queue(const DeviceRef& device, uint32_t familyIndex, uint32_t index);

	Util::WeakPointer<Device> device;
	QueueHandle handle;
	uint32_t familyIndex;
	uint32_t index;
	QueueFamily capabilities;
};

inline QueueFamily operator | (QueueFamily lhs, QueueFamily rhs) {
	return static_cast<QueueFamily>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

inline QueueFamily operator & (QueueFamily lhs, QueueFamily rhs) {
	return static_cast<QueueFamily>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

inline bool Queue::supports(QueueFamily type) const { return (capabilities & type) != QueueFamily::None; }

} /* Rendering */

#endif /* end of include guard: RENDERING_CORE_QUEUE_H_ */
