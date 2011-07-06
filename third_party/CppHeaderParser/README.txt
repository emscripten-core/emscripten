Python package "CppHeaderParser"
--------------------------------
**Purpose:** Parse C++ header files and generate a data structure representing the class

**Author:** Jashua Cloutier (jashuac@bellsouth.net)

**Licence:** BSD

**External modules required:** PLY

**Quick start**::

    #include <vector>
    #include <string>
    using namespace std;
    class SampleClass
    {
    public:
        SampleClass();
        /*!
         * Method 1
         */
        string meth1();
    
        ///
        /// Method 2 description
        ///
        /// @param v1 Variable 1
        ///
        int meth2(int v1);
    
        /**
         * Method 3 description
         *
         * \param v1 Variable 1
         * \param v2 Variable 2
         */
        void meth3(const string & v1, vector<string> & v2);
    
        /**********************************
         * Method 4 description
         *
         * @return Return value
         *********************************/
        unsigned int meth4();
    private:
        void * meth5(){return NULL};
    
        /// prop1 description
        string prop1;
        //! prop5 description
        int prop5;
    };
    namespace Alpha
    {
        class AlphaClass
        {
        public:
            AlphaClass();
    
            void alphaMethod();
    
            string alphaString;
        };
    
        namespace Omega
        {
            class OmegaClass
            {
            public:
                OmegaClass();
    
                string omegaString;
            };
        };
    }
    

**Python code**::

    #!/usr/bin/python
    import sys
    sys.path = ["../"] + sys.path
    import CppHeaderParser
    try:
        cppHeader = CppHeaderParser.CppHeader("SampleClass.h")
    except CppHeaderParser.CppParseError,  e:
        print e
        sys.exit(1)
    
    print "CppHeaderParser view of %s"%cppHeader
    
    sampleClass = cppHeader.classes["SampleClass"]
    print "Number of public methods %d"%(len(sampleClass["methods"]["public"]))
    print "Number of private properties %d"%(len(sampleClass["properties"]["private"]))
    meth3 = [m for m in sampleClass["methods"]["public"] if m["name"] == "meth3"][0] #get meth3
    meth3ParamTypes = [t["type"] for t in meth3["parameters"]] #get meth3s parameters
    print "Parameter Types for public method meth3 %s"%(meth3ParamTypes)
    
    print "\nReturn type for meth1:"
    print cppHeader.classes["SampleClass"]["methods"]["public"][1]["rtnType"]
    
    print "\nDoxygen for meth2:"
    print cppHeader.classes["SampleClass"]["methods"]["public"][2]["doxygen"]
    
    print "\nParameters for meth3:"
    print cppHeader.classes["SampleClass"]["methods"]["public"][3]["parameters"]
    
    print "\nDoxygen for meth4:"
    print cppHeader.classes["SampleClass"]["methods"]["public"][4]["doxygen"]
    
    print "\nReturn type for meth5:"
    print cppHeader.classes["SampleClass"]["methods"]["private"][0]["rtnType"]
    
    print "\nDoxygen type for prop1:"
    print cppHeader.classes["SampleClass"]["properties"]["private"][0]["doxygen"]
    
    print "\nType for prop5:"
    print cppHeader.classes["SampleClass"]["properties"]["private"][1]["type"]
    
    print "\nNamespace for AlphaClass is:"
    print cppHeader.classes["AlphaClass"]["namespace"]
    
    print "\nReturn type for alphaMethod is:"
    print cppHeader.classes["AlphaClass"]["methods"]["public"][0]["rtnType"]
    
    print "\nNamespace for OmegaClass is:"
    print cppHeader.classes["OmegaClass"]["namespace"]
    
    print "\nType for omegaString is:"
    print cppHeader.classes["AlphaClass"]["properties"]["public"][0]["type"]

**Output**::

    CppHeaderParser view of class SampleClass
    Inherits: 
    {
    public
        // Method
        {'name': 'SampleClass', 'parameters': [], 'rtnType': 'void'}
        {'doxygen': '/*!\n* Method 1\n*/', 'name': 'meth1', 'parameters': [], 'rtnType': 'string'}
        {'doxygen': '///\n/// Method 2 description\n///\n/// @param v1 Variable 1\n///', 'name': 'meth2', 'parameters': [{'type': 'int', 'name': 'v1', 'desc': 'Variable 1'}], 'rtnType': 'int'}
        {'doxygen': '/**\n* Method 3 description\n*\n* \\param v1 Variable 1\n* \\param v2 Variable 2\n*/', 'name': 'meth3', 'parameters': [{'type': 'const string &', 'name': 'v1', 'desc': 'Variable 1'}, {'type': 'vector<string> &', 'name': 'v2', 'desc': 'Variable 2'}], 'rtnType': 'void'}
        {'doxygen': '/**********************************\n* Method 4 description\n*\n* @return Return value\n*********************************/', 'name': 'meth4', 'parameters': [], 'rtnType': 'unsigned int'}
    protected
    private
        // Properties
        {'doxygen': '/// prop1 description', 'type': 'string', 'name': 'prop1'}
        {'doxygen': '//! prop5 description', 'type': 'int', 'name': 'prop5'}
        // Method
        {'name': 'meth5', 'parameters': [], 'rtnType': 'void *'}
    }
    class Alpha::AlphaClass
    Inherits: 
    {
    public
        // Properties
        {'type': 'string', 'name': 'alphaString'}
        // Method
        {'name': 'AlphaClass', 'parameters': [], 'rtnType': 'void'}
        {'name': 'alphaMethod', 'parameters': [], 'rtnType': 'void'}
    protected
    private
    }
    class Alpha::Omega::OmegaClass
    Inherits: 
    {
    public
        // Properties
        {'type': 'string', 'name': 'omegaString'}
        // Method
        {'name': 'OmegaClass', 'parameters': [], 'rtnType': 'void'}
    protected
    private
    }
    
    Number of public methods 5
    Number of private properties 2
    Parameter Types for public method meth3 ['const string &', 'vector<string> &']
    
    Return type for meth1:
    string
    
    Doxygen for meth2:
    ///
    /// Method 2 description
    ///
    /// @param v1 Variable 1
    ///
    
    Parameters for meth3:
    [{'type': 'const string &', 'name': 'v1', 'desc': 'Variable 1'}, {'type': 'vector<string> &', 'name': 'v2', 'desc': 'Variable 2'}]
    
    Doxygen for meth4:
    /**********************************
    * Method 4 description
    *
    * @return Return value
    *********************************/
    
    Return type for meth5:
    void *
    
    Doxygen type for prop1:
    /// prop1 description
    
    Type for prop5:
    int
    
    Namespace for AlphaClass is:
    Alpha
    
    Return type for alphaMethod is:
    void
    
    Namespace for OmegaClass is:
    Alpha::Omega
    
    Type for omegaString is:
    string
    


Contributors
------------
Chris Love