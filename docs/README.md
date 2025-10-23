# Documentation

The main [README.md](../README.md) and [USER_GUIDE.md](../USER_GUIDE.md)
contain the majority of the documentation.

This directory contains scripts to generate [doxygen](https://www.doxygen.nl/)
documentation from the embedded docstrings in the code.

First, install doxygen and GNU Make if you don't already have them, with
something like the following on an Ubuntu Linux machine:

```
$ sudo apt install doxygen make
```

Second, run the `make` command to generate the HTML files under the `html`
directory:

```
$ cd docs
$ make
```

Third, open the `./docs/html/index.html` file in your web browser.
