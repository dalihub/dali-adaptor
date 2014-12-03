#include <stdio.h>
#include <string.h>
#include "tct-dali-platform-abstraction-core.h"
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <map>

int RunTestCase( struct testcase_s& testCase )
{
  int result = 1;
  if( testCase.startup )
  {
    testCase.startup();
  }
  result = testCase.function();
  if( testCase.cleanup )
  {
    testCase.cleanup();
  }
  return result;
}

#define MAX_NUM_CHILDREN 16

struct TestCase
{
  int testCase;
  const char* testCaseName;

  TestCase()
  : testCase(0),
    testCaseName(NULL)
  {
  }

  TestCase(int tc, const char* name)
  : testCase(tc),
    testCaseName(name)
  {
  }
  TestCase(const TestCase& rhs)
  : testCase(rhs.testCase),
    testCaseName(rhs.testCaseName)
  {
  }
  TestCase& operator=(const TestCase& rhs)
  {
    testCase = rhs.testCase;
    testCaseName = rhs.testCaseName;
    return *this;

  }
};


typedef std::map<int, TestCase> RunningTestCases;

int RunAll(const char* processName, bool reRunFailed)
{
  int numFailures = 0;
  int numPasses = 0;
  unsigned int numTestCases = sizeof(tc_array)/sizeof(struct testcase_s) - 1;

  // Run test cases in child process( to kill output ), but run serially.
  for( unsigned int i=0; i<numTestCases; i++)
  {
    int pid = fork();
    if( pid == 0 ) // Child process
    {
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
      exit( RunTestCase( tc_array[i] ) );
    }
    else if(pid == -1)
    {
      perror("fork");
      exit(2);
    }
    else // Parent process
    {
      int status = 0;
      int childPid = waitpid(-1, &status, 0);
      if( childPid == -1 )
      {
        perror("waitpid");
        exit(2);
      }
      if( WIFEXITED(status) )
      {
        if( childPid > 0 )
        {
          int testResult = WEXITSTATUS(status);
          if( testResult )
          {
            printf("Test case %s failed: %d\n", tc_array[i].name, testResult);
            numFailures++;
          }
          else
          {
            numPasses++;
          }
        }
      }
      else if(WIFSIGNALED(status) )
      {
        if( childPid > 0 )
        {
          printf("Test case %s exited with signal %d\n", tc_array[i].name, WTERMSIG(status));
          numFailures++;
        }
      }
    }
  }


  printf("\rNumber of test passes: %d\n", numPasses);
  printf("Number of test failures: %d\n", numFailures);

  return numFailures;
}

int FindAndRunTestCase(const char* testCaseName)
{
  int result = 2;

  for( int i = 0; tc_array[i].name; i++ )
  {
    if( !strcmp(testCaseName, tc_array[i].name) )
    {
      return RunTestCase( tc_array[i] );
    }
  }

  printf("Unknown testcase name: \"%s\"\n", testCaseName);
  return result;
}

int main(int argc, char * const argv[])
{
  int result = -1;

  const char* optString = "pr";
  bool optParallel(false);
  bool optRerunFailed(false);

  int nextOpt = 0;
  do
  {
    nextOpt = getopt( argc, argv, optString );
    switch(nextOpt)
    {
      case 'p':
        optParallel = true;
        break;
      case 'r':
        optRerunFailed = true;
        break;
    }
  } while( nextOpt != -1 );

  if( optParallel )
  {
    // For this test harness, run tests only in serial ( but without output )
    result = RunAll(argv[0], optRerunFailed);
  }
  else
  {
    if (argc != 2) {
      printf("Usage: %s <testcase name>\n", argv[0]);
      return 2;
    }
    result = FindAndRunTestCase(argv[1]);
  }
  return result;
}
