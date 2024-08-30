#ifndef DOXYGEN_DO_NOT_DOCUMENT

#include "FileHelperRK.h"

String baseDir;

#define assert_int(exp, val) \
    if (exp != val) { Log.error("exp=%d val=%d\n", (int)(exp), (int)(val)); assert(false); }

#define assert_cstr(exp, val) \
    if (strcmp(exp, val) != 0) { Log.error("exp=%s val=%s\n", exp, val); assert(false); }

#define assertDouble(exp, val, tol) \
    if (val < (exp - tol) || val > (exp + tol)) { Log.error("exp=%lf val=%lf\n", (double)exp, (double)val); assert(false); }

void runTestParsePath() {
    {
        const char *path = "/usr";
        FileHelperRK::ParsedPath parsed;
        int result = parsed.parse(path);

        assert_int(SYSTEM_ERROR_NONE, result);
        assert_int(true, parsed.getStartsWithSlash());
        assert_int(false, parsed.getEndsWithSlash());
        assert_int(1, parsed.getNumParts());
        assert_cstr("usr", parsed.getPart(0).c_str());
        assert_cstr("/usr", parsed.generatePathString().c_str());
        assert_cstr("usr", parsed.getFileBaseName().c_str());
        assert_cstr("", parsed.getFileExtension().c_str());
    }
    {
        const char *path = "/usr/foo";
        FileHelperRK::ParsedPath parsed;
        int result = parsed.parse(path);

        assert_int(true, parsed.getStartsWithSlash());
        assert_int(false, parsed.getEndsWithSlash());
        assert_int(2, parsed.getNumParts());
        assert_cstr("usr", parsed[0].c_str());
        assert_cstr("foo", parsed[1].c_str());
        assert_cstr("/usr/foo", parsed.generatePathString().c_str());
        assert_cstr("/usr", parsed.generatePathString(1).c_str());
    }
    {
        const char *path = "/usr/foo/";
        FileHelperRK::ParsedPath parsed;
        int result = parsed.parse(path);

        assert_int(SYSTEM_ERROR_NONE, result);
        assert_int(true, parsed.getStartsWithSlash());
        assert_int(true, parsed.getEndsWithSlash());
        assert_int(2, parsed.getNumParts());
        assert_cstr("usr", parsed[0].c_str());
        assert_cstr("foo", parsed[1].c_str());
        assert_cstr("/usr/foo", parsed.generatePathString().c_str());
        assert_cstr("/usr", parsed.generatePathString(1).c_str());
    }
    {
        const char *path = "/usr/foo.txt";
        FileHelperRK::ParsedPath parsed;
        int result = parsed.parse(path);

        assert_int(true, parsed.getStartsWithSlash());
        assert_int(false, parsed.getEndsWithSlash());
        assert_int(2, parsed.getNumParts());
        assert_cstr("usr", parsed[0].c_str());
        assert_cstr("foo.txt", parsed[1].c_str());
        assert_cstr("/usr/foo.txt", parsed.generatePathString().c_str());
        assert_cstr("/usr", parsed.generatePathString(1).c_str());
        assert_cstr("foo", parsed.getFileBaseName().c_str());
        assert_cstr("txt", parsed.getFileExtension().c_str());
    }
    {
        const char *path = "usr/foo";
        FileHelperRK::ParsedPath parsed;
        int result = parsed.parse(path);

        assert_int(SYSTEM_ERROR_NONE, result);
        assert_int(false, parsed.getStartsWithSlash());
        assert_int(false, parsed.getEndsWithSlash());
        assert_int(2, parsed.getNumParts());
        assert_cstr("usr", parsed[0].c_str());
        assert_cstr("foo", parsed[1].c_str());
        assert_cstr("usr/foo", parsed.generatePathString().c_str());
        assert_cstr("usr", parsed.generatePathString(1).c_str());
    }    
    {
        const char *path = "foo";
        FileHelperRK::ParsedPath parsed;
        int result = parsed.parse(path);

        assert_int(SYSTEM_ERROR_NONE, result);
        assert_int(false, parsed.getStartsWithSlash());
        assert_int(false, parsed.getEndsWithSlash());
        assert_int(1, parsed.getNumParts());
        assert_cstr("foo", parsed[0].c_str());
        assert_cstr("foo", parsed.generatePathString().c_str());
        assert_cstr("foo", parsed.generatePathString(1).c_str());
    }    
    {
        const char *path = "./foo";
        FileHelperRK::ParsedPath parsed;
        int result = parsed.parse(path);

        assert_int(SYSTEM_ERROR_NONE, result);
        assert_int(parsed.getStartsWithSlash(), false);
        assert_int(parsed.getEndsWithSlash(), false);
        assert_int(parsed.getNumParts(), 2);
        assert_cstr(".", parsed[0].c_str());
        assert_cstr("foo", parsed[1].c_str());
        assert_cstr("./foo", parsed.generatePathString().c_str());
    }    

    {
        String s;
        s = FileHelperRK::pathJoin("/usr", "foo");
        assert_cstr("/usr/foo", s.c_str());

        s = FileHelperRK::pathJoin("/usr/", "foo");
        assert_cstr("/usr/foo", s.c_str());

        s = FileHelperRK::pathJoin("/usr", "");
        assert_cstr("/usr", s.c_str());

        s = FileHelperRK::pathJoin("/usr", nullptr);
        assert_cstr("/usr", s.c_str());

        s = FileHelperRK::pathJoin(nullptr, "foo");
        assert_cstr("foo", s.c_str());

        s = FileHelperRK::pathJoin("", "foo");
        assert_cstr("foo", s.c_str());

    }
}

