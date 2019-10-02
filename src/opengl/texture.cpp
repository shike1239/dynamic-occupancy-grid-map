#include "opengl/texture.h"
#include "cuda_utils.h"

#include <cstdio>
#include <stdio.h>
#include <memory.h>

Texture::Texture(int width, int height, float anisotropy_level)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

	if (anisotropy_level > 0.0f)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy_level);
	}

	float color[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

	CHECK_ERROR(cudaGraphicsGLRegisterImage(&resource, texture, GL_TEXTURE_2D, cudaGraphicsRegisterFlagsSurfaceLoadStore));

	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
	glDeleteTextures(1, &texture);
}

void Texture::beginCudaAccess(cudaSurfaceObject_t* surfaceObject)
{
	CHECK_ERROR(cudaGraphicsMapResources(1, &resource, 0));

	cudaArray_t cudaArray;
	CHECK_ERROR(cudaGraphicsSubResourceGetMappedArray(&cudaArray, resource, 0, 0));

	cudaResourceDesc resourceDesc;
	memset(&resourceDesc, 0, sizeof(cudaResourceDesc));
	resourceDesc.resType = cudaResourceTypeArray;
	resourceDesc.res.array.array = cudaArray;

	CHECK_ERROR(cudaCreateSurfaceObject(surfaceObject, &resourceDesc));
}

void Texture::endCudaAccess(cudaSurfaceObject_t surfaceObject)
{
	CHECK_ERROR(cudaGraphicsUnmapResources(1, &resource, 0));
	CHECK_ERROR(cudaDestroySurfaceObject(surfaceObject));	
}

void Texture::generateMipMap()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::bind(GLuint texUnit)
{
	unit = texUnit;
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, texture);
}
