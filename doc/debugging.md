Debugging
=========
When RapidCheck finds a failing test case, it shrinks the test case by trying smaller versions of the original test case. It repeats this process until it cannot find a smaller failing case. Since this is probably the test case that you want to debug, RapidCheck makes it easy for you to do this by calling the `rc::beforeMinimalTestCase` function before it runs the minimal test case one last time. By setting a breakpoint on this function, you can pause execution to set up your debugging environment (breakpoints, watchpoints etc.) for the final run.

Please note that this is called for every failing property so you may want to use the facilities of a test framework (i.e. Boost UTF or Google Test) to filter out the specific property you want to debug.
