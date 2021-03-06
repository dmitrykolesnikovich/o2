#pragma once

#include "Utils/Containers/Vector.h"
#include "Utils/Data/DataNode.h"
#include "Utils/IObject.h"
#include "Utils/Reflection/Reflection.h"

using namespace o2;

namespace o2
{
	class UIWidget;
}

namespace Editor
{
	// -------------------------------
	// Editor property field interface
	// -------------------------------
	class IPropertyField: public IObject
	{
	public:
		typedef Pair<void*, const void*> TargetPair;
		typedef Vector<Pair<void*, void*>> TargetsVec;

	public:
		Function<void()> onChanged; // Immediate change value by user event

		Function<void(const String&, const Vector<DataNode>&, const Vector<DataNode>&)> onChangeCompleted; // Change completed by user event

		// Virtual destructor
		virtual ~IPropertyField() {}

		// Sets targets pointers
		virtual void SetValueAndPrototypePtr(const TargetsVec& targets, bool isProperty) {}

		// Sets targets pointers
		virtual void SetValuePtr(const Vector<void*>& targets, bool isProperty);

		// Checks common value and fill fields
		virtual void Refresh() {}

		// Reverts value to prototype value
		virtual void Revert() {}

		// Returns control widget
		virtual UIWidget* GetWidget() const { return nullptr; }

		// Returns editing by this field type
		virtual const Type* GetFieldType() const { return &TypeOf(void); }

		// Sets reflection path f target values
		void SetValuePath(const String& path);

		// Returns reflection path of target values
		const String& GetValuePath() const;

		// Specializes field type
		virtual void SpecializeType(const Type* type) {}

		IOBJECT(IPropertyField);

	protected:
		// Checks value for reverting to prototype
		virtual void CheckRevertableState() {}

		// It is called when field value changed
		virtual void OnChanged();

		// Stores values to data
		virtual void StoreValues(Vector<DataNode>& data) const {}

	protected:
		String           mValuesPath;         // Reflection path of target values
		Vector<DataNode> mBeforeChangeValues; // Serialized value data before changes started
	};
}
