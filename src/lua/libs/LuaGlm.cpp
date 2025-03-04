#include "LuaGlm.h"
#include <fmt/format.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define TYPECHECK(a, table, tname, L, errmsg) if constexpr (LUA_GLM_TYPECHECKED) { \
bool v = a[sol::metatable_key].get<sol::object>() == table[tname]; \
if(!v){luaL_error(L, errmsg);} while(false);} 


std::string LuaGlm::glm_vec2_to_string(const glm::dvec2& v)
{
	return fmt::format("({}, {})", v.x, v.y);
}

std::string LuaGlm::glm_vec3_to_string(const glm::dvec3& v)
{
	return fmt::format("({}, {}, {})", v.x, v.y, v.z);
}

std::string LuaGlm::glm_vec4_to_string(const glm::dvec4& v)
{
	return fmt::format("({}, {}, {}, {})", v.x, v.y, v.z, v.w);
}

std::string LuaGlm::glm_mat3_to_string(const glm::dmat3 & v)
{
	return fmt::format("({}, {}, {} | {}, {}, {} | {}, {}, {})",
		v[0][0], v[0][1], v[0][2],
		v[1][0], v[1][1], v[1][2],
		v[2][0], v[2][1], v[2][2]);
}

std::string LuaGlm::glm_mat4_to_string(const glm::dmat4& v)
{
	return fmt::format("({}, {}, {}, {} | {}, {}, {}, {} | {}, {}, {}, {} | {}, {}, {}, {})",
		v[0][0], v[0][1], v[0][2], v[0][3],
		v[1][0], v[1][1], v[1][2], v[1][3],
		v[2][0], v[2][1], v[2][2], v[2][3],
		v[3][0], v[3][1], v[3][2], v[3][3]);
}

std::string LuaGlm::glm_quat_to_string(const glm::dquat& v)
{
	return fmt::format("q({}, {}, {}, {})", v.w, v.x, v.y, v.z);
}

