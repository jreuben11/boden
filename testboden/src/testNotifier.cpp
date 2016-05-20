#include <bdn/init.h>
#include <bdn/test.h>

#include <bdn/Notifier.h>

using namespace bdn;

void verifySame()
{	
}

template<class T1>
void verifySame(T1 a, T1 b)
{	
	REQUIRE( a==b );
}

template<class T1, class T2>
void verifySame(T1 a1, T2 a2, T1 b1, T2 b2)
{	
	REQUIRE( a1==b1 );
	REQUIRE( a2==b2 );
}

template<class... ArgTypes>
void testNotifier(ArgTypes... args)
{
	Notifier<ArgTypes...> notifier;

	bool	 called = false;
	P<IBase> pSub;

	SECTION("subscribe")
		notifier.subscribe(
			pSub,
			[&called, args...](ArgTypes... callArgs)
			{
				verifySame(callArgs..., args...);
				
				called = true;
			} );

	SECTION("subscribeVoid")
		notifier.subscribeVoid(pSub, [&called](){ called = true; } );




	class Listener : public Base
	{
	public:
		Listener(bool* pCalled, std::function<void(ArgTypes...)> argVerifier)
		{
			_pCalled = pCalled;
			_argVerifier = argVerifier;
		}

		void onNotify(ArgTypes... args)
		{
			_argVerifier(args...);

			*_pCalled = true;			
		}
			
		void onNotifyVoid()
		{
			*_pCalled = true;
		}

		bool* _pCalled;
		std::function<void(ArgTypes...)> _argVerifier;
	};

	Listener l(
		&called,
		[args...](ArgTypes... callArgs)
		{
			verifySame(callArgs..., args...);		
		});

	SECTION("subscribeMember")			
		notifier.subscribeMember<Listener>(pSub, &l, &Listener::onNotify);

	SECTION("subscribeMemberVoid")			
		notifier.subscribeVoidMember<Listener>(pSub, &l, &Listener::onNotifyVoid);

	notifier.notify(args...);

	REQUIRE(called);

	pSub = nullptr;
	called = false;

	notifier.notify(args...);

	// the subscription should have been deleted again
	REQUIRE( !called );
}

TEST_CASE("Notifier")
{
	SECTION("noArgs")
	{
		testNotifier<>();
	}

	SECTION("oneArg")
	{
		testNotifier<int>(42);
	}

	SECTION("twoArgs")
	{
		testNotifier<int, String>(42, String("hello") );
	}

	SECTION("multipleSubscriptions")
	{
		Notifier<int> notifier;
		bool called1 = false;
		bool called2 = false;

		P<IBase> pSub1;
		notifier.subscribe(pSub1, [&called1](int){called1 = true;} );
		P<IBase> pSub2;
		notifier.subscribe(pSub2, [&called2](int){called2 = true;} );

		notifier.notify(42);

		REQUIRE( called1 );
		REQUIRE( called2 );

		called1 = false;
		called2 = false;

		SECTION("deleteFirst")
		{
			pSub1 = nullptr;
			
			notifier.notify(42);

			REQUIRE( !called1 );
			REQUIRE( called2 );
		}

		SECTION("deleteSecond")
		{
			pSub2 = nullptr;
			
			notifier.notify(42);

			REQUIRE( called1 );
			REQUIRE( !called2 );
		}
	}

	SECTION("notifierDeletedBeforeSub")
	{
		P<IBase> pSub;

		{
			Notifier<> notifier;
			
			notifier.subscribe(pSub, [](){} );
		}

		pSub = nullptr;		
	}
}


