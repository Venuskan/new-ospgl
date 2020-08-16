#pragma once
#include <glad/glad.h>
#include <string>
#include <util/Logger.h>

#include <glm/gtc/type_ptr.hpp>
#include <cpptoml.h>

// Not only do we load the shader, and make it easily usable
// we also run a preprocessor to allow includes
class Shader
{
private:


	// For the preprocessor
	std::string pkg;
	std::string name;
	
	// Run basic preprocessing on shader
	// We allow:
	// #include <absolute_path>		(Allows packages)
	// #include "relative_path"		(Doesn't allow packages, just relative paths)
	// (#define is already done by the GLSL compiler)
	// There MUST be an space after the macro name (#include<test> is INVALID) 
	// Relative paths are very strict, always use ../ for going back, etc...
	std::string preprocessor(const std::string& file);


	// Decent improvement on some drivers as for some reason the glGetUniformLocation
	// stalls the whole thing
	std::unordered_map<std::string, GLint> uniform_locations;

public:

	GLuint id;

	inline GLint get_uniform_location(const std::string& name)
	{
		auto it = uniform_locations.find(name);
		if(it == uniform_locations.end())
		{
			GLint n_pos = glGetUniformLocation(id, name.c_str());
			uniform_locations[name] = n_pos;
			return n_pos;
		}
		else
		{
			return it->second;	
		}
		
	}

	void use();

	inline void setBool(const std::string &name, bool value)
	{
		glUniform1i(get_uniform_location(name), (int)value);
	}
	inline void setInt(const std::string &name, int value)
	{
		glUniform1i(get_uniform_location(name), value);
	}

	inline void setFloat(const std::string &name, float value)
	{
		glUniform1f(get_uniform_location(name), value);
	}

	inline void setVec2(const std::string& name, glm::vec2 value)
	{
		glUniform2f(get_uniform_location(name), value.x, value.y);
	}

	inline void setVec3(const std::string& name, glm::vec3 value)
	{
		glUniform3f(get_uniform_location(name), value.x, value.y, value.z);
	}

	inline void setVec4(const std::string& name, glm::vec4 value)
	{
		glUniform4f(get_uniform_location(name), value.x, value.y, value.z, value.w);
	}

	inline void setMat4(const std::string& name, glm::mat4 value)
	{
		glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	inline void setMat3(const std::string& name, glm::mat3 value)
	{
		glUniformMatrix3fv(get_uniform_location(name), 1, GL_FALSE, glm::value_ptr(value));
	}

	Shader(const std::string& vertexData, const std::string& fragmentData, const std::string& pkg, const std::string& path);
	~Shader();
};

Shader* load_shader(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg);


