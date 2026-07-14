#ifndef BASE_API_H
#define BASE_API_H

// From https://github.com/TheSpydog/SDL_gpu_examples/blob/main/Examples/Common.h

#include <SDL3/SDL.h>



typedef struct Context
{
	const char* Name;
	const char* BasePath;
	SDL_Window* Window; // TODO this will be used initially , may be discarded when using the OpenXR 
	int windowWidth;
	int windowHeigth;
	//Add a setting options here
	SDL_GPUDevice* Device;
	float DeltaTime;

} Context;



int CommonInit(Context* context, SDL_WindowFlags windowFlags);
void CommonQuit(Context* context);

void InitializeAssetLoader();

SDL_GPUShader* LoadShader(
	SDL_GPUDevice* device,
	const char* shaderFilename,
	Uint32 samplerCount,
	Uint32 uniformBufferCount,
	Uint32 storageBufferCount,
	Uint32 storageTextureCount
);


SDL_GPUComputePipeline* CreateComputePipelineFromShader(
	SDL_GPUDevice* device,
	const char* shaderFilename,
	SDL_GPUComputePipelineCreateInfo* createInfo
);

// Vertex Formats
typedef struct PositionVertex
{
	float x, y, z;
} PositionVertex;

typedef struct PositionColorVertex
{
	float x, y, z;
	Uint8 r, g, b, a;
} PositionColorVertex;

typedef struct PositionTextureVertex
{
    float x, y, z;
    float u, v;
} PositionTextureVertex;


// typedef struct Application
// {
// 	const char* Name;
// 	int (*Init)(Context* context);
// 	int (*Update)(Context* context);
// 	int (*Draw)(Context* context);
// 	void (*Quit)(Context* context);
// } Application;


#endif