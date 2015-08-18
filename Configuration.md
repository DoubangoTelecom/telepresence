The Telepresence system is configured using **cfg** files. The main cfg file is named **telepresence.cfg** and should be on the same directory where the binary is installed unless you run the app with _"--config=PATH"_ argument. The source code contains a sample configuration file to use to get started. The sample configuration is installed after successfully building the system and running _"make samples"_.

The configuration files are parsed using code generated with [Ragel tool](http://www.complang.org/ragel/).
A configuration file contains comments, sections and entries:



## Comments ##
A comment starts with **#**
```
# I’m a comment
Age = 25 # I’m another comment
```

## Sections ##
A section name must be enclosed by square brackets. The section name is case insensitive.
```
# this is a bridge section 
[bridge]
```

## Entries ##
An entry is a key-value-pair and must be tied to a section. Both the key and the value are case insensitive. **The key must not start with a SPACE**.
```
[product] # I’m the section
version = 1.2 # I’m an entry with floating number value
name = telepresence # I’m an entry with a string value
```