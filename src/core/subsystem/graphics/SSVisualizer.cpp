#include "SSVisualizer.h"
#include <profiler/AutoProfiler.h>
const pString SSVisualizer::Name = "Equalizer";
int SSVisualizer::ID = -1;

void SSVisualizer::Startup( SubsystemCollection* const subsystemCollection ) {
	//set up FMOD
	FMOD_System_Create(&m_SoundSystem);
	FMOD_System_SetSoftwareFormat(m_SoundSystem, 48000, FMOD_SOUND_FORMAT_PCM16, 2, 0, FMOD_DSP_RESAMPLER_LINEAR);
	FMOD_System_Init(m_SoundSystem, 32, FMOD_INIT_NORMAL, nullptr);
	FMOD_System_CreateStream(m_SoundSystem, "../../../asset/music/Metroid_The_Crimson_Depths_OC_ReMix.mp3", FMOD_LOOP_NORMAL | FMOD_2D | FMOD_HARDWARE | FMOD_UNIQUE, nullptr, &m_Song);
	FMOD_System_PlaySound(m_SoundSystem, FMOD_CHANNEL_FREE, m_Song, false, &m_Channel);

	//set up GL
	m_RenderProgram.LoadCompleteShaderProgramFromFile("../../../shader/ParticleRender.glsl", true);
	glGenBuffers(1, &m_VBO);
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle2), nullptr);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle2), (unsigned char*) nullptr + sizeof(glm::vec4));
	glBufferData(GL_ARRAY_BUFFER, sizeof(Particle2) * SPECTRUM_SIZE, nullptr, GL_DYNAMIC_DRAW);
	//set up allocator
	m_Allocator = new StackAllocator(sizeof(Particle2) * SPECTRUM_SIZE * 3); //get a bit more than needed but oh well
	m_Marker = m_Allocator->GetMarker();
}

void SSVisualizer::Shutdown( SubsystemCollection* const subsystemCollection ) {
	// Perform Cleanup here (Don't forget to set shutdown order priority!)
	FMOD_Sound_Release(m_Song);
	FMOD_System_Close(m_SoundSystem);
	FMOD_System_Release(m_SoundSystem);
	if (m_Allocator) delete m_Allocator;
}

void SSVisualizer::UpdateUserLayer( const float deltaTime ) {
	// Perform non-simulation update logic here (Don't forget to set update order priority!)
	FMOD_System_Update(m_SoundSystem);
	//Get spectrum data
	PROFILE(AutoProfiler memAllocProfiler("VisualizerAllocation", Profiler::PROFILER_CATEGORY_STANDARD, true, true));
#ifdef USE_STACK_ALLOC
	float* leftSpectrum = (float*)m_Allocator->Allocate(sizeof(float) * SPECTRUM_SIZE);
	float* rightSpectrum = (float*)m_Allocator->Allocate(sizeof(float) * SPECTRUM_SIZE);
	Particle2* particles = (Particle2*)m_Allocator->Allocate(sizeof(Particle2) * SPECTRUM_SIZE);
#else
	float* leftSpectrum = (float*)malloc(sizeof(float) * SPECTRUM_SIZE);
	float* rightSpectrum = (float*)malloc(sizeof(float) * SPECTRUM_SIZE);
	Particle2* particles = (Particle2*)malloc(sizeof(Particle2) * SPECTRUM_SIZE);
#endif
	PROFILE(memAllocProfiler.Stop());

	FMOD_Channel_GetSpectrum(m_Channel, leftSpectrum, SPECTRUM_SIZE, 0, FMOD_DSP_FFT_WINDOW_TRIANGLE);
	FMOD_Channel_GetSpectrum(m_Channel, rightSpectrum, SPECTRUM_SIZE, 1, FMOD_DSP_FFT_WINDOW_TRIANGLE);
	int res = 8;
	float maxH;
	for (int i = 0; i < SPECTRUM_SIZE / res; ++i) {
		float h = -0.5f + glm::max((rightSpectrum[i] + rightSpectrum[i]) * 5.0f - 0.04f, 0.0f) + 0.015f;
		maxH = glm::max(h + 0.5f, maxH);
		for (int k = 0; k < res; k++) {
			int index = i * res + k;
			particles[index].Pos = glm::vec4((index / (float)SPECTRUM_SIZE) * 2.0f - 1.0f, h, 0.1f, 1);
			particles[index].Color = m_Color;
		}
	}
	maxH = glm::max(maxH, 0.0f);
	m_Color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) * maxH + glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * (1.0f - maxH);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle2) * SPECTRUM_SIZE, particles);
	m_RenderProgram.Apply();
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_LINE_STRIP, 0, SPECTRUM_SIZE);

	PROFILE(AutoProfiler memDellocProfiler("VisualizerDeallocation", Profiler::PROFILER_CATEGORY_STANDARD, true, true));
#ifdef USE_STACK_ALLOC
	m_Allocator->Unwind(m_Marker);
#else
	free(leftSpectrum);
	free(rightSpectrum);
	free(particles);
#endif
	PROFILE(memDellocProfiler.Stop());
}

void SSVisualizer::UpdateSimulationLayer( const float timeStep ) {
	// Perform simulation update logic here (Don't forget to set update order priority!)
}

Subsystem* SSVisualizer::CreateNew( ) const {
	return pNew( SSVisualizer, SSVisualizer::ID );
}

int SSVisualizer::GetStaticID() {
	return SSVisualizer::ID;
}
