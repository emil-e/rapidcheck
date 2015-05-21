Reporting distribution
======================
To have the confidence that your properties are testing the right thing, it can be important to know how the input data is distributed. RapidCheck provides functionality to collect this information and report it to the console.

To do this, RapidCheck uses the concept of test case "tags". Tags are a sequence of strings associated with a test case and every test case with the same tags are grouped together. When RapidCheck has run all its tests, it prints the distribution of the different categories. To add tags to a test case, use the tagging macros described below.

## Tagging macros ##

### `RC_TAG(values...)` ###
Tags the current test case with the given values. The values are converted to strings using `rc::toString`.

Example:
```C++
rc::check([](const User &user) {
  RC_TAG(user.gender);
});
```

When run:
```
OK, passed 100 tests
54.00% - Female
46.00% - Male
```

### `RC_CLASSIFY(condition, values...)` ###
If `condition` is true, tags the current test case with the given values in the same manner as `RC_TAG`. If no tags are given, a stringified version of the condition will be used as the tag.

Example:
```C++
rc::check([](const User &user) {
  RC_CLASSIFY(user.username.empty());
});
```

When run:
```
OK, passed 100 tests
 8.00% - user.username.empty()
```

## Notes ##
Tags are not treated as sets meaning that there can be duplicate tags and the order is significant. The following are not equivalent:
```C++
RC_TAG("foo", "bar"); // <- This...
RC_TAG("bar", "foo"); // <- is not the same as this
```

Also, you can use multiple tagging macros, you do not have to add all tags in a single macro. The following are equivalent:
```C++
// This:
RC_TAG("foo");
RC_TAG("bar");

// ...is equivalent to:
RC_TAG("foo", "bar");
```
