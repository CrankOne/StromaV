# Analysis infrastructure in StromaV

The StromaV classes proposes interfacing contracts yielded from following
statements:

1. User code deals with an instances set of the relatively small structure.
At least one instance of this structure can be placed in RAM at a single node.
2. Basically, two major types exist on the layer of StromaV. The first is a
dedicated arbitrary protobuf-message bearing the data of simulated event. The
second is also an arbitrary protobuf-message keeping the experimental data with
any possible processed data payload. The original StromaV's protobuf message
has these types declared.
3. At the most common case user code iterates events one-by-one performing
a sequence of operations over the read instance.

The first preposition comes from common use case appropriate for data analysis
in nuclear and high-energy physics.

The second statement is fixed in order to provide an integration with other
parts of StromaV where both types are yielded from library's primary purpose.

The third claim comes from two major practices of organizing analyzis software:
a multi-layered "onion" model and sequential model with unified event format.

The first, the "onion" model suggests using the fixed set of abstraction layers
where layer's order is fixed by strict typization mechanics of C++. E.g. let's
say the processing layer `A` receives instances of structure "`RawEvent`" and
returns instances of "`EnergySum_MeV`" which is the type that the processing
layer `B` receives then ussuing the instances of `EnergySum_MeV`. Obviously,
such architectural solution does not allow to mistakenly interchange those
layers.

The second scheme is built with the handlers receiving and issuing data
structures of the same unified type.

The `StromaV` utilizes a somewhat combined approach by using the protobuf's
inrospection media allowing one to compose strictly-typed information into
one unified structure. The type information is cheap to be retreived, however
any additional type is required to be explicitly declared at the `.proto` file.

## The `sV::AnalysisPipeline` Class

This class implements common solution built around forward-iterable data source
within handlers chain. We will use "event sequence" term further to denote the
data source and we will call handlers as "processors" to avoid disambiguations.

The class diagram depicted on fig. TODO basically describes a set of
implementation traits: an `AnalysisPipeline` instance implements iteration of
descendant instance of `iEventSequence` while processing each extracted event
within method `process()` (by `_processorsChain`). Basic activity diagram of
`process()` method is depicted at fig. TODO. More detailed activity with
participants responsibilities is proveded at fig. TODO.

## The `sV::iEventSequence` Class

TODO

Note: the diagrams is ready and can be found at .mdj file.

