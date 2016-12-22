# StromaV Pipeline Application

This project represents an executable performing pipeline processing of the
statistics.

This tool is for generic purposes. One need submit the data input source and
handlers descriptions for this tool to work.


                        Data Processing Pipeline

    +------------+    +-----------+    +-----------+         +-----------+
    | Sequential |    | Processor |    | Processor |         | Processor |
    |    data    |--->| (handler) |--->| (handler) |-> ... ->| (handler) |
    |   source   |    |    #1     |    |    #2     |         |    #N     |
    +------------+    +-----------+    +-----------+         +-----------+


Basically, the pipeline begins from data source that sequentially reads
statistics on event-by-event basis and forwards the data to the handlers
(here called "Processors") in certain order.

Each processor evolves a single task.

