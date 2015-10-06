Parallel testing
=========================
Wouldn't it be nice to use RapidCheck to find race conditions? Luckily you can. RapidCheck can provoke race conditions in stateful tests by executing a subset of the commands in a command sequence in parallel. RapidCheck then uses the verification function that is returned by the run function (see below) in each command to check if there is any valid interleaving of the commands in the sequence that can explain the behavior of the execution. 

## Writing parallel tests ##
To support parallel testing your commands have to implement an alternative overload of the run function (don't worry, your sequential tests runs fine with this overload as well). The overload of the run function that we have seen previously looks like this ```void run(const Model &s0, Sut &sut) const```, whereas the alternative overload has the following signature ```std::function<void(const ModelT&)> run(Sut &sut) const```. This overload returns a verification function in which you verify the model against the system under test. Converting a run function to this overload is quite straightforward, simply let the verification function capture any state required from the run function and move your asserts to the verification function. For example:

```
  std::function<void(const DispenserModel &)> run(Dispenser &sut) const override {
    auto ticket = sut.takeTicket();

    return [ticket](const DispenserModel &model) {
      RC_ASSERT(ticket == (model.ticket + 1));
    };
  }
```

Once you use the correct overload of the run function all that is left is to use the the checkParallel function instead of the check function that has been used previously.

```
  rc::check([] {
    DispenserModel initialState;
    Dispenser sut;
    rc::state::checkParallel(initialState, sut,
                             rc::state::gen::execOneOf<Take, Reset>);
  });
```

## Output ##
The output from the parallel tests looks a bit different from the sequntial tests.

```
Falsifiable after 97 tests and 3 shrinks

Parallel command sequence Command<DispenserModel, Dispenser>:
Sequential prefix:
Reset
Left branch:
Take
Right branch:
Reset
Take
```

In the example above we can see the base type of the commands in the test```Parallel command sequence Command<DispenserModel, Dispenser>:```. This is followed by the three sections ```Sequential prefix```, ```Left branch``` and ```Right branch```. The prefix is a sequence of commands that does not have to be executed in parallel, but still are requried to provoke the failure (remember that this is the shrunken test case we are looking at and the skhrinking process will move commands that does not have to be executed in parallel to the prefix). The left and right branches contains the commands that were executed in parallel.

## Tips ##
### Reproducing races ###
TBD
### Be careful when using the Command constructor that takes the Model as an argument ###
TBD
### Why is the exact error not printed in the parallel case? ###
TBD
### Why won't RapidCheck detect my race condition ###
Do you have a multi core processor? RapidCheck is much more likely to detect race conditions if the tests are executed on a multi core CPU.
