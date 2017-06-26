#pragma once

#include <string.h>

template<class R, class ... Args>
class Callback_t
{
public:
	template<class TObject>
	Callback_t(TObject *object, R(TObject::*func)(Args ...args))
	{
		this->Attach(object, func);
	}

	Callback_t(R (*func)(Args... args))
	{ 
		this->Attach(func);
	}

	Callback_t()
		: delegate(nullptr)
	{ }

	void Attach(R(*func)(Args... args))
	{
		memcpy(&this->func, &func, sizeof(func));
		this->delegate = &StaticFunc;
	}

	template<class TObject>
	void Attach(TObject *object, R(TObject::*func)(Args ...args))
	{
		this->object = object;
		memcpy(&this->func, &func, sizeof(func));
		this->delegate = &MemberFunc<TObject>;
	}

	R operator()(Args ... args)
	{
		return this->Call(args...);
	}

	R Call(Args ... args)
	{
		if (!this->delegate) return R();

		return this->delegate(this->object, &this->func, args ...);
	}

	bool IsValid() const
	{
		return this->delegate != nullptr;
	}
private:
	template<class TObject>
	static R MemberFunc(void * object, void * func, Args ... args)
	{
		auto castedObject = static_cast<TObject *>(object);

		auto castedFunc = *static_cast<R(TObject::**)(Args...)>(func);

		return (castedObject->*castedFunc)(args...);
	}

	static R StaticFunc(void * object, void * func, Args ... args)
	{
		return (*static_cast<R(**)(Args...)>(func))(args...);
	}

	class _class;
	union {
		void(*Static)();
		void(*BoundFunc)(_class*);
		void(_class::*Method)();
	} func;

	void *object;

	R(*delegate)(void *, void *, Args ... args);
};

//using Action = Callback<void>;
template <class ... TArgs> using Action = Callback_t<void, TArgs...>;