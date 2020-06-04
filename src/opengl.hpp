#ifndef OPENGL_NO_DEFS
#include "gl_defs.h"
#include "glext_defs.h"
#endif


typedef void (*gl_debug_fn)(GLenum, GLenum, GLuint, GLenum, GLsizei, GLchar const*, void const*);


#define OPENGL_FUNCTIONS(X) \
	X(CreateShader, 				GLuint, 			(GLenum)																			) \
	X(DeleteShader,					void,				(GLuint)																			) \
	X(ShaderSource, 				void,				(GLuint, GLsizei, GLchar const* const*, GLuint const*)								) \
	X(CompileShader,				void,				(GLuint)																			) \
	X(GetShaderiv,					void,				(GLuint, GLenum, GLint*) 															) \
	X(GetShaderInfoLog,				void,				(GLuint, GLsizei, GLsizei*, GLchar*)												) \
	X(AttachShader,					void,				(GLuint, GLuint)																	) \
	X(CreateProgram,				GLuint,				()																					) \
	X(DeleteProgram,				void,				(GLuint)																			) \
	X(LinkProgram,					void,				(GLuint)																			) \
	X(UseProgram, 					void, 				(GLuint) 																			) \
	X(GetProgramiv, 				void, 				(GLuint, GLenum, GLint*) 															) \
	X(GetProgramInfoLog, 			void, 				(GLuint, GLsizei, GLsizei*, GLchar*)												) \
	X(GetUniformLocation,			GLint, 				(GLuint, GLchar const*)																) \
	X(ProgramUniformMatrix4fv,		void,				(GLuint, GLint, GLsizei, GLboolean, GLfloat const*)									) \
	X(ProgramUniform1i,				void, 				(GLuint, GLint, GLint)	 															) \
	X(ProgramUniform1iv,			void, 				(GLuint, GLint, GLsizei, GLint const*)	 											) \
	X(CreateTextures,				void, 				(GLenum, GLsizei, GLuint*)															) \
	X(DeleteTextures,				void, 				(GLsizei, GLuint const*) 															) \
	X(IsTexture, 					GLboolean, 			(GLuint) 																			) \
	X(TextureStorage2D,				void,				(GLuint, GLsizei, GLenum, GLsizei, GLsizei)											) \
	X(TextureParameteri,			void,				(GLuint, GLenum, GLint)																) \
	X(TextureSubImage2D, 			void, 				(GLuint, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void const*)		) \
	X(BindTextureUnit, 				void, 				(GLuint, GLuint)																	) \
	X(CreateBuffers,				void, 				(GLsizei, GLuint*)																	) \
	X(DeleteBuffers,				void, 				(GLsizei, GLuint const*)															) \
	X(BindBuffer,					void, 				(GLenum, GLuint)																	) \
	X(NamedBufferData,				void, 				(GLuint, GLsizeiptr, void const*, GLenum)											) \
	X(NamedBufferSubData, 			void, 				(GLuint, GLintptr, GLsizeiptr, void const*)											) \
	X(CreateVertexArrays,			void, 				(GLsizei, GLuint*)																	) \
	X(DeleteVertexArrays,			void, 				(GLsizei, GLuint const*)															) \
	X(BindVertexArray,				void,				(GLuint)																			) \
	X(VertexArrayVertexBuffer, 		void, 				(GLuint, GLuint, GLuint, GLintptr, GLsizei)											) \
	X(EnableVertexArrayAttrib,		void, 				(GLuint, GLuint) 																	) \
	X(VertexArrayAttribBinding,		void, 				(GLuint, GLuint, GLuint) 															) \
	X(VertexArrayElementBuffer, 	void, 				(GLuint, GLuint)																	) \
	X(VertexArrayAttribFormat, 		void, 				(GLuint, GLuint, GLint, GLenum, GLboolean, GLuint) 									) \
	X(VertexArrayAttribIFormat, 	void, 				(GLuint, GLuint, GLint, GLenum, GLuint) 											) \
	X(DrawElements, 				void, 				(GLenum, GLsizei, GLenum, void const*) 												) \
	X(DrawElementsBaseVertex, 		void, 				(GLenum, GLsizei, GLenum, void*, GLint) 											) \
	X(GetIntegerv, 					void, 				(GLenum, GLint*) 																	) \
	X(GetString, 					GLubyte const*, 	(GLenum)																			) \
	X(GetStringi, 					GLubyte const*, 	(GLenum, GLuint)																	) \
	X(BlendFunc, 					void, 				(GLenum, GLenum) 																	) \
	X(BlendEquation, 				void, 				(GLenum) 																			) \
	X(BlendEquationSeparate, 		void, 				(GLenum, GLenum) 																	) \
	X(BlendFuncSeparate,			void, 				(GLenum, GLenum, GLenum, GLenum) 													) \
	X(Enable, 						void,				(GLenum) 																			) \
	X(Disable, 						void,				(GLenum) 																			) \
	X(Scissor, 						void, 				(GLint, GLint, GLsizei, GLsizei)													) \
	X(Viewport, 					void, 				(GLint, GLint, GLsizei, GLsizei) 													) \
	X(ClearColor, 					void, 				(GLclampf, GLclampf, GLclampf, GLclampf) 											) \
	X(Clear, 						void, 				(GLenum) 																			) \
	X(DebugMessageCallback, 		void, 				(gl_debug_fn, void const*) 															) \
	X(DebugMessageControl, 			void, 				(GLenum, GLenum, GLenum, GLsizei, GLuint const*, GLboolean) 						)


extern "C" {
	#define _X(name, ret, params) typedef ret gl##name##_fn params;
	OPENGL_FUNCTIONS(_X)
	#undef _X
}


struct OpenGL {
	#define _X(name, ret, params) gl##name##_fn *name;
	OPENGL_FUNCTIONS(_X)
	#undef _X
};