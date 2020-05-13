class Foo
{
public:
	Foo()
	:counter(5)
	{

	}

	int counter;
};

int testfunc()
{
	static Foo foo;
	return ++foo.counter;
}

int main()
{
	testfunc();
}
