#pragma once

#include "Utils/Containers/Vector.h"
#include "Utils/Math/Math.h"
#include "Utils/Math/Vector2.h"
#include "Utils/Property.h"
#include "Utils/Serializable.h"

namespace o2
{
	// ------------
	// Bezier curve
	// ------------
	class Curve: public ISerializable
	{
	public:
		class Key;
		typedef Vector<Key> KeysVec;

	public:
		Accessor<float, float> value;         // Value accessor by position
		Accessor<Key, float>   key;           // Key accessor by position
		Property<KeysVec>      keys;          // Keys property
		Getter<float>          length;        // Max position getter
		Function<void()>       onKeysChanged; // Keys changed event

		// Default constructor
		Curve();

		// Constructor by bezier coefficient in 0...1 range (some line as css-bezier)
		Curve(float beginCoef, float beginCoefPosition, float endCoef, float endCoefPosition);

		// Constructor from keys with smoothing
		Curve(Vector<Vec2F> values, bool smooth = true);

		// Copy-constructor
		Curve(const Curve& other);

		// Is equals operator
		bool operator==(const Curve& other) const;

		// Is not equals operator
		bool operator!=(const Curve& other) const;

		// Assign operator
		Curve& operator=(const Curve& other);

		// Append other curve at end
		Curve& operator+=(const Curve& other);

		// Returns value by position
		float Evaluate(float position);

		// Moves all keys positions to offset
		void MoveKeys(float offset);

		// Moves keys from begin position positions to offset
		void MoveKeysFrom(float begin, float offset);

		// Appends curve at end
		void AppendCurve(const Curve& curve);

		// Prepends curve at end
		void PrependCurve(const Curve& curve);

		// Inserts curve at position
		void InsertCurve(const Curve& curve, float position);

		// Adds keys with smoothing at end
		void AppendKeys(Vector<Vec2F> values, bool smooth = true);

		// Adds keys at end
		void AppendKeys(const KeysVec& keys);

		// Adds keys with smoothing at beginning
		void PrependKeys(Vector<Vec2F> values, bool smooth = true);

		// Adds keys at beginning
		void PrependKeys(const KeysVec& keys);

		// Inserts keys with smoothing at position
		void InsertKeys(Vector<Vec2F> values, float position, bool smooth = true);

		// Inserts keys at position
		void InsertKeys(const KeysVec& keys, float position);

		// Inserts key and returns index
		int InsertKey(const Key& key);

		// Inserts key and returns index
		int InsertKey(float position, float value, float leftCoef, float leftCoefPosition,
					  float rightCoef, float rightCoefPosition);

		// Inserts value at position with smoothing and returns index
		int InsertKey(float position, float value, float smoothCoef = 1.0f);

		// Inserts flat key
		int InsertFlatKey(float position, float value);

		// Appends key at end with offset and returns index
		int AppendKey(float offset, float value, float leftCoef, float leftCoefPosition,
					  float rightCoef, float rightCoefPosition);

		// Appends key at end with offset and smoothing and returns index
		int AppendKey(float offset, float value, float smoothCoef = 1.0f);

		// Appends flat key at end with offset
		int AppendKey(float offset, float value);

		// Prepends key at beginning with offset and returns index
		int PrependKey(float offset, float value, float leftCoef, float leftCoefPosition,
					   float rightCoef, float rightCoefPosition);

		 // Prepends key at beginning with offset and smoothing and returns index
		int PrependKey(float offset, float value, float smoothCoef = 1.0f);

		// Prepends flat key at beginning with offset
		int PrependKey(float offset, float value);

		// Returns key at position
		Key GetKey(float position);

		// Returns key at index
		Key GetKeyAt(int idx);

		// Removes key at position
		bool RemoveKey(float position);

		// Removes key at index
		bool RemoveKeyAt(int idx);

		// Removes all keys
		void RemoveAllKeys();

		// Returns true if contains key at position
		bool ContainsKey(float position);

		// Returns keys array
		const KeysVec& GetKeys() const;

		// Sets keys
		void SetKeys(const KeysVec& keys);

		// Sets key data at position
		void SetKey(const Key& key, int position);

		// Smooths key at position
		void SmoothKey(float position, float smoothCoef = 1.0f);

		// Smooths key at index
		void SmoothKeyAt(int idx, float smoothCoef = 1.0f);

		// Returns curve max position
		float Length() const;

		// Returns true if curve is empty
		bool IsEmpty() const;

		// Returns bounding approximated keys rectangle
		RectF GetRect() const;

		// Key access operator by position
		Key operator[](float position);

		// Returns ease in curve
		static Curve EaseIn();

		// Returns ease out curve
		static Curve EaseOut();

		// Returns ease in-out curve
		static Curve EaseInOut();

		// Returns linear curve
		static Curve Linear();

		SERIALIZABLE(Curve);

	public:
		// ---------
		// Curve key
		// ---------
		class Key: public ISerializable
		{
		public:
			enum class Type { Smooth, Flat, Free, Broken, Discrete };

		public:
			float value;                // Value @SERIALIZABLE
			float position;             // Position @SERIALIZABLE
			float leftSupportValue;     // Left bezier support value, relative to key value @SERIALIZABLE
			float leftSupportPosition;  // Left bezier support position, relative to key position @SERIALIZABLE
			float rightSupportValue;    // Right bezier support value, relative to key value @SERIALIZABLE
			float rightSupportPosition; // Right bezier support position, relative to key position @SERIALIZABLE
			Type  supportsType;         // Support points type @SERIALIZABLE

		public:
			//Default constructor
			Key();

			// Constructor from value
			Key(float value);

			// Constructor
			Key(float position, float value, float leftSupportValue, float leftSupportPosition,
				float rightSupportValue, float rightSupportPosition);

			// Copy-constructor
			Key(const Key& other);

			// Copy operator
			Key& operator=(const Key& other);

			// Assign from value operator
			Key& operator=(float value);

			// Float cast operator
			operator float() const;

			// Check equals operator
			bool operator==(const Key& other) const;

			// Check not equals operator
			bool operator!=(const Key& other) const;

			// Returns approximated points
			const Vec2F* GetApproximatedPoints() const;

			// Returns approximated points count
			int GetApproximatedPointsCount() const;

			SERIALIZABLE(Key);

		public:
			static const int mApproxValuesCount = 20; // Approximation values count
			Vec2F mApproxValues[mApproxValuesCount];  // Approximation values (x - position, y - value)

			friend class Curve;
		};

	protected:
		KeysVec mKeys; // Curve keys @SERIALIZABLE

	protected:
		// Checks all smooth keys and updates supports points
		void CheckSmoothKeys();

	    // Updates approximation
		void UpdateApproximation();

		// Returns keys (for property)
		KeysVec GetKeysNonContant();

		// Completion deserialization callback
		void OnDeserialized(const DataNode& node);

		// Smooths key at index and doesn't updates approximation
		void InternalSmoothKeyAt(int idx, float smoothCoef = 1.0f);

		// Smooths key at position and doesn't updates approximation
		void InternalSmoothKey(float position, float smoothCoef = 1.0f);

		// Initializes properties
		void InitializeProperties();
	};
}