void runTestDirs() {
    FileHelperRK::mkdirs(FileHelperRK::pathJoin(baseDir, "foo"));

    FileHelperRK::mkdirs(FileHelperRK::pathJoin(baseDir, "foo/a/b"));

    FileHelperRK::mkdirs(FileHelperRK::pathJoin(baseDir, "foo/a/c"));

}

void runTestReadStoreString() {
    String pathTest1 = FileHelperRK::pathJoin(baseDir, "foo/test1");

    {
        String s1 = "this is a test";
        int result = FileHelperRK::storeString(pathTest1, s1);
        assert_int(SYSTEM_ERROR_NONE, result);

        String s2;
        result = FileHelperRK::readString(pathTest1, s2);
        assert_int(SYSTEM_ERROR_NONE, result);
        assert_cstr(s1.c_str(), s2.c_str());

        FileHelperRK::Usage usage;
        result = usage.measure(pathTest1);
        assert_int(SYSTEM_ERROR_NONE, result);
        assert_int(1, usage.numFiles);
        assert_int(0, usage.numDirectories);
        assert_int(14, usage.fileBytes);
        assert_int(2, usage.sectors);

    }

    // Make sure string truncates
    {
        String s1 = "xxx";
        int result = FileHelperRK::storeString(pathTest1, s1);
        assert_int(SYSTEM_ERROR_NONE, result);

        String s2;
        result = FileHelperRK::readString(pathTest1, s2);
        assert_int(SYSTEM_ERROR_NONE, result);
        assert_cstr(s1.c_str(), s2.c_str());
    }

    // Make sure string grows
    {
        String s1 = "this is a test 2";
        int result = FileHelperRK::storeString(pathTest1, s1);
        assert_int(SYSTEM_ERROR_NONE, result);

        String s2;
        result = FileHelperRK::readString(pathTest1, s2);
        assert_int(SYSTEM_ERROR_NONE, result);
        assert_cstr(s1.c_str(), s2.c_str());
    }

    // c-string
    {
        const char *s1 = "this is a test 3";
        int result = FileHelperRK::storeString(pathTest1, s1);
        assert_int(SYSTEM_ERROR_NONE, result);

        String s2;
        result = FileHelperRK::readString(pathTest1, s2);
        assert_int(SYSTEM_ERROR_NONE, result);
        assert_cstr(s1, s2.c_str());
    }

    // null c-string
    {
        int result = FileHelperRK::storeString(pathTest1, NULL);
        assert_int(SYSTEM_ERROR_NONE, result);

        String s2;
        result = FileHelperRK::readString(pathTest1, s2);
        assert_int(SYSTEM_ERROR_NONE, result);
        assert_cstr("", s2.c_str());
    }

}

