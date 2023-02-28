This directory contains all kinds of documentation, from conceptional stuff to user/developer documentation.

Developer documentation/user docs and reference documentation is written with AsciiDoctor and afterwards copied to ../docs 
for publishing with github pages.

Run the build.sh scripts in the documentation subdirectories.
Also, run the linkcheck.sh scripts to test, if all cross references are valid.



## AsciiDoctor Installation

### Linux/Ubuntu

```bash

# install asciidoctor

> sudo apt install asciidoctor 

# install ruby

> sudo apt install ruby

# install GraphicsMagick (for additional image support)

> sudo apt install graphicsmagick graphicsmagick-imagemagick-compat graphicsmagick-libmagick-dev-compat

# install gems (ruby modules)

> sudo gem install asciidoctor-pdf --pre

# GraphicsMagick support
> sudo gem install prawn-gmagick

# rouge syntax highlighter extension
> sudo gem install rouge
> sudo gem install asciidoctor-rouge

# math extensions - may not work, so try to avoid!
> sudo apt install ruby-dev
> sudo gem install asciidoctor-mathematical

```
