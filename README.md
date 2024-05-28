# Setup Development Environment

The provided `Dockerfile` contains instructions for setting up all the requirements for the development, compilation, and execution of the software. Simply running `docker build -t nimlib-server-img .` from the cloned directory should be enough.

The built image has its port 8080 exposed. Once the image is built, run a container by executing `docker run -p 8080:8080 nimlib-server-img /nimlib/build_d/test_run`. When the container is up, the running server should be available by visiting `https://127.0.0.1:8080` in the browser. The server comes with self-signed certificates for testing purposes (using Firefox produces better results).

The Dockerfie includes instructions for cloning and building the [Botan cryptography library](https://botan.randombit.net/) used for the TLS layer of the server. The build can take some time (up to 30 minutes). The server is also built and can be run by `/nimlib/build_d/test_run` in a running container.

You can also compile the server on your local machine provided you have Botan library installed locally. On a MacOS system, try `brew install botan@3` to install the specific version of the library and compile against it.
