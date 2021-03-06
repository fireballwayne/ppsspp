// Copyright (c) 2012- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#pragma once

#include <list>
#include <set>
#include <algorithm>

#include "gfx/gl_common.h"
#include "ext/native/thin3d/thin3d.h"
// Keeps track of allocated FBOs.
// Also provides facilities for drawing and later converting raw
// pixel data.


#include "Globals.h"
#include "GPU/GPUCommon.h"
#include "GPU/Common/FramebufferCommon.h"
#include "Core/Config.h"

struct GLSLProgram;
class TextureCacheGLES;
class DrawEngineGLES;
class ShaderManagerGLES;

// Simple struct for asynchronous PBO readbacks
struct AsyncPBO {
	GLuint handle;
	u32 maxSize;

	u32 fb_address;
	u32 stride;
	u32 height;
	u32 size;
	GEBufferFormat format;
	bool reading;
};

class FramebufferManagerGLES : public FramebufferManagerCommon {
public:
	FramebufferManagerGLES(Draw::DrawContext *draw);
	~FramebufferManagerGLES();

	void SetTextureCache(TextureCacheGLES *tc);
	void SetShaderManager(ShaderManagerGLES *sm) {
		shaderManager_ = sm;
	}
	void SetDrawEngine(DrawEngineGLES *td) {
		drawEngine_ = td;
	}

	void DrawPixels(VirtualFramebuffer *vfb, int dstX, int dstY, const u8 *srcPixels, GEBufferFormat srcPixelFormat, int srcStride, int width, int height) override;
	void DrawFramebufferToOutput(const u8 *srcPixels, GEBufferFormat srcPixelFormat, int srcStride, bool applyPostShader) override;

	// x,y,w,h are relative to destW, destH which fill out the target completely.
	void DrawActiveTexture(float x, float y, float w, float h, float destW, float destH, float u0, float v0, float u1, float v1, GLSLProgram *program, int uvRotation, bool linearFilter);

	void DestroyAllFBOs(bool forceDelete);

	virtual void Init() override;
	void EndFrame();
	void Resized() override;
	void DeviceLost();
	void CopyDisplayToOutput();
	void SetLineWidth();
	void ReformatFramebufferFrom(VirtualFramebuffer *vfb, GEBufferFormat old) override;

	void BlitFramebufferDepth(VirtualFramebuffer *src, VirtualFramebuffer *dst) override;

	// For use when texturing from a framebuffer.  May create a duplicate if target.
	void BindFramebufferColor(int stage, u32 fbRawAddress, VirtualFramebuffer *framebuffer, int flags);

	// Reads a rectangular subregion of a framebuffer to the right position in its backing memory.
	void ReadFramebufferToMemory(VirtualFramebuffer *vfb, bool sync, int x, int y, int w, int h) override;
	void DownloadFramebufferForClut(u32 fb_address, u32 loadBytes) override;

	std::vector<FramebufferInfo> GetFramebufferList();

	bool NotifyStencilUpload(u32 addr, int size, bool skipZero = false) override;

	bool GetFramebuffer(u32 fb_address, int fb_stride, GEBufferFormat format, GPUDebugBuffer &buffer, int maxRes);
	bool GetDepthbuffer(u32 fb_address, int fb_stride, u32 z_address, int z_stride, GPUDebugBuffer &buffer);
	bool GetStencilbuffer(u32 fb_address, int fb_stride, GPUDebugBuffer &buffer);
	bool GetOutputFramebuffer(GPUDebugBuffer &buffer);

	virtual void RebindFramebuffer() override;

protected:
	void DisableState() override;
	void ClearBuffer(bool keepState = false) override;
	void FlushBeforeCopy() override;

	// Used by ReadFramebufferToMemory and later framebuffer block copies
	void BlitFramebuffer(VirtualFramebuffer *dst, int dstX, int dstY, VirtualFramebuffer *src, int srcX, int srcY, int w, int h, int bpp) override;

	bool CreateDownloadTempBuffer(VirtualFramebuffer *nvfb) override;
	void UpdateDownloadTempBuffer(VirtualFramebuffer *nvfb) override;

private:
	void MakePixelTexture(const u8 *srcPixels, GEBufferFormat srcPixelFormat, int srcStride, int width, int height);
	void UpdatePostShaderUniforms(int bufferWidth, int bufferHeight, int renderWidth, int renderHeight);
	void CompileDraw2DProgram();
	void DestroyDraw2DProgram();

	void PackFramebufferAsync_(VirtualFramebuffer *vfb);  // Not used under ES currently
	void PackFramebufferSync_(VirtualFramebuffer *vfb, int x, int y, int w, int h);
	void PackDepthbuffer(VirtualFramebuffer *vfb, int x, int y, int w, int h);

	// Used by DrawPixels
	unsigned int drawPixelsTex_;
	GEBufferFormat drawPixelsTexFormat_;
	int drawPixelsTexW_;
	int drawPixelsTexH_;

	u8 *convBuf_;
	u32 convBufSize_;
	GLSLProgram *draw2dprogram_;
	GLSLProgram *plainColorProgram_;
	GLSLProgram *postShaderProgram_;
	GLSLProgram *stencilUploadProgram_;
	int plainColorLoc_;
	int timeLoc_;
	int pixelDeltaLoc_;
	int deltaLoc_;

	TextureCacheGLES *textureCacheGL_;
	ShaderManagerGLES *shaderManager_;
	DrawEngineGLES *drawEngine_;

	bool resized_;

	// Not used under ES currently.
	AsyncPBO *pixelBufObj_; //this isn't that large
	u8 currentPBO_;
};
