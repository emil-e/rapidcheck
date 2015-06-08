Configuration
=============
RapidCheck has several parameters that affect testing. These can be configured by setting the `RC_PARAMS` environment variable to a configuration string. The configuration string as the format `<key1>=<value1> <key2>=<value2> ...`. Values containing spaces can be quoted and backslash works as an escape character, just as you would expect.

When RapidCheck is initialized, it prints the configuration that is used excluding parameters that are set to their default value. For example, when running a test without changing the defaults, the following could be printed:

```
Using configuration: seed=8482903616249937941
```

This allows you to reproduce the exact test in case of a failure.

## Reference ##
The following settings are provided:

- `seed` - The global random seed used. This is a 64-bit integer. If not set, a random one is chosen using the system random device.
- `max_success` - The maximum number of successful test cases to run before deciding that a property holds. Defaults to `100`.
- `max_size` - The maximum size to use. The size starts at `0` and increases to `max_size` as the final value. Defaults to `100`.
- `max_discard_ratio` - The maximum number of discarded test cases per successful test case. If exceeded, RapidCheck gives up on the property. Defaults to `10`.
- `noshrink` - If set to `1`, disables test case shrinking. Defaults to `0`.
- `verbose_progress` - If set to `1`, enables verbose feedback of progress during the testing of a property. For each test case that is run, a character will be printed. Default is `0`. Legend:
  - `.` - Success
  - `x` - Discarded
- `verbose_shrinking` - If set to `1`, enables verbose feedback during shrinking. For each shrink that is tried, a character will be printed. Default is `0`. Legend:
  - `.` - Unsuccessful shrink
  - `!` - Successful shrink
