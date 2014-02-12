// Begin test_fnmatch.cpp
#include <fnmatch.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

class TestCase {
public:
    TestCase(const string& pattern, const string& testString, int flags, int expected) :
        pattern(pattern),
        testString(testString),
        flags(flags),
        expected(expected)
    {}
    string pattern;
    string testString;
    int flags;
    int expected;
};

int main()
{
    vector<TestCase> testCases;

    testCases.push_back(TestCase("*","anything",0,0));
    testCases.push_back(TestCase("*.txt","readme.txt",0,0));
    testCases.push_back(TestCase("*.txt","readme.info",0,FNM_NOMATCH));
    testCases.push_back(TestCase("*.t?t","readme.txt",0,0));
    testCases.push_back(TestCase("*.t?t","readme.tot",0,0));
    testCases.push_back(TestCase("*.t?t","readme.txxt",0,FNM_NOMATCH));
    testCases.push_back(TestCase("[a-g]1","c1",0,0));
    testCases.push_back(TestCase("[a-g]1","i1",0,FNM_NOMATCH));
    testCases.push_back(TestCase("[!a-g]1","i1",0,0));
    testCases.push_back(TestCase("a\\*","anything",0,FNM_NOMATCH));
    testCases.push_back(TestCase("a\\*","a*",0,0));
    testCases.push_back(TestCase("a\\*","a*",FNM_NOESCAPE,FNM_NOMATCH));
    testCases.push_back(TestCase("a\\*","a\\*",FNM_NOESCAPE,0));
    testCases.push_back(TestCase("*readme","/etc/readme",0,0));
    testCases.push_back(TestCase("*readme","/etc/readme",FNM_PATHNAME,FNM_NOMATCH));
    testCases.push_back(TestCase("/*/readme","/etc/readme",FNM_PATHNAME,0));
    testCases.push_back(TestCase("*readme","/etc/.readme",0,0));
    testCases.push_back(TestCase("*readme",".readme",FNM_PERIOD,FNM_NOMATCH));
    testCases.push_back(TestCase("*.readme","/etc/.readme",FNM_PERIOD,0));
    testCases.push_back(TestCase("*.readme","/etc/.readme",FNM_PERIOD|FNM_PATHNAME,FNM_NOMATCH));
    testCases.push_back(TestCase("/*/.readme","/etc/.readme",FNM_PERIOD|FNM_PATHNAME,0));
    testCases.push_back(TestCase("ReAdME","readme",0,FNM_NOMATCH));

    bool pass = true;

    for (vector<TestCase>::const_iterator it = testCases.begin(); it != testCases.end(); ++it)
    {
        int result = fnmatch(it->pattern.c_str(), it->testString.c_str(), it->flags);
        if (result == it->expected)
            cout << "Pass: ";
        else
        {
            cout << "Fail: ";
            pass = false;
        }

        cout << "fnmatch(" << it->pattern << ", " << it->testString << ", "
             << it->flags << ") returned " << result << ", expected "
             << it->expected << endl;
    }

    if (pass)
    {
        cout << "All tests passed." << endl;
        return 0;
    }
    else
    {
        cout << "Some tests failed." << endl;
        return 1;
    }
}

