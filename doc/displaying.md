Displaying values
=================
RapidCheck often needs to display values as strings, such as when printing assertion messages or counterexamples. To do this, it calls the `rc::show(const T &, std::ostream &)` template function. This template takes a value to display and a stream to output the string representation to. Given a value `v` and an output stream `os`, calling this template will do one of the following

- If there is a valid overload for a call to `showValue(v, os)`, calls this overload.
- If there is a valid stream insertion operator (i.e. `std::ostream &operator<<(std::ostream &, const T &)`) defined, this is used.
- Otherwise, `<???>` is printed.

In essence, this means that you simply have to have a valid stream insertion operator available for RapidCheck to be able to display values of your type. If you don't want to define such an operator or simply want RapidCheck to display values differently than stream insertion would have done, you can provide an overload of `showValue` like this:

```C++
void showValue(const Person &person, std::ostream &os) {
  os << "First name: " << person.firstName << std::endl;
  os << "Last name: " << person.lastName << std::endl;
  os << "Age: " << person.age << std::endl;
}
```
