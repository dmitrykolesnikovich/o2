#pragma once

#include "Utils/Memory/MemoryManager.h"

namespace o2
{
	class Type;

	template<typename _type, typename _getter>
	const Type& GetTypeOf();

	template<typename T>
	struct RegularTypeGetter;

	template<typename T, typename X>
	struct GetTypeHelper; 
	
	template<typename _type>
	struct TypeSampleCreator;

	class TypeInitializer;
	class Reflection;

	// ----------------------------------------------------
	// Basic object interface with type information support
	// ----------------------------------------------------
	class IObject
	{
	public:
		// Virtual destructor
		virtual ~IObject() {}

		// Cloning interface
		virtual IObject* Clone() const = 0;

		// Returns type
		virtual const Type& GetType() const = 0;

	private:
		static Type* type;

		template<typename _type, typename _getter>
		friend const Type& o2::GetTypeOf();

		template<typename T>
		friend struct RegularTypeGetter;

		template<typename T, typename X>
		friend struct o2::GetTypeHelper;

		friend class TypeInitializer;
		friend class Reflection;
	};

	// IObject header definition
#define IOBJECT(CLASS)  							   \
private:                                               \
	static o2::Type* type;							   \
                                                       \
    template<typename _type, typename _getter>         \
	friend const o2::Type& o2::GetTypeOf();            \
                                                       \
	template<typename T>                               \
	friend struct o2::RegularTypeGetter;               \
                                                       \
	template<typename T, typename X>                   \
	friend struct o2::GetTypeHelper;                   \
                                                       \
    template<typename _type>                           \
    friend struct o2::TypeSampleCreator;               \
                                                       \
    friend class o2::TypeInitializer;                  \
    friend class o2::Reflection;                       \
    friend class o2::DataNode;                         \
                                                       \
public:                                                \
	typedef CLASS thisclass;                           \
	CLASS* Clone() const { return mnew CLASS(*this); } \
	const o2::Type& GetType() const { return *type; }; \
                                                       \
private:                                               \
	static void InitializeType(o2::Type* type)                       
}
