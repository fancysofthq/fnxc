# FNXC documentation

This directory contains documentation for FNXC: its CLI, implementation details etc.

## Documentation compilation

The documentation is written in [Asciidoctor](https://asciidoctor.org/) format.
The `index.adoc` file contains all the documentation included in one place.

To compile the documentation in HTML, you may use the following command:

```console
$ asciidoctor index.adoc -oindex.html
```

And then open the resulting `index.html` file.

Please do not include build artifacts into the repository!
