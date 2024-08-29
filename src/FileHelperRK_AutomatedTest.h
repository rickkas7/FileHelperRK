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



    FileHelperRK::deleteRecursive(FileHelperRK::pathJoin(baseDir, "foo"));
}

void runTestReadStoreString() {
    String pathTest1 = FileHelperRK::pathJoin(baseDir, "test1");

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
    String pathTest2 = FileHelperRK::pathJoin(baseDir, "test2");
    int result;

    {
        particle::Variant v1("this is a test");

        FileHelperRK::storeVariant(pathTest2, v1);        
    }
}


void runTest() {
    runTestParsePath();
    runTestDirs();
    runTestReadStoreString();
    runTestVariant();

    Log.info("runTest completed!");
}




