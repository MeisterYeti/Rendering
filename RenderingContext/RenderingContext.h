/*
	This file is part of the Rendering library.
	Copyright (C) 2007-2012 Benjamin Eikel <benjamin@eikel.org>
	Copyright (C) 2007-2012 Claudius Jähn <claudius@uni-paderborn.de>
	Copyright (C) 2007-2012 Ralf Petring <ralf@petring.net>
	Copyright (C) 2014-2018 Sascha Brandt <sascha@brandt.graphics>
	
	This library is subject to the terms of the Mozilla Public License, v. 2.0.
	You should have received a copy of the MPL along with this library; see the 
	file LICENSE. If not, you can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef RENDERING_RENDERINCONTEXT_H_
#define RENDERING_RENDERINCONTEXT_H_

#include <Util/CountedObjectWrapper.h>
#include <cstdint>
#include <memory>
#include <functional>
#include <vector>

namespace Geometry {
template<typename _T> class _Matrix4x4;
typedef _Matrix4x4<float> Matrix4x4;
template<typename _T> class _Rect;
typedef _Rect<int> Rect_i;
}
namespace Util {
class Color4f;
class StringIdentifier;
}
namespace Rendering {
class AlphaTestParameters;
class BlendingParameters;
class BufferObject;
class BufferView;
class ClipPlaneParameters;
class ColorBufferParameters;
class CullFaceParameters;
class DepthBufferParameters;
class FBO;
class ImageBindParameters;
class LightParameters;
class LightingParameters;
class LineParameters;
class MaterialParameters;
class Mesh;
class PointParameters;
class PolygonModeParameters;
class PolygonOffsetParameters;
class PrimitiveRestartParameters;
class ScissorParameters;
class StencilParameters;
class Shader;
class Texture;
class Uniform;
class UniformRegistry;
class VertexAttribute;
class VertexDescription;
enum class TexUnitUsageParameter : uint8_t;

//! @defgroup context Rendering Context

//! @ingroup context
class RenderingContext {

	//! @name General
	//	@{
private:
	// Use Pimpl idiom
	class InternalData;
	std::unique_ptr<InternalData> internalData;
public:

	RenderingContext();
	~RenderingContext();

	//! has to return true iff normal display of mesh shall be executed
	typedef std::function<void (RenderingContext & rc,Mesh * mesh,uint32_t firstElement,uint32_t elementCount)> DisplayMeshFn;
private:
	DisplayMeshFn displayMeshFn;
public:
	void setDisplayMeshFn(DisplayMeshFn fn){ displayMeshFn = fn; };
	void resetDisplayMeshFn();

	void displayMesh(Mesh * mesh,uint32_t firstElement,uint32_t elementCount){ displayMeshFn(*this, mesh,firstElement,elementCount); }
	void displayMesh(Mesh * mesh);

	void setImmediateMode(const bool enabled) __attribute((deprecated)) {}
	bool getImmediateMode() const  __attribute((deprecated)) {
		return false;
	}
	void applyChanges(bool forced = false);
	//	@}

	// -----------------------------------

	/*!	@name GL Helper */
	//	@{
	static void initGLState();
	void clearScreen(const Util::Color4f & color);
	void clearScreenRect(const Geometry::Rect_i & rect, const Util::Color4f & color, bool clearDepth=true);

	/**
	 * Flush the GL commands buffer.
	 * @see glFlush
	 */
	void flush();

	/**
	 * Block until all GL commands are complete.
	 * @see glFinish
	 */
	void finish();
	
	/**
	 * Defines a barrier ordering memory transactions.
	 * @see glMemoryBarrier
	 */
	void barrier(uint32_t flags=0);
	
	// @}

	// --------------------------------------------------------------------
	// --------------------------------------------------------------------
	// Parameters (sorted alphabetically)

	//! @name AlphaTest
	//	@{
	const AlphaTestParameters & getAlphaTestParameters() const __attribute((deprecated));
	void popAlphaTest() __attribute((deprecated)) {}
	void pushAlphaTest() __attribute((deprecated)) {}
	void pushAndSetAlphaTest(const AlphaTestParameters & alphaTestParameter) __attribute((deprecated)) {}
	void setAlphaTest(const AlphaTestParameters & alphaTestParameter) __attribute((deprecated)) {}
	// @}
	
	// ------

	//! @name Atomic counters (extension ARB_shader_atomic_counters)
	//	@{
	void setAtomicCounterTextureBuffer(uint32_t pushAtomicCounterTexture, Texture* bufferDataTexture) __attribute((deprecated));
	static bool isAtomicCountersSupported() __attribute((deprecated)) { return true; }
	static uint32_t getMaxAtomicCounterBuffers() __attribute((deprecated)) { return 1; }
	static uint32_t getMaxAtomicCounterBufferSize() __attribute((deprecated)) { return 32; }
	Texture* getAtomicCounterTextureBuffer(uint32_t index) const __attribute((deprecated)) { return nullptr; }
	void pushAtomicCounterTextureBuffer(uint32_t index) __attribute((deprecated)) {}
	void popAtomicCounterTextureBuffer(uint32_t pushAtomicCounterTexture) __attribute((deprecated)) {}
	void pushAndSetAtomicCounterTextureBuffer(uint32_t index, Texture* bufferDataTexture) __attribute((deprecated)) {
		setAtomicCounterTextureBuffer(index, bufferDataTexture);
	} 

	// ------

	//! @name Blending
	//	@{
	const BlendingParameters & getBlendingParameters() const;
	void popBlending();
	void pushBlending();
	void pushAndSetBlending(const BlendingParameters & blendingParameter);
	void setBlending(const BlendingParameters & blendingParameter);
	// @}
	
	// ------

	//! @name Buffers
	//	@{
	void registerBuffer(const Util::StringIdentifier& name, BufferView* bufferView);
	void registerBuffer(const Util::StringIdentifier& name, BufferObject* buffer);
	void unregisterBuffer(const Util::StringIdentifier& name);
	BufferView* getBuffer(const Util::StringIdentifier& name);
	void bindBuffer(BufferView* bufferView, uint32_t target, uint32_t location=0);
	void bindBuffer(BufferObject* buffer, uint32_t target, uint32_t location=0);
	void bindBuffer(const Util::StringIdentifier& name, uint32_t target, uint32_t location=0);
	void unbindBuffer(uint32_t target, uint32_t location=0);
	// @}
	
	// ------
	//! @name Scissor
	//	@{
	const ClipPlaneParameters & getClipPlane(uint8_t index) const __attribute((deprecated));
	void popClipPlane(uint8_t index) __attribute((deprecated)) {}
	void pushClipPlane(uint8_t index) __attribute((deprecated)) {}
	void pushAndSetClipPlane(uint8_t index, const ClipPlaneParameters & planeParameters) __attribute((deprecated)) {}
	void setClipPlane(uint8_t index, const ClipPlaneParameters & planeParameters) __attribute((deprecated)) {}
	// @}
	
	// ------

	//! @name ColorBuffer
	//	@{
	const ColorBufferParameters & getColorBufferParameters() const;
	void popColorBuffer();
	void pushColorBuffer();
	void pushAndSetColorBuffer(const ColorBufferParameters & colorBufferParameter);
	void setColorBuffer(const ColorBufferParameters & colorBufferParameter);

	/**
	 * Clear the color buffer.
	 *
	 * @param clearValue Color that the color buffer is filled with.
	 * The color components are clamped to [0, 1].
	 * @see glClearColor
	 * @see Parameter GL_COLOR_BUFFER_BIT of glClear
	 */
	void clearColor(const Util::Color4f & clearValue);
	// @}
	// ------

	//! @name CullFace
	//	@{
	const CullFaceParameters & getCullFaceParameters() const;
	void popCullFace();
	void pushCullFace();
	void pushAndSetCullFace(const CullFaceParameters & cullFaceParameters);
	void setCullFace(const CullFaceParameters & cullFaceParameters);
	// @}

	// ------

	//! @name DepthBuffer
	//	@{
	const DepthBufferParameters & getDepthBufferParameters() const;
	void popDepthBuffer();
	void pushDepthBuffer();
	void pushAndSetDepthBuffer(const DepthBufferParameters & depthBufferParameter);
	void setDepthBuffer(const DepthBufferParameters & depthBufferParameter);

	/**
	 * Clear the depth buffer.
	 *
	 * @param clearValue Value that the depth buffer is filled with
	 * The value is clamped to [0, 1].
	 * @see glClearDepth
	 * @see Parameter GL_DEPTH_BUFFER_BIT of glClear
	 */
	void clearDepth(float clearValue);
	// @}

	// ------

	//! @name FBO
	//	@{
	FBO * getActiveFBO() const;
	void popFBO();
	void pushFBO();
	void pushAndSetFBO(FBO * fbo);
	void setFBO(FBO * fbo);
	// @}

	// ------

	//! @name Global Uniforms
	//	@{
	void setGlobalUniform(const Uniform & u);
	const Uniform & getGlobalUniform(const Util::StringIdentifier & uniformName);
	// @}

	// ------

	//! @name Image Binding (Image load and store)
	//	@{
	const ImageBindParameters& getBoundImage(uint8_t unit)const;
	void pushBoundImage(uint8_t unit);
	void pushAndSetBoundImage(uint8_t unit, const ImageBindParameters& iParam); 
	void popBoundImage(uint8_t unit);

	//! \note the texture in iParam may be null to unbind
	void setBoundImage(uint8_t unit, const ImageBindParameters& iParam);
	// @}

	// ------

	//! @name Lighting
	//	@{
	const LightingParameters & getLightingParameters() const __attribute((deprecated));
	void popLighting() __attribute((deprecated)) {}
	void pushLighting() __attribute((deprecated)) {}
	void pushAndSetLighting(const LightingParameters & lightingParameter) __attribute((deprecated)) {}
	void setLighting(const LightingParameters & lightingParameter) __attribute((deprecated)) {}

	// ------

	//! @name Lights
	//	@{
	/**
	 * Activate the light given by the parameters.
	 *
	 * @param light Parameters of a light source.
	 * @return Light number that was used for this light. This number has to be used to deactivate the light.
	 */
	uint8_t enableLight(const LightParameters & light) __attribute((deprecated));
	
	uint8_t registerLight(const LightParameters & light);
	void setLight(uint8_t lightNumber, const LightParameters & light);
	
	void unregisterLight(uint8_t lightNumber);
	void enableLight(uint8_t lightNumber);
	/**
	 * Deactivate a previuosly activated light.
	 *
	 * @param lightNumber Light number that was returned by @a enableLight.
	 */
	void disableLight(uint8_t lightNumber);
	// @}

	// ------

	//! @name Line
	//	@{
	const LineParameters & getLineParameters() const;
	void popLine();
	void pushLine();
	void pushAndSetLine(const LineParameters & lineParameters);
	void setLine(const LineParameters & lineParameters);
	// @}

	// ------

	//! @name Material
	//	@{
	//! Return the active material.
	const MaterialParameters & getMaterial() const __attribute((deprecated)) { return getActiveMaterial(); }
	//! Pop a material from the top of the stack and activate it. Deactivate material usage if stack is empty.
	inline void popMaterial() __attribute((deprecated)) { popActiveMaterial(); }
	//! Push the given material onto the material stack.
	inline void pushMaterial() __attribute((deprecated)) { pushActiveMaterial(); }
	//! Push the given material onto the material stack and activate it.
	void pushAndSetMaterial(const MaterialParameters& material) __attribute((deprecated));
	//! Convert the given color to a material, and call @a pushAndSetMaterial
	void pushAndSetColorMaterial(const Util::Color4f& color) __attribute((deprecated));
	//! Activate the given material.
	void setMaterial(const MaterialParameters& material) __attribute((deprecated));
	
	
	void setActiveMaterial(uint8_t materialId);
	void popActiveMaterial();
	void pushActiveMaterial();
	void pushAndSetActiveMaterial(uint8_t materialId);
	const MaterialParameters& getActiveMaterial() const;
	uint8_t getActiveMaterialId() const;
	
	uint8_t registerMaterial(const MaterialParameters& material);
	void unregisterMaterial(uint8_t materialId);
	void setMaterial(uint8_t materialId, const MaterialParameters& material);


	// @}
	// ------

	/*! @name Matrix CameraToWorld / WorldToCamera
	 camera matrix == inverse world matrix of camera node == default model view matrix	*/
	//	@{
	void setMatrix_cameraToWorld(const Geometry::Matrix4x4 & matrix);	//!< formerly known as setInverseCameraMatrix
	const Geometry::Matrix4x4 & getMatrix_worldToCamera() const;		//!< formerly known as getCameraMatrix
	const Geometry::Matrix4x4 & getMatrix_cameraToWorld() const;		//!< formerly known as getInverseCameraMatrix
	//	@}

	// ------

	//! @name Matrix ModelToCamera (Legacy Model View Matrix)
	//	@{
	//! resets the model view matrix to the default (camera matrix)
	void resetMatrix();  //! \note use renderingContext.setMatrix_modelToCamera( renderingContext.getMatrix_worldToCamera() ) instead!
	const Geometry::Matrix4x4 & getMatrix_modelToCamera() const;		//!< formerly known as getMatrix
	void multMatrix_modelToCamera(const Geometry::Matrix4x4 & matrix);	//!< formerly known as multMatrix
	void pushMatrix_modelToCamera();									//!< formerly known as pushMatrix
	void pushAndSetMatrix_modelToCamera(const Geometry::Matrix4x4 & matrix);
	void setMatrix_modelToCamera(const Geometry::Matrix4x4 & matrix);	//!< formerly known as setMatrix
	void popMatrix_modelToCamera();										//!< formerly known as popMatrix
	//	@}
	
	// ------

	//! @name Matrix CameraToClipping (Legacy Projection Matrix)
	//	@{
	const Geometry::Matrix4x4 & getMatrix_cameraToClipping() const;			//! formerly known as getProjectionMatrix
	void pushAndSetMatrix_cameraToClipping(const Geometry::Matrix4x4 & matrix);
	void pushMatrix_cameraToClipping();										//! formerly known as pushProjectionMatrix
	void popMatrix_cameraToClipping();										//! formerly known as popProjectionMatrix
	void setMatrix_cameraToClipping(const Geometry::Matrix4x4 & matrix);	//! formerly known as setProjectionMatrix
	// @}
	
	// ------

	//! @name Point
	//	@{
	const PointParameters & getPointParameters() const;
	void popPointParameters();
	void pushPointParameters();
	void pushAndSetPointParameters(const PointParameters & pointParameters);
	void setPointParameters(const PointParameters & pointParameters);
	// @}
	// ------

	//! @name PolygonMode
	//	@{
	const PolygonModeParameters & getPolygonModeParameters() const;
	void popPolygonMode();
	void pushPolygonMode();
	void pushAndSetPolygonMode(const PolygonModeParameters & polygonModeParameter);
	void setPolygonMode(const PolygonModeParameters & polygonModeParameter);
	// @}

	// ------

	//! @name PolygonOffset
	//	@{
	const PolygonOffsetParameters & getPolygonOffsetParameters() const;
	void popPolygonOffset();
	void pushPolygonOffset();
	void pushAndSetPolygonOffset(const PolygonOffsetParameters & polygonOffsetParameter);
	void setPolygonOffset(const PolygonOffsetParameters & polygonOffsetParameter);
	// @}
	
	// ------

	//! @name Primitive restart
	//	@{
	const PrimitiveRestartParameters & getPrimitiveRestartParameters() const;
	void popPrimitiveRestart();
	void pushPrimitiveRestart();
	void pushAndSetPrimitiveRestart(const PrimitiveRestartParameters & parameters);
	void setPrimitiveRestart(const PrimitiveRestartParameters & parameters);
	// @}


	// ------

	//! @name Shader
	//	@{
	void pushAndSetShader(Shader * shader);
	void pushShader();
	void popShader();
	bool isShaderEnabled(Shader * shader);
	Shader * getActiveShader();
	const Shader * getActiveShader() const;
	void setShader(Shader * shader); // shader may be nullptr
	void dispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY=1, uint32_t numGroupsZ=1);
	void dispatchComputeIndirect(size_t offset=0);
	void dispatchComputeGroupSize(uint32_t numGroupsX, uint32_t groupSizeX, uint32_t numGroupsY=1, uint32_t groupSizeY=1, uint32_t numGroupsZ=1, uint32_t groupSizeZ=1);
	void loadUniformSubroutines(uint32_t shaderStage, const std::vector<uint32_t>& indices);
	void loadUniformSubroutines(uint32_t shaderStage, const std::vector<std::string>& names);

	//! (internal) called by Shader::setUniform(...)
	void _setUniformOnShader(Shader * shader, const Uniform & uniform, bool warnIfUnused, bool forced);

	// @}

	// ------

	//! @name Scissor
	//	@{
	const ScissorParameters & getScissor() const;
	void popScissor();
	void pushScissor();
	void pushAndSetScissor(const ScissorParameters & scissorParameters);
	void setScissor(const ScissorParameters & scissorParameters);
	// @}

