#include "testPlaylistModel.h"

#define EXEC_TESTS(Class, argc, argv, status)                                                      \
    {                                                                                              \
        Class tc;                                                                                  \
        status |= QTest::qExec(&tc, argc, argv);                                                   \
    }

int main(int argc, char *argv[])
{
    int status = 0;
    EXEC_TESTS(TestPlaylistModel, argc, argv, status);
}
