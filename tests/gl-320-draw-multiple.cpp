//**********************************
// OpenGL multiple draw base vertex
// 04/12/2009 - 23/02/2013
//**********************************
// Christophe Riccio
// ogl-samples@g-truc.net
//**********************************
// G-Truc Creation
// www.g-truc.net
//**********************************

#include <glf/glf.hpp>

namespace
{
	glf::window Window("gl-320-draw-multiple");

	char const * VERT_SHADER_SOURCE("gl-320/draw-multiple.vert");
	char const * FRAG_SHADER_SOURCE("gl-320/draw-multiple.frag");

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(glm::uint32);
	glm::uint32 const ElementData[ElementCount] =
	{
		0, 1, 2,
		0, 2, 3
	};

	GLsizei const VertexCount(8);
	GLsizeiptr const PositionSize = VertexCount * sizeof(glm::vec3);
	glm::vec3 const PositionData[VertexCount] =
	{
		glm::vec3(-1.0f,-1.0f, 0.5f),
		glm::vec3( 1.0f,-1.0f, 0.5f),
		glm::vec3( 1.0f, 1.0f, 0.5f),
		glm::vec3(-1.0f, 1.0f, 0.5f),
		glm::vec3(-0.5f,-1.0f,-0.5f),
		glm::vec3( 0.5f,-1.0f,-0.5f),
		glm::vec3( 1.5f, 1.0f,-0.5f),
		glm::vec3(-1.5f, 1.0f,-0.5f)
	};

	GLsizei const Count[2] = {ElementCount, ElementCount};
	GLint const BaseVertex[2] = {0, 4};
	
	namespace shader
	{
		enum type
		{
			VERT,
			FRAG,
			MAX
		};
	}//namespace shader

	std::vector<GLuint> ShaderName(shader::MAX);
	GLuint VertexArrayName;
	GLuint ProgramName;
	GLuint ArrayBufferName;
	GLuint ElementBufferName;
	GLint UniformMVP;
	GLint UniformDiffuse;
}//namespace

bool initDebugOutput()
{
#	ifdef GL_ARB_debug_output
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageCallbackARB(&glf::debugOutput, NULL);
#	endif

	return glf::checkError("initDebugOutput");
}

bool initProgram()
{
	bool Validated = true;
	
	glf::compiler Compiler;

	// Create program
	if(Validated)
	{
		ShaderName[shader::VERT] = Compiler.create(GL_VERTEX_SHADER, glf::DATA_DIRECTORY + VERT_SHADER_SOURCE, "--version 150 --profile core");
		ShaderName[shader::FRAG] = Compiler.create(GL_FRAGMENT_SHADER, glf::DATA_DIRECTORY + FRAG_SHADER_SOURCE, "--version 150 --profile core");
		Validated = Validated && Compiler.check();

		ProgramName = glCreateProgram();
		glAttachShader(ProgramName, ShaderName[shader::VERT]);
		glAttachShader(ProgramName, ShaderName[shader::FRAG]);

		glBindAttribLocation(ProgramName, glf::semantic::attr::POSITION, "Position");
		glBindFragDataLocation(ProgramName, glf::semantic::frag::COLOR, "Color");

		glLinkProgram(ProgramName);
		Validated = Validated && glf::checkProgram(ProgramName);
	}

	// Get variables locations
	if(Validated)
	{
		UniformMVP = glGetUniformLocation(ProgramName, "MVP");
		UniformDiffuse = glGetUniformLocation(ProgramName, "Diffuse");
	}

	return Validated && glf::checkError("initProgram");
}

bool initBuffer()
{
	glGenBuffers(1, &ArrayBufferName);
	glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
	glBufferData(GL_ARRAY_BUFFER, PositionSize, PositionData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &ElementBufferName);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferName);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return glf::checkError("initBuffer");
}

bool initVertexArray()
{
	glGenVertexArrays(1, &VertexArrayName);
	glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, ArrayBufferName);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	bool Validated = true;

	if(Validated && glf::checkExtension("GL_ARB_debug_output"))
		Validated = initDebugOutput();
	if(Validated)
		Validated = initProgram();
	glf::checkError("initProgram Apple workaround");
	if(Validated)
		Validated = initBuffer();
	if(Validated)
		Validated = initVertexArray();

	return Validated && glf::checkError("begin");
}

bool end()
{
	for(std::size_t i = 0; 0 < shader::MAX; ++i)
		glDeleteShader(ShaderName[i]);
	glDeleteBuffers(1, &ArrayBufferName);
	glDeleteBuffers(1, &ElementBufferName);
	glDeleteProgram(ProgramName);
	glDeleteVertexArrays(1, &VertexArrayName);

	return glf::checkError("end");
}

void display()
{
	glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	glViewport(0, 0, Window.Size.x, Window.Size.y);

	float Depth(1.0f);
	glClearBufferfv(GL_DEPTH, 0, &Depth);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)[0]);

	glUseProgram(ProgramName);
	glUniform4fv(UniformDiffuse, 1, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);
	glUniformMatrix4fv(UniformMVP, 1, GL_FALSE, &MVP[0][0]);

// Bug fix for cross platform build...
#	if defined(WIN32)// || defined(__APPLE__)
#		define CONV(x)		x
#	else
#		define CONV(x)		(GLvoid **)x
#	endif

	GLvoid const * Indexes[2] = {0, 0};
	
	glBindVertexArray(VertexArrayName);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferName);
	
	//glDrawElementsBaseVertex(GL_TRIANGLES, Count[0], GL_UNSIGNED_INT, Indexes[0], BaseVertex[0]);

	glMultiDrawElementsBaseVertex(
		GL_TRIANGLES,
		Count,
		GL_UNSIGNED_INT,
		Indexes,//CONV(Indexes),
		2,
		BaseVertex);


	glf::checkError("display");
}

int main(int argc, char* argv[])
{
	return glf::run(argc, argv, glf::CORE, 3, 2);
}