void runTestVariant() {
#if defined(SYSTEM_VERSION_560) || defined(UNITTEST)
    String pathTest2 = FileHelperRK::pathJoin(baseDir, "foo/test2");
    int result;

    {
        particle::Variant v1("this is a test");

        result = FileHelperRK::storeVariant(pathTest2, v1);       
        assert_int(SYSTEM_ERROR_NONE, result);

        particle::Variant v2;

        result =  FileHelperRK::readVariant(pathTest2, v2);
        assert_int(SYSTEM_ERROR_NONE, result);

        assert_cstr(v1.toString().c_str(), v2.toString().c_str());

    }

    {
        const char *jsonStr = "{\"a\":123,\"b\":\"test\",\"c\":true,\"d\":[1,2,3]}";

        JSONValue jsonValue = JSONValue::parseCopy(jsonStr);

        /*
        JSONObjectIterator iter(jsonValue);
        while(iter.next()) {
            Log.info("key=%s value=%s", 
            (const char *) iter.name(), 
            (const char *) iter.value().toString());
        }
        */

        particle::Variant v1 = particle::Variant::fromJSON(jsonValue);;

        result = FileHelperRK::storeVariant(pathTest2, v1);       
        assert_int(SYSTEM_ERROR_NONE, result);

        particle::Variant v2;

        result =  FileHelperRK::readVariant(pathTest2, v2);
        assert_int(SYSTEM_ERROR_NONE, result);

        String s = v2.toJSON();
        assert_cstr(jsonStr, s.c_str());
    }
#else  
    Log.info("Variant tests skipped");
#endif // defined(SYSTEM_VERSION_560) || defined(UNITTEST)
}

struct TestStruct1 {
    uint32_t f1;
    char f2[10];
};

struct TestStruct2 {
    uint32_t f1;
    char f2[10];
    uint32_t f3;
};

void runTestStruct() {
    String pathTest2 = FileHelperRK::pathJoin(baseDir, "foo/test2");
    int result;
    
    {
        TestStruct1 ts1a;
        memset(&ts1a, 0, sizeof(ts1a));
        ts1a.f1 = 0x12345678;
        strcpy(ts1a.f2, "testing!");

        FileHelperRK::storeStruct(pathTest2, ts1a);
        
        TestStruct1 ts1b;
        FileHelperRK::readStruct(pathTest2, ts1b);
        assert_int(ts1a.f1, ts1b.f1);
        assert_cstr(ts1a.f2, ts1b.f2);

    }
    {
        TestStruct2 ts2a;
        memset(&ts2a, 0, sizeof(ts2a));
        ts2a.f1 = 0x12345678;
        strcpy(ts2a.f2, "testing!");
        ts2a.f3 = 0x55aa55aa;

        TestStruct2 ts2b;
        ts2b.f3 = 0xffffffff;

        FileHelperRK::readStruct(pathTest2, ts2b);
        assert_int(ts2a.f1, ts2b.f1);
        assert_cstr(ts2a.f2, ts2b.f2);
        assert_int(0, ts2b.f3);

        FileHelperRK::storeStruct(pathTest2, ts2a);
        FileHelperRK::readStruct(pathTest2, ts2b);
        assert_int(ts2a.f1, ts2b.f1);
        assert_cstr(ts2a.f2, ts2b.f2);
        assert_int(ts2a.f3, ts2b.f3);

    }
}


void runTest() {
    runTestParsePath();
    runTestDirs();
    runTestReadStoreString();
    runTestVariant();
    runTestStruct();

    FileHelperRK::deleteRecursive(FileHelperRK::pathJoin(baseDir, "foo"));

    Log.info("runTest completed!");
}


#endif // DOXYGEN_DO_NOT_DOCUMENT