// ------

	//! @name Stencil
	//	@{
	const StencilParameters & getStencilParamters() const;
	void popStencil();
	void pushStencil();
	void pushAndSetStencil(const StencilParameters & stencilParameter);
	void setStencil(const StencilParameters & stencilParameter);

	/**
	 * Clear the stencil buffer.
	 *
	 * @param clearValue Value that the stencil buffer is filled with
	 * @see glClearStencil
	 * @see Parameter GL_STENCIL_BUFFER_BIT of glClear
	 */
	void clearStencil(int32_t clearValue);
	// @}

	// ------

	/*! @name Textures
	 \todo Move array of activeTextures to ProgramState to allow delayed binding
	 */
	//	@{
	Texture * getTexture(uint8_t unit) const;
	TexUnitUsageParameter getTextureUsage(uint8_t unit) const __attribute((deprecated));
	void pushTexture(uint8_t unit);
	void pushAndSetTexture(uint8_t unit, Texture * texture); // default usage = TexUnitUsageParameter::TEXTURE_MAPPING );
	void pushAndSetTexture(uint8_t unit, Texture * texture, TexUnitUsageParameter usage) __attribute((deprecated));
	void popTexture(uint8_t unit);

	//! \note texture may be nullptr
	void setTexture(uint8_t unit, Texture * texture); // default: usage = TexUnitUsageParameter::TEXTURE_MAPPING);
	void setTexture(uint8_t unit, Texture * texture, TexUnitUsageParameter usage) __attribute((deprecated));
	// @}
	
	// ------

	//! @name Transform Feedback
	//	@{
	static bool isTransformFeedbackSupported();
	static bool requestTransformFeedbackSupport(); //! like isTransformFeedbackSupported(), but once issues a warning on failure.
	
	BufferObject * getActiveTransformFeedbackBuffer() const;
	void popTransformFeedbackBufferStatus();
	void pushTransformFeedbackBufferStatus();
	void setTransformFeedbackBuffer(BufferObject * buffer);
	void _startTransformFeedback(uint32_t);
	void startTransformFeedback_lines();
	void startTransformFeedback_points();
	void startTransformFeedback_triangles();
	void stopTransformFeedback();
	// @}

	// ------

	//! @name Vertex Format
	// @{

	/**
	 * Sets the active vertex format for a specific binding point
	 *
	 * @param binding The index of the vertex buffer binding
	 * @param vd The vertex format description
	 * @param divisor The value for the instance step rate to apply
	 */
	void setVertexFormat(uint32_t binding, const VertexDescription& vd);
 	void bindVertexBuffer(uint32_t binding, uint32_t bufferId, uint32_t offset, uint32_t stride, uint32_t divisor=0);
	void bindIndexBuffer(uint32_t bufferId);
	
	// ------

	//! @name Draw commands
	// @{
	
	void drawArrays(uint32_t mode, uint32_t first, uint32_t count);
	void drawElements(uint32_t mode, uint32_t type, uint32_t first, uint32_t count);
	// ------

	//! @name Viewport and window's size
	// @{
	/**
	 * Get the OpenGL window's client area.
	 * In almost all cases, the position will be (0, 0).
	 * The width and height differs with the size of the window.
	 * @note This value has to be set manually by calling setWindowClientArea() after creating the RenderingContext
	 */
	const Geometry::Rect_i & getWindowClientArea() const;

	//! Read the current viewport.
	const Geometry::Rect_i & getViewport() const;
	//! Restore the viewport from the top of the viewport stack.
	void popViewport();

	//! Save the current viewport onto the viewport stack.
	void pushViewport();

	//! Set the current viewport.
	void setViewport(const Geometry::Rect_i & viewport);

	//! Save the current viewport onto the viewport stack and set the current viewport.
	void pushAndSetViewport(const Geometry::Rect_i & viewport);

	void setWindowClientArea(const Geometry::Rect_i & clientArea);
	// @}
};

}

#endif /* RENDERING_RENDERINCONTEXT_H_ */
