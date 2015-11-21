Parallel testing
=========================
Standard RapidCheck stateful testing can be used to find bugs that occur as a result of single threaded interaction with the system under test. However, another very common class of bugs are race conditions which by definition only appear when more than one thread is involved. RapidCheck can provoke race conditions in stateful tests by executing a subset of the commands in a command sequence in parallel. A parallel test is conseridered successful if there is a possible interleaving of the commands in the test that can explain the result of the execution.

## The anatomy of a parallel test ##
The execution of a parallel test case can be divided into four phases: command generation, pre-condition verification, execution and verification. 

During command generation three distinct command sequences are generated: the prefix, the left branch and the right branch. The prefix will be executed first, followed by the left and right branches that are executed in parallel. The exact order in which the commands in the left and right branches will be executed is not known to RapidCheck. This makes pre-condition verification more strict than in the sequential case as pre-condition verification for each test case has to be done for all possible command interleavings. The test case is discarded if any failing pre-condition is found.

After pre-condition verification comes the execution. As explained earlier RapidCheck first executes the prefix, followed by concurrently executing the left and right branches in different threads. Once the execution is completed, RapidCheck checks if there exists at least one valid (linearized) command interleaving. A valid command interleaving is a possible ordering of commands for which there are no failing assertions when the validation functions that were returned from the ```run``` function of each command in the sequence are invoked sequentially.

## Writing parallel tests ##
To support parallel testing your commands have to implement an alternative overload of the ```run``` function. The overload of the standard ```run``` function looks like this ```void run(const Model &s0, Sut &sut) const```, whereas the alternative overload has the following signature ```std::function<void(const ModelT&)> run(Sut &sut) const```. This overload is a generalized case of the standard one and will therefore work for sequential tests as well. The ```run``` function returns a verification function in which you verify the model against the system under test. Converting a ```run``` function to this overload is quite straightforward, simply let the verification function capture any state required from the ```run``` function and move your assertions to the verification function. For example:

```c++
  std::function<void(const DispenserModel &)> run(Dispenser &sut) const override {
    auto ticket = sut.takeTicket();

    return [ticket](const DispenserModel &model) {
      RC_ASSERT(ticket == (model.ticket + 1));
    };
  }
```

Finally, use the checkParallel function to execute the test in parallel mode.

```c++
  rc::check([] {
    DispenserModel initialState;
    Dispenser sut;
    rc::state::checkParallel(initialState, sut,
                             rc::state::gen::execOneOf<Take, Reset>);
  });
```

## Output ##
The output from a parallel test looks a bit different from a sequntial test.

```
Falsifiable after 97 tests and 3 shrinks

Parallel command sequence of Command<DispenserModel, Dispenser>:
Sequential prefix:
Reset
Left branch:
Take
Right branch:
Reset
Take
```

In the example above we can see the base type of the commands in the test```Parallel command sequence of Command<DispenserModel, Dispenser>:```. This is followed by the three sections ```Sequential prefix```, ```Left branch``` and ```Right branch``` which contains the commands that were executed in order to reproduce the bug. The prefix is a sequence of commands that were executed sequentially before any of the commands in the branches were executed. The left and right branches contains commands that were executed in parallel.

## Tips ##
### Shrinking parallel tests ###
Race conditions are often not deterministically reproducible. If this is the case you can easily end up with a failing test that RapidCheck is not able to shrink; the probability of the race condition being triggered when testing each shrink is simply too low. By using the configuration parameter ```shrink_tries``` you can increase the probability of the race condtion being triggered by increasing the number of times each shrink is tested.

### Be careful when using the command constructor that takes the model as an argument ###
For a sequential test the order in which commands are generated is the same as the order in which the commands will be executed. This is not the case in the parallel case however. The commands in the parallel branches are provided a model that is created by invoking the apply function for each command in the prefix, followed by each preceding command in the branch. This is generally not the same order that the commands will be executed in. In some cases this makes creating commands for parallel tests more challanging, since you cannot assume that the model that is provided to your command constructor reflects the state of the model at the time of the execution of the command.

Note however that this restriction does not apply to the ```apply``` function. Wheras each command is only created once, the apply function is executed once for every possible interleaving (at least conceptually).

### Why is no assertion failure printed when my parallel test fails? ###
When a parallel test fails because no possible command interleaving was found, this is caused by a lack of successful interleavings. Since the failure is caused by a lack of success rather than a particular failure, there is no failure that can be reported.

### Why won't RapidCheck detect my race condition? ###
This may be caused by using a single core processor. RapidCheck is much more likely to detect race conditions if the tests are executed on a multi core CPU.
