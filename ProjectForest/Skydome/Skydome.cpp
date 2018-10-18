#include "Skydome.h"

CSkyDome::CSkyDome()
{
	Samples = 4;
	Kr = 0.0030f;
	Km = 0.0015f;
	ESun = 16.0f;
	g = -0.995f;
	//g = -1.0f;
	Exposure = 16.0f;
	InnerRadius = 10.0f;
	OuterRadius = 10.25f;
	Scale = 1.0f;
	ScaleDepth = 0.25f;
	ScaleOverScaleDepth = Scale / ScaleDepth;

	CameraPos = glm::vec3(0.0f, InnerRadius, 0.0f);

	WavelengthRed = 0.650f;
	WavelengthGreen = 0.570f;
	WavelengthBlue = 0.475f;

	WavelengthRed4 = glm::pow(WavelengthRed, 4.0f);
	WavelengthGreen4 = glm::pow(WavelengthGreen, 4.0f);
	WavelengthBlue4 = glm::pow(WavelengthBlue, 4.0f);

	InvWaveLength = glm::vec3(1.0f / WavelengthRed4, 1.0f / WavelengthGreen4, 1.0f / WavelengthBlue4);

	SunRadius = 25.0f;
	SunRotVec = glm::normalize(glm::vec3(0.0f, -0.66f, -1.0f));
	SunCPos = glm::rotate(glm::vec3(-512.0f, 0.0f, 0.0f), -10.0f, SunRotVec);

	SunCDir = glm::vec3(0.0f);
	SunWPos = glm::vec3(0.0f);

	RefractionFactor = 0.0f;

	LightColor = glm::vec3(0.0f);
	AmbientIntensity = 0.0f;
	DiffuseIntensity = 0.0f;
}

CSkyDome::~CSkyDome()
{

}

void CSkyDome::SetShaderProgramObject(GLuint SPO)
{
	this->ShaderProgramObjectSky = SPO;
}

GLuint CSkyDome::GetShaderProgramObject(void)
{
	return(this->ShaderProgramObjectSky);
}

void CSkyDome::InitializeSky(GLuint ShaderObject)
{
	this->ShaderProgramObjectSky = ShaderObject;

	glUseProgram(ShaderProgramObjectSky);
	glUniform3fv(glGetUniformLocation(ShaderProgramObjectSky, "v3CameraPos"), 1, glm::value_ptr(CameraPos));
	glUniform3fv(glGetUniformLocation(ShaderProgramObjectSky, "v3InvWaveLength"), 1, glm::value_ptr(InvWaveLength));
	//glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fCameraHeight"), InnerRadius);
	//glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fCameraHeight2"), InnerRadius * InnerRadius);
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fCameraHeight"), glm::length(CameraPos));
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fCameraHeight2"), glm::length2(CameraPos));
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fInnerRadius"), InnerRadius);
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fInnerRadius2"), InnerRadius * InnerRadius);
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fOuterRadius"), OuterRadius);
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fOuterRadius2"), OuterRadius * OuterRadius);
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fKrESun"), Kr * ESun);
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fKmESun"), Km * ESun);
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fKr4PI"), Kr * 4.0f * glm::pi<float>());
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fKm4PI"), Km * 4.0f * glm::pi<float>());
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fScale"), Scale);
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fScaleDepth"), ScaleDepth);
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fScaleOverScaleDepth"), ScaleOverScaleDepth);
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "g"), g);
	glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "g2"), g * g);
	glUniform1i(glGetUniformLocation(ShaderProgramObjectSky, "Samples"), Samples);
	glUseProgram(0);

	ComputeSkyDome();
}

