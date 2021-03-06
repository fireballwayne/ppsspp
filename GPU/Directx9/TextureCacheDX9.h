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

#include <map>
#include <d3d9.h>

#include "../Globals.h"
#include "GPU/GPU.h"
#include "GPU/GPUInterface.h"
#include "GPU/Directx9/TextureScalerDX9.h"
#include "GPU/Common/TextureCacheCommon.h"

struct VirtualFramebuffer;

namespace DX9 {

class FramebufferManagerDX9;
class DepalShaderCacheDX9;
class ShaderManagerDX9;

class TextureCacheDX9 : public TextureCacheCommon {
public:
	TextureCacheDX9(Draw::DrawContext *draw);
	~TextureCacheDX9();

	void SetTexture(bool force = false);

	void Clear(bool delete_them);
	void StartFrame();

	void SetFramebufferManager(FramebufferManagerDX9 *fbManager);
	void SetDepalShaderCache(DepalShaderCacheDX9 *dpCache) {
		depalShaderCache_ = dpCache;
	}
	void SetShaderManager(ShaderManagerDX9 *sm) {
		shaderManager_ = sm;
	}

	// Only used by Qt UI?
	bool DecodeTexture(u8 *output, const GPUgstate &state);

	void ForgetLastTexture() override;

	void SetFramebufferSamplingParams(u16 bufferWidth, u16 bufferHeight);

	void ApplyTexture();

protected:
	void Unbind() override;

private:
	void Decimate();  // Run this once per frame to get rid of old textures.
	void DeleteTexture(TexCache::iterator it);
	void UpdateSamplingParams(TexCacheEntry &entry, bool force);
	void LoadTextureLevel(TexCacheEntry &entry, ReplacedTexture &replaced, int level, int maxLevel, bool replaceImages, int scaleFactor, u32 dstFmt);
	D3DFORMAT GetDestFormat(GETextureFormat format, GEPaletteFormat clutFormat) const;
	TexCacheEntry::Status CheckAlpha(const u32 *pixelData, u32 dstFmt, int stride, int w, int h);
	u32 GetCurrentClutHash();
	void UpdateCurrentClut(GEPaletteFormat clutFormat, u32 clutBase, bool clutIndexIsSimple);
	void ApplyTextureFramebuffer(TexCacheEntry *entry, VirtualFramebuffer *framebuffer);

	bool CheckFullHash(TexCacheEntry *const entry, bool &doDelete);
	bool HandleTextureChange(TexCacheEntry *const entry, const char *reason, bool initialMatch, bool doDelete);
	void BuildTexture(TexCacheEntry *const entry, bool replaceImages);

	LPDIRECT3DTEXTURE9 &DxTex(TexCacheEntry *entry) {
		return *(LPDIRECT3DTEXTURE9 *)&entry->texturePtr;
	}
	void ReleaseTexture(TexCacheEntry *entry) {
		LPDIRECT3DTEXTURE9 &texture = DxTex(entry);
		if (texture) {
			texture->Release();
			texture = nullptr;
		}
	}

	TextureScalerDX9 scaler;

	u32 clutHash_;

	LPDIRECT3DVERTEXDECLARATION9 pFramebufferVertexDecl;

	LPDIRECT3DTEXTURE9 lastBoundTexture;
	float maxAnisotropyLevel;

	int decimationCounter_;
	int texelsScaledThisFrame_;
	int timesInvalidatedAllThisFrame_;

	FramebufferManagerDX9 *framebufferManagerDX9_;
	DepalShaderCacheDX9 *depalShaderCache_;
	ShaderManagerDX9 *shaderManager_;
};

D3DFORMAT getClutDestFormat(GEPaletteFormat format);

};
