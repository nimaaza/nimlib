Introduction
-
In this project I intend to develop a production-ready web server compliant with the HTTP/1.1 specification (as specified in RFC7230, RFC7231, RFC7232, RFC7233, RFC7234, and RFC7235). The web server is to be implemented as a library in the C++ language. My emphasis is on simplicity of design, high performance, and security. The library should provide an easy-to-understand but flexible and extensible interface for developing web applications in C++. This project reflects my personal interest in basic programming, backend development, and software design.

The outcome of this project I expect to be a usable framework developed through application of the best practices of the field of software engineering. The result is a C++ library with a clean and minimalistic interface that can be used to develop a backend server. The resulting software should be performant, secure, and well-suited for different use cases such as quick and straightforward development of lightweight backend applications in a microservices architecture.

This project is intended to be a low-level software developed to enable the following (functional requirements):
* development of a lightweight and performant backend software
* provide internal benchmarking

The project is intended to provide support for the HTTP and WebSocket protocols out of the box (work in process) but the design is so that adding other application
layer protocols is easily achieved.

The following are the most important non-functional requirements that I am seeking in this project:

* extensibility (relatively easy to add support for additional protocols)
* usability (simple interface for rapid development of a backend server)
* observability (provide means for easily defining and collecting metrics)

From a design perspective, I deem this project a success when it allows itself to be easily extended. Examples include adding support for other versions of the HTTP protocol, other protocols such as WebSockets, and any other imaginable application layer protocol ideally without the need to modify the core connection management code. Usability is achieved when an external developer decides to adopt the project to develop their own backend service because they find it easy and pleasant to use. Finally, observability for me means the availability of internal metrics out of the box, the ease to extend the parameters to measure, and precise, concise, and sensible logging.

Architecture and Design
-
The overall architecture and design of the project is visualized and documented in the following paragraphs employing the C4 model of Context, Container, Component, and Code. Considering the nature of the project, the resulting Container diagram will be very simple and presents no useful information for the current purpose. Therefore, it's omitted.

### Context

Below is a context diagram of the project, depicting how the project may relate to other external entities and in what contexts it is intended to be used:

![Context Diagram](./context.svg)

Since the software is written in a compiled language, the component `app backend` most likely is compiled and linked to the core of the project and will run in the same process, but it doesn't have to. For example, if there is reason to do so, the `app backend` can still be an independent entity develop in another programming language and run separately.

### Components

The diagram below illustrates the major components of the software. The `connection_pool` is responsible for managing connections (currently over TCP), accepting new connections, synchronizing reads and write to sockets, and notifies the handlers components (ie, application layer protocols) as data becomes available or they need to take action.

The default design is so that data is sent and received only via the TLS handler. Other handlers are allowed to communicate with each other to synchronize their activities to achieve the requirements. The handlers can query the `connection_pool` to be informed about existing connections, but they are normally prohibited from writing to the connections' streams (this is to be handled only through the TLS handler). In the context of the HTTP protocol, since the design is stateless and is based on the flow of request/response the handler does not need to be aware of other connections. But for a protocol such as WebSockets when used, for example, for a chat application, the handler needs to be aware of other existing connections. Therefore, the `connection_pool` needs to be generally available for the handlers and provide information about available connections.

The `logger` and `metrics` components are self-explanatory and can be used by any other components of the software to log messages and record benchmarking data.

![Components Diagram](./components.svg)

### The Metrics Component (more details)
One of the most interesting components of the software is the metrics component which allows collection of numeric data that can help in benchmarking the server. The metrics can be defined and used anywhere in the code. The metrics code uses C++ templates to allow collecting metrics using integral or floating point values. The following classes work together to enable the collection of metrics:

* MetricStore: this is a singleton and acts as a central storage for all metrics objects. When the class is instantiated, the type of the metric data must be chosen (int, long, float, double, etc.). It's possible to have two metric stores, one for gathering integral data and one for gathering floating point data, if this practice is justified.

* PointMetric & TimeSeriesMetric: these classes abstract specific metrics that are collected. For example, response time of the web server. Each metric object can be identified with a name (of type std::string). When creating the metric, the types of statistic that we are interested in can be specified (min, max, average, median, and so on). TimeSeriesMetric is a decorated variant of the PointMetric class that allows the collection of metrics with timestamps.

* Builder: creating a metric object is somewhat complicated. To make this easier, the builder class can be used to create the metric object by specifying the name of the metric, its type of data collection (defaults to `long`), the statistics to calculate, and whether we want the metric to provide a time series of the data. Once the metric object is finalized, it is automatically registered in the metric store and is available to use wherever appropriate. By default, the metric objects have the type of PointMetric which collects the absolute values of the metric for the whole runtime of the server. If it is desired to collect metrics with a timestamp so the evolution of the values in time can be assessed, the builder provides a function to use the TimeSeriesMetric variant.

* Measure: the classes that implement the Measure interface are used to carry out the desired measurements. Two basic implementations are provided that measure time (duration of an activity) and increment. When instantiating an object of these classes, the constructor takes the name of a target metric. Whenever a measurement is completed, the instance of the Measure object submits the measured value to the target metric for registering the value.

* Aggregator: the different implementations of the aggregator interface take care of aggregating the measurements as measured by the implementations of the Measure interface. Each instance of the Metric class can have one or more aggregators that receive and process the data. Several aggregations are available such as max, min, average, median, and rate.

Below is a code example of how the metrics store can be used:

```
...
using builder = nimlib::Server::Metrics::Builder<long>;

std::string name {"time_to_response"};
builder::instantiate_metric(name)
    .measure_avg()
    .measure_max()
    .with_timeseries(5000)
    .build();

...
```
The integral value provided to the with_timeseries() function specifies the number of data points we would like to include in the time series (ie, with 5000 the last 5000 measurements are kept). From this point on, the metric is available through the instance of the metric store (though accessing the metric object directly is not useful on its own):

```
auto metric = nimlib::Server::Metrics::MetricsStore<long>::get_instance().get_metric("time_to_response");
```

The way the metric is used is through the instances of the Measure interface, like the duration object which is initialized by the name of the metric:

```
nimlib::Server::Metrics::Measurements::Duration<long> response_timer {"time_to_response"};
```
Whenever a measurement has to start, response_timer.start() is called. At the point when the measurement must end, response_timer.end() is called and the result of the measurement is submitted to the metric. If for some reason a started measurement is not useful anymore, response_timer.cancel() can be used to cancel the measurement. The response_timer object stays available to subsequence measurements.

The following is a diagram of the metrics component:

![Metrics Diagram](./metrics.uml.svg)