void LuaGlm::load_to(sol::table& table)
{

	using VEC4 = const glm::dvec4&;
	using VEC3 = const glm::dvec3&;
	using VEC2 = const glm::dvec2&;

	using MAT4 = const glm::dmat4&;
	using MAT3 = const glm::dmat3&;

	using QUAT = const glm::dquat&;
		
	sol::usertype<glm::dvec2> dvec2_ut = table.new_usertype<glm::dvec2>("vec2",
		sol::constructors < glm::dvec2(), glm::dvec2(double, double), glm::dvec2(glm::dvec3), glm::dvec2(glm::dvec4)>(),
		sol::meta_function::addition, sol::overload(
			sol::resolve<glm::dvec2(const glm::dvec2&, const glm::dvec2&)>(glm::operator+),
			sol::resolve<glm::dvec2(const glm::dvec2&, double)>(glm::operator+),
			sol::resolve<glm::dvec2(double, const glm::dvec2&)>(glm::operator+)
			),
		sol::meta_function::subtraction, sol::overload(
			sol::resolve<glm::dvec2(const glm::dvec2&, const glm::dvec2&)>(glm::operator-),
			sol::resolve<glm::dvec2(const glm::dvec2&, double)>(glm::operator-),
			sol::resolve<glm::dvec2(double, const glm::dvec2&)>(glm::operator-)
		),
		sol::meta_function::multiplication, sol::overload(
			sol::resolve<glm::dvec2(const glm::dvec2&, const glm::dvec2&)>(glm::operator*),
			sol::resolve<glm::dvec2(const glm::dvec2&, double)>(glm::operator*),
			sol::resolve<glm::dvec2(double, const glm::dvec2&)>(glm::operator*)
		),
		sol::meta_function::division, sol::overload(
			sol::resolve<glm::dvec2(const glm::dvec2&, const glm::dvec2&)>(glm::operator/),
			sol::resolve<glm::dvec2(const glm::dvec2&, double)>(glm::operator/),
			sol::resolve<glm::dvec2(double, const glm::dvec2&)>(glm::operator/)
		),
		sol::meta_function::unary_minus, sol::resolve<glm::dvec2(const glm::dvec2&)>(&glm::operator-),
		sol::meta_function::to_string, &glm_vec2_to_string,
		"x", &glm::dvec2::x, "y", &glm::dvec2::y,
		"unpack", [](glm::dvec2 v) {return std::make_tuple(v.x, v.y); });

	sol::usertype<glm::dvec3> dvec3_ut = table.new_usertype<glm::dvec3>("vec3",
		sol::constructors < glm::dvec3(), glm::dvec3(glm::dvec2, double),
		glm::dvec3(double, double, double), glm::dvec3(glm::dvec4)>(),
		sol::meta_function::addition, sol::overload(
			sol::resolve<glm::dvec3(const glm::dvec3&, const glm::dvec3&)>(glm::operator+),
			sol::resolve<glm::dvec3(const glm::dvec3&, double)>(glm::operator+),
			sol::resolve<glm::dvec3(double, const glm::dvec3&)>(glm::operator+)
		),
		sol::meta_function::subtraction, sol::overload(
			sol::resolve<glm::dvec3(const glm::dvec3&, const glm::dvec3&)>(glm::operator-),
			sol::resolve<glm::dvec3(const glm::dvec3&, double)>(glm::operator-),
			sol::resolve<glm::dvec3(double, const glm::dvec3&)>(glm::operator-)
		),
		sol::meta_function::multiplication, sol::overload(
			sol::resolve<glm::dvec3(const glm::dvec3&, const glm::dvec3&)>(glm::operator*),
			sol::resolve<glm::dvec3(const glm::dvec3&, double)>(glm::operator*),
			sol::resolve<glm::dvec3(double, const glm::dvec3&)>(glm::operator*),
			sol::resolve<glm::dvec3(const glm::dvec3&, const glm::dmat3&)>(glm::operator*)
		),
		sol::meta_function::division, sol::overload(
			sol::resolve<glm::dvec3(const glm::dvec3&, const glm::dvec3&)>(glm::operator/),
			sol::resolve<glm::dvec3(const glm::dvec3&, double)>(glm::operator/),
			sol::resolve<glm::dvec3(double, const glm::dvec3&)>(glm::operator/),
			sol::resolve<glm::dvec3(const glm::dvec3&, const glm::dmat3&)>(glm::operator/) 
		),
		sol::meta_function::unary_minus, sol::resolve<glm::dvec3(const glm::dvec3&)>(&glm::operator-),
		sol::meta_function::to_string, &glm_vec3_to_string,
		"x", &glm::dvec3::x, "y", &glm::dvec3::y, "z", &glm::dvec3::z,
		"to_vec2", [](glm::dvec3 v) {return glm::dvec2(v); },
		"unpack", [](glm::dvec3 v) {return std::make_tuple(v.x, v.y, v.z); });

	sol::usertype<glm::dvec4> dvec4_ut = table.new_usertype<glm::dvec4>("vec4",
		sol::constructors < glm::dvec4(), glm::dvec4(glm::dvec3, double),
		glm::dvec4(double, double, double, double)>(),
		sol::meta_function::addition, sol::overload(
			sol::resolve<glm::dvec4(const glm::dvec4&, const glm::dvec4&)>(glm::operator+),
			sol::resolve<glm::dvec4(const glm::dvec4&, double)>(glm::operator+),
			sol::resolve<glm::dvec4(double, const glm::dvec4&)>(glm::operator+)
		),
		sol::meta_function::subtraction, sol::overload(
			sol::resolve<glm::dvec4(const glm::dvec4&, const glm::dvec4&)>(glm::operator-),
			sol::resolve<glm::dvec4(const glm::dvec4&, double)>(glm::operator-),
			sol::resolve<glm::dvec4(double, const glm::dvec4&)>(glm::operator-)
		),
		sol::meta_function::multiplication, sol::overload(
			sol::resolve<glm::dvec4(const glm::dvec4&, const glm::dvec4&)>(glm::operator*),
			sol::resolve<glm::dvec4(const glm::dvec4&, double)>(glm::operator*),
			sol::resolve<glm::dvec4(double, const glm::dvec4&)>(glm::operator*),
			sol::resolve<glm::dvec4(const glm::dmat4&, const glm::dvec4&)>(glm::operator*),
			sol::resolve<glm::dvec4(const glm::dvec4&, const glm::dmat4&)>(glm::operator*)
		),
		sol::meta_function::division, sol::overload(
			sol::resolve<glm::dvec4(const glm::dvec4&, const glm::dvec4&)>(glm::operator/),
			sol::resolve<glm::dvec4(const glm::dvec4&, double)>(glm::operator/),
			sol::resolve<glm::dvec4(double, const glm::dvec4&)>(glm::operator/),
			sol::resolve<glm::dvec4(const glm::dmat4&, const glm::dvec4&)>(glm::operator/),
			sol::resolve<glm::dvec4(const glm::dvec4&, const glm::dmat4&)>(glm::operator/)
		),
		sol::meta_function::unary_minus, sol::resolve<glm::dvec4(const glm::dvec4&)>(&glm::operator-),
		sol::meta_function::to_string, &glm_vec4_to_string,
		"x", &glm::dvec4::x, "y", &glm::dvec4::y, "z", &glm::dvec4::z, "w", &glm::dvec4::w,
		"to_vec2", [](glm::dvec4 v) {return glm::dvec2(v); },
		"to_vec3", [](glm::dvec4 v) {return glm::dvec3(v); },
		"unpack", [](glm::dvec4 v) {return std::make_tuple(v.x, v.y, v.z, v.w); });

	// We only implement the typically used matrices, dmat3 and dmat4
	sol::usertype<glm::dmat3> dmat3_ut = table.new_usertype<glm::dmat3>("mat3",
		sol::constructors <glm::dmat3(), glm::dmat3(double), 
		glm::dmat3(VEC3, VEC3, VEC3)>(),
		sol::meta_function::addition, sol::overload(
			sol::resolve<glm::dmat3(const glm::dmat3&, const glm::dmat3&)>(glm::operator+),
			sol::resolve<glm::dmat3(const glm::dmat3&, double)>(glm::operator+),
			sol::resolve<glm::dmat3(double, const glm::dmat3&)>(glm::operator+)
		),
		sol::meta_function::subtraction, sol::overload(
			sol::resolve<glm::dmat3(const glm::dmat3&, const glm::dmat3&)>(glm::operator-),
			sol::resolve<glm::dmat3(const glm::dmat3&, double)>(glm::operator-),
			sol::resolve<glm::dmat3(double, const glm::dmat3&)>(glm::operator-)
		),
		sol::meta_function::multiplication, sol::overload(
			sol::resolve<glm::dmat3(const glm::dmat3&, const glm::dmat3&)>(glm::operator*),
			sol::resolve<glm::dmat3(const glm::dmat3&, double)>(glm::operator*),
			sol::resolve<glm::dmat3(double, const glm::dmat3&)>(glm::operator*),
			sol::resolve<glm::dvec3(const glm::dmat3&, const glm::dvec3&)>(glm::operator*)
		),
		sol::meta_function::division, sol::overload(
			sol::resolve<glm::dmat3(const glm::dmat3&, const glm::dmat3&)>(glm::operator/),
			sol::resolve<glm::dmat3(const glm::dmat3&, double)>(glm::operator/),
			sol::resolve<glm::dmat3(double, const glm::dmat3&)>(glm::operator/),
			sol::resolve<glm::dvec3(const glm::dmat3&, const glm::dvec3&)>(glm::operator/)
		),
		sol::meta_function::to_string, &glm_mat3_to_string);
	
	sol::usertype<glm::dmat4> dmat4_ut = table.new_usertype<glm::dmat4>("mat4",
		sol::constructors <glm::dmat4(), glm::dmat4(double),
		glm::dmat4(VEC4, VEC4, VEC4, VEC4)>(),
		sol::meta_function::addition, sol::overload(
			sol::resolve<glm::dmat4(const glm::dmat4&, const glm::dmat4&)>(glm::operator+),
			sol::resolve<glm::dmat4(const glm::dmat4&, const double&)>(glm::operator+),
			sol::resolve<glm::dmat4(const double&, const glm::dmat4&)>(glm::operator+)
		),
		sol::meta_function::subtraction, sol::overload(
			sol::resolve<glm::dmat4(const glm::dmat4&, const glm::dmat4&)>(glm::operator-),
			sol::resolve<glm::dmat4(const glm::dmat4&, const double&)>(glm::operator-),
			sol::resolve<glm::dmat4(const double&, const glm::dmat4&)>(glm::operator-)
		),
		sol::meta_function::multiplication, sol::overload(
			sol::resolve<glm::dmat4(const glm::dmat4&, const glm::dmat4&)>(glm::operator*),
			sol::resolve<glm::dmat4(const glm::dmat4&, const double&)>(glm::operator*),
			sol::resolve<glm::dmat4(const double&, const glm::dmat4&)>(glm::operator*),
			sol::resolve<glm::dvec4(const glm::dmat4&, const glm::dvec4&)>(glm::operator*)
		),
		sol::meta_function::division, sol::overload(
			sol::resolve<glm::dmat4(const glm::dmat4&, const glm::dmat4&)>(glm::operator/),
			sol::resolve<glm::dmat4(const glm::dmat4&, const double&)>(glm::operator/),
			sol::resolve<glm::dmat4(const double&, const glm::dmat4&)>(glm::operator/),
			sol::resolve<glm::dvec4(const glm::dmat4&, const glm::dvec4&)>(glm::operator/)
		),
		sol::meta_function::to_string, &glm_mat4_to_string);
		
		sol::usertype<glm::dquat> dquat_ut = table.new_usertype<glm::dquat>("quat",
		sol::constructors < glm::dquat(), glm::dquat(double, double, double, double),
		glm::dquat(double, VEC3), glm::dquat(VEC3), glm::dquat(MAT3), glm::dquat(MAT4)>(),
		sol::meta_function::multiplication, sol::overload(
			sol::resolve<glm::dquat(const glm::dquat&, const glm::dquat&)>(glm::operator*),
			sol::resolve<glm::dvec3(const glm::dquat&, const glm::dvec3&)>(glm::operator*),
			sol::resolve<glm::dvec3(const glm::dvec3&, const glm::dquat&)>(glm::operator*),
			sol::resolve<glm::dvec4(const glm::dquat&, const glm::dvec4&)>(glm::operator*),
			sol::resolve<glm::dvec4(const glm::dvec4&, const glm::dquat&)>(glm::operator*)
		),
		sol::meta_function::unary_minus, sol::resolve<glm::dquat(const glm::dquat&)>(&glm::operator-),
		sol::meta_function::to_string, &glm_quat_to_string,
		"w", &glm::dquat::w, "x", &glm::dquat::x, "y", &glm::dquat::y, "z", &glm::dquat::z,
		"unpack", [](glm::dquat v) {return std::make_tuple(v.w, v.x, v.y, v.z); });


// ================================================================
//	Core functions
// ================================================================

	table.set_function("abs", sol::overload
	(
		sol::resolve<double(double)>(glm::abs),
		sol::resolve<glm::dvec4(VEC4)>(glm::abs),
		sol::resolve<glm::dvec3(VEC3)>(glm::abs),
		sol::resolve<glm::dvec2(VEC2)>(glm::abs)
	));

	table.set_function("ceil", sol::overload
	(
		sol::resolve<double(double)>(glm::ceil),
		sol::resolve<glm::dvec4(VEC4)>(glm::ceil),
		sol::resolve<glm::dvec3(VEC3)>(glm::ceil),
		sol::resolve<glm::dvec2(VEC2)>(glm::ceil)
	));

	table.set_function("floor", sol::overload
	(
		sol::resolve<double(double)>(glm::floor),
		sol::resolve<glm::dvec4(VEC4)>(glm::floor),
		sol::resolve<glm::dvec3(VEC3)>(glm::floor),
		sol::resolve<glm::dvec2(VEC2)>(glm::floor)
	));

	table.set_function("round", sol::overload
	(
		sol::resolve<double(double)>(glm::round),
		sol::resolve<glm::dvec4(VEC4)>(glm::round),
		sol::resolve<glm::dvec3(VEC3)>(glm::round),
		sol::resolve<glm::dvec2(VEC2)>(glm::round)
	));


	table.set_function("clamp", sol::overload
	(
		sol::resolve<double(double, double, double)>(glm::clamp),
		sol::resolve<glm::dvec4(VEC4, double, double)>(glm::clamp),
		sol::resolve<glm::dvec3(VEC3, double, double)>(glm::clamp),
		sol::resolve<glm::dvec2(VEC2, double, double)>(glm::clamp),
		sol::resolve<glm::dvec4(VEC4, VEC4, VEC4)>(glm::clamp),
		sol::resolve<glm::dvec3(VEC3, VEC3, VEC3)>(glm::clamp),
		sol::resolve<glm::dvec2(VEC2, VEC2, VEC2)>(glm::clamp)
	));

	table.set_function("fract", sol::overload
	(
		sol::resolve<double(double)>(glm::fract),
		sol::resolve<glm::dvec4(VEC4)>(glm::fract),
		sol::resolve<glm::dvec3(VEC3)>(glm::fract),
		sol::resolve<glm::dvec2(VEC2)>(glm::fract)
	));

	table.set_function("isinf", sol::resolve<bool(double)>(glm::isinf));

	table.set_function("isnan", sol::resolve<bool(double)>(glm::isnan));

	table.set_function("max", sol::overload
	(
		sol::resolve<double(double, double)>(glm::max),
		sol::resolve<glm::dvec4(VEC4, double)>(glm::max),
		sol::resolve<glm::dvec3(VEC3, double)>(glm::max),
		sol::resolve<glm::dvec2(VEC2, double)>(glm::max),
		sol::resolve<glm::dvec4(VEC4, VEC4)>(glm::max),
		sol::resolve<glm::dvec3(VEC3, VEC3)>(glm::max),
		sol::resolve<glm::dvec2(VEC2, VEC2)>(glm::max)
	));

	table.set_function("min", sol::overload
	(
		sol::resolve<double(double, double)>(glm::min),
		sol::resolve<glm::dvec4(VEC4, double)>(glm::min),
		sol::resolve<glm::dvec3(VEC3, double)>(glm::min),
		sol::resolve<glm::dvec2(VEC2, double)>(glm::min),
		sol::resolve<glm::dvec4(VEC4, VEC4)>(glm::min),
		sol::resolve<glm::dvec3(VEC3, VEC3)>(glm::min),
		sol::resolve<glm::dvec2(VEC2, VEC2)>(glm::min)
	));

	table.set_function("mix", sol::overload
	(
		sol::resolve<glm::dquat(QUAT, QUAT, double)>(glm::mix),
		sol::resolve<double(double, double, double)>(glm::mix),
		sol::resolve<glm::dvec2(VEC2, VEC2, double)>(glm::mix),
		sol::resolve<glm::dvec3(VEC3, VEC3, double)>(glm::mix),
		sol::resolve<glm::dvec4(VEC4, VEC4, double)>(glm::mix)
	));

	table.set_function("mod", sol::overload
	(
		sol::resolve<double(double, double)>(glm::mod),
		sol::resolve<glm::dvec4(VEC4, VEC4)>(glm::mod),
		sol::resolve<glm::dvec3(VEC3, VEC3)>(glm::mod),
		sol::resolve<glm::dvec2(VEC2, VEC2)>(glm::mod)
	));

	// modf not implemented

	table.set_function("sign", sol::overload
	(
		sol::resolve<double(double)>(glm::sign),
		sol::resolve<glm::dvec4(VEC4)>(glm::sign),
		sol::resolve<glm::dvec3(VEC3)>(glm::sign),
		sol::resolve<glm::dvec2(VEC2)>(glm::sign)
	));

	table.set_function("smoothstep", sol::overload
	(
		sol::resolve<double(double, double, double)>(glm::smoothstep),
		sol::resolve<glm::dvec2(VEC2, VEC2, VEC2)>(glm::smoothstep),
		sol::resolve<glm::dvec3(VEC3, VEC3, VEC3)>(glm::smoothstep),
		sol::resolve<glm::dvec4(VEC4, VEC4, VEC4)>(glm::smoothstep)
	));

	// step not implemented

	table.set_function("trunc", sol::overload
	(
		sol::resolve<double(double)>(glm::trunc),
		sol::resolve<glm::dvec2(VEC2)>(glm::trunc),
		sol::resolve<glm::dvec3(VEC3)>(glm::trunc),
		sol::resolve<glm::dvec4(VEC4)>(glm::trunc)
	));
	

// ================================================================
//	Exponential functions
// ================================================================

	table.set_function("exp", sol::overload
	(
		sol::resolve<double(double)>(glm::exp),
		sol::resolve<glm::dvec4(VEC4)>(glm::exp),
		sol::resolve<glm::dvec3(VEC3)>(glm::exp),
		sol::resolve<glm::dvec2(VEC2)>(glm::exp)
	));

	table.set_function("exp2", sol::overload
	(
		sol::resolve<double(double)>(glm::exp2),
		sol::resolve<glm::dvec4(VEC4)>(glm::exp2),
		sol::resolve<glm::dvec3(VEC3)>(glm::exp2),
		sol::resolve<glm::dvec2(VEC2)>(glm::exp2)
	));

	table.set_function("inversesqrt", sol::overload
	(
		sol::resolve<double(double)>(glm::inversesqrt),
		sol::resolve<glm::dvec4(VEC4)>(glm::inversesqrt),
		sol::resolve<glm::dvec3(VEC3)>(glm::inversesqrt),
		sol::resolve<glm::dvec2(VEC2)>(glm::inversesqrt)
	));

	table.set_function("log", sol::overload
	(
		sol::resolve<double(double)>(glm::log),
		sol::resolve<glm::dvec4(VEC4)>(glm::log),
		sol::resolve<glm::dvec3(VEC3)>(glm::log),
		sol::resolve<glm::dvec2(VEC2)>(glm::log)
	));

	table.set_function("log2", sol::overload
	(
		sol::resolve<double(double)>(glm::log2),
		sol::resolve<glm::dvec4(VEC4)>(glm::log2),
		sol::resolve<glm::dvec3(VEC3)>(glm::log2),
		sol::resolve<glm::dvec2(VEC2)>(glm::log2)
	));

	table.set_function("pow", sol::overload
	(
		sol::resolve<double(double, double)>(glm::pow),
		sol::resolve<glm::dvec4(VEC4, VEC4)>(glm::pow),
		sol::resolve<glm::dvec3(VEC3, VEC3)>(glm::pow),
		sol::resolve<glm::dvec2(VEC2, VEC2)>(glm::pow)
	));

	table.set_function("sqrt", sol::overload
	(
		sol::resolve<double(double)>(glm::sqrt),
		sol::resolve<glm::dvec4(VEC4)>(glm::sqrt),
		sol::resolve<glm::dvec3(VEC3)>(glm::sqrt),
		sol::resolve<glm::dvec2(VEC2)>(glm::sqrt)
	));


// ================================================================
//	Geometric functions
// ================================================================

	table.set_function("conjugate", sol::resolve<glm::dquat(QUAT)>(glm::conjugate));

	table.set_function("cross", sol::overload
	(
		sol::resolve<glm::dquat(QUAT, QUAT)>(glm::cross),
		sol::resolve<glm::dvec3(VEC3, VEC3)>(glm::cross)
	));

	table.set_function("distance", sol::overload
	(
		sol::resolve<double(VEC4, VEC4)>(glm::distance),
		sol::resolve<double(VEC3, VEC3)>(glm::distance),
		sol::resolve<double(VEC2, VEC2)>(glm::distance)
	));


	table.set_function("dot", sol::overload
	(
		sol::resolve<double(QUAT, QUAT)>(glm::dot),
		sol::resolve<double(VEC4, VEC4)>(glm::dot),
		sol::resolve<double(VEC3, VEC3)>(glm::dot),
		sol::resolve<double(VEC2, VEC2)>(glm::dot)
	));

	table.set_function("length", sol::overload
	(
		sol::resolve<double(VEC4)>(glm::length),
		sol::resolve<double(VEC3)>(glm::length),
		sol::resolve<double(VEC2)>(glm::length)
	));

	table.set_function("normalize", sol::overload
	(
		sol::resolve<glm::dquat(QUAT)>(glm::normalize),
		sol::resolve<glm::dvec4(VEC4)>(glm::normalize),
		sol::resolve<glm::dvec3(VEC3)>(glm::normalize),
		sol::resolve<glm::dvec2(VEC2)>(glm::normalize)
	));

	table.set_function("reflect", sol::overload
	(
		sol::resolve<glm::dvec4(VEC4, VEC4)>(glm::reflect),
		sol::resolve<glm::dvec3(VEC3, VEC3)>(glm::reflect),
		sol::resolve<glm::dvec2(VEC2, VEC2)>(glm::reflect)
	));

	table.set_function("refract", sol::overload
	(
		sol::resolve<glm::dvec4(VEC4, VEC4, double)>(glm::refract),
		sol::resolve<glm::dvec3(VEC3, VEC3, double)>(glm::refract),
		sol::resolve<glm::dvec2(VEC2, VEC2, double)>(glm::refract)
	));
	


// ================================================================
//	Matrix functions
// ================================================================

	table.set_function("determinant", sol::overload
	(
		sol::resolve<double(MAT4)>(glm::determinant),
		sol::resolve<double(MAT3)>(glm::determinant)
	));

	table.set_function("inverse", sol::overload
	(
		sol::resolve<glm::dquat(QUAT)>(glm::inverse),
		sol::resolve<glm::dmat4(MAT4)>(glm::inverse),
		sol::resolve<glm::dmat3(MAT3)>(glm::inverse)
	));

	table.set_function("matrix_comp_mult", sol::overload
	(
		sol::resolve<glm::dmat4(MAT4, MAT4)>(glm::matrixCompMult),
		sol::resolve<glm::dmat3(MAT3, MAT3)>(glm::matrixCompMult)
	));

	table.set_function("transpose", sol::overload
	(
		sol::resolve<glm::dmat4(MAT4)>(glm::transpose),
		sol::resolve<glm::dmat3(MAT3)>(glm::transpose)
	));

// ================================================================
//	Trigonometric functions
// ================================================================

	table.set_function("acos", sol::overload
	(
		sol::resolve<double(double)>(glm::acos),
		sol::resolve<glm::dvec4(VEC4)>(glm::acos),
		sol::resolve<glm::dvec3(VEC3)>(glm::acos),
		sol::resolve<glm::dvec2(VEC2)>(glm::acos)
	));

	table.set_function("acosh", sol::overload
	(
		sol::resolve<double(double)>(glm::acosh),
		sol::resolve<glm::dvec4(VEC4)>(glm::acosh),
		sol::resolve<glm::dvec3(VEC3)>(glm::acosh),
		sol::resolve<glm::dvec2(VEC2)>(glm::acosh)
	));

	table.set_function("asin", sol::overload
	(
		sol::resolve<double(double)>(glm::asin),
		sol::resolve<glm::dvec4(VEC4)>(glm::asin),
		sol::resolve<glm::dvec3(VEC3)>(glm::asin),
		sol::resolve<glm::dvec2(VEC2)>(glm::asin)
	));

	table.set_function("asinh", sol::overload
	(
		sol::resolve<double(double)>(glm::asinh),
		sol::resolve<glm::dvec4(VEC4)>(glm::asinh),
		sol::resolve<glm::dvec3(VEC3)>(glm::asinh),
		sol::resolve<glm::dvec2(VEC2)>(glm::asinh)
	));

	table.set_function("atan", sol::overload
	(
		sol::resolve<double(double)>(glm::atan),
		sol::resolve<glm::dvec4(VEC4)>(glm::atan),
		sol::resolve<glm::dvec3(VEC3)>(glm::atan),
		sol::resolve<glm::dvec2(VEC2)>(glm::atan)
	));

	table.set_function("atanh", sol::overload
	(
		sol::resolve<double(double)>(glm::atanh),
		sol::resolve<glm::dvec4(VEC4)>(glm::atanh),
		sol::resolve<glm::dvec3(VEC3)>(glm::atanh),
		sol::resolve<glm::dvec2(VEC2)>(glm::atanh)
	));

	table.set_function("cos", sol::overload
	(
		sol::resolve<double(double)>(glm::cos),
		sol::resolve<glm::dvec4(VEC4)>(glm::cos),
		sol::resolve<glm::dvec3(VEC3)>(glm::cos),
		sol::resolve<glm::dvec2(VEC2)>(glm::cos)
	));

	table.set_function("cosh", sol::overload
	(
		sol::resolve<double(double)>(glm::cosh),
		sol::resolve<glm::dvec4(VEC4)>(glm::cosh),
		sol::resolve<glm::dvec3(VEC3)>(glm::cosh),
		sol::resolve<glm::dvec2(VEC2)>(glm::cosh)
	));

	table.set_function("degrees", sol::overload
	(
		sol::resolve<double(double)>(glm::degrees),
		sol::resolve<glm::dvec4(VEC4)>(glm::degrees),
		sol::resolve<glm::dvec3(VEC3)>(glm::degrees),
		sol::resolve<glm::dvec2(VEC2)>(glm::degrees)
	));

	table.set_function("radians", sol::overload
	(
		sol::resolve<double(double)>(glm::radians),
		sol::resolve<glm::dvec4(VEC4)>(glm::radians),
		sol::resolve<glm::dvec3(VEC3)>(glm::radians),
		sol::resolve<glm::dvec2(VEC2)>(glm::radians)
	));


	table.set_function("sin", sol::overload
	(
		sol::resolve<double(double)>(glm::sin),
		sol::resolve<glm::dvec4(VEC4)>(glm::sin),
		sol::resolve<glm::dvec3(VEC3)>(glm::sin),
		sol::resolve<glm::dvec2(VEC2)>(glm::sin)
	));

	table.set_function("sinh", sol::overload
	(
		sol::resolve<double(double)>(glm::sinh),
		sol::resolve<glm::dvec4(VEC4)>(glm::sinh),
		sol::resolve<glm::dvec3(VEC3)>(glm::sinh),
		sol::resolve<glm::dvec2(VEC2)>(glm::sinh)
	));

	table.set_function("tan", sol::overload
	(
		sol::resolve<double(double)>(glm::tan),
		sol::resolve<glm::dvec4(VEC4)>(glm::tan),
		sol::resolve<glm::dvec3(VEC3)>(glm::tan),
		sol::resolve<glm::dvec2(VEC2)>(glm::tan)
	));

	table.set_function("tanh", sol::overload
	(
		sol::resolve<double(double)>(glm::tanh),
		sol::resolve<glm::dvec4(VEC4)>(glm::tanh),
		sol::resolve<glm::dvec3(VEC3)>(glm::tanh),
		sol::resolve<glm::dvec2(VEC2)>(glm::tanh)
	));

	////////////////////////////////////////////////////
	/// Matrix transform functions
	////////////////////////////////////////////////////
	table.set_function("frustum",
		sol::resolve<glm::dmat4(double, double, double, double, double, double)>(glm::frustum));

	table.set_function("infinite_perspective",
		sol::resolve<glm::dmat4(double, double, double)>(glm::infinitePerspective));

	table.set_function("look_at",
		sol::resolve<glm::dmat4(VEC3, VEC3, VEC3)>(glm::lookAt));

	table.set_function("ortho", sol::overload(
		sol::resolve<glm::dmat4(double, double, double, double)>(glm::ortho),
		sol::resolve<glm::dmat4(double, double, double, double, double, double)>(glm::ortho)));

	table.set_function("perspective",
		sol::resolve<glm::dmat4(double, double, double, double)>(glm::perspective));

	table.set_function("perspective_fov",
		sol::resolve<glm::dmat4(double, double, double, double, double)>(glm::perspectiveFov));

	// TODO: implemen pickMatrix, project, unproject, etc... (maybe they will never be used)

	table.set_function("translate",
		sol::resolve<glm::dmat4(MAT4, VEC3)>(glm::translate));

	table.set_function("scale",
	   	sol::resolve<glm::dmat4(MAT4, VEC3)>(glm::scale));

	table.set_function("rotate", sol::overload
			(
		sol::resolve<glm::dmat4(MAT4, double, VEC3)>(glm::rotate),
		sol::resolve<glm::dquat(QUAT, const double&, VEC3)>(glm::rotate)
	));

	table["pi"] = glm::pi<double>();
	table["half_pi"] = glm::half_pi<double>();
	table["two_pi"] = glm::two_pi<double>();
}

LuaGlm::LuaGlm()
{

}


LuaGlm::~LuaGlm()
{
}
