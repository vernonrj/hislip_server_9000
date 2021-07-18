Hislip Server in C++20

# What

HiSLIP is an RPC protocol commonly used with test and measurement instruments.

As an excuse to learn some C++20 stuff, I threw this together.

Don't use this in production. Or do. If you dare.

# How

To run the server, you'll need some compiler that supports C++20, and cmake. Currently, this only runs on windows, but the only reason is that sockets are terrible.

I used ninja + clang 11.

```powershell
$ mkdir build && cd ./build
$ cmake -G Ninja ..
$ cmake --build .
$ ./hislip_server
```

You'll need a client to interact with the server.

The easiest way to get that is:
- get a VISA library. [I use RS-VISA](https://www.rohde-schwarz.com/us/applications/r-s-visa-application-note_56280-148812.html) because it's minimal, and it works fine.
- [install pyvisa](https://pyvisa.readthedocs.io/en/latest/) into your python environment

And from your python interpreter, run this:

```python
import pyvisa
rm = pyvisa.ResourceManager()
i = rm.open_resource("TCPIP::localhost::hislip::INSTR")
print(i.query("*IDN?"))
```