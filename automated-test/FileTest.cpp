#include <stdio.h>
#include "FileHelperRK.h"

void runTest();

const char *testPath = "./test-output";

int main(int argc, char *argv[]) {
    mkdir(testPath, 0777);
    chdir(testPath);
    runTest();
    return 0;
}

#define assert_int(exp, val) \
    if (exp != val) { printf("exp=%d val=%d\n", (int)(exp), (int)(val)); assert(false); }

#define assert_cstr(exp, val) \
    if (strcmp(exp, val) != 0) { printf("exp=%s val=%s\n", exp, val); assert(false); }

#define assertDouble(exp, val, tol) \
    if (val < (exp - tol) || val > (exp + tol)) { printf("exp=%lf val=%lf\n", (double)exp, (double)val); assert(false); }

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
}

void runTestDirs() {
    FileHelperRK::mkdirs("foo");
}

void runTestReadStoreString() {
    {
        String s1 = "this is a test";
        int result = FileHelperRK::storeString("test1", s1);
        assert_int(SYSTEM_ERROR_NONE, result);

        String s2;
        result = FileHelperRK::readString("test1", s2);
        assert_int(SYSTEM_ERROR_NONE, result);
        assert_cstr(s1.c_str(), s2.c_str());
    }

    // Make sure string truncates
    {
        String s1 = "xxx";
        int result = FileHelperRK::storeString("test1", s1);
        assert_int(SYSTEM_ERROR_NONE, result);

        String s2;
        result = FileHelperRK::readString("test1", s2);
        assert_int(SYSTEM_ERROR_NONE, result);
        assert_cstr(s1.c_str(), s2.c_str());
    }

    // Make sure string grows
    {
        String s1 = "this is a test 2";
        int result = FileHelperRK::storeString("test1", s1);
        assert_int(SYSTEM_ERROR_NONE, result);

        String s2;
        result = FileHelperRK::readString("test1", s2);
        assert_int(SYSTEM_ERROR_NONE, result);
        assert_cstr(s1.c_str(), s2.c_str());
    }

    // c-string
    {
        const char *s1 = "this is a test 3";
        int result = FileHelperRK::storeString("test1", s1);
        assert_int(SYSTEM_ERROR_NONE, result);

        String s2;
        result = FileHelperRK::readString("test1", s2);
        assert_int(SYSTEM_ERROR_NONE, result);
        assert_cstr(s1, s2.c_str());
    }

    // null c-string
    {
        int result = FileHelperRK::storeString("test1", NULL);
        assert_int(SYSTEM_ERROR_NONE, result);

        String s2;
        result = FileHelperRK::readString("test1", s2);
        assert_int(SYSTEM_ERROR_NONE, result);
        assert_cstr("", s2.c_str());
    }

}

void runTest() {
    runTestParsePath();
    runTestDirs();
    runTestReadStoreString();
}




