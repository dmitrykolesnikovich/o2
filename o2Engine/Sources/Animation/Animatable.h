#pragma once

#include "Animation/Animation.h"
#include "Animation/AnimationState.h"
#include "Utils/Debug.h"

namespace o2
{
	// ---------------------------
	// Animatable object interface
	// ---------------------------
	class Animatable: public ISerializable
	{
	public:
		// Default constructor
		Animatable();

		// Copy-constructor
		Animatable(const Animatable& other);

		// Destructor
		~Animatable();

		// Copy-operator
		Animatable& operator=(const Animatable& other);

		// Updates animations, blendings and assigning blended values
		virtual void Update(float dt);

		// Adds new animation state and returns him
		AnimationState* AddState(AnimationState* state);

		// Adds new animation state and returns him
		AnimationState* AddState(const String& name, const Animation& animation,
								 const AnimationMask& mask, float weight);

		// Adds new animation state and returns him
		AnimationState* AddState(const String& name);

		// Removes animation state
		void RemoveState(AnimationState* state);

		// Removes animation state by name
		void RemoveState(const String& name);

		// Removes all animation states
		void RemoveAllStates();

		// Returns state with specified name. Returns nullptr if can't find state with specified name
		AnimationState* GetState(const String& name);

		// Returns all states array
		const AnimationStatesVec& GetStates() const;

		// Creates new state and plays him
		AnimationState* Play(const Animation& animation, const String& name);

		// Creates new state and plays him
		AnimationState* Play(const Animation& animation);

		// Searches animation state with name and plays him
		AnimationState* Play(const String& name);

		// Creates new state, and blends animation with duration
		AnimationState* BlendTo(const Animation& animation, const String& name, float duration = 1.0f);

		// Creates new state, and blends animation with duration
		AnimationState* BlendTo(const Animation& animation, float duration = 1.0f);

		// Creates new state, and blends animation with duration
		AnimationState* BlendTo(const String& name, float duration = 1.0f);

		// Plays state and blends animation with duration
		AnimationState* BlendTo(AnimationState* state, float duration = 1.0f);

		// Stops animation with name
		void Stop(const String& animationName);

		// Stops all states
		void StopAll();

		SERIALIZABLE(Animatable);

	protected:
		// -------------------------------
		// Value assigning agent interface
		// -------------------------------
		struct IValueAgent
		{
			// Value path
			String path;

			// Updates value and blend
			virtual void Update() = 0;

			// Removes animated value from agent
			virtual void RemoveValue(IAnimatedValue* value) = 0;

			// Returns is agent hasn't no values
			virtual bool IsEmpty() const = 0;
		};
		typedef Vector<IValueAgent*> ValueAgentsVec;

		// ------------------------------
		// Template value assigning agent
		// ------------------------------
		template<typename _type>
		struct ValueAgent: public IValueAgent
		{
			typedef Dictionary<AnimationState*, AnimatedValue<_type>*> AnimatedValuesDict;

			AnimatedValuesDict animValues; // Animated values associated with animation states
			void*              targetPtr;  // Target value pointer (field or setter)

			void(ValueAgent<_type>::*assignFunc)(_type&); // Current assign function

			// Updates value and blend
			void Update();

			// Removes animated value from agent
			void RemoveValue(IAnimatedValue* value);

			// Returns is agent hasn't no values
			bool IsEmpty() const;

			// Assigns value as field
			void AssignField(_type& value);

			// Assigns value as setter
			void AssignSetter(_type& value);
		};

		// ----------------------
		// Blend state parameters
		// ----------------------
		struct BlendState
		{
			AnimationStatesVec  mBlendOffStates;           // Turning off states
			AnimationState*     mBlendOnState = nullptr;   // Turning on state
			float               duration;                  // Blending duration
			float               time = -1.0f;              // Current blending remaining time

			// Updates work weight by time
			void Update(float dt);
		};

	protected:
		AnimationStatesVec mStates; // Animation states array @SERIALIZABLE
		ValueAgentsVec     mValues; // Assigning value agents
		BlendState         mBlend;  // Current blend parameters

		ATTRIBUTES(SerializableAttribute);
		int test;

	protected:
		// Removes animated value from agent by path
		void UnregAnimatedValue(IAnimatedValue* value, const String& path);

		// Registers value by path and state
		template<typename _type>
		void RegAnimatedValue(AnimatedValue< _type >* value, const String& path, AnimationState* state);

		friend class Animation;
		friend class IAnimatedValue;

		template<typename _type>
		friend class AnimatedValue;
	};

	template<typename _type>
	void Animatable::RegAnimatedValue(AnimatedValue< _type >* value, const String& path, AnimationState* state)
	{
		for (auto val : mValues)
		{
			if (val->path == path)
			{
				ValueAgent< _type>* agent = dynamic_cast<ValueAgent<_type>*>(val);

				if (!agent)
				{
					o2Debug.LogWarning("Can't work with animated value: %s", path);
					return;
				}

				agent->animValues.Add(state, value);
				return;
			}
		}

		ValueAgent<_type>* newAgent = mnew ValueAgent <_type>();
		mValues.Add(newAgent);
		newAgent->path = path;
		newAgent->animValues.Add(state, value);

		FieldInfo* fieldInfo = nullptr;
		_type* fieldPtr = (_type*)GetType().GetFieldPtr(this, path, fieldInfo);

		if (!fieldInfo)
		{
			o2Debug.LogWarning("Can't animate value %s: can't find field", path);
			return;
		}

		newAgent->targetPtr = fieldPtr;

		if (fieldInfo->GetType()->GetUsage() == Type::Usage::Property)
			newAgent->assignFunc = &ValueAgent<_type>::AssignSetter;
		else
			newAgent->assignFunc = &ValueAgent<_type>::AssignField;
	}

	template<typename _type>
	bool Animatable::ValueAgent<_type>::IsEmpty() const
	{
		return animValues.IsEmpty();
	}

	template<typename _type>
	void Animatable::ValueAgent<_type>::RemoveValue(IAnimatedValue* value)
	{
		animValues.RemoveAll([&](auto x) { return x.Value() == value; });
	}

	template<typename _type>
	void Animatable::ValueAgent<_type>::Update()
	{
		AnimationState* firstValueState = animValues.GetIdx(0).Key();
		AnimatedValue<_type>* firstValue = animValues.GetIdx(0).Value();

		float weightsSum = firstValueState->weight*firstValueState->workWeight*firstValueState->mask.GetNodeWeight(path);
		_type valueSum = firstValue->GetValue();

		for (int i = 1; i < animValues.Count(); i++)
		{
			AnimationState* valueState = animValues.GetIdx(i).Key();
			AnimatedValue<_type>* value = animValues.GetIdx(i).Value();

			weightsSum += valueState->weight*valueState->workWeight*valueState->mask.GetNodeWeight(path);
			valueSum += value->GetValue();
		}

		_type resValue = valueSum / weightsSum;
		(this->*assignFunc)(resValue);
	}

	template<typename _type>
	void Animatable::ValueAgent<_type>::AssignSetter(_type& value)
	{
		*(Setter<_type>*)targetPtr = value;
	}

	template<typename _type>
	void Animatable::ValueAgent<_type>::AssignField(_type& value)
	{
		*(_type*)targetPtr = value;
	}
}