void CSkyDome::ComputeSkyDome(void)
{
	glm::vec3 *SkyDomeVertices = new glm::vec3[112 * 3], va, vb, vc, vd;

	float stepa = glm::pi<float>() * 2.0f / 16;
	float startb = asin(InnerRadius / OuterRadius);
	float stepb = (glm::half_pi<float>() - startb) / 4;
	int pos = 0;

	for (int y = 0; y < 3; y++)
	{
		float b = startb + stepb * y;

		for (int x = 0; x < 16; x++)
		{
			float a = stepa * x;

			va = glm::vec3(sin(a) * cos(b) * OuterRadius, sin(b) * OuterRadius, -cos(a) * cos(b) * OuterRadius);
			vb = glm::vec3(sin(a + stepa) * cos(b) * OuterRadius, sin(b) * OuterRadius, -cos(a + stepa) * cos(b) * OuterRadius);
			vc = glm::vec3(sin(a + stepa) * cos(b + stepb) * OuterRadius, sin(b + stepb) * OuterRadius, -cos(a + stepa) * cos(b + stepb) * OuterRadius);
			vd = glm::vec3(sin(a) * cos(b + stepb) * OuterRadius, sin(b + stepb) * OuterRadius, -cos(a) * cos(b + stepb) * OuterRadius);

			SkyDomeVertices[pos + 0] = va;
			SkyDomeVertices[pos + 1] = vb;
			SkyDomeVertices[pos + 2] = vc;

			pos += 3;

			SkyDomeVertices[pos + 0] = vc;
			SkyDomeVertices[pos + 1] = vd;
			SkyDomeVertices[pos + 2] = va;

			pos += 3;
		}
	}

	float b = startb + stepb * 3;

	for (int x = 0; x < 16; x++)
	{
		float a = stepa * x;

		va = glm::vec3(sin(a) * cos(b) * OuterRadius, sin(b) * OuterRadius, -cos(a) * cos(b) * OuterRadius);
		vb = glm::vec3(sin(a + stepa) * cos(b) * OuterRadius, sin(b) * OuterRadius, -cos(a + stepa) * cos(b) * OuterRadius);
		vc = glm::vec3(0, OuterRadius, 0);

		SkyDomeVertices[pos + 0] = va;
		SkyDomeVertices[pos + 1] = vb;
		SkyDomeVertices[pos + 2] = vc;

		pos += 3;
	}

	glGenVertexArrays(1, &VAOSkyDome);
	glBindVertexArray(VAOSkyDome);

	//Vbo_Model_Vertex
	glGenBuffers(1, &VBOSkyDomePosition);
	glBindBuffer(GL_ARRAY_BUFFER, VBOSkyDomePosition);
	glBufferData(GL_ARRAY_BUFFER, 112 * 3 * 3 * sizeof(float), SkyDomeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(OGL_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(OGL_ATTRIBUTE_VERTEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void CSkyDome::RenderSky(glm::vec3 CamPos, glm::mat4 MVP)
{
	this->CamPos = CamPos;
	glDisable(GL_DEPTH_TEST);
	glUseProgram(ShaderProgramObjectSky);
	
	glUniformMatrix4fv(glGetUniformLocation(ShaderProgramObjectSky, "u_mvp_matrix"), 1, GL_FALSE, glm::value_ptr(MVP));
	glUniform3fv(glGetUniformLocation(ShaderProgramObjectSky, "v3LightPos"), 1, glm::value_ptr(SunCDir));
	
	glBindVertexArray(VAOSkyDome);
	glDrawArrays(GL_TRIANGLES, 0, 112 * 3);
	glBindVertexArray(0);
	
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
}

void CSkyDome::UpdateSky(GLfloat fDelta)
{
	SunCDir = glm::normalize(SunCPos);
	SunWPos = CamPos + SunCPos;
	RefractionFactor = (1.0f - glm::sqrt(glm::max(0.0f, SunCDir.y)));
	SunColor = 1.0f - glm::vec3(0.0f, 0.5f, 1.0f) * RefractionFactor;

	LightColor = 1.0f - glm::vec3(0.0f, 0.25f, 0.5f) * RefractionFactor;
	AmbientIntensity = 0.0625f + 0.1875f * glm::min(1.0f, glm::max(0.0f, (0.375f + SunCDir.y) / 0.25f));
	DiffuseIntensity = 0.75f * glm::min(1.0f, glm::max(0.0f, (0.03125f + SunCDir.y) / 0.0625f));

	SunCPos = glm::rotate(SunCPos, TransitionSpeed * fDelta, SunRotVec);
}

GLfloat CSkyDome::GetRefractionFactor(void)
{
	return(this->RefractionFactor);
}

GLfloat CSkyDome::GetSunCDirY(void)
{
	return(this->SunCDir.y);
}

glm::vec3 CSkyDome::GetSunCPos(void)
{
	return(this->SunCPos);
}

glm::vec3 CSkyDome::GetSunCDir(void)
{
	return(this->SunCDir);
}

glm::vec3 CSkyDome::GetSunColor(void)
{
	return(this->SunColor);
}

GLfloat CSkyDome::GetInnerRadius(void)
{
	return(this->InnerRadius);
}

GLfloat CSkyDome::GetSunRadius(void)
{
	return(this->SunRadius);
}

glm::vec3 CSkyDome::GetLightColor(void)
{
	return(this->LightColor);
}

glm::vec3 CSkyDome::GetSunWPos(void)
{
	return(this->SunWPos);
}

GLfloat CSkyDome::GetAmbientIntensity(void)
{
	return(this->AmbientIntensity);
}

GLfloat CSkyDome::GetDiffuseIntensity(void)
{
	return(this->DiffuseIntensity);
}

void CSkyDome::SetSunCDirY(GLfloat SunCDirY)
{
	this->SunCDir.y = SunCDirY;
}

void CSkyDome::SetSunWPos(glm::vec3 SunWPos)
{
	this->SunWPos = SunWPos;
}

void CSkyDome::SetSunCPos(glm::vec3 SunCPos)
{
	this->SunCPos = SunCPos;
}

void CSkyDome::SetSunCDir(glm::vec3 SunCDir)
{
	this->SunCDir = SunCDir;
}
