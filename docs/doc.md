Introduction
-
In this project I intend to develop a production-ready web server compliant with the HTTP/1.1 specification (as specified in RFC7230, RFC7231, RFC7232, RFC7233, RFC7234, and RFC7235). The web server is to be implemented as a library in the C++ language. My emphasis is on simplicity of design, high performance, and security. The library should provide an easy-to-understand but flexible and extensible interface for developing web applications in C++. This project reflects my personal interest in basic programming, backend development, and software design.

The outcome of this project I expect to be a usable framework developed through application of the best practices of the field of software engineering. The result is a C++ library with a clean and minimalistic interface that can be used to develop a backend server. The resulting software should be performant, secure, and well-suited for different use cases such as quick and straightforward development of lightweight backend applications in a microservices architecture.

This project is intended to be a low-level software developed to enable the following (functional requirements):
* development of a lightweight and performant backend software
* provide internal benchmarking

The following are the most important non-functional requirements that I have sought in this project:

* extensibility
* usability
* observability

From a design perspective, I deem this project a success when it allows itself to be easily extended. Examples include adding support for other versions of HTTP, other protocols such as WebSockets, and any other imaginable application layer protocol ideally without the need to modify the core connection management code. Usability is achieved when an external developer decides to adopt the project to develop their own backend service because they find it easy and pleasant to use. Finally, observability for me means the availability of internal metrics out of the box, the ease to extend the parameters to measure, and precise, concise, and sensible logging.

Architecture and Design
-
The overall architecture and design of the project is visualized in the following paragraphs employing the C4 model of Context, Container, Component, and Code. Considering the nature of the project, the resulting Container diagram will be very simple and presents no useful information for the current purpose. Therefore, it's omitted.

### Context

Below is a context diagram of the project, depicting how the project may relate to other external entities and in what contexts it is intended to be used:

![Context Diagram](./context.svg)

Since the software is written in a compiled language, the component `app backend` most likely is compiled and linked to the core of the project and will run in the same process, but it doesn't have to. For example, if there is reason to do so, the `app backend` can still be an independent entity develop in another programming language and run separately.

### Components

The diagram below illustrates the major components of the software. The `connection_pool` is responsible for managing connections (currently over TCP), accepting new connections, synchronizing reads and write to sockets, and notifies the handlers components as data becomes available.

The default design is so that data is sent and received only via the TLS handler. Other handlers are allowed to communicate with each other to synchronize their activities to achieve the requirements. The handlers can query the `connection_pool` to be informed about existing connections, but they are normally prohibited from writing to the connections' streams (this is to happen only through the TLS handler).

The `logger` and `metrics` components are self-explanatory and can be used by any other components of the software to log messages and record benchmarking data.

![Components Diagram](./components.svg)
