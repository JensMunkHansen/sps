#include <memory>

class foo
{
    public:
		foo();

	private:
		class impl;
        struct impl_deleter
        {
            void operator()( impl * ) const;
        };
		std::unique_ptr<impl,impl_deleter> pimpl;
};

// INSTEAD simple add ~foo in the header and ~foo = default in the cpp
// copy ctor/assignment is invalidated due to unique member. move can be default

// foo.cpp - implementation file

class foo::impl
{
	public:
		void do_internal_work()
		{
			internal_data = 5;
		}

	private:
		int internal_data = 0;
};
void foo::impl_deleter::operator()( impl * ptr ) const
{
    delete ptr;
}

foo::foo()
	: pimpl{new impl}
{
	pimpl->do_internal_work();
}


int main()
{
   
}
