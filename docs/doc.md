Introduction
-
In this project I intend to develop a production-ready web server compliant with the HTTP/1.1 specification (as specified in RFC7230, RFC7231, RFC7232, RFC7233, RFC7234, and RFC7235). The web server is to be implemented as a library in the C++ language. My emphasis is on simplicity of design, high performance, and security. The library should provide an easy-to-understand but flexible and extensible interface for developing web applications in C++. This project reflects my personal interest in basic programming, backend development, and software design.

The outcome of this project I expect to be a usable framework developed through application of the best practices of the field of software engineering. The result is a C++ library with a clean and minimalistic interface that can be used to develop a backend server. The resulting software should be performant, secure, and well-suited for different use cases such as quick and straightforward development of lightweight backend applications in a micro-services architecture.

This project is intended to be a low-level software developed to enable the following (functional requirements):
* development of a lightweight and performant backend software
* provide internal benchmarking

The following are the most important non-functional requirements that I have saught in this project:

* extensibility
* usability
* observability

From a design perspective, I deem this project a success when it allows itself to be easily extended. Examples include adding support for other versions of HTTP, other protocols such as WebSockets, and any other imaginable application layer protocol ideally without the need to modify the core connection management code. Usability is achieved when an external developer decides to adopt the project to develop their own backend service because they find it easy and pleasant to use. Finally, observability for me means the availability of internal metrics out of the box, the ease to extend the parameters to measure, and precise, consice, and sensible logging.

![Context Diagram](./context.drawio.svg)
