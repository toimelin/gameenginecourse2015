#include "SkyProgram.h"
#include "RenderQueue.h"
#include "ShaderBank.h"
#include <glm/gtx/transform.hpp>
#include <resourcing/ResourceManager.h>
#include <resourcing/ModelBank.h>
using namespace gfx;

SkyProgram::SkyProgram() {

}

SkyProgram::~SkyProgram() {
	delete m_Tex;
	g_ResourceManager.ReleaseResource( m_Model );
}

void SkyProgram::Init() {
	//load Shader
	m_Shader = g_ShaderBank.LoadShaderProgram("../../../shader/DeferedSky.glsl");
	//Load model
	m_Model = HashResourceName("Model.SkyCube");
	g_ResourceManager.AquireResource(m_Model);
}
void SkyProgram::Render(const CameraData& camera) {
	ShaderProgram* prog = g_ShaderBank.GetProgramFromHandle(m_Shader);
	prog->Apply();
	prog->SetUniformMat4("g_ProjView", camera.ProjView);
	prog->SetUniformMat4("g_World", glm::translate(camera.Position));
	m_Tex->Apply(prog->FetchUniform("g_SkyTex"), 0);
	g_ModelBank.ApplyBuffers();
	ModelResource* model = (ModelResource*)g_ResourceManager.GetResourcePointer(m_Model);
	if (model == nullptr) return;
	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);
	glDrawElements(GL_TRIANGLES, model->Meshes[0].IndexCount, GL_UNSIGNED_INT,
		(GLvoid*)(0 + ((model->IndexOffset + model->Meshes[0].IndexOffset) * sizeof(unsigned int))));
	glCullFace(GL_BACK);
}
void SkyProgram::SetSkyTexture(const char* filename) {
	if (m_Tex) {
		delete m_Tex;
		m_Tex = new Texture();
	} else {
		m_Tex = new Texture();
	}
	m_Tex->Init(filename, TEXTURE_CUBE);
}
void SkyProgram::SetSkyTexture(Texture* tex) {
	if (m_Tex) {
		delete m_Tex;
		m_Tex = tex;
	} else {
		m_Tex = tex;
	}
}
