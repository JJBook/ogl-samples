//**********************************
// OpenGL Program
// 17/11/2013 - 17/11/2013
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
	glf::window Window("gl-320-program-uniform");

	char const * VERT_SHADER_SOURCE("gl-320/program-uniform.vert");
	char const * FRAG_SHADER_SOURCE("gl-320/program-uniform.frag");

	GLsizei const VertexCount(4);
	GLsizeiptr const PositionSize = VertexCount * sizeof(glm::vec2);
	glm::vec2 const PositionData[VertexCount] =
	{
		glm::vec2(-1.0f,-1.0f),
		glm::vec2( 1.0f,-1.0f),
		glm::vec2( 1.0f, 1.0f),
		glm::vec2(-1.0f, 1.0f)
	};

	GLsizei const ElementCount(6);
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLushort);
	GLushort const ElementData[ElementCount] =
	{
		0, 1, 2,
		2, 3, 0
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			TRANSFORM,
			MATERIAL,
			MAX
		};
	}//namespace buffer
	
	GLuint ProgramName(0);
	std::vector<GLuint> BufferName(buffer::MAX);
	GLuint VertexArrayName(0);
	GLint UniformTransform(0);
	GLint UniformDiffuse(0);
}//namespace

bool initDebugOutput()
{
#	ifdef GL_ARB_debug_output
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		glDebugMessageCallbackARB(&glf::debugOutput, NULL);
#	endif

	return true;
}

bool initProgram()
{
	bool Validated = true;

	glf::compiler Compiler;
	
	// Create program
	if(Validated)
	{
		GLuint VertShaderName = Compiler.create(GL_VERTEX_SHADER, glf::DATA_DIRECTORY + VERT_SHADER_SOURCE, "--version 150 --profile core");
		GLuint FragShaderName = Compiler.create(GL_FRAGMENT_SHADER, glf::DATA_DIRECTORY + FRAG_SHADER_SOURCE, "--version 150 --profile core");
		Validated = Validated && Compiler.check();

		ProgramName = glCreateProgram();
		glAttachShader(ProgramName, VertShaderName);
		glAttachShader(ProgramName, FragShaderName);
		glDeleteShader(VertShaderName);
		glDeleteShader(FragShaderName);

		glBindAttribLocation(ProgramName, glf::semantic::attr::POSITION, "Position");
		glBindFragDataLocation(ProgramName, glf::semantic::frag::COLOR, "Color");
		glLinkProgram(ProgramName);
		Validated = Validated && glf::checkProgram(ProgramName);
	}

	// Get variables locations
	if(Validated)
	{
		UniformDiffuse = glGetUniformLocation(ProgramName, "Diffuse");
		UniformTransform = glGetUniformBlockIndex(ProgramName, "transform");
	}
	
	return Validated && glf::checkError("initProgram");
}

bool initVertexArray()
{
	// Build a vertex array object
	glGenVertexArrays(1, &VertexArrayName);
	glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool initBuffer()
{
	// Generate buffer objects
	glGenBuffers(buffer::MAX, &BufferName[0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, PositionSize, PositionData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLint UniformBlockSize = 0;

	{
		glGetActiveUniformBlockiv(
			ProgramName, 
			UniformTransform,
			GL_UNIFORM_BLOCK_DATA_SIZE,
			&UniformBlockSize);

		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glBufferData(GL_UNIFORM_BUFFER, UniformBlockSize, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	return glf::checkError("initBuffer");
}

bool begin()
{
	bool Validated = true;

	if(Validated && glf::checkExtension("GL_ARB_debug_output"))
		Validated = initDebugOutput();
	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initBuffer();
	if(Validated)
		Validated = initVertexArray();

	return Validated && glf::checkError("begin");
}

bool end()
{
	glDeleteVertexArrays(1, &VertexArrayName);
	glDeleteBuffers(buffer::MAX, &BufferName[0]);
	glDeleteProgram(ProgramName);

	return glf::checkError("end");
}

void display()
{
	// Update of the uniform buffer
	{
		glBindBuffer(GL_UNIFORM_BUFFER, BufferName[buffer::TRANSFORM]);
		glm::mat4* Pointer = (glm::mat4*)glMapBufferRange(
			GL_UNIFORM_BUFFER, 0,	sizeof(glm::mat4),
			GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

		glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.0f);
		glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
		glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
		glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;
		
		*Pointer = MVP;
		
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	glViewport(0, 0, Window.Size.x, Window.Size.y);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f)[0]);

	glUseProgram(ProgramName);
	glUniformBlockBinding(ProgramName, UniformTransform, glf::semantic::uniform::TRANSFORM0);
	
	glBindBufferBase(GL_UNIFORM_BUFFER, glf::semantic::uniform::TRANSFORM0, BufferName[buffer::TRANSFORM]);

	glm::vec4 Diffuse[2] = {glm::vec4(1.0f, 0.5f, 0.0f, 1.0f), glm::vec4(0.7f, 0.7f, 0.7f, 1.0f)};

	//glUniform4fv(UniformDiffuse, 2, &Diffuse[0][0]);
	glUniform4fv(UniformDiffuse + 0, 1, &Diffuse[0][0]);
	glUniform4fv(UniformDiffuse + 1, 1, &Diffuse[1][0]);

	glBindVertexArray(VertexArrayName);
	glDrawElementsInstanced(GL_TRIANGLES, ElementCount, GL_UNSIGNED_SHORT, NULL, 1);

	glf::checkError("display");
}

int main(int argc, char* argv[])
{
	return glf::run(argc, argv, glf::CORE, 3, 2);
}
